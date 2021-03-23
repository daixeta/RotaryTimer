# RotaryTimer
Star Citizen Volatile Cargo Timer with dual encoders, created for different arduino boards/microcontrollers

This timer serves one purpose, and one purpose only - saving you from being blown up into space dust when mining Quantanium in Star Citizen.

The code is written for the atMega 32U4 Chip (Arduino Pro Micro, Arduino Leonardo - however the Leonardo won't work,
since some essential pins aren't routed to the GPIO headers).

I designed a custom board as well as an enclosure, will upload those too.
(TO BE DONE)
Only a TM1637-controlled, 4-digit 7-segment display will work with this timer.
However it's possible to directly solder the display and TM1637 to the board, or use a breakout board with its own controller chip.


The timer works without any delay() commands, and doesn't need an RTC since it uses millis() to count time.
Over the intended set times of ~ 10-15 minutes the 3s tolerance of millis is perfectly acceptable for me. Maybe I'll add support for an external RTC in the future.

The left encoder (encMin) sets time by being turned cw/ccw, and resets the timer to 0 (and stopping it) when being pushed.
The right encoder sets seconds when being rotated, and starts/pauses the timer when being pushed.

There's two LEDs - warnLED and actLED.
actLED shows the timer being active, as in couting down time.
warnLED shows blinking patters depending on the time still on the clock - the blink interval increases in arbitrary steps, becoming faster as time decreases.
