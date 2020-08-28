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
const String currVersion = "v20200825";

//
// fade delay in main loop
const int delayFadingLoop = 32;   // 32 millisec.

//
// pin layout
const int mosfetPin = 9;          // PWM pin

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

        Serial.print("\nfading TOP\n");
        ledValue_ = ledMaxValue_;
        fadeRising_ = false;
      }
    } else {
      
      fadeOut();
  
      if(ledValue_ <= ledMinValue_) {

        Serial.print("\nfading BOTTOM\n");
        
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

 const int ledMaxValue_ = 200;
 const int ledMinValue_ = 0;

};


//
// global var

//
// current LED dimmer values (0 off; 255 on at max intensity)
int mosfetValue = 0;

CapacitiveSensor   cs_4_2 = CapacitiveSensor(4,2);   // 1M resistor between pins 4 & 2, pin 2 is sensor pin,
                                                     // add a wire and or foil if desired
//
// capacitive sensor threshold (value greater than this trigger the state to pressed)
const int sensorThreshold = 450;

Led ledObj;

int counterReading = 0;

void setup() {

  //
  // init console
  Serial.begin( 9600 );
  print_debug( "led dimmer app starting ..." );
  print_debug( currVersion );

  //
  // capacitor sensor
  print_debug( "init capacity sensor lib ..." );
  cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example

  //
  // fading LED to 75%
  print_debug( "initial fading led to 50 % ..." );
  ledObj.fadeInToTargetValue(200*.5);

  print_debug( "app ready ..." );
}

void loop() {

    long loopStart = millis();

    //
    // fading enabled

    static long sensorVal = 0;

    counterReading = counterReading +1;
    
    if(counterReading >= 10) {
      counterReading = 0;
      long sensorStart = millis();
      sensorVal =  cs_4_2.capacitiveSensor(30);

      long currentTime = millis();
      Serial.print(currentTime - sensorStart);  // check on performance in milliseconds
      Serial.print("\t");                    // tab character for debug windown spacing

      Serial.print(sensorVal);               // print sensor output 1
      Serial.print("\t\n");
    }

    if( sensorVal > sensorThreshold /* button Pressed */ ) {

      ledObj.fadeInOut();
    }

    //
    // delay for fading effect
    long afterAdjust = millis();
    if(afterAdjust - loopStart < delayFadingLoop ) {

      long actualDelay = delayFadingLoop - (afterAdjust - loopStart);
      delay(actualDelay);
    }
}
