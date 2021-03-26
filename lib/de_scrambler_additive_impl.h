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

#ifndef INCLUDED_PACKETDECODERSDR_DE_SCRAMBLER_ADDITIVE_IMPL_H
#define INCLUDED_PACKETDECODERSDR_DE_SCRAMBLER_ADDITIVE_IMPL_H

#include <PacketDeCoderSDR/de_scrambler_additive.h>

namespace gr {
  namespace PacketDeCoderSDR {

    class de_scrambler_additive_impl : public de_scrambler_additive
    {
     private: 
      pmt::pmt_t out_port_0; 
       
      uint32_t m_seed;
      uint32_t m_seed_init;
      uint32_t m_mask;
      uint32_t m_shift_register;
      uint32_t m_shift_register_length; // less than 32
      int m_bits_per_byte;
      int m_byteAppendOrder;
      
      unsigned char next_bit();
      void lfsr_reset();
     public:
      de_scrambler_additive_impl(int seed, int mask, int regLength, int bits_per_byte, int byteAppendOrder);
      ~de_scrambler_additive_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
		       gr_vector_int &ninput_items,
		       gr_vector_const_void_star &input_items,
		       gr_vector_void_star &output_items);
      
      void packet_handler(pmt::pmt_t msg);
    };

  } // namespace PacketDeCoderSDR
} // namespace gr

#endif /* INCLUDED_PACKETDECODERSDR_DE_SCRAMBLER_ADDITIVE_IMPL_H */

