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

#ifndef INCLUDED_PACKETDECODERSDR_DECODER_PACKET_TEST_B_IMPL_H
#define INCLUDED_PACKETDECODERSDR_DECODER_PACKET_TEST_B_IMPL_H

#include <PacketDeCoderSDR/decoder_packet_test_b.h>
#include "util.h"

namespace gr {
  namespace PacketDeCoderSDR {

    class decoder_packet_test_b_impl : public decoder_packet_test_b
    {
     private:
      pmt::pmt_t out_port;
      
      uint8_t data[32];
      
      void shift_data_in(bool bit);
      void check_valid_packet();
     public:
      decoder_packet_test_b_impl();
      ~decoder_packet_test_b_impl();

      void packet_handler(pmt::pmt_t msg);
    };

  } // namespace PacketDeCoderSDR
} // namespace gr

#endif /* INCLUDED_PACKETDECODERSDR_DECODER_PACKET_TEST_B_IMPL_H */

