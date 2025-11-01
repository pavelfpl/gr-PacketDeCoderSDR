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

#include <gnuradio/io_signature.h>
#include "build_packet_physical_sink_impl.h"

#include <sys/ioctl.h>
#include <sys/poll.h>
#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include <gnuradio/io_signature.h>

#include <gnuradio/io_signature.h>
#include <gnuradio/blocks/pdu.h>
#include <gnuradio/digital/crc32.h>

// Include CANBus layer
#include "PacketDeCoderSDR/xFace_can/Bus/CanBus.hpp"
#include "PacketDeCoderSDR/xFace_can/Frame/CanFrame.hpp"

#include "util.h"

#define CONST_HEADER_PREAMBLE_OFFSET 0
#define CONST_HEADER_PAYLOAD_SIZE_OFFSET 2
#define CONST_HEADER_CRC32_OFFSET 4
#define CONST_PAYLOAD_OFFSET 8
#define CONST_PAYLOAD_CRC32_SIZE 4

#define CONST_SYNC_WORD 0x1337
#define CONST_SPARE_BYTE 0x00

#define CAN_BUS_DEVICE 0x00
#define CAN_PACKET_LEN 8

#define CONST_DATA_PACKED 0 
#define CONST_DATA_UNPACKED 1

namespace gr {
  namespace PacketDeCoderSDR {
      
    using namespace std;  
    
    // Define static variables ...
    // ---------------------------
    fifo_buffer_packet build_packet_physical_sink_impl::s_fifo;

    build_packet_physical_sink::sptr
    build_packet_physical_sink::make(int deviceSource, const std::string &deviceOption, int payloadLength, int dataType, int bufferLength, uint32_t packeGapSleep)
    {
      return gnuradio::get_initial_sptr
        (new build_packet_physical_sink_impl(deviceSource, deviceOption, payloadLength, dataType, bufferLength, packeGapSleep));
    }

    /*
     * The private constructor
     */
    build_packet_physical_sink_impl::build_packet_physical_sink_impl(int deviceSource, const std::string &deviceOption, int payloadLength, int dataType, int bufferLength, uint32_t packeGapSleep)
      : gr::block("build_packet_physical_sink",
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
        m_exit_requested = true;
        
        // -- Initialize PMTs IN/OUT --
	    message_port_register_in(pmt::mp("pdus"));
	    set_msg_handler(pmt::mp("pdus"), boost::bind(&build_packet_physical_sink_impl::packet_handler, this, _1));
        
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
           // Sleep gap time between packets ...
           can_params.can_sleep = packeGapSleep;
           // Invoke worker thread ...
           if(ok_dev && ok_id){
              m_thread_joined = false;
              m_exit_requested = false;
              _thread = gr::thread::thread(&build_packet_physical_sink_impl::canBus_packet_thread_handler, this, can_params);   
           }
        }
        
    }
    
    // canBus_packet_thread_handler - handles incomming CANBus packets
    void build_packet_physical_sink_impl::canBus_packet_thread_handler(struct canBusParamsRx can_params){
        
        struct timespec remaining, request = { 0, can_params.can_sleep };
        
        // Initialize Linux CANBus layer ...
        CanBus bus(can_params.canDev.c_str());
        
        if(bus.connect() < 0){
           cout << "Unable to open CANBus device: "<< can_params.canDev  << endl;
           m_thread_joined = true;
           return;
        }
        
        // Packet handler ...        
        while(true){
            
            gr_storage_packet packet_payload(0);
                        
            if(s_fifo.fifo_read_storage(&packet_payload, 1)){
               
               // Get payload vector and copy useful payload
               vector<unsigned char> payload_vector = packet_payload.getVector();
                
               can_frame c_frame;
               c_frame.can_id = can_params.can_id; 
               c_frame.can_dlc = CAN_PACKET_LEN;
               
               // Send CANBus header ...
               *(uint16_t*)&c_frame.data[0] = CONST_SYNC_WORD;
               c_frame.data[2] = payload_vector.size();
               c_frame.data[3] = CONST_SPARE_BYTE;
               *(uint32_t*)&c_frame.data[4] = packet_payload.getFlag(); // Get CRC32 of payload (flag)
               bus.send(&c_frame);
               
               // Set sleep interval ... 
               nanosleep(&request, &remaining);
               
               // Send payload divided to the CANBus packets ...
               for (int i=0;i<payload_vector.size();i++){
                   c_frame.data[i % CAN_PACKET_LEN] = payload_vector[i];
                   if((i % CAN_PACKET_LEN) == CAN_PACKET_LEN-1){
                      bus.send(&c_frame);
                      nanosleep(&request, &remaining);
                   }
               }
            }else{
                // Set the same sleep interval as for packets gap ...
                nanosleep(&request, &remaining);
            }
            
            // shared global variable - exit requested if m_exit_requested == TRUE --> break ... 
            gr::thread::scoped_lock lock(fp_mutex);   
            if(m_exit_requested){ 
               break;                
            }
        }
        
        // Disconnect from socket
        bus.disconnect();
    
    }
    
    /*
     * Our virtual destructor.
     */
    build_packet_physical_sink_impl::~build_packet_physical_sink_impl()
    {
        
        // Only for debug  ...
        cout<< "Calling DESTRUCTOR - build_packet_physical_sink_impl ... "<< endl;
        
        if(!m_thread_joined){
           stopRunningThread(); 
        }
        
    }
    
