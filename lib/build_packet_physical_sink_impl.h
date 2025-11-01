/* -*- c++ -*- */
/* 
 * Copyright 2025 <Pavel Fiala>.
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

#ifndef INCLUDED_PACKETDECODERSDR_BUILD_PACKET_PHYSICAL_SINK_IMPL_H
#define INCLUDED_PACKETDECODERSDR_BUILD_PACKET_PHYSICAL_SINK_IMPL_H

#include <PacketDeCoderSDR/build_packet_physical_sink.h>

#include <gnuradio/thread/thread.h>
#include <string>

#include "fifo_buffer_packet.h"

namespace gr {
  namespace PacketDeCoderSDR {
      
    struct canBusParamsRx{
        std::string canDev;
        uint32_t can_id;
        uint32_t can_sleep;
    };  

    class build_packet_physical_sink_impl : public build_packet_physical_sink
    {
     private:
        static fifo_buffer_packet s_fifo; 		   // Define FIFO packet buffer 
         
        int m_deviceSource;
        std::string m_deviceOption;
        int m_payloadLength;
        int m_dataType;
        bool m_thread_joined;
        bool m_exit_requested;
        int m_bufferLength;
        
        struct canBusParamsRx can_params;
        
        boost::mutex fp_mutex;
        gr::thread::thread _thread;
                
        void canBus_packet_thread_handler(struct canBusParamsRx can_params);
        void stopRunningThread();
     public:
      build_packet_physical_sink_impl(int deviceSource, const std::string &deviceOption, int payloadLength, int dataType, int bufferLength, uint32_t packeGapSleep);
      ~build_packet_physical_sink_impl();
      bool stop() override;                         // virtual reimplement ...

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

#endif /* INCLUDED_PACKETDECODERSDR_BUILD_PACKET_PHYSICAL_SINK_IMPL_H */

