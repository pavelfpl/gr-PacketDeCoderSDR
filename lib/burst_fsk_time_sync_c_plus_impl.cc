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
#include "burst_fsk_time_sync_c_plus_impl.h"

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <complex>
#include <vector>
#include <math.h>
#include <fftw3.h>

#define CONST_MEAN_OFFSET 5   // Default 3 [5] ...
#define CONST_MEAN_LENGTH 10  // Default 4 [10]...

namespace gr {
  namespace PacketDeCoderSDR {
    
    using namespace std;

    burst_fsk_time_sync_c_plus::sptr
    burst_fsk_time_sync_c_plus::make(float sample_rate, int sps, float upsample_rate, int n_offsets, bool n_freq_offset)
    {
      return gnuradio::get_initial_sptr
        (new burst_fsk_time_sync_c_plus_impl(sample_rate, sps, upsample_rate, n_offsets,n_freq_offset));
    }

    /*
     * The private constructor ...
     * ---------------------------
     */
    burst_fsk_time_sync_c_plus_impl::burst_fsk_time_sync_c_plus_impl(float sample_rate, int sps, float upsample_rate, int n_offsets,bool n_freq_offset)
      : gr::block("burst_fsk_time_sync_c_plus",
	      gr::io_signature::make(0, 0, 0), // gr::io_signature::make(<+MIN_IN+>, <+MAX_IN+>, sizeof(<+ITYPE+>)),
              gr::io_signature::make(0, 0, 0)) // gr::io_signature::make(<+MIN_OUT+>, <+MAX_OUT+>, sizeof(<+OTYPE+>)))	 
    {
      
      m_sample_rate = sample_rate;
      m_sps = sps;
      m_upsample_rate = upsample_rate;
      m_n_offsets = n_offsets;
      m_freq_offset = n_freq_offset;
      
      out_port = pmt::mp("fpdus_out");
      message_port_register_in(pmt::mp("fpdus"));
      message_port_register_out(out_port);
      set_msg_handler(pmt::mp("fpdus"), boost::bind(&burst_fsk_time_sync_c_plus_impl::packet_handler, this, _1));  
    }

    /*
     * Our virtual destructor. ...
     * ---------------------------
     */
    burst_fsk_time_sync_c_plus_impl::~burst_fsk_time_sync_c_plus_impl()
    {
    }

