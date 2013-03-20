CanonEOSArduinoIRShutter
========================

Arduino code to fire the shutter of an EOS camera using IR.
Commands are sent via Serial to the Arduino board. 
Commands are available to fire the shutter instantly, after a delay or at a specified interval.

Sends the same IR signals as the Canon RC-1 and RC-5 remotes
 * send 2 lots of 16 pulses seperated by 5.3 ms pause for 2 second delayed shutter
 * send 2 lots of 16 pulses seperated by 7.3 ms pause for instant shutter

Tested with an EOS 60D and Arduino Mega 2560 board

NOTE: Code to fire shutter was copied from a post on
http://arduino.cc/forum/index.php/topic,11167.0.html
This is all that I know about the guy that wrote it:
username : jaysee
website : http://www.wishlistbutler.com/

How to setup circuit
--------------------

All that that is needed is the arduino board and IR LED and a computer to send serial from.

On the Arduino Mega 2560 board the -ve pin of the LED is connected to the GND pin and the positive pin to the OC2A timer output pin which is pin 10 (ref: http://arduino.cc/en/Hacking/PinMapping2560).

Camera needs to be set to remote shutter.

Example Commands
----------------

Fire the shutter instantly  
<code>fire</code>

Fire the shutter after a 2 second delay  
<code>delay</code>

Fire the shutter after a 10 second delay  
<code>delay 10</code>

Fire the shutter every 120 seconds with no end time  
<code>interval 120</code>

Fire the shutter every 120 seconds a total of 10 times  
<code>interval 120 10</code>

