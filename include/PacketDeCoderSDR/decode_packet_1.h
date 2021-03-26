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


#ifndef INCLUDED_PACKETDECODERSDR_DECODE_PACKET_1_H
#define INCLUDED_PACKETDECODERSDR_DECODE_PACKET_1_H

#include <PacketDeCoderSDR/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace PacketDeCoderSDR {

    /*!
     * \brief <+description of block+>
     * \ingroup PacketDeCoderSDR
     *
     */
    class PACKETDECODERSDR_API decode_packet_1 : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<decode_packet_1> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of PacketDeCoderSDR::decode_packet_1.
       *
       * To avoid accidental use of raw pointers, PacketDeCoderSDR::decode_packet_1's
       * constructor is in a private implementation
       * class. PacketDeCoderSDR::decode_packet_1::make is the public interface for
       * creating new instances.
       */
      static sptr make(bool decodeHeader=true, int packetLength=64 , int dataTyp=0, int dataPresentation=0, const char *filename=NULL);
    };

  } // namespace PacketDeCoderSDR
} // namespace gr

#endif /* INCLUDED_PACKETDECODERSDR_DECODE_PACKET_1_H */

