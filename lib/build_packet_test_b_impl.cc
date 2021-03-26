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
#include "build_packet_test_b_impl.h"

#include <cstdlib>
#include <limits>
#include <time.h> 

#define CONST_PACKET_SIZE 32

#ifndef PDU_PORT_ID
    #define PDU_PORT_ID    pmt::mp("pdus")
#endif

namespace gr {
  namespace PacketDeCoderSDR {
    
    using namespace std;

    build_packet_test_b::sptr
    build_packet_test_b::make(unsigned int sps, unsigned int tx_id)
    {
      return gnuradio::get_initial_sptr
        (new build_packet_test_b_impl(sps, tx_id));
    }

    /*
     * The private constructor ...
     * ---------------------------
     */
    build_packet_test_b_impl::build_packet_test_b_impl(unsigned int sps, unsigned int tx_id)
      : gr::block("build_packet_test_b",
              gr::io_signature::make(0, 0, 0), // gr::io_signature::make(<+MIN_IN+>, <+MAX_IN+>, sizeof(<+ITYPE+>)),
              gr::io_signature::make(0, 0, 0)) // gr::io_signature::make(<+MIN_OUT+>, <+MAX_OUT+>, sizeof(<+OTYPE+>)))
              
    {
      sample_count = 0;
      this->sps = sps;
      this->tx_id = tx_id;
      
      message_port_register_out(PDU_PORT_ID);
      message_port_register_in(pmt::mp("generate"));
      set_msg_handler(pmt::mp("generate"), boost::bind(&build_packet_test_b_impl::build_packet, this, _1));
      
      // Initialize random seed ...
      // --------------------------
      srand(time(NULL));
    }

    /*
     * Our virtual destructor ...
     * --------------------------
     */
    build_packet_test_b_impl::~build_packet_test_b_impl()
    {
    }

    void build_packet_test_b_impl::build_packet(pmt::pmt_t msg){
      
      uint32_t imax = numeric_limits<uint32_t>::max();
      if(sample_count == imax) sample_count = 0;

      *(uint32_t*)&packet[0] = 0xAAAAAAAA;
      *(uint32_t*)&packet[4] = tx_id;
      
      packet[8] = 0x20;
      
      *(uint32_t*)&packet[9] = sample_count++; // Bytes (counter) - 9 10 11 12 ... 
      *(uint32_t*)&packet[13] = rand();	       // Bytes (random 1) - 13 14 15 16 ...
      *(uint32_t*)&packet[17] = rand();	       // Bytes (random 2) - 17 18 19 20 ...
      *(uint32_t*)&packet[21] = rand();	       // Bytes (random 2) - 21 22 23 24 ...
      *(uint32_t*)&packet[25] = rand();	       // Bytes (random 2) - 25 26 27 28 ...
      
      packet[29] = 0xA2;
      packet[30] = 0x45;
      
      // Generate simple checksum ...
      // ----------------------------
      packet[31] = generate_checksum(packet,CONST_PACKET_SIZE-1);

      std::vector<unsigned char> vec(256);
      
      for (int i=0; i<256; i++){
           vec[i] = (unsigned char)((packet[i / 8] & (1 << (7 - (i % 8)))) != 0);
      }
      
      // Send the vector ...
      // -------------------
      pmt::pmt_t vecpmt(pmt::make_blob(&vec[0], 256));
      pmt::pmt_t pdu(pmt::cons(pmt::PMT_NIL, vecpmt));

      message_port_pub(PDU_PORT_ID, pdu);
    }
    
  } /* namespace PacketDeCoderSDR */
} /* namespace gr */

