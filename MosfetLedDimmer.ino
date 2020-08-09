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
const String currVersion = "v20200801";

const int delayFadingLoop = 2;

//
// pin layout
const int mosfetPin = 9;

int mosfetValue = 0;

long appStart = 0;

CapacitiveSensor   cs_4_2 = CapacitiveSensor(4,2);   // 1M resistor between pins 4 & 2, pin 2 is sensor pin,
                                                     // add a wire and or foil if desired

const int sensorThreshold = 500;

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

  if((sensorStart - appStart) > 30000 ) {
    delay(1000);
    print_debug("fading disabled\n");

    //
    // enter low-power mode
    suspendDevice(SLEEP_8S, 1 /* period(s) count */);
    
  } else {

    //print_debug("reading button sensor ...\n");
    long sensorVal =  cs_4_2.capacitiveSensor(30);

    //Serial.print(millis() - sensorStart);  // check on performance in milliseconds
    //Serial.print("\t");                    // tab character for debug windown spacing

    //Serial.print(sensorVal);               // print sensor output 1
    //Serial.print("\t\n");

    if( sensorVal > sensorThreshold /* button Pressed */ ) {

      //print_debug("button pressed\n");
    
      fadeLed(mosfetValue);
  
      if(mosfetValue == 255) {
        delay(400);    
      }
      if(mosfetValue == 0) {
        delay(400);    
      }
    
      appStart = sensorStart;
    }
  
    delay(delayFadingLoop);
  }
  //print_debug( "led dimming on ..." );
  //
  // turn LED on with fading-in
  //for (mosfetValue = 0; mosfetValue < 255; mosfetValue++) {
  //  analogWrite( mosfetPin, mosfetValue);
  //  delay(delayFadingLoop);
  //}
  //print_debug( "led full power on ..." );

  //delay(3000);
  
  //
  // turn LED off with fading-out
  //print_debug( "led dimming off ..." );
  //for (mosfetValue = 255; mosfetValue > 0 ; mosfetValue--) {
  //  analogWrite( mosfetPin, mosfetValue);
  //  delay(delayFadingLoop);
  //}
  //print_debug( "led power off ..." );

  //delay(3000);
}

class Led {
  Led() {};

  void fadeLed() {};
};

void fadeLed(int &value) {

  static bool fadeRising = true;

  const int maxValue = 255;
  const int minValue = 0;

  if(value >= maxValue) {
    value = maxValue;
    fadeRising = false;
    
  } else if(value <= minValue) {
    value = minValue;
    fadeRising = true;
  }
  
  if(fadeRising) {
    //print_debug( "led fade-in ..." );
    analogWrite(mosfetPin, ++value);
    
  } else {
    //print_debug( "led fade-out ..." );
    analogWrite(mosfetPin, --value);
  }

  return;
}

void suspendDevice(const period_t sleepPeriod, const int periodCount) {
    //
    // go into long low-power mode
    print_debug( "low-power mode ..." );

    for(int i = 0; i < periodCount; i++) {
      //
      // low power with timer ON
      //LowPower.idle(SLEEP_8S /*sleepPeriod*/, ADC_OFF, TIMER2_ON, TIMER1_ON, TIMER0_ON, SPI_OFF, USART0_OFF, TWI_OFF);
      LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);
    }
    delay(200);

    print_debug( "resuming from low-power mode ..." );

  return ;
}
