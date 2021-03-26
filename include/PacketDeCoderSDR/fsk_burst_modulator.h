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


#ifndef INCLUDED_PACKETDECODERSDR_FSK_BURST_MODULATOR_H
#define INCLUDED_PACKETDECODERSDR_FSK_BURST_MODULATOR_H

#include <PacketDeCoderSDR/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace PacketDeCoderSDR {

    /*!
     * \brief <+description of block+>
     * \ingroup PacketDeCoderSDR
     *
     */
    class PACKETDECODERSDR_API fsk_burst_modulator : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<fsk_burst_modulator> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of PacketDeCoderSDR::fsk_burst_modulator.
       *
       * To avoid accidental use of raw pointers, PacketDeCoderSDR::fsk_burst_modulator's
       * constructor is in a private implementation
       * class. PacketDeCoderSDR::fsk_burst_modulator::make is the public interface for
       * creating new instances.
       */
      static sptr make(int fskType =0, int sps=8, float sensitivity=0.1963495405, float dev=12.5e3, float sampRate=100e3, const std::vector<float> taps = std::vector<float>());
    };

  } // namespace PacketDeCoderSDR
} // namespace gr

#endif /* INCLUDED_PACKETDECODERSDR_FSK_BURST_MODULATOR_H */

