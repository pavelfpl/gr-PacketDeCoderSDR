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


#ifndef INCLUDED_PACKETDECODERSDR_BITS_TO_SOFTBITS_H
#define INCLUDED_PACKETDECODERSDR_BITS_TO_SOFTBITS_H

#include <PacketDeCoderSDR/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace PacketDeCoderSDR {

    /*!
     * \brief <+description of block+>
     * \ingroup PacketDeCoderSDR
     *
     */
    class PACKETDECODERSDR_API bits_to_softbits : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<bits_to_softbits> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of PacketDeCoderSDR::bits_to_softbits.
       *
       * To avoid accidental use of raw pointers, PacketDeCoderSDR::bits_to_softbits's
       * constructor is in a private implementation
       * class. PacketDeCoderSDR::bits_to_softbits::make is the public interface for
       * creating new instances.
       */
      static sptr make(bool reverse=false,bool introduceErrors = false,int errorOffset = 15);
    };

  } // namespace PacketDeCoderSDR
} // namespace gr

#endif /* INCLUDED_PACKETDECODERSDR_BITS_TO_SOFTBITS_H */

