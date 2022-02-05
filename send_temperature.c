#define F_CPU 32768

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/power.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <stdint.h>

ISR (TIMER1_COMPA_vect)
{
  // Do nothing, just wake up from sleep
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
  // xor checksum
  bytes[4] = bytes[0] ^ bytes[1] ^ bytes[2] ^ bytes[3];
  // sum checksum
  uint16_t sum = bytes[0] + bytes[1] + bytes[2] + bytes[3];
  bytes[5] = (sum + (sum >> 8) + bytes[4]) & 0xFF;
  temperature += 0.1;
  if (temperature > 70) {
    temperature = -40;
  }
}

static inline void set_output_high()
{
  PORTC |= 1 << PORTC7;
}

static inline void set_output_low()
{
  PORTC &= ~(1 << PORTC7);
}

// When running the system at 32 kHz there aren't any spare clock
// cycles to do anything else than send data when we're sending the
// data. For example.. Trying to use loops when sending identical data
// introduces unexpected delays as the loop variables are incremented
// and compared. I tried to do calculations while we're idling,
// waiting for the next pulse to be sent, but the time is not enough
// to do the calculations.
//
// So, all loops are unrolled below and the delays have not been
// calculated but determined from experimentation.
static inline void send_sync()
{
  set_output_high(); _delay_loop_1(4); // 400 us, 100@250 kHz
  set_output_low(); _delay_loop_1(6); // 600 us, 150@250 kHz
  set_output_high(); _delay_loop_1(4);
  set_output_low(); _delay_loop_1(6);
  set_output_high(); _delay_loop_1(4);
  set_output_low(); _delay_loop_1(6);
  set_output_high(); _delay_loop_1(4);
  set_output_low(); _delay_loop_1(6);
  set_output_high(); _delay_loop_1(4);
  set_output_low(); _delay_loop_1(6);
  set_output_high(); _delay_loop_1(4);
  set_output_low(); _delay_loop_1(6);
  set_output_high(); _delay_loop_1(4);
  set_output_low(); _delay_loop_1(6);
  set_output_high(); _delay_loop_1(4);
  set_output_low(); _delay_loop_1(6);

  set_output_high(); _delay_loop_1(104); // 9600 us, 2400@250 kHz
  set_output_low(); _delay_loop_1(51); // 5000 us, 1250@250 kHz
}

static inline void send_one()
{
  set_output_high(); _delay_loop_1(8); // 800 us, 200@250 kHz
  set_output_low(); _delay_loop_1(3); // 680 us, 170@250 kHz
}

static inline void send_zero()
{
  set_output_high(); _delay_loop_1(19); // 1800 us, 450@250 kHz
  set_output_low(); _delay_loop_1(8); // 1120 us, 280@250 kHz
}

void send_packet()
{
  void (*send_bit_fn[49])();
  for (int i = 0; i < 6; i++) {
    for (int j = 7; j >= 0; j--) {
      if (bytes[i] & (1 << j)) {
	send_bit_fn[i*8 + 7 - j] = &send_one;
      }
      else {
	send_bit_fn[i*8 + 7 - j] = &send_zero;
      }
    }
  }
  send_bit_fn[48] = &send_one;

  send_sync();
  send_bit_fn[0]();
  send_bit_fn[1]();
  send_bit_fn[2]();
  send_bit_fn[3]();
  send_bit_fn[4]();
  send_bit_fn[5]();
  send_bit_fn[6]();
  send_bit_fn[7]();
  send_bit_fn[8]();
  send_bit_fn[9]();
  send_bit_fn[10]();
  send_bit_fn[11]();
  send_bit_fn[12]();
  send_bit_fn[13]();
  send_bit_fn[14]();
  send_bit_fn[15]();
  send_bit_fn[16]();
  send_bit_fn[17]();
  send_bit_fn[18]();
  send_bit_fn[19]();
  send_bit_fn[20]();
  send_bit_fn[21]();
  send_bit_fn[22]();
  send_bit_fn[23]();
  send_bit_fn[24]();
  send_bit_fn[25]();
  send_bit_fn[26]();
  send_bit_fn[27]();
  send_bit_fn[28]();
  send_bit_fn[29]();
  send_bit_fn[30]();
  send_bit_fn[31]();
  send_bit_fn[32]();
  send_bit_fn[33]();
  send_bit_fn[34]();
  send_bit_fn[35]();
  send_bit_fn[36]();
  send_bit_fn[37]();
  send_bit_fn[38]();
  send_bit_fn[39]();
  send_bit_fn[40]();
  send_bit_fn[41]();
  send_bit_fn[42]();
  send_bit_fn[43]();
  send_bit_fn[44]();
  send_bit_fn[45]();
  send_bit_fn[46]();
  send_bit_fn[47]();
  send_bit_fn[48]();
  _delay_ms(28);
}

int main()
{
 // Make pin 28 an output
  DDRC |= 1 << DDC7;
  
  // Disable analog comparator
  ACSR |= 1 << ACD;
  
  // Set timer CTC mode with TOP in OCR1A register    
  TCCR1B |= 1 << WGM12;
  TCCR1B &= ~(1 << WGM13);

  // Timer clock source: CLK / 1024
  TCCR1B |= 1 << CS10;
  TCCR1B &= ~(1 << CS11);
  TCCR1B |= 1 << CS12;

  // TIMER TOP value
  OCR1A = 30 * 32768/1024; // 30 seconds

  // Set timer to reach TOP soon
  TCNT1 = OCR1A * 0.9;

  // Enable interrupt on compare match
  TIMSK |= 1 << OCIE1A;
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
