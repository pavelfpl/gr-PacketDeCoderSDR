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

#ifndef INCLUDED_PACKETDECODERSDR_FSK_BURST_MODULATOR_IMPL_H
#define INCLUDED_PACKETDECODERSDR_FSK_BURST_MODULATOR_IMPL_H

#include <PacketDeCoderSDR/fsk_burst_modulator.h>
// #include <pdu_utils/constants.h>
#include <gnuradio/filter/fir_filter.h>
#include <gnuradio/fxpt.h>

namespace gr {
  namespace PacketDeCoderSDR {

    class fsk_burst_modulator_impl : public fsk_burst_modulator
    {
     private:
       pmt::pmt_t out_port_0; 
       
       int m_fskType;
       int m_sps;
       float m_sensitivity;
       float m_dev;
       float m_sample_rate;
       
       int m_prependRandom;
       int m_appendRandom;
       float m_phase;
       
       boost::mutex fp_mutex;
       std::vector<float> m_taps; 	  
       filter::kernel::fir_filter_fff *m_fir;

     public:
      fsk_burst_modulator_impl(int fskType, int sps, float sensitivity, float dev, float sampRate, const std::vector<float> taps);
      ~fsk_burst_modulator_impl();

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

#endif /* INCLUDED_PACKETDECODERSDR_FSK_BURST_MODULATOR_IMPL_H */

