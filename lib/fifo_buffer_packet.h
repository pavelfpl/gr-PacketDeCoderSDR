#ifndef FIFO_BUFFER_PACKET_H
#define FIFO_BUFFER_PACKET_H

#include <sys/types.h>
#include <iostream>

#include <gnuradio/thread/thread.h>
#include "PacketDeCoderSDR/gr_storage_packet.h"     // #include <module/gr_storage_packet.h>

struct fifo_t{
    // Using only one of these three buffers ...
    // -----------------------------------------
    gr_storage_packet *fifoBufferStorage;

    uint fifoHead;
    uint fifoTail;
    uint fifoMask;
    size_t fifoSize;
};

class fifo_buffer_packet{

public:
    fifo_buffer_packet(size_t bufferSize_ = 256); // Default allocated space :
    ~fifo_buffer_packet();
    void fifo_changeSize(size_t bufferSize_);
    uint fifo_getFifoHead() const {return m_fifo.fifoHead;}
    uint fifo_getFifoTail() const {return m_fifo.fifoTail;}
    uint fifo_getFifoSize() const {return m_fifo.fifoSize;}
    uint fifo_getFifoItemsSize() const {return m_fifo.fifoHead - m_fifo.fifoTail;}

    // -------------------------------
    // gr_storage_packet functions ...
    // -------------------------------
    uint fifo_write_storage(const gr_storage_packet *buff_storage, const uint nStorage);
    uint fifo_read_storage(gr_storage_packet *buff_storage, const uint nStorage);
private:
    boost::mutex fp_mutex;

    int m_buffer_type;
    struct fifo_t m_fifo;

    gr_storage_packet *m_fifoBufferStorage;

    unsigned isPowerOfTwo(unsigned int x);
    void fifo_init(gr_storage_packet *buff_storage, size_t bufferSize_);
};



#endif // FIFO_BUFFER_PACKET_H
