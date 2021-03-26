/* -*- c++ -*- */
/* 
 * Copyright 2019 <+YOU OR YOUR COMPANY+>.
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

#include <gnuradio/logger.h>
#include <gnuradio/io_signature.h>
#include <gnuradio/blocks/pdu.h>
#include <gnuradio/thread/thread.h>
#include <gnuradio/digital/crc32.h>

#include "decode_packet_1_impl.h"

// -------------------
// System standard ...
// -------------------
#include <sys/types.h>
#include <sys/stat.h>
#include <stdexcept>
#include <cstdlib>
#include <limits>
#include <time.h> 
#include <stdio.h>

#define CONST_HEADER_PRELOAD_OFFSET 0
#define CONST_HEADER_PAYLOAD_SIZE_OFFSET 2
#define CONST_HEADER_CRC32_OFFSET 4
#define CONST_PAYLOAD_OFFSET 8
#define CONST_PAYLOAD_CRC32_SIZE 4

#define CONST_DATA_DISPLAY_PMT 0
#define CONST_DATA_SAVE_FILE 1

#define CONST_DATA_PACKED 0 
#define CONST_DATA_UNPACKED 1

namespace gr {
  namespace PacketDeCoderSDR {
    
    using namespace std;

    decode_packet_1::sptr
    decode_packet_1::make(bool decodeHeader, int packetLength, int dataType, int dataPresentation, const char *filename)
    {
      return gnuradio::get_initial_sptr
        (new decode_packet_1_impl(decodeHeader, packetLength, dataType, dataPresentation, filename));
    }

    /*
     * The private constructor ...
     * ---------------------------
     */
    decode_packet_1_impl::decode_packet_1_impl(bool decodeHeader, int packetLength, int dataType, int dataPresentation, const char *filename)
      : gr::block("decode_packet_1",
              gr::io_signature::make(0, 0, 0), 		// gr::io_signature::make(<+MIN_IN+>, <+MAX_IN+>, sizeof(<+ITYPE+>)),
              gr::io_signature::make(0, 0, 0)),  	// gr::io_signature::make(<+MIN_OUT+>, <+MAX_OUT+>, sizeof(<+OTYPE+>)))
	      m_decodeHeader(decodeHeader),	 	// decodeHeader - decode header ...
              m_packetLength(packetLength),	 	// burst packet length / including header ...
              m_dataType(dataType),		 	// output data type - packed or unpacked ... 
              m_dataPresentation(dataPresentation)	// dataPresentation - file or display / for testing ... 
    {
      
       if(packetLength == 0) packetLength = 64; 	// Set default vector [ packet size ]... 
	    
       if(m_dataPresentation == CONST_DATA_SAVE_FILE){
	  fileOpen(filename);
       }
      
       out_port_0 = pmt::mp("out_pdus");
	  
       message_port_register_in(pmt::mp("pdus"));
       message_port_register_out(out_port_0);  
	  
       set_msg_handler(pmt::mp("pdus"), boost::bind(&decode_packet_1_impl::packet_handler, this, _1));	    
    }
    
    // fileOpen - private function ...
    // -------------------------------
    void decode_packet_1_impl::fileOpen(const char *filename){
	
        // obtain exclusive access for duration of this function ...
	// ---------------------------------------------------------
	gr::thread::scoped_lock lock(fp_mutex);
	
	fileToSave.open(filename, ios::out | ios::app | ios::binary);
    }
    
    // fileClose - private function ...
    // --------------------------------
    void decode_packet_1_impl::fileClose(){
	
	// Close file [if opened] ...
	// --------------------------
	if(fileToSave.is_open())
	   fileToSave.close();
    }
    
    /*
     * Our virtual destructor. ...
     * ---------------------------
     */
    decode_packet_1_impl::~decode_packet_1_impl(){
	fileClose();
    }

    void
    decode_packet_1_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int
    decode_packet_1_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        /*
	const <+ITYPE*> *in = (const <+ITYPE*> *) input_items[0];
        <+OTYPE*> *out = (<+OTYPE*> *) output_items[0];
	*/
	
        // Do <+signal processing+>
        // Tell runtime system how many input items we consumed on
        // each input stream.
        consume_each (noutput_items);

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }
    
    // packet_handler - public function / callback ...
    // -----------------------------------------------
    void decode_packet_1_impl::packet_handler(pmt::pmt_t msg){
	  
	// Get parameters from the pdu ...
	// -------------------------------
	vector<unsigned char> data_uint8(pmt::u8vector_elements(pmt::cdr(msg)));
	pmt::pmt_t meta = pmt::make_dict(); // pmt::pmt_t meta = pmt::car(msg);
	
	int packedSize = 0;
	int payloadSize = 0;
	
	bool headerOK = false;
	bool headerCrcOK = false;
	bool payloadCrcOK = false;
	bool packetOk = false;
	
	vector<unsigned char> data_packed; 	// packed data ...
	vector<unsigned char> data_payload; 	// payload data ...
	
	// -------------------
	// 0] Unpack bytes ... 
	// -------------------
	if(m_dataType == CONST_DATA_UNPACKED){
	  
	  unsigned char byteVal = 0x00;  
	  unsigned int c = 0;
	  
	  // Pack bytes - MSB first - we only want full bytes ...
	  // ----------------------------------------------------
	  packedSize = data_uint8.size()/8;
	  if(packedSize == 0) return;   // Already packed ?!! ...
	  
	  // Resize & fill vector data_packed ...
	  // ------------------------------------
	  data_packed.assign(packedSize,0x00);

	  // Pack bytes ... 
	  // --------------
	  for(unsigned int i=0;i<packedSize;i++){
	      byteVal = data_uint8.at(c++); // MSB ... 7
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
	      byteVal = data_uint8.at(c++); //  LSB ... 0
	      data_packed[i] = ((byteVal & 0x01) << 0);
	  } // CONST_DATA_UNPACKED end ...
	}else{
	  data_packed = data_uint8;
	  packedSize = data_packed.size();
	  if(packedSize == 0) return; // Data size error ?!! ... 
	  // CONST_DATA_PACKED end ...
	}
	 
	// ------------------------------------------------ 
	// 1] Decode header of the packet / PRE & POST  ...
	// ------------------------------------------------
	if(m_decodeHeader){
	  int offset = -1;
	  // Looking for header ...
	  // ----------------------
	  for(unsigned int i = 0;i<packedSize-1;i++){
	      if(*(uint16_t*)&data_packed[i] == 0x1337){ // Compare uint16_t ...
		offset = i; headerOK = true;
		break;  // Break statement ...
	      }
	  }
	  
	  // Invalid packet - offset ...
	  // ---------------------------
	  if(offset == -1){
	    // Set debug message ... 
	    // ---------------------
	    GR_LOG_INFO(d_logger, "Invalid packet - HEADER was not found");
	    return; 
	  }
	  
	  // Invalid packet - short packet - HEADER ...
	  // ------------------------------------------
	  if((packedSize - offset) < 8){
	    // Set debug message ... 
	    // ---------------------
	    GR_LOG_INFO(d_logger, "Invalid packet - short packet - HEADER");
	    return; 
	  }
	  
	  payloadSize = *(uint16_t*)&data_packed[offset+CONST_HEADER_PAYLOAD_SIZE_OFFSET];
	  
	  // Test header CRC32 ...
	  // ---------------------
	  if(*(uint32_t*)&data_packed[offset+CONST_HEADER_CRC32_OFFSET] == gr::digital::crc32(&data_packed[offset+CONST_HEADER_PRELOAD_OFFSET],4)){
	     headerCrcOK = true;
	  }
	  
	  if(headerCrcOK){ 
	    // Invalid packet - short packet - PAYLOAD ...
	    // ------------------------------------------
	    if((offset + CONST_PAYLOAD_OFFSET + payloadSize + CONST_PAYLOAD_CRC32_SIZE) > packedSize){
	       // Set debug message ... 
	       // ---------------------
	       GR_LOG_INFO(d_logger, "Invalid packet - short packet - PAYLOAD");
	       return; 
	    }
	    
	    // Test payload CRC32 ...
	    // ----------------------
	    if(*(uint32_t*)&data_packed[offset + payloadSize + CONST_PAYLOAD_OFFSET] == gr::digital::crc32(&data_packed[offset+CONST_PAYLOAD_OFFSET],payloadSize)){
	      payloadCrcOK = true;
	      packetOk = true;
	    }
	    
	    // CONST_DATA_SAVE_FILE ...
	    // ------------------------
	    if(payloadCrcOK && m_dataPresentation == CONST_DATA_SAVE_FILE){
	       if(fileToSave.is_open()){
		  char writeBuffer[payloadSize];
		  std::copy(&data_packed[offset+CONST_PAYLOAD_OFFSET], &data_packed[offset + payloadSize + CONST_PAYLOAD_OFFSET-1], writeBuffer);
		  fileToSave.write(writeBuffer, payloadSize);
	       }
	    }
	    
	    // CONST_DATA_DISPLAY_PMT ...
	    // --------------------------
	    if(payloadCrcOK && m_dataPresentation == CONST_DATA_DISPLAY_PMT){
	       data_payload.assign(payloadSize,0x00);
	       std::copy(&data_packed[offset+CONST_PAYLOAD_OFFSET], &data_packed[offset + payloadSize + CONST_PAYLOAD_OFFSET-1], data_payload.begin());
	    }
	 }
	 
	 // packetOk == false --> forward raw data as PMT debug vector ...  
	 // --------------------------------------------------------------
	 if(packetOk == false){
	    data_payload.assign(packedSize,0x00);
	    std::copy(data_packed.begin(), data_packed.end(), data_payload.begin());
	 }
	 
	 // Add PMT information ...
	 // -----------------------
	 // PMT_API pmt_t - from_bool (bool val) - Return #f is val is false, else return #t
	 meta = pmt::dict_add(meta, pmt::string_to_symbol("Payload CRC32 OK"), pmt::from_bool(payloadCrcOK)); // 
	 meta = pmt::dict_add(meta, pmt::string_to_symbol("Header CRC32 OK"), pmt::from_bool(headerCrcOK));  
	 meta = pmt::dict_add(meta, pmt::string_to_symbol("Payload size"), pmt::from_long(payloadSize));
	 meta = pmt::dict_add(meta, pmt::string_to_symbol("Header 0x1337"), pmt::from_bool(headerOK));
	 meta = pmt::dict_add(meta, pmt::string_to_symbol("Packet OK"), pmt::from_bool(packetOk));
	 
      }else{
	  if(data_packed.size() == 0){
	     // Set debug message ... 
	     // ---------------------
	     GR_LOG_INFO(d_logger, "Invalid packet length (mode without header) ...");
	     return; 
	  }else{
	      // Only warning ...
	      if(data_packed.size() <  m_packetLength)
		packetOk = false; // Payload is OK ...
	      else 
		packetOk = true;  // Payload is KO ...
	  
	     // CONST_DATA_SAVE_FILE ...
	     // ------------------------
	     if(m_dataPresentation == CONST_DATA_SAVE_FILE){
	        if(fileToSave.is_open()){
		  char writeBuffer[m_packetLength];
		  std::copy(&data_packed[0], &data_packed[m_packetLength-1], writeBuffer);
		  fileToSave.write(writeBuffer, m_packetLength);
		}
	     }
	     
	     // CONST_DATA_DISPLAY_PMT ...
	     // --------------------------
	     if(m_dataPresentation == CONST_DATA_DISPLAY_PMT){
	        data_payload.assign(m_packetLength, 0x00);
	        std::copy(&data_packed[0], &data_packed[m_packetLength-1], data_payload.begin());
	     }
	    
	     // Add PMT information ...
	     // -----------------------
	     meta = pmt::dict_add(meta, pmt::string_to_symbol("Packet OK"), pmt::from_bool(packetOk));
	 }
      }

      // Publish PMT data & vectors ...
      // ------------------------------
      if(m_dataPresentation == CONST_DATA_DISPLAY_PMT && data_payload.size() > 0){
	 pmt::pmt_t data_vector = pmt::init_u8vector(data_payload.size(),data_payload); 
	 message_port_pub(out_port_0, pmt::cons(meta, data_vector));
      }else{
	 message_port_pub(out_port_0, meta);
      }  
    }

  } /* namespace PacketDeCoderSDR */
} /* namespace gr */

