#include "Arduino.h"


#define MOSI 2
#define WAIT_SHORT asm("nop\n nop\n");
#define WAIT_LONG  asm("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n");
#define MOSI_HI PORTB|=(1<<MOSI);
#define MOSI_LO PORTB&=~(1<<MOSI);
#define BIT1 MOSI_HI WAIT_LONG MOSI_LO WAIT_SHORT
#define BIT0 MOSI_HI WAIT_SHORT MOSI_LO WAIT_LONG
#define BITOUT(data) if (data & 0x80) {BIT1} else {BIT0}

void send_WS_Byte(uint8_t data) {
  cli();
  BITOUT(data); data=data<<1;
  BITOUT(data); data=data<<1;
  BITOUT(data); data=data<<1;
  BITOUT(data); data=data<<1;
  BITOUT(data); data=data<<1;
  BITOUT(data); data=data<<1;
  BITOUT(data); data=data<<1;
  BITOUT(data);
  sei();
 }

void send_WS_LED(uint8_t r,uint8_t g, uint8_t b) {
    send_WS_Byte(g);
    send_WS_Byte(r);
    send_WS_Byte(b);
}

unsigned long  next_time;
void setup() {
    Serial.begin(19200);
    next_time=millis()+1000;
    DDRD|=(1<<PD5);
    DDRB|=(1<<MOSI);
    pinMode(10,1);
    digitalWrite(10,0);
}

#define MAX_LED 256
uint8_t frame_buffer[3*MAX_LED];
uint16_t receive_pointer=0;
bool has_buffer_data;

void loop() {
    if (Serial.available()) {
        uint8_t data=Serial.read();
        if (receive_pointer<3*MAX_LED) {
            frame_buffer[receive_pointer++]=data;
            has_buffer_data=true;
        }
    } else {
        if (has_buffer_data) {
        for (uint16_t i = 0; i < receive_pointer; i+=3)
        {
            send_WS_Byte(frame_buffer[i+1]);
            send_WS_Byte(frame_buffer[i+0]);
            send_WS_Byte(frame_buffer[i+2]);
        }
        receive_pointer=0;
        has_buffer_data=false;
        }
    }
    digitalWrite(10,has_buffer_data?1:0);

}