# gr-PacketDeCoderSDR

This out of tree module contains a number of blocks which help building **FSK/QPSK**  modems with ` PDU`  blocks `(PMT message passing)`.

## Dependencies

https://github.com/osh/gr-eventstream  
https://github.com/gr-vt/gr-mapper  
https://github.com/osh/gr-pyqt  
https://github.com/gr-vt/gr-burst  
https://github.com/sandialabs/gr-pdu_utils (optional)

> Extension of original **gr-burst**:
> https://github.com/pavelfpl/gr-burst

## Building
>This module requires **Gnuradio 3.7.x**
>Build is pretty standard:
```
mkdir build
cd build
cmake ..
make
sudo make install
sudo ldconfig
```
## Building local

>**Gnuradio 3.7.x** is installed to `$HOME/gr3.7`:

```
cd gr3.7
source gr3.7-source.env
cd ..
cd gr-gsSDR
mkdir build 
cd build
cmake ../ -Wno-dev -DCMAKE_INSTALL_PREFIX=~/gr3.7.13.4 
make install
sudo ldconfig
```

## Blocks (writtem in C++)
>  **Packet generators and decoders** 

- `build_packet_1` - block creates test pattern (you can `append header`,  select `packet length`, `packed/unpacked data output` and `data source`)
- `decode_packet_1` - decoder for previous block on the receiver side (you forward ` PMT`  message or `save message to file`)

![Group 1](https://github.com/pavelfpl/gr-PacketDeCoderSDR/blob/master/packet_build_decode.png)

>  **Burst FSK modem** 

- `fsk_burst_modulator` - you can select `Samples Per Symbol`, `FSK type` (FSK simple, GFSK, MSK), `deviation (sensitivity)` and `filter taps for GFSK`.
- `burst_fsk_time_sync_c_plus` -  symbol synchronizaton block for FSK (set `sps`, `sample rate`,..) and `coarse frequency offset sync.`  (option).
- `burst_fsk_bit_preamble_demap` - preamble demap after symbol synchronizaton block (set `pre_bits`, `frame block size`, `frame bits skip` and `align` option). You can optimize block performance by setting search algorithm (`FFT conv` is default). 

![Group 2](https://github.com/pavelfpl/gr-PacketDeCoderSDR/blob/master/fsk_modem_blocks.png)

>  **Helper blocks** 

- `de_scrambler_additive` - standard additive scrambler/descrambler, fully configurable ...
- `bits_to_softbits` - bits to softbits (and reverse) block with errors introduction (for testing purpose...) 

![Group 3](https://github.com/pavelfpl/gr-PacketDeCoderSDR/blob/master/scrambler_soft_bits.png)

## Example flowcharts

> fsk_test_burst_tx_fec (examples/fsk_test_burst_tx_fec.grc)

![FSK TX](https://github.com/pavelfpl/gr-PacketDeCoderSDR/blob/master/fsk_test_burst_tx_fec.png)

> fsk_test_burst_rx_fec (examples/fsk_test_burst_rx_fec.grc)

![FSK RX](https://github.com/pavelfpl/gr-PacketDeCoderSDR/blob/master/fsk_test_burst_rx_fec.png)

> psk_burst_tx_fec (examples/psk_burst_tx_fec.grc)

![PSK TX](https://github.com/pavelfpl/gr-PacketDeCoderSDR/blob/master/psk_burst_tx_fec.png)

> psk_burst_rx_fec (examples/psk_burst_rx_fec.grc)

![PSK RX](https://github.com/pavelfpl/gr-PacketDeCoderSDR/blob/master/psk_burst_rx_fec.png)

