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

#ifndef INCLUDED_PACKETDECODERSDR_BURST_FSK_BIT_PREAMBLE_DEMAP_IMPL_H
#define INCLUDED_PACKETDECODERSDR_BURST_FSK_BIT_PREAMBLE_DEMAP_IMPL_H

#include <PacketDeCoderSDR/burst_fsk_bit_preamble_demap.h>

#include <iostream>
#include <vector>

namespace gr {
  namespace PacketDeCoderSDR {
    
    class burst_fsk_bit_preamble_demap_impl : public burst_fsk_bit_preamble_demap
    {
     private:
	bool m_frameAlign;
	int m_frameSkip;
	int m_frameBlockSize;
	int m_algorithm;
	int m_section;
	bool m_shapeSoftBits;
       
       std::vector<gr_complex> preamble_cmplx;     	   // preamble symbol (cmplx) ...
       std::vector<unsigned char> preamble_uchar;     	   // preamble symbol (unsigned char) ...
       
       pmt::pmt_t out_port_0;
       pmt::pmt_t out_port_1;
      
       void conv(const gr_complex* a, const int aLen, const gr_complex* b, const int bLen, std::vector<gr_complex> &result); // complex convolution ...
       void conv_real(const float* a, const int aLen, const float* b, const int bLen, std::vector<float> &result); 	     // real convolution ...  
     public:
      burst_fsk_bit_preamble_demap_impl(std::vector<unsigned char> preamble_bits, bool frameAlign, int frameSkip, int frameBlockSize, int algorithm, int section,bool shapeSoftBits);
      ~burst_fsk_bit_preamble_demap_impl();
      
     void forecast (int noutput_items, gr_vector_int &ninput_items_required);
     int general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items);
      
      void packet_handler(pmt::pmt_t msg);
      
    };

  } // namespace PacketDeCoderSDR
} // namespace gr

#endif /* INCLUDED_PACKETDECODERSDR_BURST_FSK_BIT_PREAMBLE_DEMAP_IMPL_H */

