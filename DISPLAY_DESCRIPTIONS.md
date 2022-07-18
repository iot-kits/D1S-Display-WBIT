# D1S-Display-WBIT

## Descriptions of Displays

|Screen |Photo (click to enlarge)|Description|
|----|:----:|----|
|**Splash Screen**|<img src="/images/Splash Screen.jpg" width="100px"/>| <ul><li>Abbreviated firmware name </li> <li>Firmware version</li></ul> |
|**Data Screen**|<img src="/images/Data Screen.jpg" width="100px">| <ul><li> Location latitude / logitude </li><li> Time zone abbreviation and Olson name </li> <li>Units (metric/imperial) and screen duration in seconds </li> <li>Other display information</li></ul>|
|**Weather #1** |<img src="/images/First Weather Screen.jpg" width="100px"/> | <ul><li>Brief weather condition forecast – see list of weather conditions </li> <li>Current temperature </li> <li>Temperature forecast: Daytime High / Nighttime low  </li> <li>Probability of precipitation (rain or snow) forecast </li> <li>Accumulated amount of rain or snow forecast </li> <li>Ultraviolet Index maximum forecast – background color changes with level – see list of UV Index</li></ul>| 
|**Weather #2**|<img src="/images/Second Weather Screen.jpg" width="100px"/> | <ul><li>Feels like high and low temperatures </li> <li>Wind direction and speed</li> <li>Wind gust speed </li> <li>Cloud cover in percent </li> <li>Barometric Pressure at Sea Level </li> <li>Relative Humidity in percent</li></ul>| 
|**Almanac**|<img src="/images/Almanac Screen.jpg" width="100px"/> | <ul><li>Day and date</li> <li>Sun Rise local time (am)</li> <li>Sun Set local time (pm) </li> <li>Moon Phase</li></ul> |
|**Digital Clock**|<img src="/images/Time-Temp Screen.jpg" width="100px"/> | <ul><li>Universal Coordinated Time (UTC) </li> <li>Local Time (24-hour) </li> <li>Indoor Temperature & Humidity</li></ul>| 
|**Analog Clock**| | <ul><li>To Be Implemented</li></ul> | 
|**Configuration**|<img src="/images/Config Screen.jpg" width = "100px"/>| <ul><li>Brief instructions for software configuration </li> <li>SSID is D1Sdisplay </li> <li>Portal URL is 192.168.4.1 </li></ul>|
|**Firmware Update**|<img src="/images/Update Screen.jpg" width="100px"/> | <ul><li>Screen appears when Iot Kits has published a firmware update </li> <li>Do not turn off the unit during update </li></ul>|

