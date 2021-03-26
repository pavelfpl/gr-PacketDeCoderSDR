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

// ------------------------------------------------------------------------------------------
// !!!  ADD FFT component to ROOT CMakeList.txt - set(GR_REQUIRED_COMPONENTS RUNTIME FFT) !!!
// ------------------------------------------------------------------------------------------

#include <gnuradio/fft/fft.h>
#include "burst_fsk_bit_preamble_demap_impl.h"
#include <gnuradio/blocks/pdu.h>
#include <gnuradio/io_signature.h>

#include <volk/volk_typedefs.h>
#include <volk/volk.h>

#include <math.h>
#include <stdio.h>

#include <iterator>
#include <algorithm>    // std::reverse

#define CONST_PREAMBLE_DEBUG 

#define CONST_FFT_CONV 0
#define CONST_RECURSIVE_SEARCH 1

namespace gr {
  namespace PacketDeCoderSDR {
    
    using namespace std;

    burst_fsk_bit_preamble_demap::sptr
    burst_fsk_bit_preamble_demap::make(std::vector<unsigned char> preamble_bits, bool frameAlign, int frameSkip, int frameBlockSize, int algorithm, int section,bool shapeSoftBits)
    {
      return gnuradio::get_initial_sptr
        (new burst_fsk_bit_preamble_demap_impl(preamble_bits, frameAlign, frameSkip, frameBlockSize, algorithm, section,shapeSoftBits));
    }

    /*
     * The private constructor
     * -----------------------
     */
    burst_fsk_bit_preamble_demap_impl::burst_fsk_bit_preamble_demap_impl(std::vector<unsigned char> preamble_bits, bool frameAlign, int frameSkip, int frameBlockSize, int algorithm, int section,bool shapeSoftBits)
      : gr::block("burst_fsk_bit_preamble_demap",
              gr::io_signature::make(0, 0, 0), 	 // gr::io_signature::make(<+MIN_IN+>, <+MAX_IN+>, sizeof(<+ITYPE+>)),
              gr::io_signature::make(0, 0, 0)),  // gr::io_signature::make(<+MIN_OUT+>, <+MAX_OUT+>, sizeof(<+OTYPE+>)))
              m_frameAlign(frameAlign),
	      m_frameSkip(frameSkip),
	      m_frameBlockSize(frameBlockSize),
	      m_algorithm(algorithm),
	      m_section(section),
	      m_shapeSoftBits(shapeSoftBits)
    {
	  
	  m_algorithm = CONST_RECURSIVE_SEARCH;
      
	  // Default preamble ...
	  // --------------------
	  // C++11 only - specific ...
	  /*
	  vector<unsigned char> preamble{0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0,
                                   0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1,
                                   0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0};   
	  */
	  
	  vector<unsigned char> preamble = preamble_bits; // Set desired preamble ...
	  preamble_uchar = preamble_bits;		  // Preamble unsigned char ...	
	  
	  // Only C++11 - specific ...
	  // std::reverse(std::begin(preamble), std::end(preamble));
	  reverse(preamble.begin(), preamble.end());

	  for(unsigned int i=0;i<preamble.size();i++){
	      preamble_cmplx.push_back(gr_complex(float(preamble.at(i)),0));      // create presyms_cmplx ...
	  }
	  	  
	  out_port_0 = pmt::mp("fpdus_soft");
	  out_port_1 = pmt::mp("fpdus_hard");
	  
	  message_port_register_in(pmt::mp("fpdus"));
	  message_port_register_out(out_port_0); 	// Soft bits ...
	  message_port_register_out(out_port_1); 	// Hard bits ...
	  
	  set_msg_handler(pmt::mp("fpdus"), boost::bind(&burst_fsk_bit_preamble_demap_impl::packet_handler, this, _1));
    }
    
    /*
     * Our virtual destructor.
     * -----------------------
     */
    burst_fsk_bit_preamble_demap_impl::~burst_fsk_bit_preamble_demap_impl(){
      // Nothing todo here ...
    }
    
