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

#include <gnuradio/io_signature.h>
#include <gnuradio/blocks/pdu.h>
#include "decoder_packet_test_b_impl.h"

#include <cstdlib>
#include <limits>
#include <time.h> 

#define CONST_PACKET_SIZE 32

namespace gr {
  namespace PacketDeCoderSDR {

    using namespace std;
    
    decoder_packet_test_b::sptr
    decoder_packet_test_b::make()
    {
      return gnuradio::get_initial_sptr
        (new decoder_packet_test_b_impl());
    }

    /*
     * The private constructor
     * -----------------------
     */
    decoder_packet_test_b_impl::decoder_packet_test_b_impl()
      : gr::block("decoder_packet_test_b",
	      gr::io_signature::make(0, 0, 0), // gr::io_signature::make(<+MIN_IN+>, <+MAX_IN+>, sizeof(<+ITYPE+>)),
              gr::io_signature::make(0, 0, 0)) // gr::io_signature::make(<+MIN_OUT+>, <+MAX_OUT+>, sizeof(<+OTYPE+>)))	    
      
    {
      
      out_port = pmt::mp("out");
      message_port_register_in(pmt::mp("pdus"));
      message_port_register_out(out_port);
      set_msg_handler(pmt::mp("pdus"), boost::bind(&decoder_packet_test_b_impl::packet_handler, this, _1));
 
    }

    /*
     * Our virtual destructor.
     * -----------------------
     */
    decoder_packet_test_b_impl::~decoder_packet_test_b_impl(){
      
    }
    
    // shift_data_in - private function ...
    // ------------------------------------
    void decoder_packet_test_b_impl::shift_data_in(bool bit){
        
	int temp = 0, carry = bit;
        
	for (int i = 31; i >= 0; i--) {
            temp = !!(data[i] & 0x80);
            data[i] = (data[i] << 1) | carry;
            carry = temp;
        }
    }
    
    // check_valid_packet - private function ...
    // -----------------------------------------
    void decoder_packet_test_b_impl::check_valid_packet() {
      
        uint32_t preamble = *(uint32_t*)&data[0];
	   
	
        if (preamble == 0xAAAAAAAA) { 	// && (generate_checksum(&data[0],CONST_PACKET_SIZE-1) == data[31])
	    
	    int ok = 0;
	  
	    if(generate_checksum(&data[0],CONST_PACKET_SIZE-1) == data[31]){
	       ok = 1; // cout << "Packet checksum OK - data valid ..."<<endl;	    
	    }else{
	       ok = 0; cout << "Packet checksum FAILED - data invalid ..."<<endl;	 
	    }
	  
	    pmt::pmt_t packet = pmt::make_dict();
	    
            uint32_t tx_id = *(uint32_t*)&data[4]; 	// 4,5,6,7 --> 8 empty ...
	    uint32_t counter = *(uint32_t*)&data[9]; 	// 9,10,11,12 ... 	
	    uint32_t random_0 = *(uint32_t*)&data[13]; 	// 13,14,15,16 ...
	    uint32_t random_1 = *(uint32_t*)&data[17]; 	// 17,18,19,20 ...
	    uint32_t random_2 = *(uint32_t*)&data[21]; 	// 21,22,23,24 ...
	    uint32_t random_3 = *(uint32_t*)&data[25]; 	// 25,26,27,28 ...
	    
            packet = pmt::dict_add(packet, pmt::string_to_symbol("tx_id"), pmt::from_long(tx_id));
            packet = pmt::dict_add(packet, pmt::string_to_symbol("counter"), pmt::from_long(counter));
            packet = pmt::dict_add(packet, pmt::string_to_symbol("checksum"), pmt::from_long(ok));
	    
	    /*
	    packet = pmt::dict_add(packet, pmt::string_to_symbol("random_0"), pmt::from_long(random_0));
	    packet = pmt::dict_add(packet, pmt::string_to_symbol("random_1"), pmt::from_long(random_1));
	    packet = pmt::dict_add(packet, pmt::string_to_symbol("random_2"), pmt::from_long(random_2));
	    packet = pmt::dict_add(packet, pmt::string_to_symbol("random_3"), pmt::from_long(random_3));
	    */  
            message_port_pub(out_port, packet);
        }
    }
    
    // packet_handler - public function / callback ...
    // -----------------------------------------------
    void decoder_packet_test_b_impl::packet_handler(pmt::pmt_t msg){
	
	// pmt::pmt_t meta(pmt::car(msg));
	pmt::pmt_t bytes(pmt::cdr(msg));

	size_t msg_len = 0;
	const uint8_t* bytes_in = pmt::u8vector_elements(bytes, msg_len);
      
	for (int i = 0; i < msg_len; i++) {
	    shift_data_in(bytes_in[i]);
	    check_valid_packet();
	}
    }
    
  } /* namespace PacketDeCoderSDR */
} /* namespace gr */

