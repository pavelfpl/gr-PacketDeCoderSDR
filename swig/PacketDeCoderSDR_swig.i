/* -*- c++ -*- */

#define PACKETDECODERSDR_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "PacketDeCoderSDR_swig_doc.i"

%{
#include "PacketDeCoderSDR/build_packet_test_b.h"
#include "PacketDeCoderSDR/decoder_packet_test_b.h"
#include "PacketDeCoderSDR/burst_fsk_time_sync_c_plus.h"
#include "PacketDeCoderSDR/burst_fsk_bit_preamble_demap.h"
#include "PacketDeCoderSDR/build_packet_1.h"
#include "PacketDeCoderSDR/bits_to_softbits.h"
#include "PacketDeCoderSDR/decode_packet_1.h"
#include "PacketDeCoderSDR/de_scrambler_additive.h"
#include "PacketDeCoderSDR/fsk_burst_modulator.h"
#include "PacketDeCoderSDR/build_packet_physical_source.h"
#include "PacketDeCoderSDR/build_packet_physical_sink.h"
%}


%include "PacketDeCoderSDR/build_packet_test_b.h"
GR_SWIG_BLOCK_MAGIC2(PacketDeCoderSDR, build_packet_test_b);
%include "PacketDeCoderSDR/decoder_packet_test_b.h"
GR_SWIG_BLOCK_MAGIC2(PacketDeCoderSDR, decoder_packet_test_b);
%include "PacketDeCoderSDR/burst_fsk_time_sync_c_plus.h"
GR_SWIG_BLOCK_MAGIC2(PacketDeCoderSDR, burst_fsk_time_sync_c_plus);
%include "PacketDeCoderSDR/burst_fsk_bit_preamble_demap.h"
GR_SWIG_BLOCK_MAGIC2(PacketDeCoderSDR, burst_fsk_bit_preamble_demap);
%include "PacketDeCoderSDR/build_packet_1.h"
GR_SWIG_BLOCK_MAGIC2(PacketDeCoderSDR, build_packet_1);
%include "PacketDeCoderSDR/bits_to_softbits.h"
GR_SWIG_BLOCK_MAGIC2(PacketDeCoderSDR, bits_to_softbits);
%include "PacketDeCoderSDR/decode_packet_1.h"
GR_SWIG_BLOCK_MAGIC2(PacketDeCoderSDR, decode_packet_1);
%include "PacketDeCoderSDR/de_scrambler_additive.h"
GR_SWIG_BLOCK_MAGIC2(PacketDeCoderSDR, de_scrambler_additive);
%include "PacketDeCoderSDR/fsk_burst_modulator.h"
GR_SWIG_BLOCK_MAGIC2(PacketDeCoderSDR, fsk_burst_modulator);
%include "PacketDeCoderSDR/build_packet_physical_source.h"
GR_SWIG_BLOCK_MAGIC2(PacketDeCoderSDR, build_packet_physical_source);
%include "PacketDeCoderSDR/build_packet_physical_sink.h"
GR_SWIG_BLOCK_MAGIC2(PacketDeCoderSDR, build_packet_physical_sink);
