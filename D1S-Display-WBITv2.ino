// D1S-Display-WBIT
// this version accomodates modified and unmodified TFT screens
// it must be compiled as one or the other
// unmarked version has CS = D4
// version v2 with TFT_MOD CS = D0
// both versions autodetect AHT10 and DHT11 sensors

const int FW_VERSION = 1006;

//! uncomment to compile for modified screen
#define TFT_MOD

//! uncomment this line for serial output
//#define DEBUG

// 1006 - 2022-08-02 extensive mods
//                   revised ota.h
//                   graphic moon phase
//                   autodetect AHT10 & DHT11
//                   moved WFM to setup()
//                   revised dataScreen
//                   added config options for clocks
// 1005 - 2022-07-20 show SLP in mb always
//      - 2022-07-19 elliptical moon face on almanac
//      - 2022-07-14 sensor detection
//      - rearranged drd
// 1004 - 2020-06-10 calculate moon phase, added HTU21D
// 1003 - 2022-05-28 detect DHT11 in Digital Clock frame,
//                   remove clock checkbox from WFM,
//                   remove analog clock frame code
// 1002 - 2022-05-27 Corrected conversions, replaced vis with bp second wx frame
// 1001 - 2022-05-23 SAVE!!! WFM working with beta version

// Displays local time and weather exclusively from WeatherBit.io
/* Configured using WiFiManager captive portal at 192.168.4.1
 ? Data Needed:
 *   Wi-Fi credentials (SSID & Password)
 *   WeatherBit.io API key
 *   Latitude & longitude decimal degrees
  */

/*_____________________________________________________________________________
   Copyright(c) 2018 - 2022 Karl Berger dba IoT Kits https://w4krl.com

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   **The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.**

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
   _____________________________________________________________________________
*/

/*
******************************************************
*************** INCLUDES & MACROS*********************
******************************************************
*/

// For general sketch
#include <Arduino.h>           // PlatformIO
#include <Wire.h>              // [builtin]
#include <ESP8266WiFi.h>       // [builtin]
#include <ArduinoJson.h>       // [manager] v6.19.4 Benoit Blancho https://arduinojson.org/
#include <ESP8266HTTPClient.h> // [builtin] http

// For Wemos TFT 1.4 display shield
#include <Adafruit_GFX.h>    // [manager] v1.11.3 Core graphics library
#include <Adafruit_ST7735.h> // [manager] v1.9.3 Controller chip
#include <SPI.h>             // [builtin]
#include "colors.h"          // custom frame colors

// For indoor sensors
#include <Adafruit_Sensor.h> // v1.1.5 may not be needed
#include <DHT.h>             // v1.4.4 DHT11
#include <Adafruit_AHTX0.h>  // v2.0.1 AHT10

// Time functions by Rop Gonggrijp https://github.com/ropg/ezTime
#include <ezTime.h> // [manager] v0.8.3 NTP & timezone

// For HTTPS OTA
#ifdef TFT_MOD
const String FW_FILENAME = "D1S-Display-WBITv2";
#else
const String FW_FILENAME = "D1S-Display-WBIT";
#endif
#include <ESP8266HTTPClient.h>       // [builtin] http
#include <WiFiClientSecureBearSSL.h> // [builtin] https
#include <ESP8266httpUpdate.h>       // [builtin] OTA
#include "ota.h"                     // server paths

// For WiFiManager library
// in platformio.ini add   lib_deps = https://github.com/tzapu/WiFiManager.git
#include <LittleFS.h>            // [builtin] LittleFS File System
#include <DNSServer.h>           // [builtin] For webserver
#include <ESP8266WebServer.h>    // [builtin] For webserver
#include <WiFiManager.h>         // [manager] v2.0.11-beta+sha.9c7fed4 https://github.com/tzapu/WiFiManager
#include <DoubleResetDetector.h> // [manager] v1.0.3 Stephen Denne https://github.com/datacute/DoubleResetDetector

#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

//  bool resetFS = true;  //! Erase Little FS
bool resetFS = false; //! Do not erase Little FS

/*
******************************************************
******************* DEFAULTS *************************
******************************************************
*/
const char AP_PORTAL_NAME[] = "D1Sdisplay";               //! must be <11 characters
const String CONFIG_FILENAME = "/config.json";            // LittleFS filename
const int USB_LEFT = 0;                                   // display orientation codes
const int USB_DOWN = 1;                                   // location of USB port
const int USB_RIGHT = 2;                                  // as viewed from
const int USB_UP = 3;                                     // the front of the display
const int FRAME_ORIENTATION = USB_LEFT;                   // for IoT Kits case
const String WX_SERVER = "http://api.weatherbit.io/v2.0"; // weather api server
const String WX_FORECAST = "forecast/daily";              // forecast API
const String WX_CURRENT = "current";                      // current conditions API
const String WX_DAYS = "1";                               // number of days in forecast
const int WX_CURRENT_SHORT_INTERVAL = 10;                 // intervals are in minutes
const int WX_CURRENT_LONG_INTERVAL = 60;                  // WeatherBit limit 500 requests per day ( 2 min 53 sec )
const int WX_FORECAST_SHORT_INTERVAL = 30;                // minutes between forecast in daytime
const int WX_FORECAST_LONG_INTERVAL = 120;                // minutes between forecasts at night
const int DRD_TIMEOUT = 2;                                // seconds between resets
const int DRD_ADDRESS = 32;                               // avoid OTA area

/*
******************************************************
***************** DEVICE CONNECTIONS *****************
******************************************************
*/
//  5V - not used
// 3V3 - power to DHT11 and I2C devices
// GND - return for sensors
//  TX - GPIO 1 not used
//  D4 - GPIO 2 LED_BUILTIN
//  RX - GPIO 3 not used
//  D2 - GPIO 4 I2C SDA
//  D1 - GPIO 5 I2C SCL
//  A0 - ADC not used
//! TFT = WEMOS 1.4 shield connections
const int TFT_RST = 0; // use -1 according to Wemos
const int TFT_DC = D3; // GPIO  0
const int DHTPIN = D6; // GPIO 12 DHT11 humidty/temperature sensor
#define TFT_MOSI D7    // GPIO 13
#define TFT_SCK D5     // GPIO 14
#define SS D8          // GPIO 15 not used
#define WAKE D0        // GPIO 16 not used
#define TFT_LED NC;    // Not used
//! Mod to screen moves CS from D4 to D0 to avoid LED flash
//! Used on kits starting August 2022
#ifdef TFT_MOD
const int TFT_CS = D0; // GPIO 16 MODIFIED TFT
#warning !!!!! COMPILING FOR TFT MOD !!!!!
#else
const int TFT_CS = D4; // GPIO  2 Builtin LED
#warning !!!!! COMPILING FOR STANDARD TFT !!!!!
#endif

/*
******************************************************
**************** INSTANTIATE DEVICES *****************
******************************************************
*/
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST); // WEMOS TFT 1.4 Display
DHT dht(DHTPIN, DHT11);                                         // Temperature/Humidity Sensor
Adafruit_AHTX0 aht;                                             // Temperature/Humidity Sensor
Adafruit_Sensor *aht_humidity, *aht_temp;                       // Unified sensor library
WiFiClient client;                                              // Wi-Fi connection
Timezone myTZ;                                                  // Local timezone
DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);              // Avoid conflict with RTC memory
WiFiManager wm;                                                 // Beta version of WiFiManager
WiFiManagerParameter custom_unit;                               // Weather units param
WiFiManagerParameter custom_duration;                           // Screen duration param

/*
******************************************************
******************* GLOBALS **************************
******************************************************
*/
// structure to hold weather data & calculated values
// all are from "forecast" api unless marked "current"
struct
{
  unsigned int wxCode;     // weather code
  String timezone;         //! Olson timezone from current
  float tempC;             //! temperature (°C) from current
  float tempCdayHigh;      // from 7 am to 7 pm
  float tempCnightLow;     // from 7 pm to 7 am
  float tempCfeelHigh;     // apparent high
  float tempCfeelLow;      // apparent low
  float tempCdew;          // dew point (°C)
  unsigned int humidity;   //! relative humidity (%) from current
  float slp;               //! sea level pressure (hPa) from current
  unsigned long sunRise;   // sunrise (unix time UTC)
  unsigned long sunSet;    // sunset (unix time UTC)
  float lunation;          // 0 = New, 0.25 = 1st Q, 0.5 = Full
  float windSpeed;         // wind speed (m/s)
  float windGust;          // wind gust (m/s)
  String windDirection;    // abbreviation like ENE
  unsigned int clouds;     // clouds (%)
  float visibility;        // km
  unsigned int precipProb; // probablility of precipitation (%)
  float rain;              // equivalent liquid precip (mm)
  float snowDepth;         // accumulated snow depth (mm)
  int tzOffset;            // minutes from UTC (negative is west)
  float uvForecast;        // UV index (0-11+)
  float uvCurrent;         // current UV Index
} wx;

struct
{
  float tempC;
  float humid;
} indoor;

bool metricUnits = false;
bool digitalClock = false;
bool analogClock = false;
int screenDuration = 5;
const int SENSOR_NONE = 0;
const int SENSOR_DHT11 = 1;
const int SENSOR_AHT10 = 2;
const int SENSOR_HTU21D = 3;
int sensorType = SENSOR_NONE; // initialize for none

//! ********* WIFI MANAGER CONFIG PARAMS ***************
// these char arrays hold the user's response to each parameter
const int TZ_SIZE = 40;        // Olson timezone string
const int API_SIZE = 40;       // WeatherBit API key
const int GEO_SIZE = 12;       // Lat/Lon
bool shouldSaveConfig = false; // WFM callback

//! Initialize WiFiManager parameters
char wm_wx_api[API_SIZE] = "";       // get free account at https://www.weatherbit.io/
char wm_wx_lat[GEO_SIZE] = "38.89";  // Washington Monument
char wm_wx_lon[GEO_SIZE] = "-77.04"; // 2 decimals are sufficient
char wm_wx_dlc[2] = "";              // digital clock - none or D
char wm_wx_alc[2] = "";              // analog clock - none or A
char wm_wx_unt[2] = "I";             // Imperial units or M
char wm_wx_dur[3] = "5";             // show screens for 5 seconds

