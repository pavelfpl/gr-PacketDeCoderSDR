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

#ifndef INCLUDED_PACKETDECODERSDR_BURST_FSK_TIME_SYNC_C_PLUS_IMPL_H
#define INCLUDED_PACKETDECODERSDR_BURST_FSK_TIME_SYNC_C_PLUS_IMPL_H

#include <PacketDeCoderSDR/burst_fsk_time_sync_c_plus.h>

namespace gr {
  namespace PacketDeCoderSDR {

    class burst_fsk_time_sync_c_plus_impl : public burst_fsk_time_sync_c_plus
    {
     private:
      float m_sample_rate;
      int m_sps;
      float m_upsample_rate;
      int m_n_offsets;
      
      bool m_freq_offset;
      
      pmt::pmt_t out_port;
      void resample_fft(double *data, double *result, const int nfft, const int upnfft, const double norm);
      inline double mean_calc(const double *array,const int lenght);
     public:
      burst_fsk_time_sync_c_plus_impl(float sample_rate, int sps, float upsample_rate, int n_offsets,bool n_freq_offset);
      ~burst_fsk_time_sync_c_plus_impl();

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

#endif /* INCLUDED_PACKETDECODERSDR_BURST_FSK_TIME_SYNC_C_PLUS_IMPL_H */

