#include <Time.h>
#include <math.h>

char fileName[] = __FILE__;

// in the hardware subfolder of the Arduino IDE installation folder, create platform.txt:
//       compiler.cpp.extra_flags=-D__PATH__="{build.source.path}"
char pathName[] = __PATH__;

// pins 
#define APDS9960_INT    16  // can I poll the interrupt pin? instead of generating an interrupt?   // gesture int  // not 16       // not 15
#define SDA             2   // I2C          // not 16       // not 15
#define LED_PIN         4  // DotStar      // pulls high?  // not 15  // not 7==TX  // not 1==TX
#define CLOCK_PIN       5   // Dot Star     // pulls high?  // not 15 
#define MISO            12  // SPI          // not moveable
#define MOSI            13  // SPI          // not moveable
#define SCK             14  // SPI          // not moveable
#define SS              15  // chip select ADC  // should work on 15
#define SCL             0   // I2C          // not 16       // not 15 (pullup)




char temp[2000];
boolean trackerRun = false;
boolean constantAngular = false;

time_t frameTime = 1;
time_t frameInc = 1;

/********************************************************************************/

#include "stepper.h"

const double pi = 3.14159265;

float totalMinutes = 1;         // for timed frames

int totalFrames = 125;  
int stepsPerFrame = 1;
int rotsPerFrame  = 1;

float targetHeight = 10.0;
const double targetOrigin = 24.0;
double targetDistance;
double targetBase;
double targetAngle;

int trackerMax = horizMaximum;
int trackerMin = horizMinimum;

#include "wifi.h"
#include "http.h"
#include "httpWifi.h"

//-----------------------------------------
void setup()  
{
  Serial.begin(9600);
  
  wifiSetup();
  
  httpSetup();
  httpWifiSetup();

  stepperSetup();

  // shutter pin
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW); 
    
}

//----------------------------------------------
void loop() 
{
  
  server.handleClient();
  //ArduinoOTA.handle();
  wifiOTA();
  wifiDNS();

  // don't do anything unless horizStep and rotateStep have been set
  if (horizStep <= 0 || rotateStep < 0 ) {
    return;   // loop will restart
  }
  
  // make sure move fits within margins
  if ( horizMove < horizMinimum ) horizMove = horizMinimum; 
  if ( horizMove > horizMaximum ) horizMove = horizMaximum;
      
  if (horizMove != horizStep) {                                              // horiz move
    if ( horizMove > horizStep ) {
      stepperMove(stepperHoriz, 1, DOUBLE);
      horizStep++;
    } else {
      stepperMove(stepperHoriz, -1, DOUBLE);
      horizStep--;
    }
    
  } else if ( rotateMove != rotateStep ) {                                 // rotate move, don't try to overlap with horiz
    // take the shorter way around
    if (abs(rotateMove - rotateStep) > rotateMaximum/2) {
      if ( rotateMove > rotateStep ) {
        rotateStep += rotateMaximum;
      } else {
        rotateMove += rotateMaximum;
      }
    }
    
    if ( rotateMove > rotateStep ) {                                      // -1/1 for flat gear, 1/-1 for worm gear
      stepperMove(stepperRotate, -1);
      rotateStep++;
    }
    if ( rotateMove < rotateStep ) {
      stepperMove(stepperRotate, 1);
      rotateStep--;
    }
    
  }
  
  if ( horizMove == horizStep && rotateMove == rotateStep ) {               // got where it was going....

    if (trackerRun) {                         // automatic increment mode
      
      // timed mode
      //if ( millis() < frameTime ) return;   // not time for a frame, restart loop
      //frameTime += frameInc;
              
      // capture image        
      delay(100);                           // settling time. maybe more?
      
      digitalWrite(2, HIGH);
      delay(100);                           // 100ms pulse to trigger remote
      digitalWrite(2, LOW);

      delay(4000);                          // allow CHDK a couple seconds to complete

      // setup next one
      // SOHCAHTOA
      // SOHCAHTOA
      // SOHCAHTOA
      if (constantAngular) {                // constant Angular Motion          
        rotateMove = rotateMove + rotsPerFrame;

        targetAngle = rotateMove * 2.0 * pi / rotateMaximum;

        //targetBase  = sin(pi - pi/2.0 - targetAngle) * targetHeight / sin(targetAngle);
        targetBase = targetHeight / tan(targetAngle);
        horizMove = (targetOrigin - targetBase) * stepsInch + 0.5;
        
      } else {                            // constant Linear Motion
        horizMove = horizStep + stepsPerFrame;

        targetBase = targetOrigin - (horizMove / stepsInch);
        //targetDistance = sqrt((targetHeight * targetHeight) + (targetBase * targetBase) 
        //                    - (2.0 * targetHeight * targetBase * cos(pi/2.0))
        //                    );
        //targetAngle = asin((sin(pi/2.0) * targetHeight)/targetDistance);
        //if (targetBase < 0 ) {
        //  targetAngle = pi - targetAngle;
        //}
        targetAngle = atan2(targetHeight, targetBase);  
        rotateMove = rotateMaximum * targetAngle / 2.0 / pi + 0.5;        
      }
      
      if ( horizMove > trackerMax || horizMove < trackerMin ) {       // reached the end, shut it down          
        horizMove = horizStep;
        rotateMove = rotateStep;
        trackerRun = false;
      }
    }
    
    if ( !trackerRun ) {                                // single motion mode
      rotateStep = rotateStep % rotateMaximum;
      rotateMove = rotateStep;
    }
      

  }

}

             
       