    // conv - calculation of complex convolution ...
    // ---------------------------------------------
    void burst_fsk_bit_preamble_demap_impl::conv(const gr_complex* a, const int aLen, const gr_complex* b, const int bLen, std::vector<gr_complex> &result){

            int fftSize = aLen+bLen-1;
            result.resize(fftSize);

            fft::fft_complex fftEngine1 = fft::fft_complex(fftSize);
            gr_complex* fftInBuf = fftEngine1.get_inbuf();
            memcpy(fftInBuf, a, sizeof(gr_complex)*aLen);
            memset(fftInBuf+aLen, 0, sizeof(gr_complex)*(fftSize-aLen));

            /*
            for(int i=aLen;i<fftSize;i++){
                fftInBuf[i] = gr_complex(0,0);
            }
            */

            fftEngine1.execute();
            gr_complex* fft1OutBuf = fftEngine1.get_outbuf();

            fft::fft_complex fftEngine2 = fft::fft_complex(fftSize);
            fftInBuf = fftEngine2.get_inbuf();
            memcpy(fftInBuf, b, sizeof(gr_complex)*bLen);
            memset(fftInBuf+bLen, 0, sizeof(gr_complex)*(fftSize-bLen));

            /*
            for(int i=bLen;i<fftSize;i++){
                fftInBuf[i] = gr_complex(0,0);
            }
            */

            fftEngine2.execute();
            gr_complex* fft2OutBuf = fftEngine2.get_outbuf();

            // multiply the fft outputs
            fft::fft_complex ifftEngine = fft::fft_complex(fftSize, false);
            gr_complex* ifftInBuf = ifftEngine.get_inbuf();
            volk_32fc_x2_multiply_32fc(ifftInBuf, fft1OutBuf, fft2OutBuf, fftSize);

            // ifft
            ifftEngine.execute();

            // scale back
            volk_32fc_s32fc_multiply_32fc(&result[0], ifftEngine.get_outbuf(), 1.0/fftSize, fftSize);
    }
  
    // conv - calculation of real convolution ...
    // ------------------------------------------
    void burst_fsk_bit_preamble_demap_impl::conv_real(const float* a, const int aLen, const float* b, const int bLen, std::vector<float> &result){

	    int fftSize = aLen+bLen-1;
	    result.resize(fftSize);

	    fft::fft_real_fwd fftEngine1 = fft::fft_real_fwd(fftSize);
	    float* fftInBuf = fftEngine1.get_inbuf();
	    memcpy(fftInBuf, a, sizeof(float)*aLen);
	    memset(fftInBuf+aLen, 0, sizeof(float)*(fftSize-aLen));
	    fftEngine1.execute();
	    gr_complex* fft1OutBuf = fftEngine1.get_outbuf();

	    fft::fft_real_fwd fftEngine2 = fft::fft_real_fwd(fftSize);
	    float* fftInBuf_2 = fftEngine2.get_inbuf();
	    memcpy(fftInBuf_2, b, sizeof(float)*bLen);
	    memset(fftInBuf_2+bLen, 0, sizeof(float)*(fftSize-bLen));
	    fftEngine2.execute();
	    gr_complex* fft2OutBuf = fftEngine2.get_outbuf();

	    // Multiply the fft outputs ...
	    // ----------------------------
	    fft::fft_real_rev ifftEngine = fft::fft_real_rev(fftSize);
	    gr_complex* ifftInBuf = ifftEngine.get_inbuf();
	    volk_32fc_x2_multiply_32fc(ifftInBuf, fft1OutBuf, fft2OutBuf, fftSize/2+1);

	    // ifft ...
	    // --------
	    ifftEngine.execute();

	    // Scale back ...
	    // --------------
	    // volk_32fc_s32fc_multiply_32fc(&result[0], ifftEngine.get_outbuf(), 1.0/fftSize, fftSize);
	    volk_32f_s32f_multiply_32f(&result[0], ifftEngine.get_outbuf(), 1.0/fftSize, fftSize);
    }
    
