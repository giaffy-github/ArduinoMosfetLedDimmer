/* 
 * LED dimmer source code
 *
 *  author: roby
 *  ver. : 1.0
 *  date: 2020 08 01
*/

//
// sleep / powerdown header

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

#define print_debugnln( X )

#endif

//
// program version
const String currVersion = "v20201108";

//
// fade delay in main loop
const int delayFadingLoop = 25;   // 25 millisec
const int delayMainLoop = 1000;   // 1000 msec (i.e. 1 sec)

//
// pin layout: to mosfet gate, PWM capable pin
const int mosfetPin = 9;

const int pmwMaxValue = 230;  // approx. 90% of max value

const int ledFadeValue01 = (pmwMaxValue * 1.00);  // 80 %
const int ledFadeValue02 = (pmwMaxValue * 0.35);  // 35 %
const int ledFadeValue03 = (pmwMaxValue * 0.00);  // 0 %

//
// LED manager class
class Led {

public:

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

Led ledObj(0, true);

int counterReading = 0;

int sensorRef0 = 0;     //reference values to remove offset

long timeAppStart = 0;

const int micPowerPin = 7;
const int micADPin = 1;
const unsigned int micThreashold = 180;

bool detectSound()
{
   // power on mic
   //digitalWrite(micPowerPin, HIGH);
   //delay(20); // wait for mic to turn on

    int peakCounter = 0;
  
    unsigned int signalMax = 0;
    unsigned int signalMin = 1024;
    const int sampleWindow = 250; // Sample window width in mS (250 ms = 4Hz)

    // collect data for 250 miliseconds
    unsigned long start= millis();  // Start of sample window
    while (millis() - start < sampleWindow) {

        unsigned int value = analogRead(micADPin);

        if (value > signalMax) {

            signalMax = value;  // save just the max levels

        } else if (value < signalMin) {

            signalMin = value;  // save just the min levels
        }
        delay(5);
    }
 
    const unsigned int peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude

//   Serial.print("delta :");
//   Serial.println(peakToPeak);

    if(peakToPeak > micThreashold) {
       peakCounter++;
    }

    return ( peakCounter > 0 ? true : false );
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

  print_debugnln( "mic power pin:" );
  print_debug(micPowerPin);
  pinMode(micPowerPin, OUTPUT);
  print_debugnln( "mic power threshold: " );
  print_debug(micThreashold);
  digitalWrite(micPowerPin, HIGH);
  print_debugnln( "reading mic in ADC pin: " );
  print_debug(micADPin);

  print_debug( "app ready ..." );
}

void loop() {

  static long timeElapsedSinceEpoch = 0;
  const long timeExpire01 = 10; //* 60; // 35 min
  static int ledNextDimValue = ledFadeValue01;

  timeElapsedSinceEpoch += 1;
  if(timeElapsedSinceEpoch > timeExpire01) {

    timeElapsedSinceEpoch = 0; // restart active LED period

    // power on mic
    //digitalWrite(micPowerPin, HIGH);
    //delay(250); // wait for power up mic (around 1,5 sec.)

    //
    // after entering LED in idle state, begin loop
    // for presence detection
    long counterToNextDim = 0; // count 1 minute to next dim
    const long counterTarget = 4;  // (250ms x cycle => count 15 sec to next dim
    while( ! detectSound() ){
        
        counterToNextDim++;
        
        if(counterToNextDim >= counterTarget) {
          counterToNextDim = 0;
        
          if(ledNextDimValue >0) {
            print_debug( "fade-out led ..." );
            ledNextDimValue--;
            ledObj.fadeOutToTargetValue(ledNextDimValue);
          }
        }
    }
    //
    // presence detected: power off mic
    // digitalWrite(micPowerPin, LOW);

    //
    // fade-in LED
    print_debug( "fading in led to active state ..." );
    ledObj.fadeInToTargetValue(ledFadeValue01);
    ledNextDimValue = ledFadeValue01;
  }

  delay(1000);
}
