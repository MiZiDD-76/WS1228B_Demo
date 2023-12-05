#include "Arduino.h"

#define MOSI 2
#define WAIT_SHORT asm("nop\n nop\n");
#define WAIT_LONG asm("nop\n nop\n nop\n nop\n nop\n nop\n nop\n nop\n");
#define MOSI_HI PORTB |= (1 << MOSI);
#define MOSI_LO PORTB &= ~(1 << MOSI);
#define BIT1 MOSI_HI WAIT_LONG MOSI_LO WAIT_SHORT
#define BIT0 MOSI_HI WAIT_SHORT MOSI_LO WAIT_LONG
#define BITOUT(data) \
    if (data & 0x80) \
    {                \
        BIT1         \
    }                \
    else             \
    {                \
        BIT0         \
    }

void send_WS_Byte(uint8_t data)
{
    cli();
    BITOUT(data);
    data = data << 1;
    BITOUT(data);
    data = data << 1;
    BITOUT(data);
    data = data << 1;
    BITOUT(data);
    data = data << 1;
    BITOUT(data);
    data = data << 1;
    BITOUT(data);
    data = data << 1;
    BITOUT(data);
    data = data << 1;
    BITOUT(data);
    sei();
}

void send_WS_LED(uint8_t r, uint8_t g, uint8_t b)
{
    send_WS_Byte(g);
    send_WS_Byte(r);
    send_WS_Byte(b);
}

unsigned long receive_time;
#define MAX_LED 256
uint8_t frame_buffer[3 * MAX_LED];
uint16_t receive_pointer = 0;
bool has_buffer_data;
uint16_t data_length;
uint8_t receive_status;
enum RECEIVE_STATUS
{
    RC_IDLE,
    RC_LOW_LENGTH,
    RC_HIGH_LENGTH,
    RC_DATA
};

#define EXTEND_TIMEOUT() receive_time=millis();
#define TIMEOUT 2000

void setup()
{
    Serial.begin(19200);
    DDRD |= (1 << PD5);
    DDRB |= (1 << MOSI);
    pinMode(10, 1);
    digitalWrite(10, 0);
    receive_status = RC_IDLE;
}

void clearDisplay()
{
    for (uint16_t i = 0; i < MAX_LED * 3; i++)
    {
        send_WS_Byte(0);
    }
}

void sendFrame()
{
    uint8_t r,g,b;
    for (uint16_t i = 0; i < receive_pointer; i += 3)
    {
        r=frame_buffer[i+0];
        g=frame_buffer[i+1];
        b=frame_buffer[i+2];
        send_WS_Byte(g);
        send_WS_Byte(r);
        send_WS_Byte(b);
    }
}


void loop()
{
    /*
        Receive format:
        byte data_length_low
        byte data_length_high
        byte [data_length] rgb_data

        if data_length is zero: switch off all lights
        otherwise expect data length bytes in format: R1 G1 B1 R2 G2 B2 ... Rn Gn Bn
    */

    // Receiver state machine:
    if (Serial.available())
    {
        switch (receive_status)
        {
        case RC_IDLE:
            data_length = (uint8_t)Serial.read();
            receive_status = RC_HIGH_LENGTH;
            EXTEND_TIMEOUT();
            break;
        case RC_HIGH_LENGTH:
            data_length += ((uint8_t)Serial.read()) << 8;
            if (data_length > 0)
            {
                receive_pointer = 0;
                receive_status = RC_DATA;
                EXTEND_TIMEOUT();
            }
            else
            {
                clearDisplay();
                receive_status = RC_IDLE;
            }
            break;
        case RC_DATA:
            frame_buffer[receive_pointer++] = (uint8_t)Serial.read();
            data_length--;
            EXTEND_TIMEOUT();
            if (data_length == 0)
            {
                sendFrame();
                receive_status=RC_IDLE;
            }
            break;
        default:
            receive_status = RC_IDLE;
            break;
        }
    } else {
        // no serial data available
        if (receive_status!=RC_IDLE) {
            // Now what?
            if (millis()-receive_time > TIMEOUT) {
                clearDisplay();
                receive_status=RC_IDLE;
            }
        }
    }
}