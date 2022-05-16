# Pool thermometer transmitter (wt0122 / wt0124)

This is a 433 MHz data transmitter which uses the same data protocol
as the pool thermometers wt0122 and wt0124.

I've used an atmega8515 connected to a 433 MHz transmitter to
communicate with a real display from a wt0122 unit.

It works, but after some time the display loses synchronization with
the sender and it stops displaying temperatures. The problem is that
the display doesn't accept any clock drift at the transmitter side and
requires that the sender sends data exactly every 30 seconds. At the
moment I'm using the internal clock of the atmega8515, but it's not
accurate enough (+-3%) to lock transmissions to every 30 seconds -
transmissions drift in time.

## Display behaviour

Here are some notes and observations of the wt0122 display.

* When first turned on, the display will listen for transmissions
  continuously for 3 minutes. If a transmission is detected it will
  remember the time for the transmission.

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

* The display only shows temperatures between -40.0 C and +70.0 C.
