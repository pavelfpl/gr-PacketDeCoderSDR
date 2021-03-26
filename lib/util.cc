#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "util.h"

namespace gr {
  namespace PacketDeCoderSDR {
    
    unsigned char generate_checksum(unsigned char *data, int length){
      
      unsigned char sum = 0;
      
      for (int i = 0; i < length; i++) {
        sum += data[i];
      }
      return 256 - sum;
    }

    uint16_t _bswap16(uint16_t x) { 
      return (x << 8) | (x >> 8);
      
    }
  } /* namespace hubsan */
} /* namespace gr */

