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
    
    vector<string> split_str(const string& s_, const string &delimiter){

        vector<std::string> tokens;

        size_t pos = 0;
        string token;

        string s = s_;

        while ((pos = s.find(delimiter)) != string::npos){
            token = s.substr(0, pos);
            tokens.push_back(token);
            s = s.substr(pos + delimiter.length(), string::npos);
        }

        tokens.push_back(s);

        return tokens;
    }

    string strip_str(string s_) {

        // right strip option
        while (s_[s_.size() - 1] == ' '){
            s_ = s_.substr(0, s_.size() - 1);
        }

        // left strip option
        while (s_[0] == ' ') {
            s_ = s_.substr(1, s_.size() - 1);
        }

        return s_;
    }

    string get_value_from_key(const vector<string> &arr_s_, const string &key_s_, const string &delimiter_, bool &ok){

        size_t pos = 0;

        for(size_t  i = 0;i < arr_s_.size();i++){
            if ((pos = arr_s_[i].find(key_s_)) != string::npos){
                size_t pos_delim = arr_s_[i].find(delimiter_)+delimiter_.length();
                if(pos_delim != string::npos){
                   ok = true;
                   return strip_str(arr_s_[i].substr(pos_delim, string::npos));
                }else{
                   ok = false;
                   return string();
                }
            }
        }

        ok = false;
        return string();
    }
    
    
  } /* namespace hubsan */
} /* namespace gr */

