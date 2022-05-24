# Pool thermometer transmitter

This is a 433 MHz data transmitter which uses the same data protocol
as a Rubicson pool thermometer.

I've used an atmega8515 connected to a 433 MHz transmitter to
communicate with a real pool thermometer display.

The Rubicson thermometer display correctly decodes the data sent,
but without a reliable clock source it seems like the transmitter and
display eventually lose their synchronization and the display then
misses transmissions. The display seems to re-synchronize after some
time.

## Display behaviour

Here are some notes and observations of the pool thermometer's display.

* When first turned on, the display will listen for transmissions
  continuously for ~5 minutes. If a transmission is detected it will
  remember the time for the transmission and then listen for a new
  transmission every (55 + channel number) seconds. If no transmission
  is detected in 5 minutes the display still stop listen continuously
  and instead only listen every (55 + channel number) seconds.

* The display only displays temperatures between -50.0 and +70.0 C.

* If no temperature data is received for 60 minutes the display shows "Er",
  but recovers if transmissions start again.

* When the display listens for a transmission on the selected channel an
  antenna symbol flashes on the screen.

* If a transmission is missed the display will turn off a "signal strength"
  symbol.

## Power consumption

An atmega8515 running at 1 MHz, with 5 V VCC and connected to a 433 MHz
transmitter draws around 0.78 mA when it's in sleep mode. Deeper sleep
would be possible if an external signal is used to trigger an interrupt.

The atmega8515 datasheet says that ~0.5 mA should be possible with 5 V VCC
and a clock frequency of 1 MHz. Can anything else be shut down to save power?

If the device is transmitting continuously the current draw is ~9 mA.

The current measurements were made with a cheap multimeter and could be
inaccurate.
