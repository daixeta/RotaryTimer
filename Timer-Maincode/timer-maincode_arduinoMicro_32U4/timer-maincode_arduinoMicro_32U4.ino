#include <Arduino.h>
#include <TM1637Display.h>

#define ENC_MIN_DT (5)
#define ENC_MIN_CLK (1) // Interrupt, 11
#define encMinSW (4)

#define encSecDT (6)  //digital IN
#define encSecCLK (0) // Interrupt
#define encSecSW (8)  //digital IN

#define warnLED (22)  // digital OUT
#define actLED (23)   // digital OUT

#define displayCLK (11)  //PWM OUT
#define displayDIO (10)  //digital OUT

#define LedOnTime (100)
unsigned int prevEncMinCLK,
			 prevEncSecCLK;
unsigned long previousMillis = 0;
unsigned long last_timerGo_debounce_time;
const long interval = 1000;         //interval = 1 second
unsigned long totalSecs;
int actualMin,
			 actualSec;

bool timerGo = 0;

int encPosMin = 0;
int prevEncPosMin = 0;
int encPosSec = 0;
int prevEncPosSec = 0;
bool timerGoDetect = 0;
bool resetDetect = 0;

unsigned long lastBlink = 0;
int blinkInterval;
bool blinkGo;

int displayTime = 0;

TM1637Display display(displayCLK, displayDIO);

void setup()
{
  pinMode(ENC_MIN_DT, INPUT_PULLUP);
  pinMode(ENC_MIN_CLK, INPUT_PULLUP);
  pinMode(encMinSW, INPUT_PULLUP);
  pinMode(encSecDT, INPUT_PULLUP);
  pinMode(encSecCLK, INPUT_PULLUP);
  pinMode(encSecSW, INPUT_PULLUP);
  pinMode(warnLED, OUTPUT);
  pinMode(actLED, OUTPUT);
  pinMode(displayCLK, OUTPUT);
  pinMode(displayDIO, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(ENC_MIN_CLK), doEncoderMin, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encSecCLK), doEncoderSec, CHANGE);

  display.setBrightness(0x08);

	actualMin = actualSec = 0;

  prevEncMinCLK = digitalRead(ENC_MIN_CLK);
  prevEncSecCLK = digitalRead(encSecCLK);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void loop(){
  
  unsigned long currentMillis = millis();
  unsigned long lastDebounceTimerGo;

  resetDetect = digitalRead(encMinSW);                         // reset doesn't need debouncing, it's just assuring it reset
  if(!resetDetect){
    actualSec = 0;
    actualMin = 0;
    timerGo = 0; 
    resetDetect = HIGH;
  }

	if ((currentMillis - lastDebounceTimerGo) > 200){                // debouny bit good
	  timerGoDetect = digitalRead(encSecSW);
	  if(!timerGoDetect){
	    timerGo ^= true;
	    timerGoDetect = HIGH;
	  }
	}
  lastDebounceTimerGo = currentMillis;                         //reset debouncy bit

 totalSecs = actualSec + (actualMin * 60);

  if(timerGo && (totalSecs > 0)) {                          //this is the actual timer that counts down seconds (actualSec)
    if (currentMillis - previousMillis >= interval){
      previousMillis = currentMillis;
      actualSec--;
      }
    if(actualSec < 0){                  //dis important, actually counts 01-00-59 now
      actualSec = 59;
      actualMin--;
      }
  }
   digitalWrite(actLED, timerGo);                          //actual "Timer Active" LED being set when timer is on

  if((actualSec <= 0) && (actualMin <= 0)){                 //stops timer if time = 0 and prevents neg times
    timerGo = 0;
    actualSec = 0;
    actualMin = 0;
  }
  
  //all the actualMin and actualSec modification below here

  if(encPosMin > prevEncPosMin){                    			 //detect CW/CCW rotation --> increase/decrease minutes
  	actualMin ++;
  } else if((encPosMin < prevEncPosMin) && (actualMin > 0)){   //blocks manually decreasing into negative numbers
  	actualMin --;
  }

  if((encPosSec < prevEncPosSec) && (actualSec > 0)){    	 //count seconds down manually by 5
  	actualSec -= 5;

	  if(totalSecs <= 4){			 //prevent error when counting seconds down from below 5
	  	actualSec = 0;
	  	actualMin = 0;
	  	timerGo = 0;
	  }
  }

  if (encPosSec > prevEncPosSec)	//all cases for increasing seconds
  {
	  if(actualSec < 60){   			 //count seconds up manually by 5
	  	actualSec += 5;
	  } else {										//prevents actualSec overflow
	    actualSec = 0;
	    actualMin++;
	  }
  }

  if((actualSec == 0) && (actualMin >= 1) && (encPosSec < prevEncPosSec)){    //enables counting down through minutes with seconds encoder
    actualSec = 55;
    actualMin--;
  }
  
  prevEncPosMin = encPosMin;                                          //these two lines reset the encoder up/down counter
  prevEncPosSec = encPosSec;

  //blinkhandler below here

  if (timerGo)
  {
  	blinkGo = 1;
  	switch(totalSecs){
  		case 120 ... 240:
  			blinkInterval = 4000;
  		break;

  		case 60 ... 119:
				blinkInterval = 2000;
  		break;

  		case 30 ... 59:
				blinkInterval = 1000;
  		break;

  		case 0 ... 29:
				blinkInterval = 200;
  		break;
  		
  		default: 
  			blinkGo = 0;
  			blinkInterval = 0;
  	}

  	if (blinkGo && ((currentMillis - lastBlink) >= blinkInterval))				//detect if its time to blink
  	{
	    lastBlink = currentMillis;
	    digitalWrite(warnLED, HIGH);                          //actual lightshow is "on" here
	    
  	}
    if ((currentMillis - lastBlink) >= LedOnTime)
    {
      digitalWrite(warnLED, LOW);                          //actual lightshow is "off" here
    }
  } else {
  	blinkGo = 0;
  	digitalWrite(warnLED, LOW);																		//reset blinky stuff if timer is stopped
	}
  // blinkHandler end
  
  // output
  displayTime = (actualMin * 100) + actualSec;                   //this value is needed to be shown on display
                                                                 // makes mm:ss into mmss (12:35 -> 1235)
  display.showNumberDecEx(displayTime, 0b01000000, true, 4, 0);
}         //// END OF MAIN LOOP

/////////////////////////////////////////////////////////////////////////////////////////////////

/// ISR doEncoderMin
void doEncoderMin(){
  doEncoder(ENC_MIN_CLK, ENC_MIN_DT, &prevEncMinCLK, &encPosMin);
}

/// ISR doEncoderSec
void doEncoderSec(){
	doEncoder(encSecCLK, encSecDT, &prevEncSecCLK, &encPosSec);
}

/// doEncoder for all encoders
void doEncoder(const int clk_pin, const int data_pin, int *prevEncCLK, int *encPos){
	const bool actualCLK = digitalRead(clk_pin);
	const bool actualDT = digitalRead(data_pin);
	static unsigned long last_interrupt_time = 0;
	unsigned long interrupt_time = millis();

	if (interrupt_time - last_interrupt_time > 50){
		if (actualCLK && !(*prevEncCLK)){
			*encPos += (actualDT)? -1 : 1;
		}

		if (!actualCLK && (*prevEncCLK)){
			*encPos += (actualDT)? 1 : -1;
		}
	}
	*prevEncCLK = actualCLK;
	last_interrupt_time = interrupt_time;
}