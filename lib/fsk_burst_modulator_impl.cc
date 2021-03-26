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
#include "fsk_burst_modulator_impl.h"

// -------------------
// System standard ...
// -------------------
#include <sys/types.h>
#include <sys/stat.h>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <stdio.h>
#include <random>
#include <cmath>

#define F_PI ((float)(M_PI))
#define CONST_FSK_SIMPLE 0
#define CONST_FSK_CMP 1
#define CONST_FSK_GFSK 2 
#define CONST_FSK_MSK 3
#define CONST_FSK_GMSK 4

namespace gr {
  namespace PacketDeCoderSDR {
    
    using namespace std;

    fsk_burst_modulator::sptr
    fsk_burst_modulator::make(int fskType, int sps, float sensitivity, float dev, float sampRate, const std::vector<float> taps)
    {
      return gnuradio::get_initial_sptr
        (new fsk_burst_modulator_impl(fskType, sps, sensitivity, dev, sampRate, taps));
    }

    /*
     * The private constructor ...
     * ---------------------------
     */
    fsk_burst_modulator_impl::fsk_burst_modulator_impl(int fskType, int sps, float sensitivity, float dev, float sampRate, const std::vector<float> taps)
      : gr::block("fsk_burst_modulator",
              gr::io_signature::make(0, 0, 0), 	 // gr::io_signature::make(<+MIN_IN+>, <+MAX_IN+>, sizeof(<+ITYPE+>)),
              gr::io_signature::make(0, 0, 0)),  // gr::io_signature::make(<+MIN_OUT+>, <+MAX_OUT+>, sizeof(<+OTYPE+>)))
              m_fskType(fskType),
              m_sps(sps),
              m_sensitivity(sensitivity),
              m_dev(dev),
              m_sample_rate(sampRate),
	      m_taps(taps)
    {
       /*
	* ----------------------------------------------------
	* https://wiki.gnuradio.org/index.php/SignalProcessing
	* ----------------------------------------------------
	* sensitivity = (pi * modulation_index) / samples_per_symbol
	* gain = samples_per_symbol / (pi * modulation_index)
	* modulation_index = deviation / (baud_rate / 2)
	* sensitivity = (pi / 2) / samples_per_symbol
	**/	
      
       // FSK --> MSK and GMSK ...
       // ------------------------
       if(m_fskType == CONST_FSK_MSK ||  m_fskType == CONST_FSK_GMSK){
	  m_sensitivity = (F_PI / 2) / m_sps;
       }
       
       // m taps ...
       // ----------
       // cout << m_taps.size()<<endl;
       
       // FSK -> GMSK and GFSK ...
       // ------------------------
       if(m_taps.size() > 0 && (m_fskType == CONST_FSK_GFSK ||  m_fskType == CONST_FSK_GMSK)){
	  m_fir = new filter::kernel::fir_filter_fff(1, m_taps);
	  m_prependRandom = m_taps.size()-1;
	  m_appendRandom = m_prependRandom/2;  
       }else{
	  m_fir = 0;
	  m_prependRandom = 0;
	  m_appendRandom = 0;
       }  
       
       m_phase = 0.0;
        
       out_port_0 = pmt::mp("cfpdus");
	  
       message_port_register_in(pmt::mp("pdus"));
       message_port_register_out(out_port_0); 	// pdu - data - complex float ... 
	  
       set_msg_handler(pmt::mp("pdus"), boost::bind(&fsk_burst_modulator_impl::packet_handler, this, _1));  
    }

    /*
     * Our virtual destructor.
     * -----------------------
     */
    fsk_burst_modulator_impl::~fsk_burst_modulator_impl(){
      if(m_fir!=0) {delete m_fir; m_fir = 0;} 
      
    }

