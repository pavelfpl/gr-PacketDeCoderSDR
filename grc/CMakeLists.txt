# Copyright 2011 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

install(FILES
    PacketDeCoderSDR_build_packet_test_b.xml
    PacketDeCoderSDR_decoder_packet_test_b.xml
    PacketDeCoderSDR_burst_fsk_time_sync_c_plus.xml
    PacketDeCoderSDR_burst_fsk_bit_preamble_demap.xml
    PacketDeCoderSDR_build_packet_1.xml
    PacketDeCoderSDR_bits_to_softbits.xml
    PacketDeCoderSDR_decode_packet_1.xml
    PacketDeCoderSDR_de_scrambler_additive.xml
    PacketDeCoderSDR_fsk_burst_modulator.xml DESTINATION share/gnuradio/grc/blocks
)
