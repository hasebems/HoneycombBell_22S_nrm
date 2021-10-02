/* ========================================
 *
 *  HoneycombBell.ino
 *    description: Main Loop
 *    Latest Version : Proto 6
 *
 *  Copyright(c)2019- Masahiko Hasebe at Kigakudoh
 *  This software is released under the MIT License, see LICENSE.txt
 *
 * ========================================
 */
#include  <MsTimer2.h>
#include  <Adafruit_NeoPixel.h>
#include  <MIDI.h>

#include  "configuration.h"
#include  "TouchMIDI_AVR_if.h"

#include  "i2cdevice.h"
#include  "honeycombbell.h"

#ifdef __AVR__
  #include <avr/power.h>
#endif

/*----------------------------------------------------------------------------*/
//
//     Global Variables
//
/*----------------------------------------------------------------------------*/
Adafruit_NeoPixel led = Adafruit_NeoPixel(MAX_LED, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
MIDI_CREATE_DEFAULT_INSTANCE();

/*----------------------------------------------------------------------------*/
int maxCapSenseDevice;
bool availableEachDevice[MAX_DEVICE_MBR3110];
bool isMasterBoard;

GlobalTimer gt;
static HoneycombBell hcb;

/*----------------------------------------------------------------------------*/
void flash()
{
  gt.incGlobalTime();
}
/*----------------------------------------------------------------------------*/
void setup()
{
  int err;
  int i;

  //  Initialize Hardware
  wireBegin();
//  Serial.begin(31250);
  MIDI.setHandleNoteOff( handlerNoteOff );
  MIDI.setHandleNoteOn( handlerNoteOn );
  MIDI.setHandleControlChange( handlerCC );
  MIDI.begin();
  MIDI.turnThruOff();

#ifdef USE_ADA88
  ada88_init();
  ada88_write(1);
#endif

  //  Read Jumper Pin Setting
  pinMode(MODEPIN1, INPUT); 
  pinMode(MODEPIN2, INPUT);
  pinMode(MODEPIN3, INPUT); 
  pinMode(MODEPIN4, INPUT);

  pinMode(LED_ERR, OUTPUT);
  digitalWrite(LED_ERR, LOW);

  for (i=0; i<MAX_DEVICE_MBR3110; ++i){
    availableEachDevice[i] = true;
  }
  maxCapSenseDevice = 2;

  int errNum = 0;
#if SETUP_MODE // CapSense Setup Mode
  for (i=0; i<maxCapSenseDevice; ++i){
    err = MBR3110_setup(i);
    if (err){
      availableEachDevice[i] = false;
      digitalWrite(LED_ERR, HIGH);
      errNum += 0x01<<i;
    }
  }
  setAda88_Number(errNum*10);
  delay(2000);          // if something wrong, 2sec LED_ERR on
  for (i=0; i<3; i++){  // when finished, flash 3times.
    digitalWrite(LED_ERR, LOW);
    delay(100);
    digitalWrite(LED_ERR, HIGH);
    delay(100);
  }
  digitalWrite(LED_ERR, LOW);
#endif

  //  Normal Mode
  MBR3110_resetAll(maxCapSenseDevice);
  errNum = 0;
  for (i=0; i<maxCapSenseDevice; ++i){
    if (availableEachDevice[i]){
      err = MBR3110_init(i);
      if (err){
        availableEachDevice[i] = false;       
        errNum += 0x01<<i;
      }
    }
  }
  if (errNum){
    //  if err, stop 5sec.
    digitalWrite(LED_ERR, HIGH);
    setAda88_Number(errNum*10);
    delay(5000);  //  5sec LED_ERR on
    digitalWrite(LED_ERR, LOW);
    ada88_write(0);
  }

  //  Check who I am
  int setNumber;
  isMasterBoard = false;
  const uint8_t dip3 = digitalRead(MODEPIN3);
  const uint8_t dip4 = digitalRead(MODEPIN4);
  if      (( dip3 == HIGH ) && ( dip4 == HIGH )){ setNumber = 1;}
  else if (( dip3 == HIGH ) && ( dip4 == LOW  )){ setNumber = 2;}
  else if (( dip3 == LOW  ) && ( dip4 == LOW  )){ setNumber = 3;}
  else if (( dip3 == LOW  ) && ( dip4 == HIGH )){ setNumber = 4;}
  else { setNumber = 1;}
  hcb.setSetNumber(setNumber);
  hcb.decideOctave();

  //  Set NeoPixel Library 
  led.begin();
  led.show(); // Initialize all pixels to 'off'

  //  Set Interrupt
  MsTimer2::set(10, flash);     // 10ms Interval Timer Interrupt
  MsTimer2::start();
}
/*----------------------------------------------------------------------------*/
void loop()
{
  //  Global Timer 
  generateTimer();

  //  HoneycombBell
  hcb.mainLoop();

  //  MIDI Receive
  receiveMidi();

  if ( gt.timer10msecEvent() ){
    //  Touch Sensor
    uint16_t sw[maxCapSenseDevice] = {0};
 #ifdef USE_CY8CMBR3110
    int errNum = 0;
    for (int i=0; i<maxCapSenseDevice; ++i){
      if (availableEachDevice[i] == true){
        uint8_t swtmp[2] = {0};
        int err = MBR3110_readTouchSw(swtmp,i);
        if (err){
          errNum += 0x01<<i;          
        }
        sw[i] = (((uint16_t)swtmp[1])<<8) + swtmp[0];
      }
    }
    if (errNum){
      setAda88_Number(errNum*10);
      digitalWrite(LED_ERR, HIGH);
    }
    else {
      digitalWrite(LED_ERR, LOW);
    }
 #endif
    hcb.checkTwelveTouch(sw);
  }
}
/*----------------------------------------------------------------------------*/
//
//     Global Timer
//
/*----------------------------------------------------------------------------*/
void generateTimer( void )
{
  uint32_t diff = gt.readGlobalTimeAndClear();

  gt.clearAllTimerEvent();
  gt.updateTimer(diff);
  //setAda88_Number(diff);

  if ( gt.timer100msecEvent() == true ){
    //  for Debug
    // blink LED
    //(gt.timer100ms() & 0x0002)? digitalWrite(LED, HIGH):digitalWrite(LED, LOW);
    //setAda88_Number(gt.timer100ms());
  }
}
/*----------------------------------------------------------------------------*/
//
//     MIDI Command & UI
//
/*----------------------------------------------------------------------------*/
void receiveMidi( void ){ MIDI.read();}
/*----------------------------------------------------------------------------*/
void setMidiNoteOn( uint8_t dt0, uint8_t dt1 )
{
//  uint8_t dt[3] = { 0x90, dt0, dt1 };
//  Serial.write(dt,3);
  MIDI.sendNoteOn( dt0, dt1, 1 );
}
/*----------------------------------------------------------------------------*/
void setMidiNoteOff( uint8_t dt0, uint8_t dt1 )
{
//  uint8_t dt[3] = { 0x80, dt0, dt1 };
//  Serial.write(dt,3);
  MIDI.sendNoteOff( dt0, dt1, 1 );
}
/*----------------------------------------------------------------------------*/
void handlerNoteOn( byte channel , byte number , byte value ){ setMidiNoteOn( number, value );}
/*----------------------------------------------------------------------------*/
void handlerNoteOff( byte channel , byte number , byte value ){ setMidiNoteOff( number, value );}
/*----------------------------------------------------------------------------*/
void handlerCC( byte channel , byte number , byte value )
{
  if ( number == /*0x10*/ midi::GeneralPurposeController1 ){
    hcb.rcvClock( value );
  }
}
/*----------------------------------------------------------------------------*/
void midiClock( uint8_t msg )
{
//  uint8_t dt[3] = { 0xb0, 0x10, msg };

  if ( isMasterBoard == false ){
//  Serial.write(dt,3);
    MIDI.sendControlChange( midi::GeneralPurposeController1, msg, 1 );
  }
}
/*----------------------------------------------------------------------------*/
//
//     Hardware Access Functions
//
/*----------------------------------------------------------------------------*/
void setAda88_Number( int number )
{
#ifdef USE_ADA88
  ada88_writeNumber(number);  // -1999 - 1999
#endif
}
/*----------------------------------------------------------------------------*/
//
//     Blink LED by NeoPixel Library
//
/*----------------------------------------------------------------------------*/
const uint8_t colorTable[16][3] = {
  { 200,   0,   0 },//  C
  { 175,  30,   0 },
  { 155,  50,   0 },//  D
  { 135,  70,   0 },
  { 110,  90,   0 },//  E
  {   0, 160,   0 },//  F
  {   0, 100, 100 },
  {   0,   0, 250 },//  G
  {  30,   0, 230 },
  {  60,   0, 190 },//  A
  { 100,   0, 140 },
  { 140,   0,  80 },//  B

  { 100, 100, 100 },
  { 100, 100, 100 },
  { 100, 100, 100 },
  { 100, 100, 100 }
 };
/*----------------------------------------------------------------------------*/
uint8_t colorTbl( uint8_t doremi, uint8_t rgb ){ return colorTable[doremi][rgb];}
void setLed( int ledNum, uint8_t red, uint8_t green, uint8_t blue )
{
  led.setPixelColor(ledNum,led.Color(red, green, blue));
}
void lightLed( void )
{
  led.show();
}