//! *********** WEMOS TFT 1.4 SETTINGS ******************
const int SCREEN_W = 128;           // width in pixels
const int SCREEN_H = 128;           // height in pixels
const int SCREEN_W2 = SCREEN_W / 2; // half width
const int SCREEN_H2 = SCREEN_H / 2; // half height

/*
******************************************************
************* FUNCTION PROTOTYPES ********************
******************************************************
*/
void configModeCallback(WiFiManager *wm); // called if parameters change
void saveConfigCallback();                // called when parameters must be saved
void saveCustomParamCallback();           // called for custom params (radio & checkbox)
String getParam(String name);             // get custom parameter ID
void saveParamCallback();                 // save custom parameters
void readConfig();                        // read parameters from LittleFS
void writeConfig();                       // write parameters to LittleFS
void splashScreen();                      // splash screen
void dataScreen();                        // show configured parameters screen
void otaUpdateScreen(int version);        // shows https update
void wfmInstructionScreen();              // instructions for entering parameters
void rebootScreen();                      // reboot if connection fails
void updateFrame();                       // display frames for selected durations
void firstWXframe();                      // current conditions
void secondWXframe();                     // forecasted conditions
void almanacFrame();                      // date, sun, and moon data
void digitalClockFrame(bool firstRender); // UTC & local time
void analogClockFrame(bool firstRender);  // UTC & local time, indoor temp & humid
int initSensor();                         // detect & initialize indoor sensor
void readSensor(int type);                // read indoor sensor
void checkOTAupdate();                    // check for https update
void getWXforecast();                     // get forecasted weather
void getWXcurrent();                      // get current conditions
String getAPI(String getQuery);           // API request
void parseWXforecast(String json);        // parse forecasted conditions
void parseWXcurrent(String json);         // parse current conditions
float moonPhase();                        // moon phase lunation fraction from Unix timestamp
String wxObservation(int wxCode);         // convert weather code to short terms
float DEGtoRAD(float deg);                // convert degrees to radians
float CtoF(float tempC);                  // convert Celsius to Fahrenheit
float MStoMPH(float ms);                  // convert m/s to miles per hour
float MMtoIN(float mm);                   // convert millimeters to inches
float KMtoMILES(float km);                // convert kilometers to miles
float HPAtoINHG(float hpa);               // convert hPa (mb) to inches of mercury
void drawMoonFace(int xc, int yc, int r, float fract, int litColor, int darkColor);
void displayFlushRight(String text, int rightColumn, int row, int textSize);
void displayCenter(String text, int centerColumn, int row, int textSize);

/*
******************************************************
*********************** SETUP ************************
******************************************************
*/
void setup()
{
  Serial.begin(115200);
  Wire.begin(SDA, SCL);               // define I2C pins & initialize
  sensorType = initSensor();          // detect & initialize indoor sensor
  tft.initR(INITR_144GREENTAB);       // Initialize 1.4-in. TFT ST7735S chip, green tab
  tft.setRotation(FRAME_ORIENTATION); // set screen orientation
  splashScreen();                     // stays on until logon is complete

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // turn off LED
  WiFi.mode(WIFI_STA);             // explicitly set mode, esp defaults to STA+AP

  // erase FS config data - used only for testing
  if (resetFS)
  {
    DEBUG_PRINTLN("Erase FS");
    LittleFS.format();
  }
  readConfig(); // read previous configuration
#ifdef DEBUG
  wm.setDebugOutput(true);
#else
  wm.setDebugOutput(false);
#endif
  wm.setConfigPortalTimeout(240); // in seconds
  // wm.setAPCallback(configModeCallback); ????
  wm.setSaveConfigCallback(saveConfigCallback);
  wm.setSaveParamsCallback(saveCustomParamCallback);

  /*******************************************************
   * define & add custom parameters to config web portal *
   *******************************************************/
  wm.setCustomHeadElement("<h1 style=\"color:red;\">D1S-WBIT by IoT Kits<sup>©</sup></h1>");
  // parameter name (json id, Prompt, user input variable, length)
  WiFiManagerParameter custom_text_instruction("<p style=\"color:red; font-weight: bold\">Enter your WeatherBit parameters:</p>");
  wm.addParameter(&custom_text_instruction); // appears below Wi-Fi credentials and above WeatherBit params
  WiFiManagerParameter custom_wx_api("wx_api", "API Key:", wm_wx_api, API_SIZE);
  wm.addParameter(&custom_wx_api);
  WiFiManagerParameter custom_wx_lat("wx_lat", "Latitude:", wm_wx_lat, GEO_SIZE);
  wm.addParameter(&custom_wx_lat);
  WiFiManagerParameter custom_wx_lon("wx_lon", "Longitude:", wm_wx_lon, GEO_SIZE);
  wm.addParameter(&custom_wx_lon);

  // radio buttons for weather dimensional units
  // https://github.com/tzapu/WiFiManager/blob/master/examples/Advanced/Advanced.ino
  //! Caution - this is one long string!
  const char *custom_unit_selection_str = "<label for='customUnitID'><br>Select weather units:<br></label>"
                                          "<input type = 'radio' name = 'customUnitID' value = 'I' checked > Imperial &emsp;"
                                          "<input type = 'radio' name = 'customUnitID' value = 'M'> Metric";
  // WiFiManagerParameter custom_unit(unit_selection_str);
  new (&custom_unit) WiFiManagerParameter(custom_unit_selection_str);
  // WiFiManagerParameter custom_unit("")
  wm.addParameter(&custom_unit);

// check boxes for clocks
#ifndef TFT_MOD
  WiFiManagerParameter custom_text_flash("<br>LED will flash if clock selected");
  wm.addParameter(&custom_text_flash); // appears above clock checkboxes
#endif
  // https://github.com/kentaylor/WiFiManager/blob/master/examples/ConfigOnSwitchFS/ConfigOnSwitchFS.ino
  char customhtml[40] = "type=\"checkbox\"";
  WiFiManagerParameter custom_wx_dlc("wx_dlc", "Digital Clock (24-hr local & UTC)", "D", 2, customhtml, WFM_LABEL_AFTER);
  wm.addParameter(&custom_wx_dlc);
  WiFiManagerParameter custom_wx_alc("wx_alc", "Analog Clock Dial", "A", 2, customhtml, WFM_LABEL_AFTER);
  wm.addParameter(&custom_wx_alc);

  // radio buttons for screen durations !!!one long string!!!
  const char *custom_duration_str = "<label for='customDurationID'><br>Set screen durations (seconds):<br></label>"
                                    "<input type = 'radio' name = 'customDurationID' value = '5' checked > 5 &emsp;"
                                    "<input type = 'radio' name = 'customDurationID' value = '7'> 7 &emsp;"
                                    "<input type = 'radio' name = 'customDurationID' value = '10'> 10 &emsp;"
                                    "<input type = 'radio' name = 'customDurationID' value = '12'> 12 &emsp;"
                                    "<input type = 'radio' name = 'customDurationID' value = '15'> 15";
  // WiFiManagerParameter custom_duration(duration_str);
  new (&custom_duration) WiFiManagerParameter(custom_duration_str);
  wm.addParameter(&custom_duration);

  // portal menu tokens, "wifi","wifinoscan","info","param","close","sep","erase","restart","exit"
  // sep is a seperator (horizontal rule)
  //! if param is in menu, params will not show up in wifi page!
  const char *menu[] = {"wifi", "info", "sep", "exit"};
  wm.setMenu(menu, 4);
  wm.setMinimumSignalQuality(40); // set min RSSI (percentage) to show in scans, null = 8%

  if (drd.detectDoubleReset())
  {
    DEBUG_PRINTLN("Open config portal");
    wfmInstructionScreen();         // shows a reminder on the TFT display
    digitalWrite(LED_BUILTIN, LOW); // turn on led
    if (!wm.startConfigPortal(AP_PORTAL_NAME))
    {
      rebootScreen();
      DEBUG_PRINTLN("failed to connect and hit timeout");
      delay(3000);
      // if we still have not connected restart and try all over again
      ESP.restart();
      delay(50);
    }
  }
  else
  {
    wm.autoConnect(AP_PORTAL_NAME);
  }
  drd.stop();
  // we are connected
  DEBUG_PRINTLN("Get custom params from WFM");
  strcpy(wm_wx_api, custom_wx_api.getValue());
  strcpy(wm_wx_lat, custom_wx_lat.getValue());
  strcpy(wm_wx_lon, custom_wx_lon.getValue());
  strcpy(wm_wx_dlc, custom_wx_dlc.getValue());
  strcpy(wm_wx_alc, custom_wx_alc.getValue());
  // units and screen durations are set in saveCustomParamCallback()

  if (shouldSaveConfig)
  {
    writeConfig();
    shouldSaveConfig = false;
  }
  digitalWrite(LED_BUILTIN, HIGH); // D1 Mini LED OFF

  // continue with display code
  checkOTAupdate(); // check on power up
  getWXcurrent();   // find timezone from Current weather
  waitForSync();    // ezTime sync with NTP

  /* if timezone has not been cached in EEPROM
   or user is asking for a new timezone
   set the timezone
   */
  if (!myTZ.setCache(0) || myTZ.getOlson() != wx.timezone)
  {
    myTZ.setLocation(wx.timezone);
  }
  wx.tzOffset = -myTZ.getOffset(); // tz offset in minutes negative for west
  myTZ.setDefault();

  dataScreen();    // show configuration data
  getWXforecast(); // initialize weather data
  delay(2000);     // delay to show connection info
} // setup()