    void
    burst_fsk_time_sync_c_plus_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int
    burst_fsk_time_sync_c_plus_impl::general_work (int noutput_items,
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
    
    // resample_fft - private function  ...
    // ------------------------------------
    void burst_fsk_time_sync_c_plus_impl::resample_fft(double *data, double *result, const int nfft, const int upnfft, const double norm){

	// Allocate ...
	// ------------
	fftw_complex * tmp_fd = (fftw_complex*)fftw_malloc(upnfft*sizeof(fftw_complex));

	// Create fftw plans ...
	// ---------------------
	fftw_plan fft_plan = fftw_plan_dft_r2c_1d(nfft, data, tmp_fd, FFTW_ESTIMATE);
	fftw_plan ifft_plan = fftw_plan_dft_c2r_1d(upnfft, tmp_fd, result, FFTW_ESTIMATE);

	// Zero out tmp_fd ...
	// -------------------
	memset(tmp_fd, 0, upnfft*sizeof(fftw_complex));

	// Execute the plans (forward then reverse) ...
	// --------------------------------------------
	fftw_execute_dft_r2c(fft_plan, data, tmp_fd);

	int offset = upnfft - nfft;

	for (int i = (nfft/2)+1; i<nfft; i++) {
	    tmp_fd[i+offset][0] = tmp_fd[nfft-i][0];
	    tmp_fd[i+offset][1] = -1.0*tmp_fd[nfft-i][1];
	}

	if((nfft%2)==0){
	    int index = nfft/2;
	    tmp_fd[index][0] = tmp_fd[index][0]/2;
	    tmp_fd[index][1] = tmp_fd[index][1]/2;

	    tmp_fd[index+offset][0] = tmp_fd[index][0];
	    tmp_fd[index+offset][1] = tmp_fd[index][1];
	}

	// Only for debug - disable in production ...
	// ------------------------------------------
	/*
	for(int i=0;i<upnfft;i++){
	    cout<<"Result:"<<tmp_fd[i][0]<<";"<<tmp_fd[i][1]<<endl;
	}
	*/

	fftw_execute_dft_c2r(ifft_plan, tmp_fd, result);

	// Cleanup / tmp_fd ...
	// --------------------
	fftw_destroy_plan(fft_plan);
	fftw_destroy_plan(ifft_plan);
	fftw_free(tmp_fd);

	for(int i=0;i<upnfft;i++){
	    // cout<<"Resample result:"<<(result[i]*norm)<<endl;
	    result[i] *= norm;
	}
    }
    
    // mean_calc - private function  ...
    // ---------------------------------
    inline double burst_fsk_time_sync_c_plus_impl::mean_calc(const double *array,const int lenght){

	double mean = 0.0;

	for(int i=0;i<lenght;i++){
	    mean+=array[i];
	}

	return (mean / double(lenght));
    }
    
    // packet_handler - public function / callback ...
    // -----------------------------------------------
    void burst_fsk_time_sync_c_plus_impl::packet_handler(pmt::pmt_t msg){
	
	pmt::pmt_t meta(pmt::car(msg));
	// pmt::pmt_t data(pmt::cdr(msg)); 

	vector<float> data_f(pmt::f32vector_elements(pmt::cdr(msg)));
	vector<double> data(data_f.begin(), data_f.end());
	
	// Define sync parameters ...
	// --------------------------
	int sps = m_sps;		// Default value is 8 ...	
	int n_offset = m_n_offsets;    	// Default value is 8 ...
	int n_resample = 20;		// Default value is 10 ...
	int data_length = data.size();  // Set size ...
	
	// Derived parameters ...
	// ----------------------
	int nsyms = (data_length/sps)-1.0;

	double nfft_d = double(nsyms*sps);
	int nfft = (int)nfft_d;
	double upnfft_d = data_length*double(n_resample)/8.0;
	int upnfft = (int)upnfft_d;

	// Set norm factor ...
	double norm = (upnfft_d/nfft_d)/double(upnfft);

	// Allocate result array ...
	double *result = (double*)fftw_malloc(upnfft*sizeof(double));

	// Calculate length norm length ...
	int length_norm = upnfft - (upnfft%n_resample);

	// Set best_dist & best syms output vector ...
	float best_dist = 0;		// double --> float ...
	vector<float> best_syms;	// double --> float ...
	
	// Basic coarse freq synchronization - for non - coherent FSK variant ...
	// ----------------------------------------------------------------------
	if(m_freq_offset){
       int pre_offset = data_length/5;
       int mean_length = data_length/2;
       float mean_freq_offset = mean_calc(data.data()+pre_offset,mean_length); 
       
       // Only for debug - disable in production ...
       cout<<"Mean offset: "<<mean_freq_offset<<endl;
       
       for(int o=0;o<data_length;o++){
           data[o] = data[o]-mean_freq_offset;    
       }
    }

	for(int o=0;o<n_offset;o++){
	    double dist = 0;
	    vector<float> tmp_syms;	// double --> float ...

	    // Calculate parameters / enable or disable  ...
	    // ---------------------------------------------
	    upnfft_d = (data_length-o)*double(n_resample)/8.0;
	    upnfft = (int)upnfft_d;
	    norm = (upnfft_d/nfft_d)/double(upnfft);
	    length_norm = upnfft - (upnfft%n_resample);

	    // Set loop parameters ...
	    // -----------------------
	    resample_fft(data.data()+o,result,nfft,upnfft,norm);   // data --> using data.data() + o [offset]

	    // Calculate "local" mean ...
	    // --------------------------
	    for(int i = 0;i < length_norm;i = i + n_resample){
		float mean = (float)mean_calc(&result[i+CONST_MEAN_OFFSET], CONST_MEAN_LENGTH);
		tmp_syms.push_back(mean);
	    }

	    // Calculate "global" mean ...
	    // ---------------------------
	    for(uint j = 0;j < tmp_syms.size();j++){
		dist+= fabs(tmp_syms.at(j));
	    }

	    dist = dist / float(tmp_syms.size());

	    if(dist > best_dist){
	      best_dist = dist;
	      best_syms = tmp_syms;    // using copy constructor ...
	    }
	}
	
	// Cleanup / tmp_fd ...
	// --------------------
	fftw_free(result);
	 
	// -----------------------------
	// Encapsulate & sent vector ...
	// -----------------------------
	pmt::pmt_t new_msg;
	// pmt::pmt_t meta = pmt::make_dict();
	pmt::pmt_t newvec = pmt::init_f32vector(best_syms.size(), best_syms);
	meta = pmt::dict_add(meta, pmt::string_to_symbol("best_dist"), pmt::from_float(best_dist));
	new_msg = pmt::cons(meta, newvec);
	message_port_pub(out_port,new_msg);
    }
  } /* namespace PacketDeCoderSDR */
} /* namespace gr */

