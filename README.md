# Pool thermometer transmitter

This is a 433 MHz data transmitter which uses the same data protocol
as a Rubicson pool thermometer.

I've used an atmega8515 connected to a 433 MHz transmitter to
communicate with a Rubicson pool thermometer display.

It works, but sometimes the display loses synchronization with
the sender. When synchronization is lost it seems like synchronization is
restored again after some time.

## Display behaviour

Here are some notes and observations of the pool thermometer's display.

* When first turned on, the display will listen for transmissions
  continuously for ~5 minutes. If a transmission is detected it will
  remember the time for the transmission.

* The display supports (displays) temperatures between -50.0 and +70.0 C










Need to check the behaviours below:

* After the initial 3 minutes the display will listen for
  transmissions only every N*30 seconds after the transmission it
  detected in the initial 3 minute period. The display will listen for
  data for only a short period of time (1-2 seconds).

* In addition to the listen window described above the display also
  seems to wake up at second 0 and 30 according to its internal
  clock. If a transmission is received in these windows, then the
  display will stop listening for data in the previously discovered
  windows. Very strange behaviour.

* If no temperature readings are received for 3 minutes the display
  shows "--.-".

* If the display loses connection with the transmitter it will not try
  to recover by listening for transmissions for a longer time or at
  different times.

* When data has been received and properly decoded an antenna symbol
  flashes on the screen.
