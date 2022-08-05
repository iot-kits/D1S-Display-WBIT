// *******************************************************
// ****************** colors.h ***************************
// *******************************************************

// This configuration file must reside in the same Arduino
// directory as the remote display sketch

// Colors must use 16-bit definitions in RGB565 format
// https://ee-programming-notepad.blogspot.com/2016/10/16-bit-color-generator-picker.html
// tool for RGB565 to 16-bit hex
// http://www.rinkydinkelectronics.com/calc_rgb565.php

// Color definitions similar to the resistor color code
// By Bruce E. Hall
// http://w8bh.net/pi/rgb565.py
const int BLACK     =  0x0000;
const int BROWN     =  0xA145;
const int RED       =  0xF800;
const int ORANGE    =  0xFD20;
const int YELLOW    =  0xFFE0;
const int GREEN     =  0x07E0;
const int BLUE      =  0x001F;
const int VIOLET    =  0xEC1D;
const int GRAY      =  0x8410;
const int WHITE     =  0xFFFF;
const int GOLD      =  0xFEA0;
const int SILVER    =  0xC618;

// Grays & others
const int CYAN      =  0x07FF;
const int MAGENTA   =  0xF81F;
const int ORANGERED =  0xFA20;
const int LIME      =  0x07E0;
const int LIGHTGRAY =  0xD69A;
const int LIGHTGREEN = 0x9772;

// 12-COLOR PALLETE FOR COLOR BLINDNESS
const int JAZZBERRY_JAM     = 0x980C;
const int JEEPERS_CREEPERS  = 0x04F0;
const int BARBIE_PINK       = 0xFAD5;
const int AQUAMARINE        = 0x07F9;
const int FRENCH_VIOLET     = 0x8019;
const int DODGER_BLUE       = 0x047F;
const int CAPRI             = 0x061F;
const int PLUM              = 0xFD9F;
const int CARMINE           = 0xA004;
const int ALIZARIN_CRIMSON  = 0xE006;
const int OUTRAGEOUS_ORANGE = 0xFB67;
const int BRIGHT_SPARK      = 0xFE07;

// *******************************************************
// ************ FRAME COLOR CHOICES **********************
// *******************************************************

// ANALOG CLOCK FRAME
const int C_ANALOG_FRAME_EDGE    = RED;
const int C_ANALOG_FRAME_BG      = GRAY;
const int C_ANALOG_DIAL_BG       = WHITE;
const int C_ANALOG_TZ            = YELLOW;      // timezone & AM/PM text
const int C_ANALOG_DIAL_EDGE     = BLACK;
const int C_ANALOG_DIAL_NUMERALS = BLACK;
const int C_ANALOG_DIAL_TICKS    = BLACK;
const int C_ANALOG_SEC_HAND      = ORANGERED;
const int C_ANALOG_MIN_HAND      = BLUE;
const int C_ANALOG_HOUR_HAND     = BLUE;
const int C_ANALOG_HUB           = BLACK;       // prints over hands
const int C_ANALOG_INDOOR        = RED;

// DIGITAL CLOCK FRAME
const int C_DIGITAL_FRAME_EDGE  = WHITE;
const int C_DIGITAL_BG          = BLACK;
const int C_DIGITAL_ALT_TZ      = ORANGERED;  // UTC
const int C_DIGITAL_LOCAL_TZ    = LIME;       // local timezone
const int C_DIGITAL_INDOOR      = YELLOW;     // Indoor temp & humid

// WEATHER FRAMES
const int C_WX_TOP_BG           = YELLOW;
const int C_WX_TOP_TEXT         = BLUE;
const int C_WX_BOTTOM_BG        = BLUE;
const int C_WX_BOTTOM_TEXT      = YELLOW;

// ALMANAC FRAME
const int C_ALM_TOP_BG          = BLUE;
const int C_ALM_TOP_TEXT        = YELLOW;
const int C_ALM_BOTTOM_BG       = YELLOW;
const int C_ALM_BOTTOM_TEXT     = BLUE;
const int C_ALM_MOON_BG         = GRAY;
const int C_ALM_MOON_FACE       = WHITE;
