#ifndef HTTP_H
#define HTTP_H
#include <ESP8266WebServer.h>

ESP8266WebServer server ( 80 );
#endif

//--------------------------------------------
//--------------------------------------------
void handleRoot() {

  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
        
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.sendHeader("Content-Type","text/html",true);
  server.sendHeader("Cache-Control","no-cache");
  server.send(200);
   
//--------------------------------------------
  sprintf ( temp,
"<html>\
  <head>\
    <title>Tracker</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Tracker</h1>"
  );
  server.sendContent ( temp );

//--------------------------------------------
  sprintf ( temp,
"<form action='/setPosition'>\
<table border='1' cellpadding='5'>\
<tr><th colspan='2'>Current Location</th></tr>\
<tr><td>Horiz</td><td><input type='number' min='4' max='44' step='0.001' name='locationInches' value='%s'></td></tr>\
<tr><td>Rotate</td><td><input type='number' min='0' max='359' step='0.01' name='locationDegrees' value='%s'></td></tr>\
<tr><td colspan=2><button type='submit' name='submit'>Set</button></td></tr>\
</table>\
</form>",
  String(horizStep/stepsInch+.0075,3).c_str(),
  String(rotateStep/float(rotateMaximum)*360.0,2).c_str()
  );
  server.sendContent ( temp );

//--------------------------------------------
  sprintf ( temp,
"<form action='/manualMove'>\
<table border='1' cellpadding='5'>\
<tr><th colspan='3'>Manual Move</th></tr>\
<tr><td>Horiz</td><td><input type='number' min='4' max='44' step='0.001' name='moveInches' value='%s'></td>\
  <td><button type='submit' name='horiz'>Move</button></td></tr>\
</form>\
<form action='/manualMove'>\
<tr><td>Rotate</td><td><input type='number' min='0' max='359' step='0.01' name='moveDegrees' value='%s'></td>\
  <td><button type='submit' name='rotate'>Move</button></td></tr>\
</table>\
</form>",
  String(horizStep/stepsInch+.0075,3).c_str(),
  String(rotateStep/float(rotateMaximum)*360.0,2).c_str()
  );
  server.sendContent ( temp );



//--------------------------------------------
  sprintf ( temp,
"<form action='/trackMove'>\
<table border='1' cellpadding='5'>\
<tr><th colspan='2'>Move with Tracking</th></tr>\
<tr><td>Target</td><td><input type='number' min='0'          step='0.001' name='height' value='%s'></td></tr>\
<tr><td>Horiz</td> <td><input type='number' min='4' max='44' step='0.001' name='horiz'  value='%s'></td></tr>\
<tr><td colspan=2><button type='submit' name='submit'>Move</button></td></tr>\
</table>\
</form>\r\n",
  String(targetHeight,3).c_str(),
  String(horizStep/stepsInch+.0075,3).c_str()
  );
  server.sendContent ( temp );

//--------------------------------------------
  sprintf ( temp,
"<form action='/autoMove'>\
<table border='1' cellpadding='5'>\
<tr><th colspan='3'>Automatic Tracking</th></tr>\
<tr><td>Target</td><td><input type='number' min='0' max='44' step='0.001' name='height' value='%s'></td></tr>\
<tr><td>Frames</td><td><input type='number' name='frames' value='%d'></td></tr>\
<tr><td>Constant</td><td>\
  Angular: <input style='margin-right:15px' type='radio' name='constantAngular' value='on' %s>\
  Linear: <input                            type='radio' name='constantAngular' value='off' %s>\
</td></tr>\
<tr><td>Distance</td><td>\
  Auto: <input style='margin-right:15px' type='radio' name='trackerSpan' value='auto' %s>\
  Max:  <input                            type='radio' name='trackerSpan' value='max' %s>\
</td></tr>\
<tr><td>Position</td><td>\
  <button style='margin-right:35px'   type='submit' name='submit' value='left'>Begin</button>\
  <button                             type='submit' name='submit' value='mid'>Mid</button>\
  <button style='margin-left:35px'    type='submit' name='submit' value='right'>End</button>\
</td></tr>\
<tr><td colspan=2><button             type='submit' name='submit' value='move'>Start</button></td></tr>\
</table>\
</form>\r\n",
  String(targetHeight,3).c_str(),
  totalFrames,
  constantAngular ? "checked" : "",
  constantAngular ? "" : "checked",
  trackerSpan ? "" : "checked" ,
  trackerSpan ? "checked" : ""
  );
  server.sendContent ( temp );

//--------------------------------------------

  sprintf ( temp,
"<form action='/releaseStepper'>\
<table border='1' cellpadding='5'>\
<tr><td><button type='submit' name='submit' value='release' >Release</button></td></tr>\
</table>\
</form>\r\n"
  );
  server.sendContent ( temp );
  
//--------------------------------------------
/*     
sprintf ( temp,
"<pre>horizStep=%s\nrotateStep=%s</pre>",
    String(horizStep).c_str(), String(rotateStep).c_str() );
  server.sendContent ( temp );
*/

  sprintf ( temp,
"<br><p>Uptime: %02d:%02d:%02d</p>\
    <p>%s/%s %s %s</p>\
    </body></html>",
    hr, min % 60, sec % 60,
    pathName, fileName, __DATE__, __TIME__);
  server.sendContent ( temp );

  server.sendContent (" ");

  }

