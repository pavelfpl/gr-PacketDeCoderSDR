
#ifndef INCLUDED_HUBSAN_UTIL_H
#define INCLUDED_HUBSAN_UTIL_H

#include <stdint.h>

namespace gr {
  namespace PacketDeCoderSDR {

    unsigned char generate_checksum(unsigned char *data, int length);
    uint16_t _bswap16(uint16_t x);

  } // namespace PacketDeCoderSDR
} // namespace gr

#endif /* INCLUDED_HUBSAN_UTIL_H */

