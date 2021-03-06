#define F_CPU 1000000

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdint.h>

const unsigned DEVICE_ID = 682; // 1-1023
const unsigned DEVICE_CHANNEL = 2; // 1-8

/*
Rubicson pool thermometer packet format:

    71       ba       4e       60       ba       0
    01110001 10111010 01001110 01100000 10111010 0
    CCCCRRRR RRRRRR1? BTTTTTTT TTTTPPPP XXXXXXXX 0

- C = channel
- R = random device id (changes after power cycle)
- 1 = always 1, must be 1
- ? = unknown, any value accepted
- B = battery low indicator
- T = 11 bits of temperature with 1024 bias and scaled by 10
- P = Padding / unused (transmitter sends 0000, but display doesn't care)
- X = CRC
*/

#define RF_ON() (PORTC |= 1 << PORTC0)
#define RF_OFF() (PORTC &= ~(1 << PORTC0))

static uint8_t bytes[5] = {
  (DEVICE_CHANNEL - 1) << 4 | DEVICE_ID >> 6,
  DEVICE_ID << 2 | 0x2,
  0, // battery low flag + temperature msb
  0, // temperature lsb + zero padding
  0}; // crc

ISR (TIMER1_COMPA_vect)
{
  // Do nothing
}

inline uint8_t crc8(uint8_t const message[], unsigned nBytes,
		    uint8_t polynomial, uint8_t init)
{
  uint8_t remainder = init;
  unsigned byte, bit;

  for (byte = 0; byte < nBytes; ++byte) {
    remainder ^= message[byte];
    for (bit = 0; bit < 8; ++bit) {
      if (remainder & 0x80) {
	remainder = (remainder << 1) ^ polynomial;
      } else {
	remainder = (remainder << 1);
      }
    }
  }
  return remainder;
}

static inline void update_packet_temperature()
{
  static float temperature = -50;
  if (temperature > 70) {
    temperature = -50;
  }
  uint16_t encoded_temperature = 1024 + temperature * 10;
  temperature += 0.1;

  bytes[2] = encoded_temperature >> 4;
  bytes[3] = encoded_temperature << 4;
  bytes[4] = crc8(bytes, 4, 0x31, 0x00);
}

// Sending the preamble seems unnecessary. The display decodes the signal
// even without the preamble. But, maybe it improves reliability?
static inline void send_preamble()
{
  RF_ON();
  _delay_us(480);
  RF_OFF();
  _delay_us(976);
  for (int i = 0; i < 4; ++i) {
    RF_ON();
    _delay_us(976);
    RF_OFF();
    _delay_us(976);
  }
  _delay_us(4860 - 976);
}

static inline void send_sync()
{
  for (int i = 0; i < 4; ++i) {
    RF_ON();
    _delay_us(732);
    RF_OFF();
    _delay_us(732);
  }
}

static inline void send_one()
{
  RF_ON();
  _delay_us(484);
  RF_OFF();
  _delay_us(244);
}

static inline void send_zero()
{
  RF_ON();
  _delay_us(244);
  RF_OFF();
  _delay_us(484);
}

void send_packet()
{
  send_sync();
  for (int i = 0; i < 5; ++i) {
    for (int j = 7; j >= 0; j--) {
      if (bytes[i] & (1 << j)) {
	send_one();
      }
      else {
	send_zero();
      }
    }
  }
  send_zero();
}

int main()
{
  // Disable analog comparator
  ACSR |= 1 << ACD;

  // Enable internal pull-up for unconnected pins
  PORTA = 0xFF;
  PORTB = 0xFF;
  PORTC = 0xFE;
  PORTD = 0xFF;
  PORTE = 0x07;

  // PC0 is the RF output pin
  DDRC |= 1 << DDC0;

  // Set timer CTC mode with TOP in OCR1A register
  TCCR1B |= 1 << WGM12;
  TCCR1B &= ~(1 << WGM13);

  // Timer clock source: CLK / 1024
  TCCR1B |= 1 << CS10;
  TCCR1B &= ~(1 << CS11);
  TCCR1B |= 1 << CS12;

  // TIMER TOP value
  OCR1A = 968 * (55 + DEVICE_CHANNEL);
  //OCR1A = 55164; // 57 seconds, channel 2

  // Set timer to reach TOP soon
  TCNT1 = OCR1A - 2000;

  // Enable interrupt on compare match
  TIMSK |= 1 << OCIE1A;
  sei();

  set_sleep_mode(SLEEP_MODE_IDLE);

  while (1) {
    sleep_mode();

    update_packet_temperature();
    send_preamble();
    send_packet();
    _delay_ms(5);
    send_packet();
  }
}
