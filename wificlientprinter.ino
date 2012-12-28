/*
 * A simple sketch that uses WiServer to get the hourly weather data from LAX and prints
 * it via the Serial API
 */

#include <WiServer.h>
#include <SoftwareSerial.h>
#include "configuration.h"

#define disconnected 0
#define requestComplete 1
#define pausing 2

#define servaddr "localhost"
#define dataquery "/data/test.txt"
uint8 ip[] = {           // IP servers
  192,168,0,70};
  
#define rxPin 6
#define txPin 7
//#define ledPin 5



SoftwareSerial printer(rxPin, txPin); // RX, TX


int countlf = 0, stats = 0;
boolean prok = false;
// Time (in millis) when the data should be retrieved
long lastCompletionTime = 0;

// Function that prints data from the server
void printData(char* data, int len) {

  // Print the data returned by the server
  // Note that the data is not null-terminated, may be broken up into smaller packets, and 
  // includes the HTTP header. 
  while (len-- > 0) {

    if(countlf < 10){
    if(*(data) =='\r' ) {*(data++); countlf++;}
    else {
    *(data++);
    //len--;
    }
    }
    else if(!prok){
      if(*(data) =='&' ) {*(data++); prok = true;}
      else *(data++);
    }
    
    else if(*(data) =='&' ) {*(data++); Feed(5); Cut();}
    else printer.print(*(data++));

  }

}




// A request that gets the latest METAR weather data for LAX
GETrequest getreq(ip, 80, servaddr, dataquery);


void setup() {
  // Initialize WiServer (we'll pass NULL for the page serving function since we don't need to serve web pages) 
  WiServer.init(NULL);

  // Enable Serial output and ask WiServer to generate log messages (optional)
  //Serial.begin(9600);
  //WiServer.enableVerboseMode(true);
  printer.begin(9600);
  // Have the processData function called when data is returned by the server
  getreq.setReturnFunc(printData);
  // dunno why
  delay(1000);
}

void loop(){

stateCheck();
  // Run WiServer
  WiServer.server_task();
  delay(10);
}

void stateCheck() {
  switch (stats) {
  case disconnected:
    connect();
    break;
  case pausing:
    waitForNextRequest();
  }
}

void connect() {
  countlf = 0;
  prok = false;
  getreq.submit();
  lastCompletionTime = millis();
  stats = pausing;
}



void waitForNextRequest() {
  if (millis() - lastCompletionTime >= 20000) {
    stats = disconnected;
  } 
}

//Cut paper function

void Cut(){

  printer.write(0x1b);
  printer.write("@");
  printer.write(0x1b);
  printer.write(0x64);
  printer.write(1);
}

//Feed papper
void Feed(int n){

  printer.write(0x1b);
  printer.write("@");
  printer.write(0x1b);
  printer.write(0x61);
  printer.write(n);
}