    void
    burst_fsk_bit_preamble_demap_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int
    burst_fsk_bit_preamble_demap_impl::general_work (int noutput_items,
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
    void burst_fsk_bit_preamble_demap_impl::packet_handler(pmt::pmt_t msg){
      
	// Get parameters from the pdu ...
	// -------------------------------
	vector<float> data_float(pmt::f32vector_elements(pmt::cdr(msg)));
	pmt::pmt_t meta = pmt::car(msg);
	
	if(data_float.size() == 0) return;
	
	int preambleIdxStart = 0;
	int maxIdx = 0;
	float maxVal = -99999.0;
	int maxValInt = -99999; 
	
	vector<uint8_t> data_uchar(data_float.size());	            // data symbols (unsigned char - aka uint8_t) - SIZE OF data_uchar ==  data_float ...
	
	// FFT CONV (xcorr) ...
	// --------------------
	if(m_algorithm == CONST_FFT_CONV){
	   vector<gr_complex> data_cmplx(data_float.size());	    // data symbols (cmplx) - SIZE OF data_cmplx ==  data_float ...	
	   vector<gr_complex> conv_result_cmplx;       	     	    // conv result (cmplx) - SIZE - aLen + bLen - 1  ...
	
	   for(unsigned int i=0;i<data_float.size();i++){
	      if(data_float.at(i) > 0){
		data_cmplx[i] = gr_complex(float(1.0),0); data_uchar[i] = 0x01;
		if(m_shapeSoftBits) data_float[i] = (float)(1.0);
	      }else{
		data_cmplx[i] = gr_complex(float(0.0),0); data_uchar[i] = 0x00;
		if(m_shapeSoftBits) data_float[i] = (float)(-1.0);
	      }
	   }
	    
	   // ------------------------
	   // Do cmplx convolution ...
	   // ------------------------
	   conv(&data_cmplx[0],data_cmplx.size(),&preamble_cmplx[0],preamble_cmplx.size(),conv_result_cmplx);
	    
	   vector<float> preCrossCorr(conv_result_cmplx.size());
	    
	   for(unsigned int j=0; j<preCrossCorr.size(); j++) {
		preCrossCorr[j] = abs(conv_result_cmplx[j]);

		if(preCrossCorr[j] > maxVal) {
		  maxIdx = j;
		  maxVal = preCrossCorr[j];
		}
	   } 
	  
	   preambleIdxStart = maxIdx-preamble_cmplx.size()+1;
	}
	
	// RECURSIVE SEARCH ...
	// --------------------
	if(m_algorithm == CONST_RECURSIVE_SEARCH){
	  
	  unsigned int data_search_length = (data_uchar.size()/m_section) - preamble_uchar.size(); 
	  
	  for(unsigned int i=0;i<data_float.size();i++){
	      if(data_float.at(i) > 0){
		 data_uchar[i] = 0x01; 
		 if(m_shapeSoftBits) data_float[i] = (float)(1.0);
	      }else{
		 data_uchar[i] = 0x00; 
		 if(m_shapeSoftBits) data_float[i] = (float)(-1.0);
	      }
	  }
	  
	  for(unsigned int i=0;i<data_search_length;i++){
	      int hit=0;
	      for(unsigned int j=0;j<preamble_uchar.size();j++){
		  if(data_uchar[i+j] == preamble_uchar[j]){hit++;}
	      }
	   
	      if(hit > maxValInt){
		 preambleIdxStart = i;
		 maxValInt = hit;
	      }
	  }
	  
	  maxVal = maxValInt;
	} 
	
	if(preambleIdxStart < 0 || preambleIdxStart > data_float.size()) return;

#ifdef CONST_PREAMBLE_DEBUG	
	cout<<"Preamble start: "<<preambleIdxStart<<endl;
	cout<<"Max IDx (CONV FFT only): "<< maxIdx<<endl;
	cout<<"Max value: "<< maxVal<<endl;
#endif
	int newOffset = preambleIdxStart;		// Preamble new start index ...
	int newSize = data_float.size() - newOffset;	// Vector new size
	int n_blocks = 0; int n_blocs_mod = 0;
	
	if(newSize <=0) return;
	
	// frameAlign option ...
	// ---------------------
	if(m_frameAlign){
	   // Preamble ...
	   // ------------
	   newOffset += m_frameSkip;			// Preamble new start index - add m_frameSkip ...
	   newSize = newSize - m_frameSkip;		// Vector new size - sub newOffset ...
	   
	   // Payload blocks ...
	   // ------------------
	   if(m_frameBlockSize!=0){
	      n_blocks = newSize / m_frameBlockSize;
	      n_blocs_mod = newSize % m_frameBlockSize;
	   
	      if(n_blocks == 0) return; 			// Short data - no block - return ...
	      newSize = newSize - n_blocs_mod; 
	   }
	}
	
	if(newSize <=0) return;
	
	// Erase - beginning of the vector ...
	// -----------------------------------
	data_float.erase(data_float.begin(),data_float.begin()+newOffset);
	data_uchar.erase(data_uchar.begin(),data_uchar.begin()+newOffset);
	
	// Erase - end of the vector ...
	// -----------------------------
	if(m_frameBlockSize!=0 && n_blocks!=0){
	   data_float.erase(data_float.end()-n_blocs_mod,data_float.end());
	   data_uchar.erase(data_uchar.end()-n_blocs_mod,data_uchar.end());
	}
	
	// Soft BITS shaping ...
	// ---------------------
	
	// Publish SOFT bits ...
	// ---------------------
	pmt::pmt_t soft_vec = pmt::init_f32vector(data_float.size(), data_float);
	message_port_pub(out_port_0, pmt::cons(meta, soft_vec));
	
	// Publish HARD bits ...
	// ---------------------
	pmt::pmt_t hard_vec = pmt::init_u8vector(data_uchar.size(), data_uchar);
	message_port_pub(out_port_1, pmt::cons(meta, hard_vec));
    }
    
  } /* namespace PacketDeCoderSDR */
} /* namespace gr */

