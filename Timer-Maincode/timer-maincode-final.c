#include <Arduino.h>
#include <TM1637Display.h>

#define encMinDT 24
#define encMinCLK 20 // Interrupt
#define encMinSW 19

#define encSecDT 22
#define encSecCLK 21 // Interrupt
#define encSecSW 18

#define warnLED 13
#define actLED 14

#define displayCLK 4
#define displayDIO 5

int prevEncMinDT;
int prevEncMinCLK;
int prevEncMinSW;
int prevEncSecDT;
int prevEncSecCLK;
int prevEncSecSW;
unsigned long previousMillis = 0;
unsigned long last_timerGo_debounce_time;
const long interval = 1000;         //interval = 1 second
int actualMin = 0;
int actualSec = 0;
int blinktimes = 0;
bool timerGo = 0;

int encPosMin = 0;
int prevEncPosMin = 0;
int encPosSec = 0;
int prevEncPosSec = 0;
bool timerGoDetect = 0;
bool prevTimerGoDetect = 0;
bool resetDetect = 0;
bool prevResetDetect = 0;

unsigned long prevBlinkMillis = 0;
unsigned long lastBlink = 0;
int blinkInterval;
bool blinkGo;
int n = 0;
bool warnLEDState = 0;

int displayTime = 0;
TM1637Display display(displayCLK, displayDIO);

void setup()
{
  //Serial.begin(9600);                     //activate here to enable serial output
  pinMode(encMinDT, INPUT_PULLUP);
  pinMode(encMinCLK, INPUT_PULLUP); 
  pinMode(encMinSW, INPUT_PULLUP);
  pinMode(encSecDT, INPUT_PULLUP);
  pinMode(encSecCLK, INPUT_PULLUP); 
  pinMode(encSecSW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encMinCLK), doEncoderMin, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encSecCLK), doEncoderSec, CHANGE);
  prevEncMinDT = digitalRead(encMinDT);
  prevEncMinCLK = digitalRead(encMinCLK);
  prevEncMinSW = digitalRead(encMinSW);
  prevEncSecDT = digitalRead(encSecDT);
  prevEncSecCLK = digitalRead(encSecCLK);
  prevEncSecSW = digitalRead(encSecSW);
  pinMode(warnLED, OUTPUT);
  pinMode(actLED, OUTPUT);
  pinMode(displayCLK, OUTPUT);
  pinMode(displayDIO, OUTPUT);
  
  display.setBrightness(0x08);


}

