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

#ifndef INCLUDED_PACKETDECODERSDR_BUILD_PACKET_TEST_B_IMPL_H
#define INCLUDED_PACKETDECODERSDR_BUILD_PACKET_TEST_B_IMPL_H

#include <PacketDeCoderSDR/build_packet_test_b.h>
#include "util.h"

namespace gr {
  namespace PacketDeCoderSDR {

    class build_packet_test_b_impl : public build_packet_test_b
    {
     private:
	unsigned int sps;
	unsigned int tx_id;
	unsigned int sample_count;
	unsigned char packet[32];
     public:
	build_packet_test_b_impl(unsigned int sps, unsigned int tx_id);
	~build_packet_test_b_impl();

	void build_packet(pmt::pmt_t msg);
    };
  } // namespace PacketDeCoderSDR
} // namespace gr

#endif /* INCLUDED_PACKETDECODERSDR_BUILD_PACKET_TEST_B_IMPL_H */

