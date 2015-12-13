
#include <FastLED.h>
#include <IRremote.h>
#include <IRremoteInt.h>
#include <avr/pgmspace.h>
String LaufText = "PCL";
 
 

uint8_t Rain[] = {         0,0,0,0,0,0,0,0,0,0,0,0,
                       0,0,0,0,0,0,0,0,0,0,0,0,
                       0,0,0,0,0,0,0,0,0,0,0,0,
                       0,0,0,0,0,0,0,0,0,0,0,0,
                       0,0,0,0,0,0,0,0,0,0,0,0};

uint8_t text[] = {     0,1,1,1,0,1,1,1,0,1,1,1,
                       0,1,0,1,0,1,0,1,0,1,0,0,
                       0,1,1,1,0,1,1,0,0,1,0,0,
                       0,1,0,1,0,1,0,1,0,1,0,0,
                       0,1,0,1,0,1,1,1,0,1,1,1};

const uint8_t P[] PROGMEM = { 1,1,1,
                1,0,1,
                1,1,1,
                1,0,0,
                1,0,0};
const uint8_t C[] PROGMEM = { 1,1,1,
                1,0,0,
                1,0,0,
                1,0,0,
                1,1,1};
const uint8_t L[] PROGMEM = { 1,0,0,
                1,0,0,
                1,0,0,
                1,0,0,
                1,1,1};                
// 1 = 16580863
// 2 = 16613503
// 3 = 16597183
// 4 = 16589023
// 5 = 16621663
// 6 = 16605343
// 7 = 16584943
// 8 = 16617583
// 9 = 16601263
// * = 16593103
// 0 = 16625743
// # = 16609423
// pfeil hoch = 16615543
// pfeil links = 16591063
// pfeil rechts = 16607383
// pfeil runter = 16619623
// OK = 16623703
#define PIN  6

#define COLOR_ORDER RGB
#define CHIPSET     WS2812B

#define BRIGHTNESS 255

const int IRpin = 11;                                         // Pin fr den IR Empfaenger
const int LED = 13;                                           // LED Pin

// -- globale variablen zur verbesserung der IrRemote funktion --

int Last;
uint32_t RepeatTimer; 

// -- globale variablen für Video und animations rendering --

uint32_t FrameTimer;                                          // frametimerbuffer
uint8_t StroboCounter = 0;                                    // strobo Timerbuffer
uint8_t StroboSpeed = 10;                                     // Strobo alle x frames
uint16_t farbe = 0;                                           // globale farbbuffer (HSV.hue)

// -- globale Variablen für funktionsschalter --

bool Textactive = true;                                       // Textanimation an/aus
bool Rainactive = true;                                       // Regenanimation an/aus
bool Stroboactive = false;                                    //Stroboanimation an/aus

// IrRemote

IRrecv irrecv(IRpin);                     
decode_results results;

// filter qoutient
const float FilterQout = 0.8;

// Params for width and height of led matrix
const uint8_t kMatrixWidth = 12;
const uint8_t kMatrixHeight = 5;

// Param for different pixel layouts
const bool    kMatrixSerpentineLayout = true;


#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
CRGB leds_plus_safety_pixel[ NUM_LEDS + 1];                   // Frame array
CRGB* leds( leds_plus_safety_pixel + 1);
CRGB filter[NUM_LEDS];                                        // Filterarray 

void String2Pixels(String Text){                              // String auslesen und in pixel werte umwandeln / schreibt in text[]
  int TextLaenge = Text.length();
  for (int Position = 0; Position < TextLaenge;Position++){
     char Letter = Text.charAt(Position);
     
  }
  
}

uint16_t XY( uint8_t x, uint8_t y)                    // Berrechnet anhand des layouts der led strips und der grösse der matrix die nummer von led an der Stelle x,y
{
  uint16_t i;
  
  if( kMatrixSerpentineLayout == false) {
    i = (y * kMatrixWidth) + x;
  }

  if( kMatrixSerpentineLayout == true) {
    if( y & 0x01) {
      // Odd rows run backwards
      uint8_t reverseX = (kMatrixWidth - 1) - x;
      i = (y * kMatrixWidth) + reverseX;
    } else {
      // Even rows run forwards
      i = (y * kMatrixWidth) + x;
    }
  }
  
  return i;
}

