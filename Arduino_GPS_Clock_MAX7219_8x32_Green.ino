#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <Timezone.h>
#include <Time.h>
#include <Ticker.h>

Ticker flipper;

static const uint8_t RXPin = 4, TXPin = 5; //D2 - RX, D1 - TX

static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus GPS;

// The serial connection to the GPS device
SoftwareSerial SerialGPS(RXPin, TXPin);

TimeChangeRule myDST = {"CEST", Last, Sun, Mar, 2, 180}; // Central European Summer Time; CEST (Центральноевропейское Летнее Время) - одно из общеизвестных названий для UTC+2 часового пояса
TimeChangeRule mySTD = {"CET", Last, Sun, Oct, 2, 120};   // Central European Time(UTC+1)
Timezone myTZ(myDST, mySTD);//DST - правило для начала летнего времени или летнего времени для любого года;
                            //STD - правило начала поясного времени на любой год.
TimeChangeRule *tcr;


#define DIN_PIN D8//15 //D8
#define CS_PIN  D6//12 //D6
#define CLK_PIN D7//13 //D7


#define NUM_MAX 4

//#define rotation180

#include "max7219_hr.h"
#include "fonts.h"

#define MAX_DIGITS byte(16)
byte dig[MAX_DIGITS] = {0};
byte digold[MAX_DIGITS] = {0};
byte digtrans[MAX_DIGITS] = {0};

int dx = 0;
int dy = 0;
int del = 0;
byte dots = 0;
byte h, m, s, satelit;
//----------------------------------------------------------------------------------------------------------------------------------

void setup()
{
  SerialGPS.begin(GPSBaud);
  initMAX7219();
//  flipper.attach(0.25, readGPS);
  flipper.attach_scheduled(0.5, tochka);
//  clr();
//  pinMode(LED_BUILTIN, OUTPUT);
  sendCmdAll(CMD_SHUTDOWN, 1);
  sendCmdAll(CMD_INTENSITY, 8);
  clr();
//  xPos=0;
    printStringConnect();

      while (h == 0 && m == 0)
    {
        while (SerialGPS.available())
              {
                GPS.encode(SerialGPS.read());

                if (GPS.time.isValid() && GPS.date.isValid())
                   {
                      h = GPS.time.hour();
                      m = GPS.time.minute();
                   }
              }
      
    }
}

//--------------------------------------------------------------------------------------------------------------------------------
void tochka()
{
     dots = !dots; 
}

