#define F_CPU 1000000

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdint.h>

ISR (TIMER1_COMPA_vect)
{
  // Do nothing
}

static uint8_t bytes[6];
void build_packet()
{
  uint16_t id = 0xab;
  uint8_t channel = 1;

  bytes[0] = 0x50 | ((id >> 4) & 0x0F);
  bytes[1] = (id & 0x0F) << 4;
  bytes[3] = ((0x08 | channel) << 4) | 0x0F;
}

void update_packet_temperature()
{
  static float temperature = -40;
  uint16_t encoded_temperature = 0x990 + temperature * 10;

  bytes[1] = (bytes[1] & 0xF0) | (encoded_temperature >> 8);
  bytes[2] = encoded_temperature & 0xFF;
  bytes[4] = bytes[0] ^ bytes[1] ^ bytes[2] ^ bytes[3];
  uint16_t sum = bytes[0] + bytes[1] + bytes[2] + bytes[3];
  bytes[5] = (sum + (sum >> 8) + bytes[4]) & 0xFF;
  temperature += 0.1;
}

void send_sync()
{
  for (int i = 0; i < 8; ++i) {
    PORTC |= (1 << PORTC7);
    _delay_us(400);
    PORTC &= ~(1 << PORTC7);
    _delay_us(600);
  }

  PORTC |= (1 << PORTC7);
  _delay_us(9600);
  PORTC &= ~(1 << PORTC7);
  _delay_us(5000);
}

static inline void send_one()
{
  PORTC |= (1 << PORTC7);
  _delay_us(800);
  PORTC &= ~(1 << PORTC7);
  _delay_us(680);
}

static inline void send_zero()
{
  PORTC |= (1 << PORTC7);
  _delay_us(1800);
  PORTC &= ~(1 << PORTC7);
  _delay_us(1120);
}

void send_packet()
{
  send_sync();
  for (int i = 0; i < 6; i++) {
    for (int j = 7; j >= 0; j--) {
      if (bytes[i] & (1 << j)) {
	send_one();
      }
      else {
	send_zero();
      }
    }
  }
  send_one();
  _delay_ms(28);
}

int main()
{
  DDRC |= (1 << DDC7);    // Make pin 28 be an output.

  // Disable analog comparator
  ACSR |= 1 << ACD;
  // Disable watchdog TODO: Disabling the watchdog seems to break things. Why?
  /* WDTCR |= (1 << WDCE) | (1 << WDE); */
  // Disable brown out detector?
  
  // Set timer CTC mode with TOP in OCR1A register    
  TCCR1B |= 1 << WGM12;
  TCCR1B &= ~(1 << WGM13);

  // Timer clock source: CLK / 1024
  TCCR1B |= 1 << CS10;
  TCCR1B &= ~(1 << CS11);
  TCCR1B |= 1 << CS12;

  // TIMER TOP value
  OCR1A = 28990; // 30 seconds. Measured, but varies with temperature/voltage

  // Set timer to reach TOP soon
  TCNT1 = 25000;

  // Enable interrupt on compare match
  TIMSK |= (1 << OCIE1A);
  sei();

  set_sleep_mode(SLEEP_MODE_IDLE);

  build_packet();
  while (1) {
    sleep_mode();

    update_packet_temperature();
    for (int i = 0; i < 2; i++) {
      send_packet();
    }
  }
}
