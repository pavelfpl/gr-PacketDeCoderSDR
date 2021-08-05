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

#ifndef INCLUDED_PACKETDECODERSDR_BUILD_PACKET_1_IMPL_H
#define INCLUDED_PACKETDECODERSDR_BUILD_PACKET_1_IMPL_H

#include <PacketDeCoderSDR/build_packet_1.h>
#include <fstream>

namespace gr {
  namespace PacketDeCoderSDR {

    class build_packet_1_impl : public build_packet_1
    {
     private:
       boost::mutex fp_mutex;
       std::ifstream fileToTransfer;
       
       pmt::pmt_t out_port_0;
       pmt::pmt_t out_port_1;
       
       bool m_appendHeader;	 // appendHeader - append header to beginning of the packet ...
       int m_packetLength;	 // burst packet length / including header ...
       int m_dataType;		 // output data type - packed or unpacked ...
       int m_dataFrom;		 // dataFrom - fromFile or random / for testing ...
       unsigned long m_fileSize;
       unsigned long m_fileIncrement;
       
       void fileOpen(const char *filename);
       void fileClose();
       
       std::vector<unsigned char> packet_bytes; 
     public:
      build_packet_1_impl(bool appendHeader, int packetLength, int dataType, int dataFrom, const char *filename, std::vector<unsigned char> packet_bytes_h);
      ~build_packet_1_impl();

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

#endif /* INCLUDED_PACKETDECODERSDR_BUILD_PACKET_1_IMPL_H */