/////////////////////////////////////////////////////////////////////////////////////////////////
void loop(){
  
  unsigned long currentMillis = millis();
  unsigned long totalSecs;
  unsigned long lastDebounceTimerGo;

  resetDetect = digitalRead(encMinSW);                         // reset doesn't need debouncing, it's just assuring it reset
  if(resetDetect == LOW){
    actualSec = 0;
    actualMin = 0;
    timerGo = 0; 
    resetDetect = HIGH;
  }

if (currentMillis - lastDebounceTimerGo > 200){                // debouny bit good
  timerGoDetect = digitalRead(encSecSW);
  if(timerGoDetect == LOW){
    timerGo = !timerGo;
    timerGoDetect = HIGH;
  }
}
  lastDebounceTimerGo = currentMillis;                         //reset debouncy bit

 totalSecs = actualSec + (actualMin * 60);

  if(timerGo == 1 && totalSecs > 0) {                          //this is the actual timer that counts down seconds (actualSec)
    if (currentMillis - previousMillis >= interval){
      previousMillis = currentMillis;
      actualSec--;
      }
    if(timerGo == 1 && actualSec < 0){                  //dis important, actually counts 01-00-59 now
      actualSec = 59;
      actualMin--;
      }
   //Serial.print("Timer Active");                        //debug thing
   digitalWrite(actLED, HIGH);                          //actual "Timer Active" LED being set when timer is on
  }

  if(actualSec <= 0 && actualMin <= 0){                 //stops timer if time = 0 and prevents neg times
    timerGo = 0;
    actualSec = 0;
    actualMin = 0;
    }
  
  if(timerGo == 0){
    //Serial.print("Timer INACTIVE");                     //debug thing
    digitalWrite(actLED, LOW);
  }
  
  //all the actualMin and actualSec modification below here

  if(encPosMin > prevEncPosMin){                    //detect CW/CCW rotation --> increase/decrease times (actualMin, actualSec)
  actualMin ++;
  }
  if(encPosMin < prevEncPosMin && actualMin > 0){   //blocks manually decreasing into negative numbers
  actualMin --;
  }
  if(encPosSec > prevEncPosSec && actualSec < 60){   //each step of the encoder increases/decreases Seconds by 5
  actualSec = actualSec + 5;
  }
  if(actualSec >=60 && encPosSec > prevEncPosSec){   //prevents actualSec overflow
    actualSec = 0;
    actualMin++;
  }
  if(actualSec == 0 && actualMin >=1 && encPosSec < prevEncPosSec){    //enables counting down through minutes with seconds encoder
    actualSec = 60;
    actualMin--;
  }
  
  if(encPosSec < prevEncPosSec && actualSec > 0){                     //count seconds down manually
  actualSec = actualSec - 5;
  }  
  prevEncPosMin = encPosMin;                                          //these two lines reset the encoder up/down counter
  prevEncPosSec = encPosSec;


  //blinkhandler below here

      if(actualMin < 4 && actualMin >= 2 && timerGo == 1){            //these bunch of ifs define the blink rythm      
          blinkInterval = 4000;
          warnLEDState = 0;
          blinkGo = 1;
          }
      if(actualMin < 2 && actualMin >= 1 && timerGo == 1){
          blinkInterval = 2000;
          warnLEDState = 0;
          blinkGo = 1;
          }
      if(actualMin < 1 && actualSec >= 30 && timerGo == 1){
          blinkInterval = 1000;
          warnLEDState = 0;
          blinkGo = 1;
          }

      if(actualMin == 0 && actualSec <= 30 && timerGo == 1){
          blinkInterval = 200;
          warnLEDState = 0;
          blinkGo = 1;      
      }

      if(blinkGo == 1 && (currentMillis - lastBlink >= blinkInterval) && timerGo == 1){    //this triggers a blink each 5 seconds
                lastBlink = currentMillis;
                warnLEDState = !warnLEDState;
        }

            
      if(actualMin > 4){                                            //resets all blinky stuff if more than 4 minutes on the clock
        warnLEDState = 0;
        blinkGo = 0;
      }
      
      if(timerGo == 0){                                             //resets all blinky stuff if timer is stopped
        blinkGo = 0;
        warnLEDState = 0;
      }
      
      digitalWrite(warnLED, warnLEDState);                          //actual lightshow is here
      
  // blinkHandler end


  // output

      displayTime = (actualMin * 100) + actualSec;                   //this value is needed to be shown on display
                                                                     // makes mm:ss into mmss (12:35 -> 1235)
      //debugOutput();                                               //activate here to get serial output
      display.showNumberDecEx(displayTime, 0b01000000, true, 4, 0);
      //display.showNumberDecEx(displayTime, 0b11100000, true, 4, 0);  //for display with dots between numbers


  
}         //// END OF MAIN LOOP

/////////////////////////////////////////////////////////////////////////////////////////////////

/// ISR doEncoderMin - I dont dare touch those!
void doEncoderMin(){
  int actualMinCLK = digitalRead(encMinCLK);
  int actualencMinDT = digitalRead(encMinDT);
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
 if (interrupt_time - last_interrupt_time > 50){
  if ((actualMinCLK == 1) and (prevEncMinCLK == 0)){
    if (actualencMinDT == 1)
    encPosMin--;
    else
    encPosMin++;
  }

  if ((actualMinCLK == 0) and (prevEncMinCLK == 1)){
    if (actualencMinDT == 1)
    encPosMin++;
    else
    encPosMin--;
  }
   }
     prevEncMinCLK = actualMinCLK;
  last_interrupt_time = interrupt_time;
}

/// ISR doEncoderSec - I dont dare touch those!
void doEncoderSec(){
  int actualSecCLK = digitalRead(encSecCLK);
  int actualencSecDT = digitalRead(encSecDT);
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
 if (interrupt_time - last_interrupt_time > 50){
  if ((actualSecCLK == 1) and (prevEncSecCLK == 0)){
    if (actualencSecDT == 1)
    encPosSec--;
    else
    encPosSec++;
  }

  if ((actualSecCLK == 0) and (prevEncSecCLK == 1)){
    if (actualencSecDT == 1)
    encPosSec++;
    else
    encPosSec--;
  }
 }
   prevEncSecCLK = actualSecCLK;
  last_interrupt_time = interrupt_time;
}



/// SUBROUTINE DEBUG OUTPUT
void debugOutput(){
  Serial.print("   displayTime = ");
  Serial.print(displayTime);
  Serial.print("   actual Time: ");
  Serial.print(" ");
  Serial.print(actualMin);
  Serial.print(" : ");
  Serial.print(actualSec);
  Serial.println();
  delay(100);
  if(warnLEDState == 1){
    Serial.print("    BLINK   ");
    }
}