/*
******************************************************
********************* LOOP ***************************
******************************************************
*/
void loop()
{
  static bool shouldUpdateCurrent = true;
  static bool shouldUpdateForecast = true;

  //  drd.loop(); // process user reset request
  events(); // ezTime events - autoconnect to NTP server

  if (secondChanged()) // ezTime function
  {
    updateFrame();
  }
  // set and check weather update intervals every minute
  int currentInterval = WX_CURRENT_SHORT_INTERVAL;
  int forecastInterval = WX_FORECAST_SHORT_INTERVAL;
  static int oldMinute = 0;
  if (oldMinute != minute())
  {
    oldMinute = minute();
    {
      if (hour() < 7) // long intervals between midnight and 7 am
      {
        currentInterval = WX_CURRENT_LONG_INTERVAL;
        forecastInterval = WX_FORECAST_LONG_INTERVAL;
      }
      // check if this interval needs current WX update
      if (minute() % currentInterval == 0)
      {
        if (shouldUpdateCurrent)
        {
          getWXcurrent();
          shouldUpdateCurrent = false;
        }
      }
      else
      {
        shouldUpdateCurrent = true;
      }
      //   check if this interval needs WX forecast update
      if ((60 * hour() + minute()) % forecastInterval == 0)
      {
        if (shouldUpdateForecast)
        {
          getWXforecast();
          shouldUpdateForecast = false;
        }
      }
      else
      {
        shouldUpdateForecast = true;
      }
    }
  }
} // loop()

/*
******************************************************
************* WIFIMANAGER CALLBACKS ******************
******************************************************
*/
void configModeCallback(WiFiManager *wm)
{
  digitalWrite(LED_BUILTIN, LOW); // turn D1 Mini LED ON
  DEBUG_PRINTLN(F("[CALLBACK] config mode"));
  wfmInstructionScreen(); // shows a reminder on the TFT display
} // configModeCallback()

// callback when there is need to save config
void saveConfigCallback()
{
  DEBUG_PRINTLN("[CALLBACK] save config");
  shouldSaveConfig = true;
} // saveConfigCallback()

void saveCustomParamCallback()
{
  DEBUG_PRINTLN("[CALLBACK] saveCustomParamCallback");
  // process custom html parameters
  String unit = getParam("customUnitID");
  strcpy(wm_wx_unt, getParam("customUnitID").c_str());
  String duration = getParam("customDurationID");
  strcpy(wm_wx_dur, duration.c_str());
  DEBUG_PRINTLN("PARAM customUnitID = " + unit);
  DEBUG_PRINTLN("PARAM customDurationID = " + duration);
  DEBUG_PRINT("Units: ");
  metricUnits = (unit == "M") ? true : false;
  screenDuration = duration.toInt();
  DEBUG_PRINT("Duration: ");
  DEBUG_PRINTLN(screenDuration);
} // saveCustomParamCallback()

void saveParamCallback()
{
  DEBUG_PRINTLN("[CALLBACK] saveParamCallback fired");
  DEBUG_PRINTLN("PARAM customfieldid = " + getParam("customfieldid"));
}

String getParam(String name)
{
  DEBUG_PRINTLN("[CALLBACK] getParam");
  // read parameter from server, for custom hmtl input
  String value;
  if (wm.server->hasArg(name))
  {
    value = wm.server->arg(name);
  }
  DEBUG_PRINT("PARAM value = ");
  DEBUG_PRINTLN(value);
  return value;
} // getParam(String name)

