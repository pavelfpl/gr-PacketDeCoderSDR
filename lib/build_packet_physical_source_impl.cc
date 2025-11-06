/* -*- c++ -*- */
/* 
 * Copyright 2025 <Pavel Fiala>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/ioctl.h>
#include <sys/poll.h>
#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

#include <gnuradio/io_signature.h>
#include "build_packet_physical_source_impl.h"

#include <gnuradio/io_signature.h>
#include <gnuradio/blocks/pdu.h>
#include <gnuradio/digital/crc32.h>

// Include CANBus layer
#include "PacketDeCoderSDR/xFace_can/Bus/CanBus.hpp"
#include "PacketDeCoderSDR/xFace_can/Frame/CanFrame.hpp"

#include "util.h"

#define CONST_HEADER_PRELOAD_BYTES 2
#define CONST_HEADER_PACKET_SIZE 2
#define CONST_HEADER_CRC32 4
#define CONST_PAYLOAD_CRC32 4

#define CONST_BITS_PER_BYTE 8
#define CONST_PAYLOAD_OFFSET (CONST_HEADER_PRELOAD_BYTES + CONST_HEADER_PACKET_SIZE + CONST_HEADER_CRC32)

#define CONST_SYNC_WORD 0x1337
#define CONST_SPARE_BYTE 0x00

#define CAN_PACKET_LEN_DEFAULT 8
#define CAN_BUS_DEVICE 0x00

// Simple packet handler state machine
#define PACKET_HEADER_SEARCH 0xA1
#define PACKET_PAYLOAD_GET   0xA2

#define CONST_DATA_PACKED 0 
#define CONST_DATA_UNPACKED 1

namespace gr {
  namespace PacketDeCoderSDR {
      
    using namespace std;

    // Define static variables ...
    // ---------------------------
    fifo_buffer_packet build_packet_physical_source_impl::s_fifo;

    build_packet_physical_source::sptr
    build_packet_physical_source::make(int deviceSource, const std::string &deviceOption, int payloadLength, int dataType, int bufferLength)
    {
      return gnuradio::get_initial_sptr
        (new build_packet_physical_source_impl(deviceSource, deviceOption, payloadLength, dataType, bufferLength));
    }

    /*
     * The private constructor
     */
    build_packet_physical_source_impl::build_packet_physical_source_impl(int deviceSource, const std::string &deviceOption, int payloadLength, int dataType, int bufferLength)
      : gr::block("build_packet_physical_source",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(0, 0, 0)),
              m_deviceSource(deviceSource),
              m_deviceOption(deviceOption),
              m_payloadLength(payloadLength),
              m_dataType(dataType),
              m_bufferLength(bufferLength)
    {
        
        // -- Initial status of thread joined flag (before we enter device selection and argument parse) --
        m_thread_joined = true;
        
        // -- Initialize pipe for proper thread shutdown -- 
        int res_pipes = pipe(pipes);
   
        if(res_pipes < 0){
           cout << "Could not initialize Linux pipes" << endl;
        }
        
        // -- Initialize PMTs IN/OUT --
        out_port_0 = pmt::mp("out_pdus");
	  
	    message_port_register_in(pmt::mp("pdus"));
	    message_port_register_out(out_port_0);      // out vectors ...
	  
	    set_msg_handler(pmt::mp("pdus"), boost::bind(&build_packet_physical_source_impl::packet_handler, this, _1));
        
        // -- Set desired packet FIFO size --
        if(m_bufferLength > 0){
           s_fifo.fifo_changeSize(m_bufferLength); 
        }
        
        // -- CAN BUS device --
        if(m_deviceSource == CAN_BUS_DEVICE){ // 1st option ...
           bool ok_dev = false;
           bool ok_id = false;
           // Example of the string to parse - we expected hex string for CAN ID ... 
           // --> string str_t = "can_dev:can0,can_id:0x123";
           
           // Parse device option ... 
           vector<string> array_str = split_str(m_deviceOption, ","); 
           // Get CAN device ... 
           can_params.canDev = get_value_from_key(array_str, "can_dev", ":", ok_dev);
           // Gey CAN ID for incomming packets ...
           can_params.can_id = stoul(get_value_from_key(array_str, "can_id", ":", ok_id), 0, 16);                
           // Invoke worker thread ...
           if(res_pipes == 0  && ok_dev && ok_id){
              m_thread_joined = false;
              _thread = gr::thread::thread(&build_packet_physical_source_impl::canBus_packet_thread_handler, this, can_params);   
           }
        }
        
        // -- END OF INIT --
        
    }
    
    // canBus_packet_thread_handler - handles incomming CANBus packets
    void build_packet_physical_source_impl::canBus_packet_thread_handler(struct canBusParamsTx can_params){
        
        int current_frame_len = 0;
        uint8_t payload_len = 0x00;
        uint32_t crc32_check = 0;

        vector<unsigned char> payload_vector, can_data_packet;
        
        int fd = 0, res = 0;
        int state = PACKET_HEADER_SEARCH;
        
        // Initialize Linux CANBus layer ...
        CanBus bus(can_params.canDev.c_str());
                
        if((fd = bus.connect()) < 0){
           cout << "Unable to open CANBus device: " << can_params.canDev << endl;        
           m_thread_joined = true;
           return;
        }
        
        cout << "CANBus device opened succesfully: "<< can_params.canDev << endl;         
        
        struct pollfd pfds[2] = {0};

        // Set to zero ...
        memset(&pfds, 0, sizeof(pfds));
        
        pfds[0].fd = pipes[0];
        pfds[0].events = POLLIN; 
        
        pfds[1].fd = fd;
        pfds[1].events = POLLIN;
        
        // Packet handler ...        
        while(true){
            
            if(poll(pfds,2,-1)){ 
               // Check for errors ...
               if(errno == EINTR){
                  continue;
               }
          
               // Terminate thread - if requested ...
               if(pfds[0].revents & POLLIN){
                  read(pipes[0], &res, sizeof(res));
                  if(res == 1) break;
               }
              
               // Read incomming packets ...
               if(pfds[1].revents & POLLIN){
                  CanFrame frame;
                  bus.read(&frame); 
                  
                  // Check incomming CAN packet structure and id,
                  // CAN frame length is no more checked: frame.length() != CAN_PACKET_LEN
                  if(frame.id() != can_params.can_id){
                     continue;   
                  }
                  
                  // Set current frame length ...
                  current_frame_len = frame.length();
                  
                  // Push back frame bytes to std vector ...
                  for (int i = 0; i < frame.length(); i++) {
                       can_data_packet.push_back(frame.data()[i]);
                  }
               }
            }
            
            // -- State - PACKET_HEADER_SEARCH --
            if (state == PACKET_HEADER_SEARCH){
                // Check for packet header
                if((current_frame_len == CAN_PACKET_LEN_DEFAULT) && (*(uint16_t*)&can_data_packet[0] == CONST_SYNC_WORD)){
                    // Check payload len and spare byte	      
                    if(can_data_packet[2] == m_payloadLength && can_data_packet[3] == CONST_SPARE_BYTE){
                      // Set current payload len
                      payload_len = can_data_packet[2];
                      // Set CRC32 to be later calculated based on payload content
                      crc32_check = *(uint32_t*)&can_data_packet[4];
                      // Move to state PACKET_PAYLOAD_GET
                      state = PACKET_PAYLOAD_GET;
                    }
                }
            // -- State - PACKET_PAYLOAD_GET -- 
            } else if (state == PACKET_PAYLOAD_GET){ 	
                payload_vector.insert(payload_vector.end(), can_data_packet.begin(), can_data_packet.end());

                if(payload_vector.size() == payload_len){
                   // Check CRC32 ...
                   if(gr::digital::crc32(&payload_vector[0], payload_vector.size()) == crc32_check){
                      // Add payload to the FIFO  and finally clear payload_vector ...
                      gr_storage_packet packet_wr(crc32_check, payload_vector);
                        
                      if(!s_fifo.fifo_write_storage(&packet_wr,1)){
                          cout << "Packet TX FIFO overflow" << endl;
                      }
                        
                      payload_vector.clear();
                   }
                 // Go back to PACKET_HEADER_SEARCH		       
                 state = PACKET_HEADER_SEARCH;	
                 }
            }
            
            // Clear can_data_packet ...
            can_data_packet.clear();
            
            // End of while(1) loop ...
        }
        
        // Disconnect from socket
        cout << "CANBus thread is about to be terminated" <<endl; 
        
        bus.disconnect();

    }
    

    /*
     * Our virtual destructor.
     */
    build_packet_physical_source_impl::~build_packet_physical_source_impl(){
        
        // Only for debug  ...
        // -------------------
        cout<< "Calling DESTRUCTOR - build_packet_physical_source_impl ... "<< endl;
        
        if(!m_thread_joined){
           stopRunningThread(); 
        }
        
    }
    
    bool build_packet_physical_source_impl::stop(){
        
        // Only for debug  ...
        // -------------------
        cout<< "Calling STOP - build_packet_physical_source_impl ... "<< endl;
        
        if(!m_thread_joined){
           stopRunningThread(); 
        }
        
        return true;
    }
    
    void build_packet_physical_source_impl::stopRunningThread(){
        
        // Stop running thread flag
        int i = 1; 
        // Write pipe ... 
        write(pipes[1],&i,sizeof(i));
        // Wait until the thread will join ...
        _thread.join();
        // Close pipe ...
        close(pipes[0]);
        close(pipes[1]);
        // Set m_thread_joined variable ...  
        m_thread_joined = true;
    }

    void
    build_packet_physical_source_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required){
      
        /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int build_packet_physical_source_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      
      /*  
      const <+ITYPE+> *in = (const <+ITYPE+> *) input_items[0];
      <+OTYPE+> *out = (<+OTYPE+> *) output_items[0];
      */
      
      // Do <+signal processing+>
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (noutput_items);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }
    
    // packet_handler - public function / callback /
    void build_packet_physical_source_impl::packet_handler(pmt::pmt_t msg){

        uint16_t offset = 0;
        uint16_t packetLength = m_payloadLength + CONST_HEADER_PRELOAD_BYTES + CONST_HEADER_PACKET_SIZE 
						+ CONST_HEADER_CRC32 + CONST_PAYLOAD_CRC32;    
        
        gr_storage_packet packet_payload(0);
                        
        if(!s_fifo.fifo_read_storage(&packet_payload, 1)){
           // No data to process - exit ...
           return; 
        }
        
        // Get parameters from the pdu / or make dict ...
        // pmt::pmt_t meta = pmt::car(msg);
        pmt::pmt_t meta = pmt::make_dict();
        
        vector<unsigned char> data_packet(packetLength, 0x00); 				 // Set default values ...

        // Generate header of the packet - sync word, payload_length, CRC32 header
        *(uint16_t*)&data_packet[0] = CONST_SYNC_WORD;       				 // Header == 0x1337 [2 bytes]; aka CONST_HEADER_PRELOAD_BYTES
        *(uint16_t*)&data_packet[2] = m_payloadLength;				         // [2 bytes]; aka CONST_HEADER_PACKET_SIZE
        *(uint32_t*)&data_packet[4] = gr::digital::crc32(&data_packet[0],4); // [4 bytes]; aka CONST_HEADER_CRC32 
        offset+=8; 
        
        // Get payload vector and copy useful payload
        vector<unsigned char> payload_vector = packet_payload.getVector();
        
        vector<unsigned char>::iterator it = data_packet.begin();
        advance(it, offset);
        std::copy(payload_vector.begin(), payload_vector.end(), inserter(data_packet, it));
        offset += payload_vector.size();
        
        // Generate CRC for payload
        *(uint32_t*)&data_packet[offset] = packet_payload.getFlag();  // gr::digital::crc32(&data_packet[CONST_PAYLOAD_OFFSET], payload_vector.size());

        // -- Data packed --
        if(m_dataType == CONST_DATA_PACKED){
           pmt::pmt_t packed_vec = pmt::init_u8vector(packetLength, data_packet);
           message_port_pub(out_port_0, pmt::cons(meta, packed_vec));
        }

        // -- Data unpacked --
        /* in 0b11110000 out 0b00000001 0b00000001 0b00000001 0b00000001 0b00000000 0b00000000 0b00000000 0b00000000
         * https://stackoverflow.com/questions/50977399/what-do-packed-to-unpacked-blocks-do-in-gnu-radio
         */
	
        if(m_dataType == CONST_DATA_UNPACKED){
           int unpackedLength = CONST_BITS_PER_BYTE * packetLength;  
           vector<unsigned char> data_unpacked(unpackedLength,0x00); // unpacked bytes ...
	   
           for (int i=0; i < unpackedLength; i++){
                data_unpacked[i] = (unsigned char)((data_packet.at(i/8) & (1 << (7 - (i % 8)))) != 0);
           }
	   
           pmt::pmt_t unpacked_vec = pmt::init_u8vector(unpackedLength, data_unpacked);
           message_port_pub(out_port_0, pmt::cons(meta, unpacked_vec));
        }
    }
    
  } /* namespace PacketDeCoderSDR */
} /* namespace gr */

