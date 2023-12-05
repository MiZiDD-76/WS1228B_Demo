# Demo for WS2812B LED chain

## Architecture

An ATMEGA32U4 micro is waiting for data bytes from the Serial port over USB.
The main loop reads one byte at a time if there are bytes in the receive buffer.
If the transmission stops, it will output the data to the WS2812B string on pin
"MOSI", and starts waiting again.

A python demo application simulates moving "sprites" on the strip and sends
a new frame every 1/60s. The data format is: datalength + one byte per RGB value for each LED:
  
  lowbyte(data_length), highbyte(data_length)
  R1 G1 B1 R2 G2 B3 .... Rn Gn Bn.

If data_length is zero, all LEDs will be turned off.

If a 2 sec timeout occurs, before all expected data bytes (including high byte of data_length)
are received, all LEDs will be switched off and a new data frame is expected.

## Hardware

The ATMEGA has an internal buffer for 256 LEDs. If more data is sent from the host
application, it will be ignored.

The Serial data stream to the WS2812B strip is generated in Software - I found no way
to use the ATMEGA's hardware modules. Both SPI and USART modules use logic High as 
idle line state, which is opposite from what the WS2812B expects. Nothing, what 
a transistor inverter couldn't fix, but the transfer rate is quite high, with ~300ns
high time for a short pulse (frequency ~3MHz).


