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
#include "de_scrambler_additive_impl.h"
#include <gnuradio/blocks/pdu.h>

// -------------------
// System standard ...
// -------------------
#include <sys/types.h>
#include <sys/stat.h>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <stdio.h>

#define CONST_BYTE_APPEND_MSB 0 
#define CONST_BYTE_APPEND_LSB 1

namespace gr {
  namespace PacketDeCoderSDR {
    
    using namespace std;

    de_scrambler_additive::sptr
    de_scrambler_additive::make(int seed, int mask, int regLength, int bits_per_byte, int byteAppendOrder)
    {
      return gnuradio::get_initial_sptr
        (new de_scrambler_additive_impl(seed, mask, regLength, bits_per_byte, byteAppendOrder));
    }

    /*
     * The private constructor
     * -----------------------
     */
    de_scrambler_additive_impl::de_scrambler_additive_impl(int seed, int mask, int regLength, int bits_per_byte, int byteAppendOrder)
      : gr::block("de_scrambler_additive",
	           gr::io_signature::make(0, 0, 0),  // gr::io_signature::make(<+MIN_IN+>, <+MAX_IN+>, sizeof(<+ITYPE+>)),
		   gr::io_signature::make(0, 0, 0)), // gr::io_signature::make(<+MIN_OUT+>, <+MAX_OUT+>, sizeof(<+OTYPE+>)))	      
		   m_seed(seed),
		   m_seed_init(seed),
		   m_mask(mask),
		   m_shift_register(seed),
		   m_shift_register_length(regLength), // less than 32
		   m_bits_per_byte(bits_per_byte),
		   m_byteAppendOrder(byteAppendOrder)
    {
      
      if(m_shift_register_length > 32) throw std::invalid_argument("shift_register_length must be less than 32 ...");
      if (m_bits_per_byte < 1 || m_bits_per_byte > 8) throw std::invalid_argument("bits_per_byte must be in [1, 8] ...");
      
      out_port_0 = pmt::mp("out_pdus");
	  
      message_port_register_in(pmt::mp("pdus"));
      message_port_register_out(out_port_0);  
	  
      set_msg_handler(pmt::mp("pdus"), boost::bind(&de_scrambler_additive_impl::packet_handler, this, _1));
      
    }
    
    // popCount function ...
    // ---------------------
    // https://stackoverflow.com/questions/109023/how-to-count-the-number-of-set-bits-in-a-32-bit-integer
    static uint32_t popCount(uint32_t x){
	  uint32_t r = x - ((x >> 1) & 033333333333) - ((x >> 2) & 011111111111);
	  return ((r + (r >> 3)) & 030707070707) % 63;
    }

    // next_bit function ...
    // ---------------------
    unsigned char de_scrambler_additive_impl::next_bit(){
	    unsigned char output = m_shift_register & 1;
	    unsigned char newbit = popCount(m_shift_register & m_mask )%2; // popCount - counts 1 ...
	    m_shift_register = ((m_shift_register>>1) | (newbit<<(m_shift_register_length-1))); // length correction -1
	    return output;
    }
    
    // lfsr_reset function ...
    // -----------------------
    void de_scrambler_additive_impl::lfsr_reset(){
      
      m_seed = m_seed_init;
      m_shift_register = m_seed_init;
    }

    /*
     * Our virtual destructor.
     * -----------------------
     */
    de_scrambler_additive_impl::~de_scrambler_additive_impl()
    {
    }

    void
    de_scrambler_additive_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
        /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    int
    de_scrambler_additive_impl::general_work (int noutput_items,
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
    void de_scrambler_additive_impl::packet_handler(pmt::pmt_t msg){
	  
	// Get parameters from the pdu ...
	// -------------------------------
	vector<unsigned char> in_data_uint8(pmt::u8vector_elements(pmt::cdr(msg)));
	pmt::pmt_t meta = pmt::car(msg);
	
	// Reset lfsr ...
	// --------------
	lfsr_reset();
	
	// Create and init output vector ...
	// ---------------------------------
	vector<unsigned char> out_data_uint8(in_data_uint8.size());	
	
	for(int i = 0; i < in_data_uint8.size(); i++) {
	    unsigned char scramble_byte = 0x00;
	    // m_byteAppendOrder == CONST_BYTE_APPEND_MSB ...
	    // ----------------------------------------------
	    if(m_byteAppendOrder == CONST_BYTE_APPEND_MSB){
	       for(int k = m_bits_per_byte-1; k >= 0; k--) {
		   scramble_byte ^= (next_bit() << k);
	       }   
	    }
	    // m_byteAppendOrder == CONST_BYTE_APPEND_LSB ...
	    // ----------------------------------------------
	    if(m_byteAppendOrder == CONST_BYTE_APPEND_LSB){
	       for(int k = 0; k <m_bits_per_byte; k++) {
		   scramble_byte ^= (next_bit() << k);
	       }   
	    }
	    
	    out_data_uint8[i] = in_data_uint8.at(i) ^ scramble_byte;   
	}
	
	 pmt::pmt_t data_vector = pmt::init_u8vector(out_data_uint8.size(),out_data_uint8); 
	 message_port_pub(out_port_0, pmt::cons(meta, data_vector));
    }
    
  } /* namespace PacketDeCoderSDR */
} /* namespace gr */