/*
******************************************************
*********** WiFi Manager Functions *******************
******************************************************
*/
void readConfig()
{
  // LittleFS = Serial Peripheral Inteface Flash File System
  // https://arduino-esp8266.readthedocs.io/en/3.0.2/filesystem.html
  // from wifiManager example AutoConnectWithFSParameters
  // https://github.com/tzapu/WiFiManager/tree/master/examples/Parameters/SPIFFS/AutoConnectWithFSParameters
  if (LittleFS.begin())
  {
    DEBUG_PRINTLN("Mounted file system");
    if (LittleFS.exists(CONFIG_FILENAME))
    {
      // file exists - read and copy to globals
      File configFile = LittleFS.open(CONFIG_FILENAME, "r");
      if (configFile)
      {
        DEBUG_PRINTLN("Opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]); // was + 1

        configFile.readBytes(buf.get(), size);

        DynamicJsonDocument json(1024);

        auto deserializeError = deserializeJson(json, buf.get());
        serializeJson(json, Serial);
        if (!deserializeError)
        {
          // copy from json to global strings
          strcpy(wm_wx_api, json["wx_api"]);
          strcpy(wm_wx_lat, json["wx_lat"]);
          strcpy(wm_wx_lon, json["wx_lon"]);
          strcpy(wm_wx_unt, json["wx_met"]);
          strcpy(wm_wx_dlc, json["wx_dlc"]);
          strcpy(wm_wx_alc, json["wx_alc"]);
          strcpy(wm_wx_dur, json["wx_dur"]);

          digitalClock = (strcmp(wm_wx_dlc, "D") == 0) ? true : false;
          analogClock = (strcmp(wm_wx_alc, "A") == 0) ? true : false;
          metricUnits = (strcmp(wm_wx_unt, "M") == 0) ? true : false;
          screenDuration = atoi(wm_wx_dur);

          DEBUG_PRINTLN("\nreadConfig():");
          DEBUG_PRINT("\tAPI: ");
          DEBUG_PRINTLN(wm_wx_api);
          DEBUG_PRINT("\tLAT: ");
          DEBUG_PRINTLN(wm_wx_lat);
          DEBUG_PRINT("\tLON: ");
          DEBUG_PRINTLN(wm_wx_lon);
          DEBUG_PRINT("\tUNT: ");
          DEBUG_PRINTLN(wm_wx_unt);
          DEBUG_PRINT("\tDLC: ");
          DEBUG_PRINTLN(wm_wx_dlc);
          DEBUG_PRINT("\tALC: ");
          DEBUG_PRINTLN(wm_wx_alc);
          DEBUG_PRINT("\tDUR: ");
          DEBUG_PRINTLN(wm_wx_dur);
        }
        else
        {
          DEBUG_PRINTLN("Failed to load WFM config");
        }
        configFile.close();
      }
    }
  }
  else
  {
    DEBUG_PRINTLN("Failed to mount FS");
  }
} // readConfig()

// saves user parameters in LittleFS
void writeConfig()
{
  DEBUG_PRINTLN(F("Saving WFM config"));
  DynamicJsonDocument json(1024);
  // copy globals to json
  json["wx_api"] = wm_wx_api;
  json["wx_lat"] = wm_wx_lat;
  json["wx_lon"] = wm_wx_lon;
  json["wx_met"] = wm_wx_unt;
  json["wx_dlc"] = wm_wx_dlc;
  json["wx_alc"] = wm_wx_alc;
  json["wx_dur"] = wm_wx_dur;

  digitalClock = (strcmp(wm_wx_dlc, "D") == 0) ? true : false;
  analogClock = (strcmp(wm_wx_alc, "A") == 0) ? true : false;
  metricUnits = (strcmp(wm_wx_unt, "M") == 0) ? true : false;
  screenDuration = atoi(wm_wx_dur);

  DEBUG_PRINTLN("\nwriteConfig():");
  DEBUG_PRINT("\tAPI: ");
  DEBUG_PRINTLN(wm_wx_api);
  DEBUG_PRINT("\tLAT: ");
  DEBUG_PRINTLN(wm_wx_lat);
  DEBUG_PRINT("\tLON: ");
  DEBUG_PRINTLN(wm_wx_lon);
  DEBUG_PRINT("\tUNT: ");
  DEBUG_PRINTLN(wm_wx_unt);
  DEBUG_PRINT("\tDLC: ");
  DEBUG_PRINTLN(wm_wx_dlc);
  DEBUG_PRINT("\tALC: ");
  DEBUG_PRINTLN(wm_wx_alc);
  DEBUG_PRINT("\tDUR: ");
  DEBUG_PRINTLN(wm_wx_dur);

  File configFile = LittleFS.open(CONFIG_FILENAME, "w");
  if (!configFile)
  {
    DEBUG_PRINTLN("Failed to write WFM config");
  }

  serializeJson(json, Serial);
  serializeJson(json, configFile);
  configFile.close();
} // writeConfig()

/*
******************************************************
*********** DISPLAY TEXT FORMATTING ******************
******************************************************
*/
void displayFlushRight(String text, int rightColumn, int row, int textSize)
{
  int numChars = text.length();
  int widthText = textSize * (6 * numChars - 1);
  tft.setCursor(rightColumn - widthText, row);
  tft.setTextSize(textSize);
  tft.print(text);
} // displayFlushRight()

void displayCenter(String text, int centerColumn, int row, int textSize)
{
  int numChars = text.length();
  int widthText = textSize * (6 * numChars - 1);
  tft.setCursor(centerColumn - widthText / 2, row);
  tft.setTextSize(textSize);
  tft.print(text);
} // displayCenter()

/*
******************************************************
******************* SCREENS **************************
******************************************************
*/

/*
******************************************************
**************** Splash Screen ***********************
******************************************************
*/
void splashScreen()
{
  tft.fillScreen(BLUE);
  tft.setTextSize(2);
  tft.setTextColor(YELLOW);
  int topLine = 19;
  displayCenter("D1S-WBIT", SCREEN_W2, topLine, 2);
  displayCenter("Display", SCREEN_W2, topLine + 20, 2);
  displayCenter("by", SCREEN_W2, topLine + 40, 2);
  displayCenter("IoT Kits", SCREEN_W2, topLine + 60, 2);
  #ifdef TFT_MOD
  displayCenter("v" + String(FW_VERSION) + "V2", SCREEN_W2, topLine + 80, 2);
  #else
  displayCenter("v" + String(FW_VERSION), SCREEN_W2, topLine + 80, 2);
  #endif
  for (int i = 0; i < 4; i++)
  {
    tft.drawRoundRect(12 - 3 * i, 12 - 3 * i, SCREEN_W - 12, SCREEN_H - 12, 8, YELLOW);
  }
} // splashScreen()

/*
******************************************************
****************** Data Screen ***********************
******************************************************
*/
void dataScreen()
{
  tft.fillScreen(BLACK); // clear screen
  tft.setTextColor(YELLOW);
  displayCenter("Location", SCREEN_W2, 0, 2);
  displayCenter((String)wm_wx_lat + "/" + (String)wm_wx_lon, SCREEN_W2, 20, 1);
  displayCenter("Time Zone", SCREEN_W2, 32, 2);
  displayCenter(getTimezoneName(), SCREEN_W2, 52, 2);
  displayCenter(wx.timezone, SCREEN_W2, 72, 1);
  displayCenter("Units/Dur", SCREEN_W2, 82, 2);
  String units = "Imperial";
  if (metricUnits)
  {
    units = "Metric";
  }
  displayCenter(units + " " + wm_wx_dur + " sec", SCREEN_W2, 102, 1);
  String clock = "no clock";
  if (analogClock)
  {
    clock = "Analog";
    if (digitalClock)
    {
      clock += "&";
    }
  }
  if (digitalClock)
  {
    clock += "Digital";
  }
  if (analogClock || digitalClock)
  {
    clock += " Clock";
  }
  displayCenter(clock, SCREEN_W2, 112, 1);

} // dataScreen()

/*
******************************************************
*************** OTA Update Screen ********************
******************************************************
*/
void otaUpdateScreen(int version)
{
  tft.fillScreen(RED);
  tft.setTextColor(BLACK);
  displayCenter("Firmware", SCREEN_W2, 0, 2);
  displayCenter("Update", SCREEN_W2, 16, 2);
  displayCenter("DO NOT", SCREEN_W2, 32, 2);
  displayCenter("UNPLUG!!!", SCREEN_W2, 48, 2);
  displayCenter("Will", SCREEN_W2, 64, 2);
  displayCenter("Reboot", SCREEN_W2, 80, 2);
  displayCenter("Old v" + String(FW_VERSION), SCREEN_W2, 96, 2);
  displayCenter("New v" + String(version), SCREEN_W2, 112, 2);
} // otaUpdateScreen()

/*
******************************************************
************ WFM Instruction Screen ******************
******************************************************
*/
void wfmInstructionScreen()
{
  tft.fillScreen(YELLOW);
  tft.setTextColor(BLUE);
  int TOP_LINE = 5;
  displayCenter("Connect to", SCREEN_W2, TOP_LINE, 2);
  displayCenter("Browse to", SCREEN_W2, TOP_LINE + 43, 2);
  displayCenter("Fill form", SCREEN_W2, TOP_LINE + 105, 2);

  tft.setTextColor(RED);
  displayCenter("D1Sdisplay", SCREEN_W2, TOP_LINE + 20, 2);
  displayCenter("192.", SCREEN_W2, TOP_LINE + 60, 2);
  displayCenter("168.4.1", SCREEN_W2, TOP_LINE + 80, 2);
} // wfmInstructionScreen()

void rebootScreen()
{
  tft.fillScreen(RED);
  tft.setTextColor(BLACK);
  displayCenter("Failed to", SCREEN_W2, 20, 2);
  displayCenter("Connect", SCREEN_W2, 40, 2);
  displayCenter("REBOOTING", SCREEN_W2, 80, 2);
}
/*
******************************************************
**************** FRAME UPDATE ************************
******************************************************
*/
void updateFrame()
{
  static int duration = 5;
  int maxFrames = 3; // number of display frames w/o clocks
  if (analogClock)
  {
    maxFrames++;
  }
  if (digitalClock)
  {
    maxFrames++;
  }

  static boolean firstTime = true;        // update frame background first time only
  static int frame = second() % duration; // = 1 ????
  if (second() % duration == 0)
  {
    firstTime = true;
    frame++;
    if (frame > maxFrames)
    {
      frame = 1;
    }
  }

  switch (frame) // choose frame to display
  {
  case 1:
    if (firstTime)
    {
      firstWXframe();
      firstTime = false;
    }
    duration = screenDuration;
    break;
  case 2:
    if (firstTime)
    {
      secondWXframe();
      firstTime = false;
    }
    duration = screenDuration;
    break;
  case 3:
    if (firstTime)
    {
      almanacFrame();
      firstTime = false;
    }
    duration = screenDuration;
    break;
  case 4:
    if (analogClock)
    {
      analogClockFrame(firstTime);
      duration = screenDuration;
      firstTime = false;
    }
    else
    {
      digitalClockFrame(firstTime);
      duration = screenDuration;
      firstTime = false;
    }
    break;
  case 5:
    digitalClockFrame(firstTime);
    duration = screenDuration;
    firstTime = false;
    break;
  default:
    break;
  }
} // updateFrame()

/*
******************************************************
************* FIRST WEATHER FRAME ********************
******************************************************
*/
void firstWXframe()
{
  int headerY = 40;
  int radius = 8;
  // top panel
  tft.fillRoundRect(0, 0, SCREEN_W, 2 * radius, radius, C_WX_TOP_BG);
  tft.fillRect(0, radius, SCREEN_W, headerY - radius, C_WX_TOP_BG);
  // bottom panel
  tft.fillRect(0, headerY, SCREEN_W, SCREEN_H - headerY - radius, C_WX_BOTTOM_BG);
  tft.fillRoundRect(0, SCREEN_H - 2 * radius, SCREEN_W, 2 * radius, radius, C_WX_BOTTOM_BG);

  tft.setTextColor(C_WX_TOP_TEXT);
  displayCenter("Weather", SCREEN_W2, 3, 2);
  displayCenter(wxObservation(wx.wxCode), SCREEN_W2, 23, 2);

  String dispTempNow = String(wx.tempC, 0) + "C";
  String dispTempHiLo = String(wx.tempCdayHigh, 0) + "/" + String(wx.tempCnightLow, 0) + "C";
  String dispPrecipType = "Rain";
  String dispPrecipProb = String(wx.precipProb) + "%";
  String dispPrecipAmt = String(wx.rain, 2) + "mm";

  if (!metricUnits)
  {
    dispTempNow = String(CtoF(wx.tempC), 0) + "F";
    dispTempHiLo = String(CtoF(wx.tempCdayHigh), 0) + "/" + String(CtoF(wx.tempCnightLow), 0) + "F";
    dispPrecipAmt = String(MMtoIN(wx.rain), 2) + "in";
  }

  if (wx.snowDepth > 0)
  {
    dispPrecipType = "Snow";
    if (metricUnits)
    {
      dispPrecipAmt = String(wx.snowDepth, 1) + "mm";
    }
    else
    {
      dispPrecipAmt = String(MMtoIN(wx.snowDepth), 1) + "in";
    }
  }

  // https://www.epa.gov/sunsafety/uv-index-scale-0
  String uvLabel = "";
  int uvText = YELLOW;
  int uvBG = 0;

  int uvi = (hour() > 13) ? wx.uvCurrent : wx.uvForecast;

  if (uvi < 3)
  {
    uvLabel = "Low";
    uvBG = GREEN;
    uvText = BLACK;
  }
  else if (uvi < 6)
  {
    uvLabel = "Mdrte";
    uvBG = YELLOW;
    uvText = BLUE;
  }
  else if (uvi < 8)
  {
    uvLabel = "High";
    uvBG = OUTRAGEOUS_ORANGE;
  }
  else if (uvi < 11)
  {
    uvLabel = "VryHi";
    uvBG = ALIZARIN_CRIMSON; // ORANGERED;
  }
  else
  {
    uvLabel = "Xtrm";
    uvBG = MAGENTA;
  }

  tft.setTextColor(C_WX_BOTTOM_TEXT);
  //! left edge must clear border
  int leftEdge = 6;
  int rightEdge = 125;
  tft.setCursor(leftEdge, 42);
  tft.print("Now");
  tft.setCursor(leftEdge, 59);
  tft.print("H/L");
  tft.setCursor(leftEdge, 76);
  tft.print(dispPrecipType);
  //! blank label

  displayFlushRight(dispTempNow, rightEdge, 42, 2);
  displayFlushRight(dispTempHiLo, rightEdge, 59, 2);
  displayFlushRight(dispPrecipProb, rightEdge, 76, 2);
  displayFlushRight(dispPrecipAmt, rightEdge, 93, 2);

  tft.fillRect(0, 109, 128, 128, uvBG);
  tft.setTextColor(uvText);
  tft.setCursor(leftEdge, 111);
  tft.print("UV" + String(uvi));
  displayFlushRight(uvLabel, rightEdge, 111, 2);
} // firstWXframe()

/*
******************************************************
*************** SECOND WEATHER FRAME *****************
******************************************************
*/
void secondWXframe()
{
  // v1002 - replace vis with bp, corrected unit conversions
  int headerY = 40;
  int radius = 8;
  // top panel
  tft.fillRoundRect(0, 0, SCREEN_W, 2 * radius, radius, C_WX_TOP_BG);
  tft.fillRect(0, radius, SCREEN_W, headerY - radius, C_WX_TOP_BG);
  tft.fillRect(0, headerY, SCREEN_W, SCREEN_H - headerY - radius, C_WX_BOTTOM_BG);
  tft.fillRoundRect(0, SCREEN_H - 2 * radius, SCREEN_W, 2 * radius, radius, C_WX_BOTTOM_BG);
  tft.setTextColor(C_WX_TOP_TEXT);

  displayCenter("Feel Hi/Lo", SCREEN_W2, 3, 2);

  if (metricUnits)
  {
    displayCenter(String(wx.tempCfeelHigh, 0) + "/" + String(wx.tempCfeelLow, 0) + " C", SCREEN_W2, 23, 2);
  }
  else
  {
    displayCenter(String(CtoF(wx.tempCfeelHigh), 0) + "/" + String(CtoF(wx.tempCfeelLow), 0) + " F", SCREEN_W2, 23, 2);
  }

  if (wx.windGust < wx.windSpeed)
  {
    wx.windGust = wx.windSpeed;
  }

  String dispWindSpeed = String(wx.windSpeed, 0) + "kph";
  String dispGust = String(wx.windGust, 0) + "kph";
  String dispVis = String(wx.visibility, 0) + "km";
  String dispSLP = String(wx.slp, 1) + "mb";
  String dispCloud = String(wx.clouds) + "%";
  String dispHumid = String(wx.humidity) + "%";

  if (!metricUnits)
  {
    dispWindSpeed = String(MStoMPH(wx.windSpeed), 0) + "mph";
    dispGust = String(MStoMPH(wx.windGust), 0) + "mph";
    dispVis = String(KMtoMILES(wx.visibility), 0) + "mi";
    dispSLP = String(HPAtoINHG(wx.slp), 2) + "Hg";
  }

  tft.setTextColor(C_WX_BOTTOM_TEXT);
  int leftEdge = 6;
  int rightEdge = 125;

  // labels
  tft.setCursor(leftEdge, 42);
  tft.print(wx.windDirection);
  tft.setCursor(leftEdge, 59);
  tft.print("Gust");
  tft.setCursor(leftEdge, 76);
  tft.print("Cloud");
  tft.setCursor(leftEdge, 93);
  // tft.print("Visi");
  tft.print("BP");
  tft.setCursor(leftEdge, 110);
  tft.print("Humid");

  // values
  displayFlushRight(dispWindSpeed, rightEdge, 42, 2);
  displayFlushRight(dispGust, rightEdge, 59, 2);
  displayFlushRight(dispCloud, rightEdge, 76, 2);
  // displayFlushRight(dispVis, rightEdge, 93, 2);
  displayFlushRight(dispSLP, rightEdge, 93, 2);
  displayFlushRight(dispHumid, rightEdge, 110, 2);
} // secondWXframe()

/*
******************************************************
**************  ALMANAC FRAME ************************
******************************************************
*/
void almanacFrame()
{
  int bufSize = 11;
  char timeString[bufSize] = "";
  int hh, mm;

  int headerY = 40;
  int radius = 8;
  // top panel
  tft.fillRoundRect(0, 0, SCREEN_W, 2 * radius, radius, C_ALM_TOP_BG);
  tft.fillRect(0, radius, SCREEN_W, headerY - radius, C_ALM_TOP_BG);
  tft.fillRect(0, headerY, SCREEN_W, SCREEN_H - headerY - radius, C_ALM_BOTTOM_BG);
  tft.fillRoundRect(0, SCREEN_H - 2 * radius, SCREEN_W, 2 * radius, radius, C_ALM_BOTTOM_BG);
  tft.setTextColor(C_ALM_TOP_TEXT);
  displayCenter("Almanac", SCREEN_W2, 3, 2);
  displayCenter(dateTime("D M j"), SCREEN_W2, 23, 2); // "Sat Aug 7"

  tft.setTextColor(C_ALM_BOTTOM_TEXT);
  displayCenter("SUN", SCREEN_W2, 41, 2); // 41
  hh = ((wx.sunRise - 60 * getOffset()) % 86400UL) / 3600;
  mm = ((wx.sunRise - 60 * getOffset()) % 3600UL) / 60;
  snprintf(timeString, bufSize, "Rise%2d:%02da", hh, mm); // no leading zero
  displayCenter(timeString, SCREEN_W2, 56, 2);            // 59

  hh = ((wx.sunSet - 60 * getOffset()) % 86400UL) / 3600;
  mm = ((wx.sunSet - 60 * getOffset()) % 3600UL) / 60;
  if (hh > 12)
  {
    hh -= 12;
  }
  snprintf(timeString, bufSize, "Set %2d:%02dp", hh, mm);
  displayCenter(timeString, SCREEN_W2, 72, 2); // 76

  // panel for moon data
  tft.fillRect(0, 90, SCREEN_W, SCREEN_H - 90, C_ALM_MOON_BG);
  // process moon phase
  float fraction = moonPhase(); // find moon phase
  float age = fraction * 29.53; // days in synodic month

  // crescent is abbreviated to avoid overwriting the moon face
  String phaseTop[9] = {"New", "Waxing", "First", "Waxing",
                        "Full", "Waning", "Last", "Waning", "New"};
  String phaseBot[9] = {"", "Crescnt", "Quarter", "Gibbous",
                        "", "Gibbous", "Quarter", "Crescnt", ""};
  // Start age for each phase. New, First Quarter, Full, and Last Quarter
  // last 1 day, all others last 6.38 days
  float phaseStart[10] = {0, 0.5, 6.88, 8.88,
                          14.26, 15.26, 21.64, 22.64, 29.03, 30};
  int i = 0; // must be declared outside of for loop for scope
  for (i = 0; i < 9; i++)
  {
    if (age >= phaseStart[i] && age < phaseStart[i + 1])
    {
      break;
    }
  }
  // display moon phase and name
  tft.setTextColor(C_ALM_TOP_TEXT, C_ALM_MOON_BG);
  drawMoonFace(20, SCREEN_H - 19, 15, fraction, C_ALM_MOON_FACE, C_ALM_MOON_BG);
  tft.setCursor(42, 93);
  tft.print(phaseTop[i]);
  tft.setCursor(42, 110);
  tft.print(phaseBot[i]);
} // almanacFrame()

/*
******************************************************
************* DIGITAL CLOCK FRAME ********************
******************************************************
*/
void digitalClockFrame(boolean firstRender)
{
  int topLine = 10; // sets location of top line
  tft.setTextSize(2);

  if (firstRender)
  {
    tft.fillScreen(BLACK);
    // frame
    tft.drawRoundRect(0, 0, SCREEN_W, SCREEN_H, 8, C_DIGITAL_FRAME_EDGE);

    // Coordinated Universal Time
    tft.setTextColor(C_DIGITAL_ALT_TZ, C_DIGITAL_BG);
    displayCenter("UTC", SCREEN_W2, topLine, 2);

    // local time zone
    tft.setTextColor(C_DIGITAL_LOCAL_TZ, C_DIGITAL_BG);
    displayCenter(myTZ.getTimezoneName(), SCREEN_W2, topLine + 40, 2);
    // indoor temperature & humidity
    if (sensorType != SENSOR_NONE)
    {
      tft.setTextColor(C_DIGITAL_INDOOR, C_DIGITAL_BG);
      displayCenter("Indoor", SCREEN_W2, topLine + 80, 2);
      readSensor(sensorType);
      tft.setTextColor(C_DIGITAL_INDOOR, C_DIGITAL_BG); // print over dial
      if (metricUnits)
      {
        displayCenter(String(indoor.tempC, 1) + "C/" + String(indoor.humid, 0) + "%", SCREEN_W2, topLine + 96, 2);
      }
      else
      {
        float temp = CtoF(indoor.tempC);
        displayCenter(String(temp, 0) + "F/" + String(indoor.humid, 0) + "%", SCREEN_W2, topLine + 96, 2);
      }
    }
  }
#ifdef TFT_MOD
  tft.setTextColor(C_DIGITAL_ALT_TZ, C_DIGITAL_BG);
  displayCenter(UTC.dateTime("H:i~:s"), SCREEN_W2, topLine + 16, 2);
  tft.setTextColor(C_DIGITAL_LOCAL_TZ, C_DIGITAL_BG);
  displayCenter(myTZ.dateTime("H:i~:s"), SCREEN_W2, topLine + 56, 2);
#else
  tft.setTextColor(C_DIGITAL_ALT_TZ, C_DIGITAL_BG);
  displayCenter(UTC.dateTime("H:i"), SCREEN_W2, topLine + 16, 2);
  tft.setTextColor(C_DIGITAL_LOCAL_TZ, C_DIGITAL_BG);
  displayCenter(myTZ.dateTime("H:i"), SCREEN_W2, topLine + 56, 2);
#endif

} // digitalClockFrame()

/*
******************************************************
************* ANALOG CLOCK FRAME *********************
******************************************************
*/
void analogClockFrame(bool firstRender)
{
  // 05/12/2022 - added indoor temp/humid
  // 02/06/2022 - fixed hour hand (deg not rad)
  const int CENTER_X = SCREEN_W2;
  const int CENTER_Y = SCREEN_H2;
  //? scale dimensions from dial radius
  const int DIAL_RADIUS = SCREEN_H2; // outer clock dial
  const int NUMERAL_R = DIAL_RADIUS - 6;
  const int OUTER_TICK_R = NUMERAL_R - 6;
  const int MIN_TICK_R = OUTER_TICK_R - 3;
  const int INNER_TICK_R = MIN_TICK_R - 3;
  const int MIN_HAND = INNER_TICK_R;                 // longest hand
  const int SEC_DOT_R = 3;                           // dot radius at end of second hand
  const int SEC_HAND = INNER_TICK_R - SEC_DOT_R - 1; // dot with radius 2 on end of hand
  const int HOUR_HAND = 3 * MIN_HAND / 4;            // hour hand is 3/4 minute hand length
  const int HUB_R = 4;                               // hub radius
  int x1, x2, x3, y1, y2, y3;                        // various coordinates
  int deg;
  float rad, rad1, rad2, rad3;

  //? draw clock face first time only to speed up graphics
  if (firstRender)
  {
    tft.fillScreen(C_ANALOG_FRAME_BG);
    tft.drawRoundRect(0, 0, SCREEN_W, SCREEN_H, 8, C_ANALOG_FRAME_EDGE);
    tft.fillCircle(CENTER_X, CENTER_Y, DIAL_RADIUS, C_ANALOG_DIAL_BG);
    tft.drawCircle(CENTER_X, CENTER_Y, DIAL_RADIUS + 1, C_ANALOG_DIAL_EDGE);
    tft.setTextSize(1); // for dial numerals
    tft.setTextColor(C_ANALOG_DIAL_NUMERALS);

    //? add minute & hour tick marks
    for (int minTick = 1; minTick < 61; minTick++)
    {
      int tickR = MIN_TICK_R;
      deg = 6 * minTick; // 60 ticks, one every 6 degrees
      if (deg % 30 == 0) // it is an hour tick
      {
        rad = DEGtoRAD(deg);
        tickR = INNER_TICK_R;
        // place numeral a little beyond tick line
        x1 = (CENTER_X + (sin(rad) * NUMERAL_R));
        y1 = (CENTER_Y - (cos(rad) * NUMERAL_R));
        tft.setCursor(x1 - 2, y1 - 4); // minus third character width, half height
        if (minTick / 5 == 12)         // different placement for 12 noon
        {
          tft.setCursor(x1 - 5, y1 - 4);
        }
        tft.print(minTick / 5);
      }
      //? draw tick
      rad = DEGtoRAD(deg); // Convert degrees to radians
      x2 = (CENTER_X + (sin(rad) * OUTER_TICK_R));
      y2 = (CENTER_Y - (cos(rad) * OUTER_TICK_R));
      x3 = (CENTER_X + (sin(rad) * tickR));
      y3 = (CENTER_Y - (cos(rad) * tickR));
      tft.drawLine(x2, y2, x3, y3, C_ANALOG_DIAL_TICKS); // tick line
    }
    tft.setTextColor(C_ANALOG_TZ, C_ANALOG_FRAME_BG); // print over dial
    // show timezone
    tft.setCursor(3, 8);
    tft.setTextSize(1);
    tft.print(getTimezoneName());
    String amPM = (hour() > 12) ? "PM" : "AM";
    displayFlushRight(amPM, 125, 8, 1);

    // print indoor temperature & humidity
    // show only if there is a sensor and no digital clock
    if (!digitalClock && sensorType != SENSOR_NONE)
    {
      readSensor(sensorType);
      tft.setTextColor(C_ANALOG_INDOOR, C_ANALOG_DIAL_BG); // print over dial
      if (metricUnits)
      {
        displayCenter(String(indoor.tempC, 1) + "C", CENTER_X, CENTER_Y - 30, 2);
      }
      else
      {
        displayCenter(String(CtoF(indoor.tempC), 0) + "F", CENTER_X, CENTER_Y - 30, 2);
      }
      displayCenter(String(indoor.humid, 0) + "%", CENTER_X, CENTER_Y + 15, 2);
    }
  } // if firstRender

  //? **** Process second hand ****
  deg = second() * 6;  // each second advances 6 degrees
  rad = DEGtoRAD(deg); // Convert degrees to radians
  static float oldSrad = rad;

  //? erase previous second hand
  x3 = (CENTER_X + (sin(oldSrad) * SEC_HAND));
  y3 = (CENTER_Y - (cos(oldSrad) * SEC_HAND));
  tft.drawLine(CENTER_X, CENTER_Y, x3, y3, C_ANALOG_DIAL_BG);
  tft.fillCircle(x3, y3, SEC_DOT_R, C_ANALOG_DIAL_BG);
  oldSrad = rad; // save current radians for erase next time

  //? draw new second hand
  x3 = (CENTER_X + (sin(rad) * SEC_HAND));
  y3 = (CENTER_Y - (cos(rad) * SEC_HAND));
  tft.drawLine(CENTER_X, CENTER_Y, x3, y3, C_ANALOG_SEC_HAND);
  tft.fillCircle(x3, y3, SEC_DOT_R, C_ANALOG_SEC_HAND);

  //? **** Process minute hand ****
  deg = minute() * 6;         // each minute advances 6 degrees
  rad1 = DEGtoRAD(deg + 90);  // base of triangular hand
  rad2 = DEGtoRAD(deg - 90);  // base of triangular hand
  rad3 = DEGtoRAD(deg);       // point of hand
  static float oldMdeg = deg; // save current degrees for triangle

  //? erase previous minute hand
  if (deg != oldMdeg || firstRender)
  {
    float oldMrad1 = DEGtoRAD(oldMdeg + 90);
    float oldMrad2 = DEGtoRAD(oldMdeg - 90);
    float oldMrad3 = DEGtoRAD(oldMdeg);
    x1 = (CENTER_X + (sin(oldMrad1) * HUB_R));
    y1 = (CENTER_Y - (cos(oldMrad1) * HUB_R));
    x2 = (CENTER_X + (sin(oldMrad2) * HUB_R));
    y2 = (CENTER_Y - (cos(oldMrad2) * HUB_R));
    x3 = (CENTER_X + (sin(oldMrad3) * MIN_HAND));
    y3 = (CENTER_Y - (cos(oldMrad3) * MIN_HAND));
    tft.fillTriangle(x1, y1, x2, y2, x3, y3, C_ANALOG_DIAL_BG);
    oldMdeg = deg;
  }

  //? draw new minute hand
  x1 = (CENTER_X + (sin(rad1) * HUB_R));
  y1 = (CENTER_Y - (cos(rad1) * HUB_R));
  x2 = (CENTER_X + (sin(rad2) * HUB_R));
  y2 = (CENTER_Y - (cos(rad2) * HUB_R));
  x3 = (CENTER_X + (sin(rad3) * MIN_HAND));
  y3 = (CENTER_Y - (cos(rad3) * MIN_HAND));
  tft.fillTriangle(x1, y1, x2, y2, x3, y3, C_ANALOG_MIN_HAND);

  //? **** Process hour hand ****
  int dialHour = hour();
  // convert to 12-hour time
  if (dialHour > 13)
  {
    dialHour = dialHour - 12;
  }
  // 30 degree increments + adjust for minutes
  // the Swiss prefer incrementing the minute hand in minute steps
  deg = dialHour * 30 + int((minute() / 12) * 6);
  static float oldHdeg = deg;
  rad1 = DEGtoRAD(deg + 90);
  rad2 = DEGtoRAD(deg - 90);
  rad3 = DEGtoRAD(deg);

  //? erase previous hour hand
  if (deg != oldHdeg || firstRender)
  {
    float oldHrad1 = DEGtoRAD(oldHdeg + 90);
    float oldHrad2 = DEGtoRAD(oldHdeg - 90);
    float oldHrad3 = DEGtoRAD(oldHdeg);
    x1 = (CENTER_X + (sin(oldHrad1) * HUB_R));
    y1 = (CENTER_Y - (cos(oldHrad1) * HUB_R));
    x2 = (CENTER_X + (sin(oldHrad2) * HUB_R));
    y2 = (CENTER_Y - (cos(oldHrad2) * HUB_R));
    x3 = (CENTER_X + (sin(oldHrad3) * HOUR_HAND));
    y3 = (CENTER_Y - (cos(oldHrad3) * HOUR_HAND));
    tft.fillTriangle(x1, y1, x2, y2, x3, y3, C_ANALOG_DIAL_BG);
    oldHdeg = deg; // save current degrees for triangle
  }
  //? draw new hour hand
  x1 = (CENTER_X + (sin(rad1) * HUB_R));
  y1 = (CENTER_Y - (cos(rad1) * HUB_R));
  x2 = (CENTER_X + (sin(rad2) * HUB_R));
  y2 = (CENTER_Y - (cos(rad2) * HUB_R));
  x3 = (CENTER_X + (sin(rad3) * HOUR_HAND));
  y3 = (CENTER_Y - (cos(rad3) * HOUR_HAND));
  tft.fillTriangle(x1, y1, x2, y2, x3, y3, C_ANALOG_HOUR_HAND);

  //? draw hub with little dot in center of hub
  tft.fillCircle(CENTER_X, CENTER_Y, HUB_R, C_ANALOG_HUB);
  tft.fillCircle(CENTER_X, CENTER_Y, 1, C_ANALOG_DIAL_BG);

} // analogClockFrame()

/*
******************************************************
**************** Get WeatherBit API ******************
******************************************************
*/
void getWXforecast()
{
  // Documentation: https://www.weatherbit.io/api/weather-forecast-16-day
  String getQuery = WX_SERVER;
  getQuery += "/" + WX_FORECAST;
  getQuery += "?lat=" + (String)wm_wx_lat + "&lon=" + (String)wm_wx_lon;
  getQuery += "&days=" + WX_DAYS;
  getQuery += "&key=" + (String)wm_wx_api;

  parseWXforecast(getAPI(getQuery));
} // getWXforecast()

void getWXcurrent()
{
  // Documentation: https://www.weatherbit.io/api/weather-current
  String getQuery = WX_SERVER;
  getQuery += "/" + WX_CURRENT;
  getQuery += "?lat=" + (String)wm_wx_lat + "&lon=" + (String)wm_wx_lon;
  getQuery += "&key=" + (String)wm_wx_api;

  parseWXcurrent(getAPI(getQuery));
} // getWXcurrent()

void parseWXcurrent(String json)
{
  // zero out all values to account for empty elements
  wx.slp = 1000;
  wx.tempC = 20;
  wx.humidity = 0;

  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, json);

  if (error)
  {
    DEBUG_PRINT(F("deserialize Current failed: "));
    DEBUG_PRINTLN(error.f_str());
    return;
  }
  JsonObject data_0 = doc["data"][0];

  // not needed with this selection of json elements
  // JsonObject data_0_weather = data_0["weather"];

  wx.tempC = data_0["temp"];                // 14.7
  wx.humidity = data_0["rh"];               // 82
  wx.slp = data_0["slp"];                   // 1021.5
  wx.timezone = (String)data_0["timezone"]; // "America/New_York"

  // These are data elements available from Current for reference.
  // Elements marked with ==> are used in this program.

  //! temperatures
  // float data_0_dewpt = data_0["dewpt"]; // 5.9
  // float data_0_app_temp = data_0["app_temp"]; // 9.2
  //!==>float data_0_temp = data_0["temp"]; // 9.2

  //! precipitation
  // const char* data_0_pod = data_0["pod"]; // "d"
  // int data_0_snow = data_0["snow"]; // 0
  // int data_0_precip = data_0["precip"]; // 0

  //! wind
  // float data_0_wind_spd = data_0["wind_spd"]; // 2.24
  // const char* data_0_wind_cdir_full = data_0["wind_cdir_full"]; // "south-southwest"
  // const char* data_0_wind_cdir = data_0["wind_cdir"]; // "SSW"
  // int data_0_wind_dir = data_0["wind_dir"]; // 200

  //! clouds
  // int data_0_clouds = data_0["clouds"]; // 0
  // long data_0_ts = data_0["ts"]; // 1644424560
  // float data_0_solar_rad = data_0["solar_rad"]; // 543.1
  // int data_0_vis = data_0["vis"]; // 5
  // int data_0_h_angle = data_0["h_angle"]; // -18
  wx.uvCurrent = data_0["uv"]; // 5.26672

  //! barometric pressure
  // float data_0_pres = data_0["pres"]; // 1007.1
  //!==>float data_0_slp = data_0["slp"]; // 1017.9

  //! observations
  //!==>int data_0_rh = data_0["rh"]; // 80
  // const char* data_0_ob_time = data_0["ob_time"]; // "2022-02-09 16:36"
  // const char* data_0_weather_icon = data_0_weather["icon"]; // "c01d"
  // int data_0_weather_code = data_0_weather["code"]; // 800
  // const char* data_0_weather_description = data_0_weather["description"]; // "Clear sky"
  // const char* data_0_datetime = data_0["datetime"]; // "2022-02-09:16"
  // const char* data_0_station = data_0["station"]; // "C3766"
  // float data_0_dni = data_0["dni"]; // 827.57
  // float data_0_ghi = data_0["ghi"]; // 543.14
  // float data_0_dhi = data_0["dhi"]; // 100.49
  // int data_0_aqi = data_0["aqi"]; // 72
  // float data_0_elev_angle = data_0["elev_angle"]; // 32.88
  // int count = doc["count"]; // 1

  //! almanac
  // const char* data_0_state_code = data_0["state_code"]; // "VA"
  // const char* data_0_city_name = data_0["city_name"]; // "Centreville"
  // const char* data_0_country_code = data_0["country_code"]; // "US"
  // float data_0_lat = data_0["lat"]; // 38.8448
  // float data_0_lon = data_0["lon"]; // -77.467
  //!==>const char* data_0_timezone = data_0["timezone"]; // "America/New_York"
  // const char* data_0_sunset = data_0["sunset"]; // "22:40"
  // const char* data_0_sunrise = data_0["sunrise"]; // "12:06"
}

