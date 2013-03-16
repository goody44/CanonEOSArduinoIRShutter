/**
 * Camera_EOS_Arduino_IR_Shutter
 *
 * Allows an EOS camera to be remotely fired via IR
 * Provides commands to fire the shutter instantly, with a delay or at a set interval.
 * Parses commands sent to it via serial.
 *
 * Sends the same commands as the Canon RC-1 and RC-5 remotes
 *
 * Tested with an EOS 60D and Arduino Mega 2560 board
 *
 * @author Matthew Goodson
 *
 * NOTE: Code to fire shutter was copied from a post on
 * http://arduino.cc/forum/index.php/topic,11167.0.html
 * This is all that I know about the guy that wrote it:
 * username : jaysee
 * website : http://www.wishlistbutler.com/
 */
extern volatile unsigned long timer0_millis; //timer0_millis is defined in wiring.c

#ifndef cbi // Definitions for setting and clearing register bits
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define SYSCLOCK 16000000  // main system clock of Arduino Board (Hz)

#define IROUT 10    // pin 10 is OC2A output from TIMER2
#define LED 13

#define BURST_FREQUENCY 32600 //IR Burst Frequency in Hz

char command[30]; //
int charCount = 0; //

unsigned long time = 0;

unsigned int count = 3;
boolean toggle = LOW;

boolean wasLow = true;

unsigned long microseconds();
void setup_timer2(unsigned int freq);
uint8_t timer2top(unsigned int freq) ;

//#########################################################

void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(IROUT, OUTPUT) ;

  Serial.begin(9600);

  setup_timer2(BURST_FREQUENCY);
}

//#########################################################

/*
 * Read from serial and when a newline is received parse the command.
 * Max command length is 30 chars
 *
 */
void loop() {

  //while there is serail to read read it into the command char array.
  while (Serial.available() > 0) {
    char c = (char) Serial.read();

    if (c == '\n') {
      //when a newline is hit then we have a command ready to be processed
      if (strcmp(command, "help") == 0) {
        //print out the commands that are available
        Serial.println();
        Serial.println("Available commands:");
        Serial.println("\tfire - fires shutter immediately");
        Serial.println("\tdelay <seconds (optional)>- fires shutter after <?> second delay (default 2 seconds)");
        Serial.println("\tinterval <seconds> - fires shutter every <?> seconds");
        Serial.println();
      } else if (strcmp(command, "fire") == 0) {
        //fire the shutter immediatly
        Serial.println("Firing Instant Shutter...");
        fireShutter(true);
      } else if (strncmp(command, "delay", 5) == 0) {
        //fire the shutter after the specified delay. If no delay is specified then
        //fire after 2 second delay

        long delaySecs = 0;
        if (strlen(command) > 6) {
          //there is a delay period specified. Parse it
          delaySecs = strtol(&command[6], NULL, 0);
        } else {
          //no delay was specified - use the default delay of 2 seconds
          delaySecs = 2;
        }
        
        //only delays 2 seconds or above are valid
        if (delaySecs <= 1) {
          Serial.println("Specified delay was not valid. Delay must be >= 2 seconds.");
        } 
        
        fireDelayShutter(delaySecs);
      } else if (strncmp(command, "interval", 8) == 0) {        
        //fire the shutter at a given interval rate
        
        if (strlen(command) < 10) {
          Serial.println("You did not specify a valid interval.");
        } else {
          //parse the interval
          long interval = strtol(&command[9], NULL, 0);
          if (interval <= 1) {
            Serial.println("Specified interval was not valid. Interval must be >= 2 seconds.");
          } else {
            Serial.print("Firing Shutter at ");
            Serial.print(interval);
            Serial.println(" second intervals...");

            //fire the shutter at the parsed interval. There is no end point
            while (true) {
              fireDelayShutter(interval);
            }
          }
        }
      } else {
        Serial.println("Did not understand command");
      }
      //reset command array
      command[0] = '\0'; 
      charCount = 0;
    } else {
      //add the parse char to the command array
      command[charCount] = c;
      command[charCount+1] = '\0';
      charCount++;  
    }
  }
}

/*
 * Delay for specified amount of seconds before firing shutter.
 * This will signal the shutter to fire using the 2 second delay signal.
 * This allows the user to see (via the light on the camera) that the shutter is
 * about to fire.
 *
 * This also prints out messages counting down to the time that the shutter will fire.
 */
void fireDelayShutter(int delaySecs) {
  for (int i=delaySecs; i>0; i--) {
    Serial.print("Firing in ");
    Serial.print(i);
    Serial.println(" seconds...");
    if (i == 2) {
      //fire delayed shutter (shutter will fire in 2 seconds)
      fireShutter(false);
      
      //wait 2 seconds for shutter to fire and then return
      delay(1000);
      Serial.println("Firing in 1 second");
      delay(1000);
      Serial.println("Firing shutter");
      return;
    }
    delay(1000);
  }
}

/*
 * Fire the shutter. 
 * 
 * @param instant if true send signal to fire shutter instantly
 * if false send signal to fire shutter with 2 second delay
 */
void fireShutter(boolean instant) {

  unsigned long pulses[] = {
    500,0,500          };
  if (instant) {
    //set delay between pulses to 7.3 ms which will cause the shutter to
    //fire instantly
    pulses[1] = 7300;
  } 
  else {
    //set delay between pulses to 5.3 ms which will cause the shutter to
    //fire after 2 seconds delay
    pulses[1] = 5300;
  }

  TCNT0 = 0;
  time = microseconds();
  sbi(TCCR2A,COM2A0) ;   // connect pulse clock
  for(int i=0;i<count;i++)
  {
    time = time + pulses[i];
    while(microseconds()<time){
    };
    if(toggle)
    {
      sbi(TCCR2A,COM2A0) ;   // connect pulse clock
    }
    else
    {
      cbi(TCCR2A,COM2A0) ;   // disconnect pulse clock
    }
    toggle = !toggle;
  }
  toggle = LOW; 
}

//#########################################################

// return TIMER2 TOP value per given desired frequency (Hz)
uint8_t timer2top(unsigned int freq)
{
  return((byte)((unsigned long)SYSCLOCK/2/freq) - 1) ;
}

void setup_timer2(unsigned int freq)
{
  cbi(TCCR2A,COM2A1); // disconnect OC2A for now (COM2A0 = 0)
  cbi(TCCR2A,COM2A0);

  cbi(TCCR2B,WGM22);  // CTC mode for TIMER2
  sbi(TCCR2A,WGM21);
  cbi(TCCR2A,WGM20);

  TCNT2 = 0;

  cbi(ASSR,AS2);  // use system clock for timer 2

    OCR2A = timer2top(freq);

  cbi(TCCR2B,CS22);  // TIMER2 prescale = 1
  cbi(TCCR2B,CS21);
  sbi(TCCR2B,CS20);

  cbi(TCCR2B,FOC2A);  // clear forced output compare bits
  cbi(TCCR2B,FOC2B);
}

unsigned long microseconds()
{
  return (timer0_millis * 1000 + 4*TCNT0);
}
