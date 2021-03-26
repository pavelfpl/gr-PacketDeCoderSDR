# Copyright 2011,2012 Free Software Foundation, Inc.
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

########################################################################
# Setup library
########################################################################
include(GrPlatform) #define LIB_SUFFIX

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

list(APPEND PacketDeCoderSDR_sources
    util.cc
    build_packet_test_b_impl.cc
    decoder_packet_test_b_impl.cc
    burst_fsk_time_sync_c_plus_impl.cc
    burst_fsk_bit_preamble_demap_impl.cc
    build_packet_1_impl.cc
    bits_to_softbits_impl.cc
    decode_packet_1_impl.cc
    de_scrambler_additive_impl.cc
    fsk_burst_modulator_impl.cc
)

set(PacketDeCoderSDR_sources "${PacketDeCoderSDR_sources}" PARENT_SCOPE)
if(NOT PacketDeCoderSDR_sources)
	MESSAGE(STATUS "No C++ sources... skipping lib/")
	return()
endif(NOT PacketDeCoderSDR_sources)

add_library(gnuradio-PacketDeCoderSDR SHARED ${PacketDeCoderSDR_sources})
target_link_libraries(gnuradio-PacketDeCoderSDR ${Boost_LIBRARIES} ${GNURADIO_ALL_LIBRARIES} fftw3)
set_target_properties(gnuradio-PacketDeCoderSDR PROPERTIES DEFINE_SYMBOL "gnuradio_PacketDeCoderSDR_EXPORTS")

if(APPLE)
    set_target_properties(gnuradio-PacketDeCoderSDR PROPERTIES
        INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
    )
endif(APPLE)

########################################################################
# Install built library files
########################################################################
install(TARGETS gnuradio-PacketDeCoderSDR
    LIBRARY DESTINATION lib${LIB_SUFFIX} # .so/.dylib file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
    RUNTIME DESTINATION bin              # .dll file
)

########################################################################
# Build and register unit test
########################################################################
include(GrTest)

include_directories(${CPPUNIT_INCLUDE_DIRS})

list(APPEND test_PacketDeCoderSDR_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/test_PacketDeCoderSDR.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/qa_PacketDeCoderSDR.cc
)

add_executable(test-PacketDeCoderSDR ${test_PacketDeCoderSDR_sources})

target_link_libraries(
  test-PacketDeCoderSDR
  ${GNURADIO_RUNTIME_LIBRARIES}
  ${Boost_LIBRARIES}
  ${CPPUNIT_LIBRARIES}
  gnuradio-PacketDeCoderSDR
)

GR_ADD_TEST(test_PacketDeCoderSDR test-PacketDeCoderSDR)
