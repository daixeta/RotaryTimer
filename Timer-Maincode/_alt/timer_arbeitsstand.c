
#define encMinDT 24
#define encMinCLK 20 // Interrupt
#define encMinSW 19  // Interrupt

#define encSecDT 22
#define encSecCLK 21 // Interrupt
#define encSecSW 18  // Interrupt

#define warnLED 13
//#define actLED xx


int prevEncMinDT;
int prevEncMinCLK;
int prevEncMinSW;
int prevEncSecDT;
int prevEncSecCLK;
int prevEncSecSW;
unsigned long previousMillis = 0;
const long interval = 1000;         //interval = 1 second
int actualMin = 0;
int actualSec = 0;
int blinktimes = 0;
bool timerGo = 0;
bool reset = 0;

int encPosMin = 0;
int prevEncPosMin = 0;
int encPosSec = 0;
int prevEncPosSec = 0;

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
  attachInterrupt(digitalPinToInterrupt(encMinSW), doReset, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encSecSW), doStopGo, CHANGE);
  prevEncMinDT = digitalRead(encMinDT);
  prevEncMinCLK = digitalRead(encMinCLK);
  prevEncMinSW = digitalRead(encMinSW);
  prevEncSecDT = digitalRead(encSecDT);
  prevEncSecCLK = digitalRead(encSecCLK);
  prevEncSecSW = digitalRead(encSecSW);
  pinMode(warnLED, OUTPUT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
  unsigned long currentMillis = millis();

  if(timerGo == 1){
  Serial.print("timerGo received");
  }
  
  // millis-counter
  
  if(timerGo == 1 && actualSec > 0) {
    if (currentMillis - previousMillis >= interval){
      previousMillis = currentMillis;
      actualSec--;
      }
    if(actualSec == 0 && actualMin > 0){
      actualSec = 59;
      actualMin--;
      }
      else if(actualSec == 0 && actualMin == 0){
      timerGo = 0;
      }
   Serial.print("Timer Active");
   //digitalWrite(actLED, HIGH);
  }
  else{
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
  
  /*
  if(actualMin <= 0 && actualSec <= 0){       //resets timerEnable to 0 if 00:00 is reached
    timerGo = 0;
    actualSec = 0;
    actualMin = 0;
  }
    */
  Serial.print(" ");
  Serial.print(actualMin);
  Serial.print(" : ");
  Serial.print(actualSec);
  Serial.print("   timerGo = ");
  Serial.print(timerGo);
  Serial.println();
  delay(100);
}

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



/// ISR doReset
void doReset(){
  int actualReset;
  int prevReset;
  actualReset = digitalRead(encMinSW);
  if(actualReset != prevReset){
  actualMin = 0;
  actualSec = 0;
  //Serial.println("Timer Reset");
  timerGo = 0;
  }
}


/// ISR doStopGo
void doStopGo(){
  int actualTimerGoDetect;
  int prevTimerGoDetect;
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
 if (interrupt_time - last_interrupt_time > 20){
    actualTimerGoDetect = digitalRead(encSecSW);
      if(actualTimerGoDetect != prevTimerGoDetect){               //detects if actualtimerGo has changed from prevTimerGo
         timerGo = !timerGo;                                      //inverts timerGo (startup default is negative, so makes positive)
         }
    Serial.print(" timerGo triggered ");
    Serial.print("   timerGo = ");
    Serial.print(timerGo);
 }
 prevTimerGoDetect = actualTimerGoDetect;                         //stores actual as prev
 last_interrupt_time = interrupt_time;
}


/// Subroutine blinkhandler
/*
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
      */
