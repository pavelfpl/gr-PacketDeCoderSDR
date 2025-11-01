#include "fifo_buffer_packet.h"

// fifo_buffer constructor [public]
// --------------------------------
fifo_buffer_packet::fifo_buffer_packet(size_t bufferSize_){

    m_fifoBufferStorage = 0;
    m_fifoBufferStorage = new gr_storage_packet[bufferSize_];

    // Init FIFO ...
    fifo_init(m_fifoBufferStorage, bufferSize_+1);
}

// fifo_buffer destructor [public]
// -------------------------------
fifo_buffer_packet::~fifo_buffer_packet(){

    if(m_fifoBufferStorage != 0){
        delete[] m_fifoBufferStorage;
        m_fifoBufferStorage = 0;
    }
}

// fifo_init function [private]
// ----------------------------
void fifo_buffer_packet::fifo_init(gr_storage_packet *buff_storage, size_t bufferSize_){

    m_fifo.fifoBufferStorage = buff_storage;
    m_fifo.fifoHead = 0;
    m_fifo.fifoTail = 0;
    m_fifo.fifoMask = bufferSize_-1;
    m_fifo.fifoSize = bufferSize_;
}

// fifo_changeSize function [public]
// ---------------------------------
void fifo_buffer_packet::fifo_changeSize(size_t bufferSize_){

    if(m_fifoBufferStorage != 0){
       delete[] m_fifoBufferStorage;
       m_fifoBufferStorage = 0;
    }

    m_fifoBufferStorage = new gr_storage_packet[bufferSize_];

    // Init FIFO ...
    fifo_init(m_fifoBufferStorage, bufferSize_+1);

}

// gr_storage functions ...
// ------------------------
uint fifo_buffer_packet::fifo_write_storage(const gr_storage_packet *buff_storage, const uint nStorage){

  uint j = 0;
  const gr_storage_packet *p_gr = buff_storage;

  gr::thread::scoped_lock lock(fp_mutex);   // shared resources ...

  for(j=0;j<nStorage;j++){
        if(m_fifo.fifoHead+1 == m_fifo.fifoTail || ((m_fifo.fifoHead+1 == m_fifo.fifoSize) && (m_fifo.fifoTail==0))){
           return j;                                          // fifo buffer overflow - no more space to write ...
        }else{
            // uint fifoHeadTmp = m_fifo.fifoHead;
            m_fifo.fifoBufferStorage[m_fifo.fifoHead] =*p_gr++;
            m_fifo.fifoHead++;
            // m_fifo.fifoHead = (fifoHeadTmp & m_fifo.fifoMask);
            if(m_fifo.fifoHead == m_fifo.fifoSize) {
               m_fifo.fifoHead = 0;
            }
        }
  }

  return nStorage;  // fifo write OK ...
}

uint fifo_buffer_packet::fifo_read_storage(gr_storage_packet *buff_storage, const uint nStorage){

  uint j = 0;

  gr::thread::scoped_lock lock(fp_mutex);   // shared resources ...

  for(j=0;j<nStorage;j++){
        if(m_fifo.fifoHead == m_fifo.fifoTail ){
           return j; // no or more data to be read ...
        }else{
            // uint fifoTailTmp = m_fifo.fifoTail;
            *buff_storage++ = m_fifo.fifoBufferStorage[m_fifo.fifoTail];
            m_fifo.fifoTail++;
            //m_fifo.fifoTail = (fifoTailTmp & m_fifo.fifoMask);
            if(m_fifo.fifoTail == m_fifo.fifoSize){
               m_fifo.fifoTail = 0;
            }
        }
  }

  return nStorage;  // fifo read OK ..
}
