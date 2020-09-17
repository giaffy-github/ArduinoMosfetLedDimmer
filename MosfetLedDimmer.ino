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
const String currVersion = "v20200902";

//
// fade delay in main loop
const int delayFadingLoop = 24;   // 24 millisec. used in LED fading in/out

//
// pin layout: to mosfet gate, PWM capable pin
const int mosfetPin = 9;

//
// pin layout: capacity sensor out and int
const int capacitySensorOutPin = 4;
const int capacitySensorInPin = 2;

//
// capacitive sensor threshold (value greater than this trigger the state to pressed)
const int sensorThreshold = 550;

//
//
const int pmwMaxValue = 230;  // approx. 90% of max value

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
      }
    } else {
      
      decreaseDim();
  
      if(ledCurrentValue_ <= ledMinValue_) {

        print_debug("\nfading BOTTOM\n");
        ledCurrentValue_ = ledMinValue_;
        fadeRising_ = true;
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
      delay(delayFadingLoop);
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

 const int ledMaxValue_ = pmwMaxValue;  // approx. 90% of max value
 const int ledMinValue_ = 0;

};


//
// global var

// capacitive sensor ports pair: pin 4 out signal, 2 reading capacitance
CapacitiveSensor   cs_4_2 = CapacitiveSensor(capacitySensorOutPin,capacitySensorInPin);
                          // pin 4 === 1M resistor === foil === 10 K resistor === pin 2
                          // pin 4 out, pin 2 is sensor pin,
                                                     // 100 pF on pin 2 and ground to stabilize reading;

Led ledObj;

int counterReading = 0;

void setup() {

  //
  // init console
  Serial.begin( 9600 );
  print_debug( "led dimmer app starting ..." );
  print_debug( currVersion );

  //
  // fading-in LED from 0 to 75%
  print_debug( "initial fading led to 75 % ..." );
  ledObj.fadeInToTargetValue(pmwMaxValue * 0.75);

  //
  // capacitor sensor
  print_debug( "init capacity sensor lib ..." );
  cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example

  print_debug( "app ready ..." );
}

void loop() {

    long loopStart = millis();

    counterReading = counterReading +1;

    // value returned by capacitive sensor button: needs to be static as
    // reading is done avery 10th iterations to limit the time consumed;
    // last reading is kepr till next reading, and fading take place if required
    static long sensorVal = 0;

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
    // delay for fading effect: keep into account delay to read sensor
    // and try to delay the same time interval
    long afterAdjust = millis();
    if(afterAdjust - loopStart < delayFadingLoop ) {

      long actualDelay = delayFadingLoop - (afterAdjust - loopStart);
      delay(actualDelay);
    }
}
