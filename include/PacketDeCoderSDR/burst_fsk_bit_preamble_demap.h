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


#ifndef INCLUDED_PACKETDECODERSDR_BURST_FSK_BIT_PREAMBLE_DEMAP_H
#define INCLUDED_PACKETDECODERSDR_BURST_FSK_BIT_PREAMBLE_DEMAP_H

#include <PacketDeCoderSDR/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace PacketDeCoderSDR {

    /*!
     * \brief <+description of block+>
     * \ingroup PacketDeCoderSDR
     *
     */
    class PACKETDECODERSDR_API burst_fsk_bit_preamble_demap : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<burst_fsk_bit_preamble_demap> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of PacketDeCoderSDR::burst_fsk_bit_preamble_demap.
       *
       * To avoid accidental use of raw pointers, PacketDeCoderSDR::burst_fsk_bit_preamble_demap's
       * constructor is in a private implementation
       * class. PacketDeCoderSDR::burst_fsk_bit_preamble_demap::make is the public interface for
       * creating new instances.
       */
      static sptr make(std::vector<unsigned char> preamble_bits, bool frameAlign = false, int frameSkip = 96, int frameBlockSize = 0, int algorithm = 1, int section = 4,bool shapeSoftBits = false);
    };

  } // namespace PacketDeCoderSDR
} // namespace gr

#endif /* INCLUDED_PACKETDECODERSDR_BURST_FSK_BIT_PREAMBLE_DEMAP_H */