    void
    fsk_burst_modulator_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int
    fsk_burst_modulator_impl::general_work (int noutput_items,
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
    void fsk_burst_modulator_impl::packet_handler(pmt::pmt_t msg){
      
	// Do <+signal processing+>
        // ------------------------
        gr::thread::scoped_lock lock(fp_mutex);          // shared resources ...
	  
	// Get parameters from the pdu / unpacked bits - unsigned char ...
	// ---------------------------------------------------------------
	vector<unsigned char> in_data_uint8(pmt::u8vector_elements(pmt::cdr(msg)));
	// pmt::pmt_t meta = pmt::car(msg);
	// pmt::pmt_t meta = pmt::make_dict();
	
	vector<float> in_data_float(m_prependRandom + m_sps*in_data_uint8.size() + m_appendRandom);
	vector<gr_complex> out_data_mod; 	     // output_vector - baseband modulated data ...
	
	// Reset m_phase ...
	m_phase = 0.0; 
	
	// ----------------------
	// Append random bits ...
	// ----------------------
	if(m_prependRandom!=0 || m_appendRandom!=0){
	  std::random_device rd;        // Will be used to obtain a seed for the random number engine ...
	  std::mt19937 generator(rd()); // Standard mersenne_twister_engine seeded with rd() ...
	      
	  // std::default_random_engine generator ...
	  // ----------------------------------------
	  std::uniform_real_distribution<float> distribution(0.0, 1.0);
	  
	  int append_offset = m_prependRandom + (m_sps*in_data_uint8.size());
	  
	  // Prepend BITS / NRZ ...
	  // ----------------------
	  for(int i = 0;i< m_prependRandom;i++){
	      float randNum = distribution(generator); 
	    
	      if(randNum <= 0.5){
		 in_data_float[i] = float(-1.0); 
	      }else{
		 in_data_float[i] = float(1.0);  
	      }   
	  }
	  
	  // Append BITS / NRZ ...
	  // ---------------------
	  for(int i = append_offset;i< (append_offset + m_appendRandom);i++){
	      float randNum = distribution(generator); 
	    
	      if(randNum <= 0.5){
		 in_data_float[i] = float(-1.0); 
	      }else{
		 in_data_float[i] = float(1.0);  
	      }   
	   }    
	}
	
	// --------------------
	// Oversample / NRZ ...
	// --------------------
	for(unsigned int i=0;i<in_data_uint8.size();i++){
	    int offset = m_prependRandom + (m_sps*i); 		// calculate offset ...
	    float f_bit = 0;
	    (in_data_uint8[i] > 0) ? f_bit = 1.0 : f_bit = -1.0; // NRZ coding ...
	  
	    // Oversample ...
	    // --------------
	    for(int j=0;j<m_sps;j++){
		// (j == 0) ? in_data_float[offset+j] = f_bit : 0;
	        in_data_float[offset+j] = f_bit;
	    }
	}
	
	// -----------------------------------------------------
	// CONST_FSK_GFSK || CONST_FSK_MSK || CONST_FSK_GMSK ...
	// -----------------------------------------------------
	if(m_fskType == CONST_FSK_CMP || m_fskType == CONST_FSK_GFSK || m_fskType == CONST_FSK_MSK ||  m_fskType == CONST_FSK_GMSK){
	   
	   // Calculate startOffset ...
	   // -------------------------
	   int startOffset =  0; 
	   
	   bool doFiltering = false;
	  
	   if(m_fir != 0 && m_taps.size() > 0 && (m_fskType == CONST_FSK_GFSK || m_fskType == CONST_FSK_GMSK)){
	      doFiltering = true;
	      startOffset = m_taps.size() - 1;
	   }
	   
	   out_data_mod.resize(in_data_float.size() - startOffset);
	   
	   for(unsigned int j = 0; j < (in_data_float.size()-startOffset); j++){
	       float in_filt = 0.0;
	       float rem_mod = 0.0;
	       float oi_cos = 0.0, oq_sin = 0.0;
	       
	       // Enable filtering ...	      
	       // --------------------
	       if(doFiltering){
		/*
		for (int jj = 0; jj < m_taps.size(); jj++) {
		    in_filt += in_data_float[j + startOffset - jj] * m_taps[jj];
		}
		*/
		
		in_filt = m_fir->filter(&in_data_float[j]);
		
		m_phase = m_phase + m_sensitivity * in_filt; // GFSK || GMSK ...
	      } else {
		m_phase = m_phase + m_sensitivity * in_data_float[j]; // FSK || MSK ... 
	      }
	      
	      // Matlab: array_out(k) = rem(d_phase + pi,2*pi)-pi;
	      rem_mod = std::fmod(m_phase + F_PI, 2.0f * F_PI) - F_PI;
	      
	      int32_t angle = gr::fxpt::float_to_fixed(rem_mod);
	      gr::fxpt::sincos(angle, &oq_sin, &oi_cos);    // sincos (int32_t x, float *s, float *c)
	      
	      out_data_mod[j] = gr_complex(oi_cos, oq_sin); // I(cos(.)) <--> Q(sin(.))
	  } 
       }
       
       // ----------------
       // CONST_FSK_SIMPLE
       // ----------------
       if(m_fskType == CONST_FSK_SIMPLE){
	  out_data_mod.resize(in_data_float.size());
	 
	  for(unsigned int j = 0; j < in_data_float.size(); j++) {
	      float oi_cos = 0.0, oq_sin = 0.0;
	      
	      // Python ...
	      // ----------
	      // numpy.array(numpy.exp(1j*2*numpy.pi*((dev*x*numpy.arange(len(x)))/samp_rate)),dtype="complex64")
		     
	      m_phase = 2*F_PI*(in_data_float[j]*j*m_dev)/m_sample_rate;
	    
	      int32_t angle = gr::fxpt::float_to_fixed(m_phase);
	      gr::fxpt::sincos(angle, &oq_sin, &oi_cos); // sincos (int32_t x, float *s, float *c)
	     
		      
	     out_data_mod[j] = gr_complex(oi_cos, oq_sin); // I(cos(.)) <--> Q(sin(.))
	   
	  }
      }
      
       // Send data ...
       // -------------
       pmt::pmt_t f_cmplx_vector = pmt::init_c32vector(out_data_mod.size(), out_data_mod);
       // message_port_pub(out_port_0, pmt::cons(meta, f_cmplx_vector));  
       message_port_pub(out_port_0, pmt::cons(pmt::PMT_NIL , f_cmplx_vector)); 
    }
    

  } /* namespace PacketDeCoderSDR */
} /* namespace gr */

