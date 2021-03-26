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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include <gnuradio/blocks/pdu.h>
#include "bits_to_softbits_impl.h"

#include <random>

/*
 * ------------------------------
 * # Set C++ 11 standard - ENABLE
 * # ----------------------------
 * set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
 */

namespace gr {
  namespace PacketDeCoderSDR {
    
    using namespace std;
    

    bits_to_softbits::sptr
    bits_to_softbits::make(bool reverse,bool introduceErrors,int errorOffset)
    {
      return gnuradio::get_initial_sptr
        (new bits_to_softbits_impl(reverse,introduceErrors,errorOffset));
    }

    /*
     * The private constructor
     * -----------------------
     */
    bits_to_softbits_impl::bits_to_softbits_impl(bool reverse,bool introduceErrors,int errorOffset)
      : gr::block("bits_to_softbits",
              gr::io_signature::make(0, 0, 0), 	 // gr::io_signature::make(<+MIN_IN+>, <+MAX_IN+>, sizeof(<+ITYPE+>)),
              gr::io_signature::make(0, 0, 0)),  // gr::io_signature::make(<+MIN_OUT+>, <+MAX_OUT+>, sizeof(<+OTYPE+>)))
              m_reverse(reverse),
              m_introduceErrors(introduceErrors),
              m_errorOffset(errorOffset)
    {
	        
      	  out_port_0 = pmt::mp("fcpdus");
	  
	  message_port_register_in(pmt::mp("pdus"));
	  message_port_register_out(out_port_0); 	// pdu - data - float ... 
	  
	  set_msg_handler(pmt::mp("pdus"), boost::bind(&bits_to_softbits_impl::packet_handler, this, _1));
    }

    /*
     * Our virtual destructor.
     * -----------------------
     */
    bits_to_softbits_impl::~bits_to_softbits_impl()
    {
    }

    void
    bits_to_softbits_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int
    bits_to_softbits_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        /*
	const <+ITYPE*> *in = (const <+ITYPE*> *) input_items[0];
        <+OTYPE*> *out = (<+OTYPE*> *) output_items[0];
	*/
	
        // Do <+signal processing+>
        // Tell runtime system how many input items we consumed on
        // each input stream.
        consume_each (noutput_items);

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }
    
    // packet_handler - public function / callback ...
    // -----------------------------------------------
    void bits_to_softbits_impl::packet_handler(pmt::pmt_t msg){
	
      
	if(m_reverse == false){
	   // Get parameters from the pdu / unsigned char ...
	   // -----------------------------------------------
	   vector<unsigned char> data_uint8(pmt::u8vector_elements(pmt::cdr(msg)));
	   // pmt::pmt_t meta = pmt::car(msg);
	   pmt::pmt_t meta = pmt::make_dict();
	   
	  // Introduce errors ...
	  // --------------------
	  std::random_device rd;        // Will be used to obtain a seed for the random number engine
	  std::mt19937 generator(rd()); // Standard mersenne_twister_engine seeded with rd()
	      
	  // std::default_random_engine generator ...
	  // ----------------------------------------
	  std::uniform_real_distribution<float> distribution(0.0, 1.0);
	  std::uniform_real_distribution<float> distribution_2(1, 128);
	  std::uniform_real_distribution<float> distribution_3(1, 512);
	  
	  unsigned int prepend_vector_size = (unsigned int)distribution_2(generator); 
	  unsigned int append_vector_size = (unsigned int)distribution_3(generator); 
	  
	  // Create float vector of given length ...
	  // ---------------------------------------
	  vector<float> data_float(prepend_vector_size+data_uint8.size() + append_vector_size,0.0); // Set default values ...
	   
	  // Prepend random vector ...
	  // -------------------------
	  // std::default_random_engine generator ...
	  // ----------------------------------------
	  for(unsigned int j=0;j<prepend_vector_size;j++){
	       float randNum = distribution(generator);
		  
	       if(randNum <= 0.5){
		  data_float[j] = float(1.0);
	       }else{
		  data_float[j] = float(-1.0);
	       }
	  }
	   
	  for(unsigned int i=prepend_vector_size;i < (data_float.size() - append_vector_size);i++){
	       if(data_uint8.at(i-prepend_vector_size) > 0){
		  data_float[i] = float(1.0);
	       }else{
		  data_float[i] = float(-1.0);
	       }
	  }
	   
	  // Append random vector ...
	  // ------------------------
	  // std::default_random_engine generator ...
	  // ---------------------------------------- 
	  for(unsigned int k=data_float.size();k<data_float.size();k++){
	      float randNum = distribution(generator); 
	    
	      if(randNum <= 0.5){
		 data_float[k] = float(1.0);
	      }else{
		 data_float[k] = float(-1.0);
	      }
	  }
	  
	  if(m_introduceErrors){

	     int offset = m_errorOffset;
	      
	     for(unsigned int j=0;j<data_float.size();j=j+offset){
		 float randNum = distribution(generator);
		  
		 if(randNum <= 0.5){
		    data_float[j] = float(1.0);
		 } else {
		    data_float[j] = float(-1.0);
		 }
		 // Generate offset ...
		 // -------------------
		 offset = (int)((randNum+0.1)*(float)m_errorOffset);
	      }	      
	  }
	  
	  pmt::pmt_t float_vector = pmt::init_f32vector(data_float.size(), data_float);
	  message_port_pub(out_port_0, pmt::cons(meta, float_vector));  
	}else{
	   // Get parameters from the pdu / float ...
	   // ---------------------------------------
	   vector<float> data_float(pmt::f32vector_elements(pmt::cdr(msg)));
	   // pmt::pmt_t meta = pmt::car(msg);
	   pmt::pmt_t meta = pmt::make_dict();
	  
	   // Create float vector of given length ...
	   // ---------------------------------------
	   vector<unsigned char> data_uint8(data_float.size(),0x00); // Set default values ...
	    
	   for(unsigned int i=0;i<data_uint8.size();i++){
	       if(data_float.at(i) > 0.0){
		  data_uint8[i] = (unsigned char)(0x01);
	       }else{
		  data_uint8[i] = (unsigned char)(0x00);
	       }
	   }
	    
	   pmt::pmt_t uint8_vector = pmt::init_u8vector(data_uint8.size(), data_uint8);
	   message_port_pub(out_port_0, pmt::cons(meta, uint8_vector));   
	}
    }
  } /* namespace PacketDeCoderSDR */
} /* namespace gr */

