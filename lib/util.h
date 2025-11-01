
#ifndef INCLUDED_HUBSAN_UTIL_H
#define INCLUDED_HUBSAN_UTIL_H

#include <stdint.h>

namespace gr {
  namespace PacketDeCoderSDR {
      
    using namespace std;  

    unsigned char generate_checksum(unsigned char *data, int length);
    uint16_t _bswap16(uint16_t x);
    
    vector<string> split_str(const string& s_, const string &delimiter);
    string strip_str(string s_);
    string get_value_from_key(const vector<string> &arr_s_, const string &key_s_, const string &delimiter_, bool &ok);

  } // namespace PacketDeCoderSDR
} // namespace gr

#endif /* INCLUDED_HUBSAN_UTIL_H */

