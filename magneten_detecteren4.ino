// begin Neopixel

#include <Adafruit_NeoPixel.h>
 
#define PIN A5
 
// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, PIN, NEO_GRB + NEO_KHZ800);

// einde Neopixel

// begin mp3shield

#include <SFEMP3Shield.h>
#include <SFEMP3ShieldConfig.h>
#include <SFEMP3Shieldmainpage.h>

#include <BlockDriver.h>
#include <FreeStack.h>
#include <MinimumSerial.h>
#include <SdFat.h>
#include <SdFatConfig.h>
#include <SysCall.h>

SdFat sd; // Create object to handle SD functions

SFEMP3Shield MP3player; // Create Mp3 library object
// These variables are used in the MP3 initialization to set up
// some stereo options:
const uint8_t volume = 0; // MP3 Player volume 0=max, 255=lowest (off)
const uint16_t monoMode = 3;  // Mono setting 0=off, 3=max
int playingTrack = 0; // afspelen track uit

//tracks
int trackHit = 1;
int trackDead = 2;
int trackEvil = 3;
int trackWon = 4;

// einde mp3shield

// add servo
#include <Servo.h>
Servo servo1;

// pins magneet sensoren en welke waarde ze hebben
const int analogInPinP1 = A0;  // Analog input pin that the potentiometer is attached to
const int analogOutPinP1 = 0; // Analog output
const int analogInPinP2 = A1;  // Analog input pin that the potentiometer is attached to
const int analogOutPinP2 = 0; // Analog output
const int analogInPinP3 = A2;  // Analog input pin that the potentiometer is attached to
const int analogOutPinP3 = 0; // Analog output
const int analogInPinP4 = A3;  // Analog input pin that the potentiometer is attached to
const int analogOutPinP4 = 0; // Analog output

const int switchPin = A4; //switch

int standbyState = 1;         // standby status
int switchState = 0;          // motor staat uit
int sensorValue = 0;          // waarde sensor
int snelheid = 85;            // snelheid motor

int outputValueP1 = 0;        // value output to the PWM (analog out)
int outputValueP2 = 0;        // value output to the PWM (analog out)
int outputValueP3 = 0;        // value output to the PWM (analog out)
int outputValueP4 = 0;        // value output to the PWM (analog out)

// levens
int levensP1 = 3;
int levensP2 = 3;
int levensP3 = 3;
int levensP4 = 3;
int levenSpelers = levensP1+levensP2+levensP3+levensP4;

// iedereen is levend
bool aliveP1 = true;
bool aliveP2 = true;
bool aliveP3 = true;
bool aliveP4 = true;

//check of er een leven eraf is
bool levenEraf = true;

// tijd voor rage fuctie
int tijd = 0;

// ragemode uit
int rageMode = 0;

// spel is nog niet afgelopen
bool eindespel = false;

void setup() {
  //kleuren
  strip.begin();
  strip.setBrightness(40); //adjust brightness here
//  strip.show(); // Initialize all pixels to 'off'
  
  pinMode(switchPin, INPUT);

  // staaat de motor aan of uit
  switchState = digitalRead(switchPin);
  startStop();

  //levens tellen
  levenSpelers = levensP1+levensP2+levensP3+levensP4;  

  //alle kleuren verwijderen
  strip.clear();
      
  // Some example procedures showing how to display to the pixels:
  colorP1(strip.Color(255, 0, 0), levensP1); // Rood
  colorP2(strip.Color(255, 255, 0), levensP2); // Geel
  colorP3(strip.Color(0, 255, 0), levensP3); // Groen
  colorP4(strip.Color(199, 0, 232), levensP4); // Roze 

  //kleuren laten zien
  strip.show();
  
  /* Set up all trigger pins as inputs, with pull-ups activated: */
  initSD();  // Initialize the SD card
  initMP3Player(); // Initialize the MP3 Shield
  
  Serial.begin(9600);
}

// Fill the dots one after the other with a color
void colorP1(uint32_t c, int l) {
  int t = 0; // waar start het licht
  int a = t+(l*2); // aantal lichten die bij de speler horen
  // forloop licht om en om aan en uit
  for(uint16_t i=t; i<a; i+=2) {
      strip.setPixelColor(i, c);
  }
}

void colorP2(uint32_t c, int l) {
  int t = 18;
  int a = t+(l*2);
  for(uint16_t i=t; i<a; i+=2) {
      strip.setPixelColor(i, c);
  }
}

void colorP3(uint32_t c, int l) {
  int t = 12;
  int a = t+(l*2);
  for(uint16_t i=t; i<a; i+=2) {
      strip.setPixelColor(i, c);
  }
}

void colorP4(uint32_t c, int l) {
  int t = 6;
  int a = t+(l*2);
  for(uint16_t i=t; i<a; i+=2) {
      strip.setPixelColor(i, c);
  }
}

// alle lichten dezelfde kleur
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

// initSD() initializes the SD card and checks for an error.
void initSD()
{
  //Initialize the SdCard.
  if(!sd.begin(SD_SEL, SPI_HALF_SPEED)) 
    sd.initErrorHalt();
  if(!sd.chdir("/")) 
    sd.errorHalt("sd.chdir");
}

