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
#include "CapacitiveSensor.h"

//
// enable debug printout
#define DEBUG 1

#ifdef DEBUG
//
// enable debug print
// note: a delay of 200 msec is used to allow the console
//       to finish the output of message
#define print_debug( X ) do {   \
    Serial.println( X ); \
    delay(1);          \
  } while(0)
#else
//
// disable debug print
#define print_debug( X )

#endif

//
// program version
const String currVersion = "v20200821";

const int delayFadingLoop = 24;   // 12 millisec.

//
// pin layout
const int mosfetPin = 9;          // PWM pin

//
// LED manager class
class Led {

public:

  Led() : ledValue_(0), fadeRising_(true) {
    
    analogWrite(mosfetPin, ledValue_);
    return;
  };

  Led(const int initialLedValue) : ledValue_(initialLedValue), fadeRising_(true) {
  
    analogWrite(mosfetPin, ledValue_);
    return;
  };

  void setLedValue(const int initialLedValue) {
  
    ledValue_ = initialLedValue;

    if(ledValue_ >= ledMaxValue_) {
        
        ledValue_ = ledMaxValue_;
  
    } else if(ledValue_ <= ledMinValue_) {
        
        ledValue_ = ledMinValue_;
    }

    analogWrite(mosfetPin, ledValue_);
    return;
  };

  void fadeInOut() {
    
    if(fadeRising_) {
      
      fadeIn();

      if(ledValue_ >= ledMaxValue_) {
        
        ledValue_ = ledMaxValue_;
        fadeRising_ = false;
      }
  
    } else {
      
      fadeOut();
  
      if(ledValue_ <= ledMinValue_) {
        
        ledValue_ = ledMinValue_;
        fadeRising_ = true;
      }
    }

    return;
  }

  void fadeIn() {

    ledValue_ = ledValue_ +1;
    analogWrite(mosfetPin, ledValue_);
    return;
  }

  void fadeOut() {

    ledValue_ = ledValue_ -1;
    analogWrite(mosfetPin, ledValue_);
    return;
  }

protected:

 int ledValue_ = 0;

 bool fadeRising_ = true;

 const int ledMaxValue_ = 255;
 const int ledMinValue_ = 0;

};


//
// global var
int mosfetValue = 0;

long appStart = 0;

CapacitiveSensor   cs_4_2 = CapacitiveSensor(4,2);   // 1M resistor between pins 4 & 2, pin 2 is sensor pin,
                                                     // add a wire and or foil if desired
const int sensorThreshold = 40;

Led ledObj;

int couterReading = 0;

void setup() {

  //
  // init console
  Serial.begin( 9600 );
  print_debug( "led dimmer app starting ..." );
  print_debug( currVersion );

  // init app startup time
  appStart = millis();

  //
  // mosfetPin: initialize as an output.
  print_debug( "set mosfet pin output ..." );
  print_debug( mosfetPin );
  pinMode( mosfetPin, OUTPUT);

  analogWrite( mosfetPin, mosfetValue);

  //
  // capacitor sensor
  print_debug( "init capacity sensor lib ..." );
  cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example

  print_debug( "app ready ..." );
}

void loop() {

    long sensorStart = millis();

    //
    // fading enabled

    static long sensorVal = 0;

    couterReading = couterReading +1;
    
    if(couterReading >= 20) {
      couterReading = 0;
      long sensorStart2 = millis();
      sensorVal =  cs_4_2.capacitiveSensor(30);

      long currentTime = millis();
      Serial.print(currentTime - sensorStart2);  // check on performance in milliseconds
      Serial.print("\t");                    // tab character for debug windown spacing

      Serial.print(sensorVal);               // print sensor output 1
      Serial.print("\t\n");
    }

    if( sensorVal > sensorThreshold /* button Pressed */ ) {

      ledObj.fadeInOut();

      appStart = sensorStart;
    }

    //delay(delayFadingLoop);
    long afterAdjust = millis();
    if(afterAdjust - sensorStart < delayFadingLoop ) {

      long actualDelay = delayFadingLoop - (afterAdjust - sensorStart);
      delay(actualDelay);
    }
}