## Time and Weather Information
The display shows time from the [National Institute of Standards and Technology (NIST)](https://en.wikipedia.org/wiki/National_Institute_of_Standards_and_Technology). The time is automatically adjusted for standard and daylight saving for the [Olson time zone](https://en.wikipedia.org/wiki/Tz_database) based on the display's latitude and longitude. 

Weather information is provided by [weatherbit.io](https://www.weatherbit.io/). Conditions are localized with the display's latitude and longitude. During the day, current conditions are updated every 10 minutes. The forecast is updated every hour. Between midnight and 7 am the updates occur every hour for current conditions and every two hours for the forecast.

## List of Weather Conditions
Reference: [www.weatherbit.io/api/codes](https://www.weatherbit.io/api/codes)

Note that these weather observation codes are very similar to those used by [Open Weather Map](https://openweathermap.org/weather-conditions)

|Screen Display |Condition | WeatherBit code |
|:---:|:---:|----|
|Thundrstrm |Thunderstorm  | 200 - 233 |
|Lte Drzzle |Light Drizzle | 300|
|Drizzle    |Drizzle       | 301, 310 - 321 |
|Hvy Drzzle |Heavy Drizzle | 302|
|Light Rain |Light Rain    | 500|
|Modrt Rain |Moderate rain | 501|
|Heavy Rain |Heavy Rain    | 502|
|Extrm Rain |Extreme Rain  | 503, 504 |
|Frzng Rain |Freezing Rain | 511|
|Rain Shwrs |Rain Showers  | 520 - 531 |
|Light Snow |Light Snow    | 600, 620 |
|Heavy Snow |Heavy Snow    | 601, 602, 622 |
|Rain&Snow  |Mixed Rain and Snow | 610, 615, 616|
|Sleet      |Sleet         | 611, 613|
|Snow Showr |Snow Shower   | 621|
|Flurries   |Flurries      | 623|
|Mist       |Mist          | 700, 701|
|Smoke      |Smoke         | 711|
|Haze       |Haze          | 721|
|Sand/Dust  |Sand/Dust     | 731|
|Frzng Fog  |Freezing Fog  | 751|
|Dust       |Dust          | 761|
|Clear Sky  |Clear Sky     | 800|
|Few Clouds |Few Clouds    | 801|
|Sctrd Clds |Scattered Clouds | 802|
|Brkn Clds  |Broken Clouds | 803|
|Overcast   |Overcast Clouds | 804|
|Ukn Precip |Unknown Precipitation | 900|
|Undefined  |Undefined     | Undefined|

## Moon Phases

|Screen Display|Name|Days|
|:----:|:----:|:----:|
|New       |New             |0 - 1|
|Wax Crscnt| Waxing Crescent| 2 - 6|
|First Qrtr| First Quarter  | 7 - 8|
|Wax Gibous| Waxing Gibbous | 9 - 13|
|Full      | Full           |14 - 16|
|Wan Gibous| Waning Gibbous |17 - 21|
|Last Qtr  | Last Quarter   |22 - 23|
|Wan Crscnt| Waning Crescent|24 - 28|
|New       | New            |29|
* The moon goes through a 29.53-day cycle.
* Waxing: The lighted portion is growing. 
* Waning: The lighted portion is shrinking.
* Crescent: The lighted portion is less than half. 
* Gibbous: The lighted portion is more than half.

## Ultraviolet Index
The UV Index is an indication of how quickly your skin will burn when outdoors.
|Screen Display|Risk|Color|Protection|
|:----:|:----:|:----:|---|
|0 to 2 </br>**Low**     |Low        |Green |No protection needed. You can safely stay outside using minimal sun protection.|
|3 to 5 </br>**Mdrte**   |Moderate   |Yellow|Protection needed. Seek shade during late morning through mid-afternoon. When outside, generously apply broad-spectrum SPF-15 or higher sunscreen on exposed skin, and wear protective clothing, a wide-brimmed hat, and sunglasses.|
|6 to 7</br>**High**    |High|Orange|Protection against skin and eye damage is needed. Reduce time in the sun between 10 a.m. and 4 p.m. If outdoors, seek shade and wear sun-protective clothing, a wide-brimmed hat, and UV-blocking sunglasses. Generously apply broad spectrum SPF 15+ sunscreen every 1.5 hours, even on cloudy days, and after swimming or sweating. Bright surfaces, such as sand, water, and snow, will increase UV exposure.|
|8 to 10 </br>**VryHi**  |Very High  |Red   |Extra protection needed. Be careful outside, especially during late morning through mid-afternoon. Seek shade and wear protective clothing, a wide-brimmed hat, and sunglasses, and generously apply a minimum of  SPF-15, broad-spectrum sunscreen on exposed skin.|
|11 or more </br>**Xtrm**|Extreme    |Purple|Take all precautions because unprotected skin and eyes can burn in minutes. Try to avoid sun exposure between 10 a.m. and 4 p.m. If outdoors, seek shade and wear sun-protective clothing, a wide-brimmed hat, and UV-blocking sunglasses. Generously apply broad spectrum SPF 15+ sunscreen every 1.5 hours, even on cloudy days, and after swimming or sweating. Bright surfaces, such as sand, water, and snow, will increase UV exposure.|
