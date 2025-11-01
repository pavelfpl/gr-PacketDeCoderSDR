#ifndef GR_STORAGE_PACKET_H
#define GR_STORAGE_PACKET_H

#include <sys/types.h>
#include <vector>

class gr_storage_packet{

public:
    gr_storage_packet(uint flag_ = 0, const  std::vector<unsigned char> &data_packet_ = std::vector<unsigned char>())
        : flag(flag_), data_packet(data_packet_) {}

    // Set functions
    void setFlag(uint flag_){flag = flag_;}
    void setVector(const  std::vector<unsigned char> &data_packet_){data_packet = data_packet_;}

    // Get functions
    uint getFlag() const {return flag;}
    size_t getVectorSize() const {return data_packet.size(); }
    std::vector<unsigned char> getVector() const {return data_packet;}
private:
    int flag;
    std::vector<unsigned char> data_packet;
};

#endif // GR_STORAGE_PACKET_H
