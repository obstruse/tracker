#include <Time.h>
#include <math.h>

char fileName[] = __FILE__;

// in the hardware subfolder of the Arduino IDE installation folder, create platform.txt:
//       compiler.cpp.extra_flags=-D__PATH__="{build.source.path}"
char pathName[] = __PATH__;

char temp[2000];
boolean trackerRun = false;
boolean constantAngular = true;

//time_t frameTime = 1;
//time_t frameInc = 1;

/********************************************************************************/

#include "stepper.h"

const double pi = 3.14159265;

//float totalMinutes = 1;         // for timed frames

int totalFrames = 125;  
int stepsPerFrame = 1;
int rotsPerFrame  = 1;

float targetHeight = 10.0;
const double targetOrigin = 24.0;
double targetBase;
double targetAngle;

int trackerMax = horizMaximum;
int trackerMin = horizMinimum;
bool trackerSpan = false;

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

      delay(4000);                          // depends on how long CHDK takes. Might need to increase

      // setup next one
      // SOHCAHTOA
      // SOHCAHTOA
      // SOHCAHTOA
      if (constantAngular) {                // constant Angular Motion          
        rotateMove = rotateMove + rotsPerFrame;

        targetAngle = rotateMove * 2.0 * pi / rotateMaximum;

        targetBase = targetHeight / tan(targetAngle);
        horizMove = (targetOrigin - targetBase) * stepsInch + 0.5;
        
      } else {                            // constant Linear Motion
        horizMove = horizStep + stepsPerFrame;

        targetBase = targetOrigin - (horizMove / stepsInch);

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

             
       