// initMP3Player() sets up all of the initialization for the
// MP3 Player Shield. It runs the begin() function, checks
// for errors, applies a patch if found, and sets the volume/
// stero mode.
void initMP3Player()
{
  uint8_t result = MP3player.begin(); // init the mp3 player shield
  if(result != 0) // check result, see readme for error codes.
  {
    // Error checking can go here!
  }
  MP3player.setVolume(volume, volume);
  MP3player.setMonoMode(monoMode);
}

// track afspelen
void afspelen(int track) {
  if (MP3player.isPlaying()) {
    MP3player.stopTrack();
  }   
  uint8_t result = MP3player.playTrack(track);
}

int valueMagneet(int analogInPin, int analogOutPin) {
  // read the analog in value:
  sensorValue = analogRead(analogInPin);
  // map it to the range of the analog out:
  return map(sensorValue, 0, 1023, 0, 255);
}

// levens verwijderen
int lifes(int levens) {  
  levens--;
  levenEraf = false;
  afspelen(trackHit); 
  delay(200);
  levenEraf = true;  
  return levens;
}

// check of de speler dood is
bool playerAlive(bool levens) {
  if  (levens <= 0) {
    afspelen(trackDead); 
    return false;
  }
  return true;
}

// wie is de winnaar
void winner(bool player1, bool player2, bool player3, bool player4) {

  if (player2 == false && player3 == false && player4 == false) {
    einde();
    colorWipe(strip.Color(255, 0, 0), 50); // Rood
  }
  if (player1 == false && player3 == false && player4 == false) {
    einde();
    colorWipe(strip.Color(255, 255, 0), 50); // Geel
  }
  if (player1 == false && player2 == false && player4 == false) {
    einde();
    colorWipe(strip.Color(0, 255, 0), 50); // Groen
  }
  if (player1 == false && player2 == false && player3 == false) {
    einde();
    colorWipe(strip.Color(199, 0, 232), 50); // Roze
  }
}

// het einde van het spel
void einde() {
  afspelen(trackWon); 
  eindespel = true;
  servo1.detach();

  // laat de lampjes branden
  // stop het spel na een aantal seconden
}

// staat het spel aan of uit
void startStop () {
  if (switchState == 0) {
    servo1.detach();
  }
  if (switchState == 1) {
    servo1.attach(5);
    servo1.write(snelheid);
  }
}

void loop() {
  if (eindespel == false) {

    // is er in de tussentijd een lever eraf gegaan?
    if (levenSpelers != levensP1+levensP2+levensP3+levensP4) {
      levenSpelers = levensP1+levensP2+levensP3+levensP4; 
  
      //kleuren verwijderen
      strip.clear();
      
      // Some example procedures showing how to display to the pixels:
      colorP1(strip.Color(255, 0, 0), levensP1); // Rood
      colorP2(strip.Color(255, 255, 0), levensP2); // Geel
      colorP3(strip.Color(0, 255, 0), levensP3); // Groen
      colorP4(strip.Color(199, 0, 232), levensP4); // Roze 
  
      //kleuren laten zien
      strip.show();
    }
  
    if (switchState != digitalRead(switchPin)) {
      switchState = digitalRead(switchPin);
      startStop();
    }  

    if (switchState == 1) {
      tijd++;
    }
  
    // waardes van magneten berekenen
    outputValueP1 = valueMagneet(analogInPinP1, analogOutPinP1);
    outputValueP2 = valueMagneet(analogInPinP2, analogOutPinP2);
    outputValueP3 = valueMagneet(analogInPinP3, analogOutPinP3);
    outputValueP4 = valueMagneet(analogInPinP4, analogOutPinP4);
  
    // levens eraf & checken of de speler dood is
    if (outputValueP1 <= 20 && outputValueP1 >= 2 && levenEraf && aliveP1) { 
       levensP1 = lifes(levensP1);
       aliveP1 = playerAlive(levensP1);
    }
  
    if (outputValueP2 <= 20 && outputValueP2 >= 2 && levenEraf && aliveP2) { 
       levensP2 = lifes(levensP2);
       aliveP2 = playerAlive(levensP2);
    }
  
    if (outputValueP3 <= 20 && outputValueP3 >= 2 && levenEraf && aliveP3) { 
       levensP3 = lifes(levensP3);
       aliveP3 = playerAlive(levensP3);
    }
  
    if (outputValueP4 <= 20 && outputValueP4 >= 2 && levenEraf && aliveP4) { 
       levensP4 = lifes(levensP4);
       aliveP4 = playerAlive(levensP4);
    }
  
    if (levenSpelers < 7 && rageMode == 0 || tijd > 10000 && rageMode == 0) {
      rageMode = 1;
      snelheid = 90; 
      servo1.write(snelheid);
    }
  
    //winnaar
    if (eindespel == false) {
      winner(aliveP1, aliveP2, aliveP3, aliveP4);
    }

  }
  // wait 2 milliseconds before the next loop
  // for the analog-to-digital converter to settle
  // after the last reading:
  delay(2);
}