void parseWXforecast(String json)
{
  // zero out all values to account for empty elements
  wx.clouds = 50;
  wx.visibility = 100;
  wx.windSpeed = 0;
  wx.windGust = 0;
  wx.windDirection = "N";
  wx.sunRise = 0;
  wx.sunSet = 0;
  wx.lunation = 0;
  wx.uvForecast = 0;
  wx.rain = 0;
  wx.snowDepth = 0;
  wx.precipProb = 0;
  wx.wxCode = 0;
  wx.tempCdayHigh = 20;
  wx.tempCnightLow = 20;
  wx.tempCdew = 20;

  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, json);

  if (error)
  {
    DEBUG_PRINT(F("deserialize Forecast failed: "));
    DEBUG_PRINTLN(error.f_str());
    return;
  }
  JsonObject data_0 = doc["data"][0];
  JsonObject data_0_weather = data_0["weather"];

  wx.clouds = data_0["clouds"];      // 45
  wx.visibility = data_0["vis"];     // km
  wx.windSpeed = data_0["wind_spd"]; // 3.09
  wx.windGust = data_0["wind_gust_spd"];
  wx.windDirection = (String)data_0["wind_cdir"]; // "NW"
  wx.sunRise = data_0["sunrise_ts"];              // unix UTC
  wx.sunSet = data_0["sunset_ts"];                // unix UTC
  wx.lunation = data_0["moon_phase_lunation"];
  wx.uvForecast = data_0["uv"]; // 6.2195 MAXIMUM
  wx.rain = data_0["precip"];   // 0
  wx.snowDepth = data_0["snow"];
  wx.precipProb = data_0["pop"];         // 0 probabilty of precipitation
  wx.wxCode = data_0_weather["code"];    // 801
  wx.tempCdayHigh = data_0["high_temp"]; // 3
  wx.tempCnightLow = data_0["low_temp"]; // -6.3
  wx.tempCfeelHigh = data_0["app_max_temp"];
  wx.tempCfeelLow = data_0["app_min_temp"];

  // These are the elements available from forecast/daily for reference.
  // Elements marked with ==> are used in this program.

  //! temperatures
  //  int data_0_temp = data_0["temp"]; // -2 AVERAGE
  //  ==>int data_0_high_temp = data_0["high_temp"]; // 3 daytime
  //  ==>float data_0_low_temp = data_0["low_temp"]; // -6.3 nighttime
  //  ==>int data_0_dewpt = data_0["dewpt"]; // -7
  //  float data_0_min_temp = data_0["min_temp"]; // -5.8
  //  ==>float data_0_app_max_temp = data_0["app_max_temp"]; // -0.5
  //  ==>float data_0_app_min_temp = data_0["app_min_temp"]; // -12.2
  //  int data_0_max_temp = data_0["max_temp"]; // 3

  //! precipitation
  //  ==>int data_0_pop = data_0["pop"]; // 0 probabilty of precipitation
  //  float data_0_snow_depth = data_0["snow_depth"]; // 3.6
  //  ==>int data_0_snow = data_0["snow"]; // 0
  //  ==>int data_0_precip = data_0["precip"]; // 0
  //
  //! wind
  //  ==>const char* data_0_wind_cdir = data_0["wind_cdir"]; // "ENE"
  //  ==>float data_0_wind_spd = data_0["wind_spd"]; // 2.6
  //  ==>float data_0_wind_gust_spd = data_0["wind_gust_spd"]; // 5.1
  //  const char* data_0_wind_cdir_full = data_0["wind_cdir_full"]; // "east-northeast"
  //  int data_0_wind_dir = data_0["wind_dir"]; // 66
  //
  //! clouds
  //  ==>int data_0_clouds = data_0["clouds"]; // 42
  //  int data_0_clouds_hi = data_0["clouds_hi"]; // 1
  //  int data_0_clouds_mid = data_0["clouds_mid"]; // 37
  //  int data_0_clouds_low = data_0["clouds_low"]; // 9
  //  ==>float data_0_vis = data_0["vis"]; // 25.209
  //  ==>float data_0_uv = data_0["uv"]; // 0.2
  //
  //! barometric pressure
  //  float data_0_pres = data_0["pres"]; // 1011.6
  //  float data_0_slp = data_0["slp"]; // 1025.3
  //
  //! observation
  //  int data_0_rh = data_0["rh"]; // 68
  //  float data_0_ozone = data_0["ozone"]; // 334.1
  //  const char* data_0_weather_icon = data_0_weather["icon"]; // "c03d"
  //  ==>int data_0_weather_code = data_0_weather["code"]; // 803
  //  const char* data_0_weather_description = data_0_weather["description"]; // "Broken clouds"
  //
  //! almanac
  //  ==>long data_0_sunrise_ts = data_0["sunrise_ts"]; // 1643631415
  //  ==>long data_0_sunset_ts = data_0["sunset_ts"]; // 1643668293 UTC
  //  long data_0_moonrise_ts = data_0["moonrise_ts"]; // 1643631155 UTC
  //  long data_0_moonset_ts = data_0["moonset_ts"]; // 1643670319
  //  float data_0_moon_phase = data_0["moon_phase"]; // 0.00943295
  //  ==>float data_0_moon_phase_lunation = data_0["moon_phase_lunation"]; // 0.99

  //! dates & times
  //  long data_0_ts = data_0["ts"]; // 1643612460
  //  const char* data_0_valid_date = data_0["valid_date"]; // "2022-01-31"
  //  const char* data_0_datetime = data_0["datetime"]; // "2022-01-31"
  //  const char* city_name = doc["city_name"]; // "Centreville"
  //  const char* lon = doc["lon"]; // "-77.42888"
  //  const char* timezone = doc["timezone"]; // "America/New_York"
  //  const char* lat = doc["lat"]; // "38.84039"
  //  const char* country_code = doc["country_code"]; // "US"
  //  const char* state_code = doc["state_code"]; // "VA"
} // parseWXforecast()