//--------------------------------------------
//--------------------------------------------
void setPosition() {

  horizStep = server.arg("locationInches").toFloat() * stepsInch;
  horizMove = horizStep;

  rotateStep = server.arg("locationDegrees").toFloat() / 360.0 * rotateMaximum;
  rotateMove = rotateStep;

  // refresh root
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
  
}

//--------------------------------------------
//--------------------------------------------
void manualMove() {
  int inArgs = server.args();

  for ( int i = 0; i < inArgs; i++ ) {
    if ( server.argName(i) == "moveInches" ) {
      horizMove = server.arg(i).toFloat() * stepsInch;
    }
    if ( server.argName(i) == "moveDegrees" ) {
      rotateMove = server.arg(i).toFloat() / 360.0 * rotateMaximum;
    }
  }

  // refresh root
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
  
}

//--------------------------------------------
void trackingMove(float horizArg) {    // horizArg is float inches

  horizMove = horizArg * stepsInch;    // int step number of that location

  targetBase = targetOrigin - horizArg;

  targetAngle = atan2(targetHeight, targetBase);  
  rotateMove = rotateMaximum * targetAngle / 2.0 / pi;        
}

//--------------------------------------------
void trackMove() {

  targetHeight = server.arg("height").toFloat();
  trackingMove( server.arg("horiz").toFloat() );

  // 204: No Content
  //      The server successfully processed the request and is not returning any content
  //server.send ( 204, "text/plain", "");

  // refresh root
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
 
}


//--------------------------------------------
void autoMove() {

  targetHeight = server.arg("height").toFloat();
  if ( server.arg("trackerSpan") == "max" ) {
    trackerMin = horizMinimum;
    trackerMax = horizMaximum;
    trackerSpan = true;
  } else {
    trackerMin = stepsInch * (targetOrigin - targetHeight);
    trackerMax = stepsInch * (targetOrigin + targetHeight);
    trackerSpan = false;
  }
  
  totalFrames = server.arg("frames").toInt();
  //totalMinutes = server.arg("tmins").toFloat();

  if ( server.arg("constantAngular") == "on" ) { constantAngular = true; } else { constantAngular = false; }

  String inSubmit = server.arg("submit"); 
  if ( inSubmit == "left" )  {trackingMove(trackerMin / stepsInch); }
  if ( inSubmit == "mid" )   {trackingMove(targetOrigin); }
  if ( inSubmit == "right" ) {trackingMove(trackerMax / stepsInch); }
  if ( inSubmit == "move" ) {
    trackerRun = true;
    horizMove = horizStep;
    rotateMove = rotateStep;
  
    // calculate either rotsPerFrame (constantAngular) or stepsPerFrame (constantLinear)
    if (constantAngular) {
      targetBase = targetOrigin - (trackerMin / stepsInch);
      targetAngle = atan2(targetBase, targetHeight) * 2.0;
      rotsPerFrame = rotateMaximum * targetAngle / 2.0 / pi / totalFrames;
    } else {
      stepsPerFrame = (trackerMax - trackerMin) / totalFrames;
    }
  
    //frameTime = millis();
    //frameInc = totalMinutes * 60.0 * 1000.0 / totalFrames;    
  }

  // 204: No Content
  //      The server successfully processed the request and is not returning any content
  //server.send ( 204, "text/plain", "");

  // refresh root
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");

}

//--------------------------------------------
void releaseStepper() {
  stepperRelease();
  trackerRun = false;
  horizMove = horizStep;
  rotateMove = rotateStep;

  // 204: No Content
  //      The server successfully processed the request and is not returning any content
  server.send ( 204, "text/plain", "");
  
}
//--------------------------------------------
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
}


//--------------------------------------------
void httpSetup() {

  server.on ( "/", handleRoot );
  server.on ( "/setPosition", setPosition );
  server.on ( "/manualMove", manualMove );
  server.on ( "/autoMove", autoMove );
  server.on ( "/trackMove", trackMove );
  server.on ( "/releaseStepper", releaseStepper );
  server.onNotFound ( handleNotFound );
  server.begin();

Serial.println("http started");
}