uint8_t PIXEL[2] ={6,2};
uint16_t XYsafe( uint8_t x, uint8_t y)                                                        // nicht verwendet
{
  if( x >= kMatrixWidth) return -1;
  if( y >= kMatrixHeight) return -1;
  return XY(x,y);
}

void PrintPixel(uint8_t x,uint8_t y){
//  for (int i=0; i <= NUM_LEDS; i++){
//    leds[i] = CHSV(0,0,0);
//  }
  leds[ XY(x, y)] = CHSV( farbe/4, 255, 255);
  //Filter();
  //FastLED.show();
}
void PrintRandomPixel(){
  if (farbe >= 1023)
    farbe = 0;
  int PixelAnzahl = (random( 6)+random( 6)-5)/2; 
  if (PixelAnzahl <0)
    PixelAnzahl = 0;
  //int PixelAnzahl = random8( 6);
  //Serial.println(PixelAnzahl, DEC);
  for (int i=0; i <= NUM_LEDS; i++){
    leds[i] = CHSV(farbe/4, random( 200, 255), random( 20, 30));                              // paint background
  }
  for (int j=0; j < PixelAnzahl; j++){                                                        // create random location dots
  leds[ XY(random(kMatrixWidth), random(kMatrixHeight))] = CHSV( random( farbe/4-10, farbe/4+10), random( 150, 200), random( 100, 150));
  }
  AddRain(Rainactive);
  
  Filter();
  AddText(text, Textactive);
  StroboMod(StroboSpeed, Stroboactive);
  FastLED.show();
  farbe++;
}
void AddRain(bool Active){                                                                    // animiert regentropfen
  if(Active){
  int RainAnzahl = (random( 4)+random( 4)-3)/2;
  if (RainAnzahl <0)
    RainAnzahl = 0;
    
  for(int i = kMatrixHeight -1; i>0;i--){                                                     // shift down existing rows
    for(int j = kMatrixWidth -1; j>=0;j--){
      Rain[XY(j,i)] = Rain[XY(j,i-1)];
    }
  }
  for (int i = 0; i < kMatrixWidth;i++){                                                      // create new empty row
    Rain[XY(i,0)] = 0;
  }
  int Col = random();
  
  if ((Col <10) && (RainAnzahl >0)){
    //Serial.println(RainAnzahl, DEC);
    for(int i = 0; i < RainAnzahl;i++){                                                       // create random dots in first row
      Rain[XY(random( kMatrixWidth -1),0)] = 1;
    } 
  }
  for(int i = 0; i <= NUM_LEDS-1;i++){
    if (Rain[i]>0){
      leds[i] = CHSV( farbe/4, 255, 160);
    }
  }
  }
}
void AddText(uint8_t Text[], bool Active){                                                    //schreibt den inhalt vom text array ins ausgabe RGB array
  if(Active){
    for(int row = 0; row < kMatrixHeight; row++) { 
      for (int col = 0;col < kMatrixWidth;col++){  
         if (Text[(row * kMatrixWidth) + col] > 0)
            leds[XY(col,row)] = CHSV((farbe/4+64*Text[(row * kMatrixWidth) + col]),(82),110);
         }
      }
  }
}
void Filter(){                                                                                // Filter funktion auf der zeitachse
  for (int i=0; i <= NUM_LEDS; i++){
    leds[i] = filter[i] + leds [i];
    filter[i].r = leds[i].r * FilterQout;
    filter[i].g = leds[i].g * FilterQout;
    filter[i].b = leds[i].b * FilterQout;    
  }
}
void schalten(){                                                                              // alle pixel hell aufblitzen bei remote eingabe
  for (int i = 0; i < NUM_LEDS;i++){
    leds[i] =CHSV(farbe/4,249,255);
  }
}
void StroboMod(int Interval, bool Active){                                                    // stroboskop blitz alle Interval-Frames
  if(Active){
  if (StroboCounter >= Interval){
      StroboCounter = 0;
      schalten();
      //Filter();
  } else 
      StroboCounter++;
}
}
void MovePixel(int Richtung){
  switch(Richtung) 
 {
 
 case 1: // up
  if (PIXEL[1] - 1 < 0)
    PIXEL[1] = kMatrixHeight - 1;
  else
    PIXEL[1]--;
 break;
 
 case 2: //down
 if (PIXEL[1] + 1 > kMatrixHeight -1)
    PIXEL[1] = 0;
  else
    PIXEL[1]++;
 break;
 
 case 3: //left
 if (PIXEL[0] - 1 < 0)
    PIXEL[0] = kMatrixWidth - 1;
  else
    PIXEL[0]--;
 break;
 
 case 4: //right
 if (PIXEL[0] + 1 > kMatrixWidth - 1)
    PIXEL[0] = 0;
  else
    PIXEL[0]++;
 break;
 
 default: // Standard
 digitalWrite(LED, HIGH);
 
 }
}
 
