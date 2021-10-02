/* ========================================
 *
 *	configuration.h
 *		description: TouchMidi Configuration
 *
 *	Copyright(c)2017- Masahiko Hasebe at Kigakudoh
 *  This software is released under the MIT License, see LICENSE.txt.
 *
 * ========================================
*/
#ifndef CONFIGURATION_H
#define CONFIGURATION_H


//---------------------------------------------------------
//    Touch Sensor Setup Mode
//---------------------------------------------------------
#define SETUP_MODE    0   //  1: Setup Mode, 0: Normal Mode

//---------------------------------------------------------
//    Hardware Setting
//---------------------------------------------------------
#define NEOPIXEL_PIN  2   // PD2

#define MODEPIN1      3   // PD3
#define MODEPIN2      4   // PD4
#define MODEPIN3      5   // PD5
#define MODEPIN4      6   // PD6

#define LED_ERR       7   // PD7

//---------------------------------------------------------
//    HoneycombBell Setting
//---------------------------------------------------------
#define   USE_TWELVE_TOUCH_SENS
#define   MAX_LED       12

//---------------------------------------------------------
//    I2C Device Configuration
//---------------------------------------------------------
#define   USE_CY8CMBR3110
#define   USE_ADA88
//#define   USE_LPS22HB
//#define   USE_LPS25H
//#define   USE_AQM1602XA
//#define   USE_ADXL345
//#define   USE_PCA9685
//#define   USE_ATTINY
//#define   USE_PCA9544A

#endif
