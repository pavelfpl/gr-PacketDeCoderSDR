/* -*- c++ -*- */
/* 
 * Copyright 2019 <Pavel Fiala>.
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


#ifndef INCLUDED_PACKETDECODERSDR_BUILD_PACKET_PHYSICAL_SOURCE_H
#define INCLUDED_PACKETDECODERSDR_BUILD_PACKET_PHYSICAL_SOURCE_H

#include <PacketDeCoderSDR/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace PacketDeCoderSDR {

    /*!
     * \brief <+description of block+>
     * \ingroup PacketDeCoderSDR
     *
     */
    class PACKETDECODERSDR_API build_packet_physical_source : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<build_packet_physical_source> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of PacketDeCoderSDR::build_packet_physical_source.
       *
       * To avoid accidental use of raw pointers, PacketDeCoderSDR::build_packet_physical_source's
       * constructor is in a private implementation
       * class. PacketDeCoderSDR::build_packet_physical_source::make is the public interface for
       * creating new instances.
       */
      static sptr make(int deviceSource = 0, const std::string &deviceOption = "can_dev:can0,can_id:0x123", int payloadLength = 132, int dataType = 0, int bufferLength = 256);
    };

  } // namespace PacketDeCoderSDR
} // namespace gr

#endif /* INCLUDED_PACKETDECODERSDR_BUILD_PACKET_PHYSICAL_SOURCE_H */

