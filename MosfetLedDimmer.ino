/* 
 * LED dimmer source code
 *
 *  author: roby
 *  ver. : 1.0
 *  date: 2020 08 01
*/

//
// sleep / powerdown header
#include "LowPower.h"

//
// capacitive sensor header
#include "ADCTouch.h"

//
// enable debug printout
#define DEBUG 1

#ifdef DEBUG
//
// enable debug print
// note: a delay of 1 msec. is used to allow the console
//       to finish the output of message
#define print_debug( X ) do {   \
    Serial.println( X ); \
    delay(1);          \
  } while(0)

#define print_debugnln( X ) do {   \
    Serial.print( X ); \
    delay(1);          \
  } while(0)
  
#else
//
// disable debug print
#define print_debug( X )

#endif

//
// program version
const String currVersion = "v20201005";

//
// fade delay in main loop
const int delayFadingLoop = 25;   // 25 millisec
const int delayMainLoop = 1000;   // 1000 msec (i.e. 1 sec)

//
// pin layout: to mosfet gate, PWM capable pin
const int mosfetPin = 9;

//
// pin layout: capacity sensor A0

//
// capacitive sensor threshold (value greater than this trigger the state to pressed)
const int sensorThreshold = 40;

//
//
const int pmwMaxValue = 200;  // approx. 80% of max value

const int ledFadeValue01 = (pmwMaxValue * 0.90);  // 90 %
const int ledFadeValue02 = (pmwMaxValue * 0.35);  // 35 %
const int ledFadeValue03 = (pmwMaxValue * 0.10);  // 10 %

//
// LED manager class
class Led {

public:

  Led() : ledCurrentValue_(0), fadeRising_(true) {

    //
    // mosfetPin: initialize as an output and write initial value
    pinMode( mosfetPin, OUTPUT);
    analogWrite(mosfetPin, ledCurrentValue_);
  
    return;
  };
  
  Led(const Led&) = delete;

  Led operator=(const Led&) = delete;

  Led(const int initialLedValue, const bool rising) : ledCurrentValue_(initialLedValue), fadeRising_(rising) {
    //
    // mosfetPin: initialize as an output and write initial value
    pinMode( mosfetPin, OUTPUT);
    analogWrite(mosfetPin, ledCurrentValue_);
   
    return;
  };

  void setLedValue(const int initialLedValue) {
  
    ledCurrentValue_ = initialLedValue;

    if(ledCurrentValue_ >= ledMaxValue_) {
        
        ledCurrentValue_ = ledMaxValue_;
  
    } else if(ledCurrentValue_ <= ledMinValue_) {
        
        ledCurrentValue_ = ledMinValue_;
    }

    analogWrite(mosfetPin, ledCurrentValue_);
    return;
  };

  void fadeInOut() {
    
    if(fadeRising_) {
      
      increaseDim();

      if(ledCurrentValue_ >= ledMaxValue_) {

        print_debug("\nfading TOP\n");
        ledCurrentValue_ = ledMaxValue_;
        fadeRising_ = false;
        delay(500); // keep top for 1/2 sec
      }
    } else {
      
      decreaseDim();
  
      if(ledCurrentValue_ <= ledMinValue_) {

        print_debug("\nfading BOTTOM\n");
        ledCurrentValue_ = ledMinValue_;
        fadeRising_ = true;
        delay(500); // keep bottom for 1/2 sec
      }
    }

    return;
  }

  void increaseDim() {

    ledCurrentValue_ = ledCurrentValue_ +1;
    analogWrite(mosfetPin, ledCurrentValue_);
    return;
  }

  void decreaseDim() {

    ledCurrentValue_ = ledCurrentValue_ -1;
    analogWrite(mosfetPin, ledCurrentValue_);
    return;
  }

  void fadeInToTargetValue(const int targetValue) {

    while(ledCurrentValue_ < targetValue) {
      increaseDim();
      if(ledCurrentValue_ < targetValue / 3) {
        delay(delayFadingLoop * 1.5); // 50 % slower
      } else {
        delay(delayFadingLoop * 0.5); // 50 % faster
      }
    }
    return;
  }

  void fadeOutToTargetValue(const int targetValue) {

    while(ledCurrentValue_ > targetValue) {
      decreaseDim();
      delay(delayFadingLoop);
    }
    return;
  }

protected:

 int ledCurrentValue_ = 0;

 bool fadeRising_ = true;

 const int ledMaxValue_ = pmwMaxValue;
 const int ledMinValue_ = 0;

};

//
// global var

Led ledObj;

int counterReading = 0;

int sensorRef0 = 0;     //reference values to remove offset

long timeAppStart = 0;

