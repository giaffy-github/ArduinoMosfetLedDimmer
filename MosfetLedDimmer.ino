/*
   LED dimmer source code
   author: roby
   ver. : 1.0
   date: 2020 08 01
*/

//
// sleep / powerdown header
//#include "LowPower.h"

//
// enable debug printout
#define DEBUG 1

#ifdef DEBUG
//
// enable debug print
// note: a delay of 200 msec is used to allow the console
//       to finish the output of message
#define print_debug( X ) do {                      \
    Serial.println( X ); \
    delay(200);          \
  } while(0)
#else
//
// disable debug print
#define print_debug( X )

#endif

//
// program version
const String currVersion = "v20200801";

const int delayFadingLoop = 12;

//
// pin layout
const int mosfetPin = 9;

int mosfetValue = 0;

void setup() {

  //
  // init console
  Serial.begin( 9600 );
  print_debug( "led dimmer app starting ..." );
  print_debug( currVersion );

  //
  // mosfetPin: initialize as an output.
  print_debug( "set mosfet pin output ..." );
  print_debug( mosfetPin );
  pinMode( mosfetPin, OUTPUT);

  analogWrite( mosfetPin, mosfetValue);

  print_debug( "app ready ..." );
}

void loop() {

  print_debug( "led dimming on ..." );
  //
  // turn LED on with fading-in
  for (mosfetValue = 0; mosfetValue < 255; mosfetValue++) {
    analogWrite( mosfetPin, mosfetValue);
    delay(delayFadingLoop);
  }
  print_debug( "led full power on ..." );

  delay(3000);
  
  //
  // turn LED off with fading-out
  print_debug( "led dimming off ..." );
  for (mosfetValue = 255; mosfetValue > 0 ; mosfetValue--) {
    analogWrite( mosfetPin, mosfetValue);
    delay(delayFadingLoop);
  }
  print_debug( "led power off ..." );

  delay(3000);

}
