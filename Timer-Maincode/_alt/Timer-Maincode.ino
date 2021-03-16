
#define encMinDT 24
#define encMinCLK 20 // Interrupt
#define encMinSW 19

#define encSecDT 22
#define encSecCLK 21 // Interrupt
#define encSecSW 18

#define warnLED 13
//#define actLED xx

int prevEncMinDT;
int prevEncMinCLK;
int prevEncMinSW;
int prevEncSecDT;
int prevEncSecCLK;
int prevEncSecSW;
unsigned long previousMillis = 0;
unsigned long last_timerGo_debounce_time;
unsigned long last_Reset_debounce_time;  
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

void setup()
{
  Serial.begin(9600);
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


}

/////////////////////////////////////////////////////////////////////////////////////////////////
void loop(){
  
  unsigned long currentMillis = millis();
  unsigned long totalSecs;

  resetDetect = digitalRead(encMinSW);
  if(resetDetect == LOW){
    actualSec = 0;
    actualMin = 0;
    timerGo = 0; 
    resetDetect = HIGH;
  }


  timerGoDetect = digitalRead(encSecSW);                // ADD DEBOUNCE
  if(timerGoDetect == LOW){
    timerGo = !timerGo;
    timerGoDetect = HIGH;
  }

 totalSecs = actualSec + (actualMin * 60);

  if(timerGo == 1 && totalSecs > 0) {
    if (currentMillis - previousMillis >= interval){
      previousMillis = currentMillis;
      actualSec--;
      }
    if(timerGo == 1 && actualSec < 0){                  // dis important, actually counts 01-00-59 now
      actualSec = 59;
      actualMin--;
      }
   Serial.print("Timer Active");
   //digitalWrite(actLED, HIGH);
  }

  if(actualSec <= 0 && actualMin <= 0){                                       // stops timer if time = 0 and prevents neg times
    timerGo = 0;
    actualSec = 0;
    actualMin = 0;
    }

  
  if(timerGo == 0){
    Serial.print("Timer INACTIVE");
    //digitalWrite(actLED, LOW);
  }
  
  //all the actualMin and actualSec modification below here

  if(encPosMin > prevEncPosMin){                    // detect CW/CCW rotation --> increase/decrease times (actualMin, actualSec)
  actualMin ++;
  }
  if(encPosMin < prevEncPosMin && actualMin > 0){   //blocks manually decreasing into negative numbers
  actualMin --;
  }
  if(encPosSec > prevEncPosSec){                    //each step of the encoder increases/decreases Seconds by 5
  actualSec = actualSec + 5;
  }
  if(encPosSec < prevEncPosSec && actualSec > 0){
  actualSec = actualSec - 5;
  }

  // all the Resets happen below this line
  
  prevEncPosMin = encPosMin;
  prevEncPosSec = encPosSec;

  Serial.print(" ");
  Serial.print(actualMin);
  Serial.print(" : ");
  Serial.print(actualSec);
  Serial.println();
  delay(100);
}         //// END OF MAIN LOOP

/////////////////////////////////////////////////////////////////////////////////////////////////

/// ISR doEncoderMin
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



/// ISR doEncoderSec
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


/// Subroutine blinkhandler

int blinkhandler(){
  unsigned long curBlinkMillis = millis();
  unsigned long prevBlinkMillis = 0;
  int blinkIntervalShort = 100;
  int blinkIntervalLong = 1000;
  int blinkTimes = 0;
  bool warnLEDState = 0;
  int n = 0;
  
    if (curBlinkMillis - prevBlinkMillis >= blinkIntervalLong){
    prevBlinkMillis = curBlinkMillis;
      if(actualMin < 4){
          blinkTimes = 1;
            while(n < blinkTimes){
              if (curBlinkMillis - prevBlinkMillis >= blinkIntervalShort){
                    prevBlinkMillis = curBlinkMillis;
                    warnLEDState = !warnLEDState;
                    digitalWrite(warnLED, warnLEDState);
                    n++;
                    }}}
                    
      if(actualMin < 2){
      blinkTimes = 2;
      while(n < blinkTimes){
              if (curBlinkMillis - prevBlinkMillis >= blinkIntervalShort){
                    prevBlinkMillis = curBlinkMillis;
                    warnLEDState = !warnLEDState;
                    digitalWrite(warnLED, warnLEDState);
                    n++;
                    }}}
                    
      if(actualMin < 1){
      blinkTimes = 3;
      while(n < blinkTimes){
              if (curBlinkMillis - prevBlinkMillis >= blinkIntervalShort){
                    prevBlinkMillis = curBlinkMillis;
                    warnLEDState = !warnLEDState;
                    digitalWrite(warnLED, warnLEDState);
                    n++;
                    }}}
                    
      if(actualMin == 0 && actualSec < 30){                                    //30s Warnblinker permanent
            if (curBlinkMillis - prevBlinkMillis >= blinkIntervalShort){
                    prevBlinkMillis = curBlinkMillis;
                    warnLEDState = !warnLEDState;
                    digitalWrite(warnLED, warnLEDState);
                    n++;
                    }}
      else{
        warnLEDState = 0;
      }}
      return;
      }
      // */