void suspendDevice(const period_t sleepPeriod, const int periodCount) {
    //
    // go into long low-power mode
    print_debug( "low-power mode ..." );

    for(int i = 0; i < periodCount; i++) {
      //
      // enter power down state with ADC and BOD module disabled
      LowPower.powerDown(sleepPeriod, ADC_OFF, BOD_OFF);
    }
    delay(200);

    print_debug( "resuming from low-power mode ..." );

  return ;
}

const int micPowerPin = 7;
const unsigned int micThreashold = 100;

bool detectSound()
{
   // power on mic
   //digitalWrite(micPowerPin, HIGH);
   //delay(20); // wait for mic to turn on

   unsigned long start= millis();  // Start of sample window
   

   unsigned int signalMax = 0;
   unsigned int signalMin = 1024;
   const int sampleWindow = 250; // Sample window width in mS (250 mS = 4Hz)

   // collect data for 250 miliseconds
   while (millis() - start < sampleWindow) {

       unsigned int knock = analogRead(0);

       if (knock > signalMax) {

           signalMax = knock;  // save just the max levels

       } else if (knock < signalMin) {

           signalMin = knock;  // save just the min levels
       }
   }
 
   const unsigned int peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
   //double volts = (peakToPeak * 3.3) / 1024;  // convert to volts

   Serial.print("max :");
   Serial.println(signalMax);
   Serial.print("min :");
   Serial.println(signalMin);
   Serial.print("delta :");
   Serial.println(peakToPeak);
   Serial.println("----");

   return ( peakToPeak > micThreashold ? true : false );
}
void setup() {

  //
  // init console
  Serial.begin( 9600 );
  print_debugnln( "led dimmer app starting " );
  print_debug( currVersion );

  //
  // fading-in LED from 0 to ledFadeValue01
  print_debugnln( "PMW pin out to mosfet: " );
  print_debug( mosfetPin );
  print_debugnln( "PMW min / max values: 0 / " );
  print_debug( pmwMaxValue );
  print_debugnln( "initial fading led to: " );
  print_debug(ledFadeValue01);
  ledObj.fadeInToTargetValue(ledFadeValue01);
  print_debugnln( "fading value idle period: " );
  print_debug( ledFadeValue03 );

  print_debugnln( "mic power pin :" );
  print_debug(micPowerPin);
  pinMode(micPowerPin, OUTPUT);
  print_debugnln( "mic power threshold: " );
  print_debug(micThreashold);

  
//  print_debugnln( "Fading value initial period: " );
//  print_debug( ledFadeValue01 );
//
//  print_debugnln( "Fading value 2nd period: " );
//  print_debug( ledFadeValue02 );
//
//  print_debugnln( "Fading value 3nd period: " );
//  print_debug( ledFadeValue03 );

//  print_debugnln( "period 1: " );
//  print_debug( timeExpire01 );
//
//  print_debugnln( "period 2: " );
//  print_debug( timeExpire02 );
//
//  print_debugnln( "period 3: " );
//  print_debug( timeExpire03 );

  //
  // fading delay
//  print_debugnln( "Fading delay ms: " );
//  print_debug( delayFadingLoop );

  //
  // capacity sensor threshold
  //print_debugnln( "capacity sensor threshold number: " );
  //print_debug( sensorThreshold );
  //print_debug( "capacity sensor pin: A0" );

  //
  // capacitor sensor
  //print_debug( "create reference values to account for the capacitance of the pad ..." );
  //sensorRef0 = ADCTouchClass::read(A0, 500);    //create reference values to account for the capacitance of the pad

  //print_debugnln( "capacitance reference: " );
  //print_debug(sensorRef0);

  //timeAppStart = millis();

  print_debug( "app ready ..." );
}

void loop() {

  static long timeElapsedSinceEpoch = 0;
  const long timeExpire01 = 10; //* 60; // 35 min

  //
  // avoid counter overflow
  if(timeElapsedSinceEpoch < timeExpire01 +1000) {
    timeElapsedSinceEpoch += 1;      
  }

  if(timeElapsedSinceEpoch > timeExpire01) {

    // power on mic
    digitalWrite(micPowerPin, HIGH);
    delay(250); // wait for power up mic (around 1,5 sec.)

    print_debug( "fade-out led to idle state ..." );
    ledObj.fadeOutToTargetValue(ledFadeValue03);

    //
    // after entering LED in idle state, begin loop
    // for presence detection
    while( ! detectSound() ){
      delay(250);
    }

    //
    // presence detected: power off mic and fade-in LED
    digitalWrite(micPowerPin, LOW);
    print_debugnln( "fading in led to active state ..." );
    ledObj.fadeInToTargetValue(ledFadeValue01);
    timeElapsedSinceEpoch = 0; // restart active LED period
  }

  delay(1000);
}