void loop()
{
//  static byte j,i = 0;
    static uint32_t lastTime = 0;

//  if (i++ > 240)
//  {
//    i = 0;
//    dots = !dots;
//  }
//    if (j++ > 253)
//        {
//          j = 0;
//          readGPS();
//        }
        if (millis() - lastTime > 250)
          {
            lastTime = millis();
            readGPS();
          }

    showAnimClock();
//    showSimpleClock();
//    printStringConnect();
//    clr();
}
//---------------------------------------------------------------------------------------------------------------------
void readGPS()
{
    while (SerialGPS.available())
    {
      GPS.encode(SerialGPS.read());
      if (GPS.time.isValid() && GPS.date.isValid())
      {
        setTime(GPS.time.hour(), GPS.time.minute(), GPS.time.second(), GPS.date.day(), GPS.date.month(), GPS.date.year());
        m = minute();
        h = hour(myTZ.toLocal(now()));
      }
    }
      
//      else     printStringConnect();
      
     
//      time_t utc = now();
//      time_t local = CE.toLocal(utc, &tcr);
  
//      h = GPS.time.hour();
//      m = GPS.time.minute();
        
//        h = hour();
//        h = hour(myTZ.toLocal(now(), &tcr));
          
          
//      satelit = GPS.satellites.value();
}
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
void showSimpleClock()
{
  clr();
  dx = dy = 0;
  #ifdef rotation180
  showDigit(h/10,  0, dig6x8);
  showDigit(h%10,  8, dig6x8);
  showDigit(m/10, 17, dig6x8);
  showDigit(m%10, 25, dig6x8);
//  showDigit(m/10,  9, dig6x8);
//  showDigit(m%10,  1, dig6x8);
//  showDigit(h/10, 25, dig6x8);
//  showDigit(h%10, 16, dig6x8);
  #else
  showDigit(h/10,  0, dig6x8);
  showDigit(h%10,  8, dig6x8);
  showDigit(m/10, 17, dig6x8);
  showDigit(m%10, 25, dig6x8);
  #endif
//  showDigit(s/10, 38, dig6x8);
//  showDigit(s%10, 46, dig6x8);
  #ifdef rotation180
  setCol(15, dots ? B00100100 : 0);
//  setCol(23, dots ? B00100100 : 0);
  #else
  setCol(15, dots ? B00100100 : 0);
  #endif
//  setCol(32,dots ? B00100100 : 0);
//  refreshAll();
  #ifdef rotation180
  refreshAllRot270();
  #else
  refreshAllRot90();
  #endif
}
//-------------------------------------------------------------------------------------------------------------
void showAnimClock()
{
//  byte digPos[6]={4,12,21,29,38,46};
  byte digPos[6] = {0, 8, 18, 26, 34, 42};//Положение цифр - часы, минуты, секунды (по 2 цифры)
  byte digHt = 12;
  byte num = 6; 
  byte i;
  if(!del) 
  {
    del = digHt;
    for(i = 0; i < num; i++) digold[i] = dig[i];
    dig[0] = h/10 ? h/10 : 10;//Без нуля в начале - часы
//    dig[0] = h/10;
    dig[1] = h%10;
    dig[2] = m/10;//satelit/10;
    dig[3] = m%10;//satelit%10;
//    dig[4] = s/10;
//    dig[5] = s%10;
    for(i = 0; i < num; i++)  digtrans[i] = (dig[i] == digold[i]) ? 0 : digHt;
  } 
  else
    del--;
  
  clr();
  for(i = 0; i < num; i++) 
  {
    if(digtrans[i] == 0) 
    {
      dy = 0;
      showDigit(dig[i], digPos[i], dig6x8);
    } 
    else 
    {
      dy = digHt-digtrans[i];
      showDigit(digold[i], digPos[i], dig6x8);
      dy = -digtrans[i];
      showDigit(dig[i], digPos[i], dig6x8);
      digtrans[i]--;
      delay(30);
    }
  }
  dy = 0;
  setCol(14, dots ? B00000000 : B00000000);//мигание двоеточия  
  setCol(15, dots ? B01000100 : B00100010);//мигание двоеточия
  setCol(16, dots ? B01000100 : B00100010);//мигание двоеточия
  setCol(17, dots ? B00000000 : B00000000);//мигание двоеточия

//  setCol(14, dots ? B00000000 : B00000000);//мигание двоеточия  
//  setCol(15, dots ? B00100100 : B00000000);//мигание двоеточия
//  setCol(16, dots ? B00000000 : B00100010);//мигание двоеточия
//  setCol(17, dots ? B00000000 : B00000000);//мигание двоеточия

//  setCol(14, dots ? B00000000 : B00000000);//мигание двоеточия  
//  setCol(15, dots ? B01111100 : B00100000);//мигание двоеточия
//  setCol(16, dots ? B00000100 : B00111110);//мигание двоеточия
//  setCol(17, dots ? B00000000 : B00000000);//мигание двоеточия

//  setCol(14, dots ? B00000000 : B00000000);//мигание двоеточия  
//  setCol(15, dots ? B01101100 : B00000000);//мигание двоеточия
//  setCol(16, dots ? B00000000 : B00110110);//мигание двоеточия
//  setCol(17, dots ? B00000000 : B00000000);//мигание двоеточия

//  setCol(14, dots ? B00000000 : B00000000);//мигание двоеточия  
//  setCol(15, dots ? B01100000 : B00000110);//мигание двоеточия
//  setCol(16, dots ? B01100000 : B00000110);//мигание двоеточия
//  setCol(17, dots ? B00000000 : B00000000);//мигание двоеточия

//  setCol(14, dots ? B00000000 : B00000000);//мигание двоеточия  
//  setCol(15, dots ? B01000100 : B00100100);//мигание двоеточия
//  setCol(16, dots ? B00101000 : B01000010);//мигание двоеточия
//  setCol(17, dots ? B00000000 : B00000000);//мигание двоеточия
  
//  setCol(32,dots ? B00100100 : 0);
//  refreshAll();

  #ifdef rotation180
  refreshAllRot270();
  #else
  refreshAllRot90();
  #endif
}
//---------------------------------------------------------------------------------------------------------------------
void printStringConnect()
{
    printStringWithShift("to GPS", 100);
}
//----------------------------------------------------------------------------------------------------
void printStringWithShift(const char* s, int shiftDelay)
{
  while (*s) 
  {
    printCharWithShift(*s, shiftDelay);
    s++;
  }
}
//--------------------------------------------------------------------------------------------------------------
void printCharWithShift(byte c, int shiftDelay) 
{
  if (c < ' ' || c > '~' + 25) return;
  c -= 32;
  byte w = showChar(c, font);
  for (byte i = 0; i < (w + 1); i++) 
//  for (byte i = (w+1); i > 0; i--)
  {
    delay(shiftDelay);

  #ifdef rotation180
//  scrollRight();
  scrollLeft();
  refreshAllRot270();
  #else
  scrollLeft();
  refreshAllRot90();
  #endif
  }
}
//--------------------------------------------------------------------------------------------------------------
void showDigit(char _char, byte column, const uint8_t *font)
{
  if(dy < -8 | dy > 8) return;
  byte len = pgm_read_byte(font);
  byte w = pgm_read_byte(font + 1 + _char * len);
  column += dx;
  for (byte i = 0; i < w; i++)
    if(column + i >= 0 && column + i < 8 * NUM_MAX) 
    {
      byte _value = pgm_read_byte(font + 1 + _char * len + 1 + i);
      if(!dy) scr[column + i] = _value;
      else scr[column + i] |= dy > 0 ? _value >> dy : _value <<- dy;
//        if(!dy) scr[column + i] = _value; else scr[column + i] |= dy > 0 ? _value <<- dy : _value >> dy;
    }
}
//--------------------------------------------------------------------------------------------------------------
void setCol(byte column, byte _value)
{
  if(dy <- 8 | dy > 8) return;
  column += dx;
  if(column >= 0 && column < 8 * NUM_MAX)
    if(!dy) scr[column] = _value;
    else scr[column] |= dy > 0 ? _value >> dy : _value <<- dy;
}
//------------------------------------------------------------------------------------------------------------
byte showChar(char _char, const uint8_t *font)
{
  byte len = pgm_read_byte(font);
  byte i,w = pgm_read_byte(font + 1 + _char * len);
  for (i = 0; i < w; i++)
    scr[NUM_MAX * 8 + i] = pgm_read_byte(font + 1 + _char * len + 1 + i);
    scr[NUM_MAX * 8 + i] = 0;
  return w;
}
//-------------------------------------------------------------------------------------------------------------*/