String getAPI(String getQuery)
{
  DEBUG_PRINT("Start http: ");
  DEBUG_PRINTLN(myTZ.dateTime("H~:i~:s"));
  // WiFiClient client;
  HTTPClient http;
  String payload = "";
  if (http.begin(client, getQuery))
  {
    DEBUG_PRINTLN(getQuery);
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0)
    {
      DEBUG_PRINT("GET code: ");
      DEBUG_PRINTLN(httpCode);
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
      {
        payload = http.getString();
        DEBUG_PRINTLN("Payload: ");
        DEBUG_PRINTLN(payload);
      }
    }
    else
    {
      DEBUG_PRINT("GET error: ");
      DEBUG_PRINTLN(http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  else
  {
    DEBUG_PRINTLN("[HTTP] Unable to connect");
  }
  DEBUG_PRINT("End http: ");
  DEBUG_PRINTLN(myTZ.dateTime("H~:i~:s"));
  return payload;
} // getAPI()

/*
******************************************************
*********** Weather Observation Codes ****************
******************************************************
*/
String wxObservation(int wxCode)
{
  // translate the Open Weather Map weather ID into text
  // https://openweathermap.org/weather-conditions
  // many codes are shared with WeatherBit.io
  // https://www.weatherbit.io/api/codes
  // returns a short string ( <=10 characters ) to fit on display

  String obs = "";
  if (wxCode >= 200 && wxCode <= 233)
  {
    obs = "Thundrstrm";
  }
  else if (wxCode >= 310 && wxCode <= 321)
  {
    obs = "Drizzle";
  }
  else if (wxCode == 503 || wxCode == 504)
  { // OWM only
    obs = "Extrm Rain";
  }
  else if (wxCode >= 520 && wxCode <= 531)
  { // 531 OWM only
    obs = "Rain Shwrs";
  }
  else if (wxCode == 600 || wxCode == 620)
  {
    obs = "Light Snow";
  }
  else if (wxCode == 601 || wxCode == 602 || wxCode == 622)
  {
    obs = "Heavy Snow";
  }
  else if (wxCode == 621)
  {
    obs = "Snow Showr";
  }
  else if (wxCode >= 611 && wxCode <= 613)
  {
    obs = "Sleet";
  }
  else if (wxCode == 610 || wxCode == 615 || wxCode == 616)
  {
    obs = "Rain&Snow";
  }
  else if (wxCode == 700 || wxCode == 701)
  {
    obs = "Mist";
  }
  else
  {
    switch (wxCode)
    {
    case 300:
      obs = "Lte Drzzle";
      break;
    case 301:
      obs = "Drizzle";
      break;
    case 302:
      obs = "Hvy Drzzle";
      break;
    case 500:
      obs = "Light Rain";
      break; // WB only
    case 501:
      obs = "Modrt Rain";
      break;
    case 502:
      obs = "Heavy Rain";
      break;
    case 511:
      obs = "Frzng Rain";
      break;
    case 623:
      obs = "Flurries";
      break; // WB only
    case 711:
      obs = "Smoke";
      break;
    case 721:
      obs = "Haze";
      break;
    case 731:
      obs = "Sand/Dust";
      break;
    case 741:
      obs = "Fog";
      break;
    case 751:
      obs = "Frzng Fog";
      break; // OWM = sand
    case 761:
      obs = "Dust";
      break;
    case 762:
      obs = "Volcnc Ash";
      break; // OWM
    case 771:
      obs = "Squalls";
      break; // OWM
    case 781:
      obs = "Tornado";
      break; // OWM
    case 800:
      obs = "Clear Sky";
      break;
    case 801:
      obs = "Few Clouds";
      break;
    case 802:
      obs = "Sctrd Clds";
      break;
    case 803:
      obs = "Brokn Clds";
      break;
    case 804:
      obs = "Overcast";
      break;
    case 900:
      obs = "Ukn Precip";
    default:
      obs = "Undefined";
      break;
    }
  }
  return obs;
} // wxObservation()

/*
******************************************************
******************* Moon Phase ***********************
******************************************************
*/
float moonPhase()
{
  // adapted by K. W. Berger from
  // https://minkukel.com/en/various/calculating-moon-phase/
  // two decimal precision is sufficient as lunar month is variable
  // https://en.wikipedia.org/wiki/Lunar_phase
  unsigned long unixTime = now();                         // UTC unix time
  float lunarDays = 29.53059;                             // average days between new moons (synodic)
  unsigned int lunarSecs = lunarDays * 24 * 60 * 60;      // seconds between new moons
  unsigned int new2000 = 947182440;                       // new moon January 6, 2000 18:14 UTC
  unsigned int totalSecs = unixTime - new2000;            // seconds since January 2000 new moon
  unsigned int currentSecs = fmod(totalSecs, lunarSecs);  // seconds since most recent new moon
  float fraction = (float)currentSecs / (float)lunarSecs; // fraction of lunation 0=new, 0.25=1Q, 0.5 is full, 1=new

  return fraction;
} // moonPhase()

/*
******************************************************
****************** drawMoonFace **********************
******************************************************
*/
void drawMoonFace(int xc, int yc, int r, float fract, int liteColor, int darkColor)
{
  // this has been simplified for the dark face having the same
  // color as the background. The code for teh more general case
  // has been commented out

  int face; // type of face depending phase fraction
  float f;  // multiplier for ellipse width
            // int leftColor, rghtColor; // colors for left and right lines

  if (fract >= 0 && fract <= 0.25)
  {
    face = 1; // waxing crescent, new to first quarter
    f = -4 * fract + 1;
    // leftColor = darkColor;
    // rghtColor = liteColor;
  }
  else if (fract > 0.25 && fract <= 0.5)
  {
    face = 2; // waxing gibbous, first quarter to full
    f = 4 * fract - 1;
    // leftColor = darkColor;
    // rghtColor = liteColor;
  }
  else if (fract > 0.5 && fract <= 0.75)
  {
    face = 3; // waning gibbous, full to last quarter
    f = -4 * fract + 3;
    // leftColor = liteColor;
    // rghtColor = darkColor;
  }
  else
  {
    face = 4; // waning crescent, last quarter to new
    f = 4 * fract - 3;
    // leftColor = liteColor;
    // rghtColor = darkColor;
  }

  int hh = r * r;              // height squared
  int wwe = (f * r) * (f * r); // ellipse half-width squared
  int wwm = r * r;             // moon half-width squared
  int hhwwe = hh * wwe;        // for ellipse
  int hhwwm = hh * wwm;        // for moon
  int x0e = f * r;             // half-width of ellipse starting value
  int x0m = r;                 // half width of moon disk starting value
  int dxe = 0;                 // ellipse x value
  int dxm = 0;                 // moon x value

  // calculate x values for circle and ellipse
  for (int y = 0; y <= r; y++)
  {
    // for ellipse
    int x1e = x0e - (dxe - 1);
    while (x1e > 0)
    {
      if (x1e * x1e * hh + y * y * wwe <= hhwwe)
      {
        break;
      }
      x1e--;
    }
    dxe = x0e - x1e;
    x0e = x1e;

    // for circle
    int x1m = x0m - (dxm - 1); // for circle
    while (x1m > 0)
    {
      if (x1m * x1m * hh + y * y * wwm <= hhwwm)
      {
        break;
      }
      x1m--;
    }
    dxm = x0m - x1m;
    x0m = x1m;

    // draw graphic moon
    int xl = xc - x0m; // left end of left line
    int xr = xc + x0m; // right end of right line
    int xp;            // point between left and right lines

    // draw only the lit lines assuming dark face is
    // the same as the background
    switch (face)
    {
    case 1: // waxing crescent - new to first quarter
      xp = xc + x0e;
      tft.drawFastHLine(xp, yc - y, xr - xp, liteColor);
      tft.drawFastHLine(xp, yc + y, xr - xp, liteColor);
      break;
    case 2: // waxing gibbous  - first quarter to full
      xp = xc - x0e;
      tft.drawFastHLine(xp, yc - y, xr - xp, liteColor);
      tft.drawFastHLine(xp, yc + y, xr - xp, liteColor);
      break;
    case 3: // waning gibbous  - full to last quarter
      xp = xc + x0e;
      tft.drawFastHLine(xl, yc - y, xp - xl, liteColor);
      tft.drawFastHLine(xl, yc + y, xp - xl, liteColor);
      break;
    case 4: // waning crescent - last quarter to new
      xp = xc - x0e;
      tft.drawFastHLine(xl, yc - y, xp - xl, liteColor);
      tft.drawFastHLine(xl, yc + y, xp - xl, liteColor);
      break;
    }
    // draw abutting lines above and below x-axis
    // dark face is different than background
    // if the dark color is not the same as the background
    // tft.drawFastHLine(xl, yc - y, xp - xl, leftColor);
    // tft.drawFastHLine(xp, yc - y, xr - xp, rghtColor);
    // tft.drawFastHLine(xl, yc + y, xp - xl, leftColor);
    // tft.drawFastHLine(xp, yc + y, xr - xp, rghtColor);
  }
} // drawMoonFace()

/*
******************************************************
**************** UNIT CONVERSIONS ********************
******************************************************
*/

float CtoF(float tempC) // convert celsius to fahrenheit
{
  return 1.8 * tempC + 32;
} // CtoF()

float MStoMPH(float ms) // convert meters per second to miles per hour
{
  return 2.23694 * ms;
} // MStoMPH()

float MMtoIN(float mm) // convert millimeters to inches
{
  return mm / 25.4;
} // MMtoIN()

float KMtoMILES(float km) // convert kilometers to miles
{
  return km * 0.621371;
} // KMtoMILES()

float DEGtoRAD(float deg) // convert degrees to radians
{
  return 0.0174532925 * deg;
} // DEGtoRAD()

float HPAtoINHG(float hpa) // convert hectoPascal (millibar) to inches of mercury
{
  return 0.02953 * hpa;
} // HPAtoINHG()

/*
******************************************************
**************** checkOTAupdate **********************
******************************************************
*/
void checkOTAupdate()
{
  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  client->setInsecure(); // doesn't need fingerprint!!!!

  HTTPClient https;

  if (https.begin(*client, fwVersionURL))
  {
    // start connection and send HTTP header
    int httpCode = https.GET();
    if (httpCode > 0)
    {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
      {
        String newFWVersion = https.getString();
        int newVersion = newFWVersion.toInt();
        DEBUG_PRINT(F("Using version: "));
        DEBUG_PRINTLN(FW_VERSION);
        DEBUG_PRINT(F("Found version: "));
        DEBUG_PRINTLN(newVersion);
        if (newVersion > FW_VERSION)
        {
          otaUpdateScreen(newVersion);
          ESPhttpUpdate.update(*client, fwImageURL); // must be *client
          // if OTA is sucessful processor will reboot here
        }
      }
    }
    https.end();
  }
} // checkOTAupdate()

/*
******************************************************
******************* initSensor ***********************
******************************************************
*/
int initSensor(void)
{
  // detect sensor type and initialize
  int type = SENSOR_NONE;
  if (aht.begin()) // found AHT10 sensor if true
  {
    type = SENSOR_AHT10;
  }
  else
  {
    dht.begin(); // must be a DHT11 if AHT10 not found
    type = (isnan(dht.readTemperature()) || isnan(dht.readHumidity())) ? SENSOR_NONE : SENSOR_DHT11;
  }
  return type;
} // initSensor()

/*
******************************************************
******************* readSensor ***********************
******************************************************
*/
void readSensor(int sensorType)
{
  switch (sensorType)
  {
  case SENSOR_NONE:
    break;
  case SENSOR_DHT11:
    indoor.tempC = dht.readTemperature();
    indoor.humid = dht.readHumidity();
    break;
  case SENSOR_AHT10:
    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);
    indoor.tempC = temp.temperature;
    indoor.humid = humidity.relative_humidity;
    break;
  default:
    break;
  }
} // readSensor()