    bool build_packet_physical_sink_impl::stop(){
        
        // Only for debug  ...
        cout<< "Calling STOP - build_packet_physical_sink_impl ... "<< endl;
        
        if(!m_thread_joined){
           stopRunningThread(); 
        }
        
        return true;
    }
    
    // stopRunningThread function [private] ...
    void build_packet_physical_sink_impl::stopRunningThread(){

        gr::thread::scoped_lock lock(fp_mutex);   // shared resources ...
        m_exit_requested = true;
        lock.unlock();

        _thread.join();

        // Set m_thread_joined variable ...  
        m_thread_joined = true;
    }

    void
    build_packet_physical_sink_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int
    build_packet_physical_sink_impl::general_work (int noutput_items,
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
    
    void build_packet_physical_sink_impl::packet_handler(pmt::pmt_t msg){
	  
        // Get parameters from the pdu ...
        vector<unsigned char> data_uint8(pmt::u8vector_elements(pmt::cdr(msg)));
                
        int packedSize = 0;
        int payloadSize = 0;
        
        int offset = -1;
        
        bool headerOK = false;
        bool headerCrcOK = false;
        bool payloadCrcOK = false;
        bool packetOk = false;
        
        vector<unsigned char> data_packed; 	    // packed data ...
        vector<unsigned char> data_payload; 	// payload data ..
        
        if(data_uint8.size() == 0){
           return;
        }
        
        // Extract data
        if(m_dataType == CONST_DATA_UNPACKED){
        
            unsigned char byteVal = 0x00;  
            unsigned int c = 0;
            
            // Pack bytes - MSB first - we only want full bytes ...
            packedSize = data_uint8.size()/8;
            
            // Resize & fill vector data_packed ...
            data_packed.assign(packedSize,0x00);

            // Pack bytes ... 
            for(unsigned int i=0;i<packedSize;i++){
                byteVal = data_uint8.at(c++); // MSB bit ... 7
                data_packed[i] = ((byteVal & 0x01) << 7);
                byteVal = data_uint8.at(c++); //  ... 6
                data_packed[i] = ((byteVal & 0x01) << 6);
                byteVal = data_uint8.at(c++); //  ... 5
                data_packed[i] = ((byteVal & 0x01) << 5);
                byteVal = data_uint8.at(c++); //  ... 4
                data_packed[i] = ((byteVal & 0x01) << 4);
                byteVal = data_uint8.at(c++); //  ... 3
                data_packed[i] = ((byteVal & 0x01) << 3);
                byteVal = data_uint8.at(c++); //  ... 2
                data_packed[i] = ((byteVal & 0x01) << 2);
                byteVal = data_uint8.at(c++); //  ... 1
                data_packed[i] = ((byteVal & 0x01) << 1);
                byteVal = data_uint8.at(c++); //  LSB bit ... 0
                data_packed[i] = ((byteVal & 0x01) << 0);
            }
        }else{
            data_packed = data_uint8;
            packedSize = data_packed.size(); 
        }
        
        // Decode header of the packet ...
        // Looking for header ...
        for(unsigned int i = 0;i<packedSize-1;i++){
            if(*(uint16_t*)&data_packed[i] == CONST_SYNC_WORD){     // Compare uint16_t ...
                offset = i; headerOK = true;
                break;
            }
        }
        
        // Invalid packet - offset ...
        if(offset == -1){
           GR_LOG_INFO(d_logger, "Invalid packet - HEADER was not found");
           return; 
        }
        
        // Extract payload size ...
        payloadSize = *(uint16_t*)&data_packed[offset + CONST_HEADER_PAYLOAD_SIZE_OFFSET];

        // Test header CRC32 ...
        if(*(uint32_t*)&data_packed[offset + CONST_HEADER_CRC32_OFFSET] == gr::digital::crc32(&data_packed[offset + CONST_HEADER_PREAMBLE_OFFSET], 4)){
            headerCrcOK = true;
        }
        
        // IF headerCrcOK is OK ...
        if(headerCrcOK){ 
            // Invalid packet - short packet - PAYLOAD ...
            if((offset + CONST_PAYLOAD_OFFSET + payloadSize + CONST_PAYLOAD_CRC32_SIZE) > packedSize){
                GR_LOG_INFO(d_logger, "Invalid packet - short packet - PAYLOAD");
                return; 
            }
            
            
            uint32_t payload_crc32 = gr::digital::crc32(&data_packed[offset+CONST_PAYLOAD_OFFSET], payloadSize);
            
            // Test payload CRC32 ...
            if(*(uint32_t*)&data_packed[offset + payloadSize + CONST_PAYLOAD_OFFSET] == payload_crc32){
               payloadCrcOK = true;
               packetOk = true;
            }
            
            if(payloadCrcOK && packetOk){
               data_payload.assign(payloadSize, 0x00);
               copy(&data_packed[offset+CONST_PAYLOAD_OFFSET], &data_packed[offset + payloadSize + CONST_PAYLOAD_OFFSET-1], data_payload.begin());
               // Add to packet FIFO ...
               gr_storage_packet packet_wr(payload_crc32, data_payload);
                        
               if(!s_fifo.fifo_write_storage(&packet_wr,1)){
                   cout << "Packet RX FIFO overflow" << endl;
               }
               
            }
        }
    }
    
  } /* namespace PacketDeCoderSDR */
} /* namespace gr */