void setup()
{
 Serial.begin(9600);
 FastLED.addLeds<WS2812B, PIN, RGB>(leds, NUM_LEDS);
 irrecv.enableIRIn();                                                                         // IR Empfaenger starten
 pinMode(LED, OUTPUT);
 PrintPixel(PIXEL[0],PIXEL[1]);
 randomSeed(analogRead(0));
}
 
void loop(){
if (irrecv.decode(&results)) {
  irrecv.resume();                                                                            // Das Signal empfangen
  Serial.println(results.value, HEX);
  switch(results.value){                                                                      // Abhaengig vom Signal Funktionen/animationen ein/aus schalten
   
  case 16615543:// up
   
   Last = 1;
   MovePixel(1);
   PrintPixel(PIXEL[0],PIXEL[1]);
   RepeatTimer = millis();
   break;
 
  case 16619623: // down
   Last = 2;
   MovePixel(2);
   PrintPixel(PIXEL[0],PIXEL[1]);
   RepeatTimer = millis();
   break;
 
  case 16591063: // Links
     if(StroboSpeed-1 >1)
        StroboSpeed--;
     else
        StroboSpeed = 1;

   Last = 3;
//   MovePixel(3);
//   PrintPixel(PIXEL[0],PIXEL[1]);
   RepeatTimer = millis();
   break;
 
  case 16607383: // Rechts
     if(StroboSpeed+1 <50)
        StroboSpeed++;
     else
        StroboSpeed = 50;
   Last = 4;
//   MovePixel(4);
//   PrintPixel(PIXEL[0],PIXEL[1]);
   RepeatTimer = millis();
   break;

  case 0xffffffff:
   if(millis() - RepeatTimer < 250){ 
    Serial.println(StroboSpeed, DEC);
    switch(Last){
    
    case 3:
    
    if(StroboSpeed-1 >1)
        StroboSpeed--;
     else
        StroboSpeed = 1;
    break;
    
    case 4:
    Serial.println(StroboSpeed, DEC);
    if(StroboSpeed+1 <100)
        StroboSpeed++;
     else
        StroboSpeed = 100;
    break;
    default:
    break;
    //PrintPixel(PIXEL[0],PIXEL[1]);
    }
    RepeatTimer = millis();
   }
   break;
  case 16580863: // "eins"
   Serial.println("eins");
   Textactive =  !Textactive;
   schalten();
   Filter();
   FastLED.show();
  break;
  case 16613503: // "zwei"
   Serial.println("zwei");
   Rainactive =  !Rainactive;
   schalten();
   Filter();
   FastLED.show();
  break;

  case 16597183: // "drei"
   Serial.println("zwei");
   Stroboactive =  !Stroboactive;
   schalten();
   Filter();
   FastLED.show();
  break;
  default: // Standard
   digitalWrite(LED, HIGH);
  }
 }
//Serial.println(StroboSpeed, DEC);
  if ((millis() - FrameTimer >= 20) && (irrecv.isIdle())){ 
    //PrintPixel(PIXEL[0],PIXEL[1]);
    PrintRandomPixel();
    FrameTimer = millis();
    //irrecv.resume();
   }
//delay (50);
}
