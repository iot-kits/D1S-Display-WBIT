# D1S-Display-WBIT
These instructions are for the Remote Display kit for users without an IoT Kits weather station. The microcontroller is preprogrammed! IF YOU REPROGRAM IT, THESE INSTRUCTIONS WILL NOT WORK.

## Assembly and Configuration Instructions
### Features:
*	Obtains localized weather information from WeatherBit.io
*	Shows current conditions and daily maximum/minimum
*	Includes wind speed and direction, precipitation type, probability and amount, pressure, temperature, humidity, cloud cover
*	Built in sensor for indoor temperature and humidity
*	User can choose metric and imperial units
*	Precise time provided by National Institute of Standards and Technology (NIST)
*	Displays Universal Coordinated Time (UTC) and your local time.
*	Daylight saving time is automatically determined
*	Almanac for local sunrise and sunset times and moon phase
### What you will need:
1.	Open a free WeatherBit account.
    *	Go to https://www.weatherbit.io/. Click the *Sign up* button, choose a username and password. Write them down! After a few minutes WeatherBit will e-mail a link to confirm your credentials.
    * Log in to WeatherBit. Fill out the form on the *Welcome to WeatherBit* page. The objective is to receive a free Application Programming Interface (API) key. For *API Tier* select “Free (Limited) | $0/mo”. For *API Usage Purpose* choose “Personal Use”. 
    *	WeatherBit will send an e-mail with your API key. You can also find your key on the Dashboard page. Record the API key.
    *	NOTE: YOU CANNOT CONFIGURE THE DISPLAY UNLESS YOU HAVE THE API KEY. It may take a few minutes for WeatherBit to provision your account. Wait for it! 
2.	Find your latitude and longitude. 
    * The weather station uses the location only to localize the weather and find the time zone.
    *	Use https://www.latlong.net/ or a similar service.
    *	The coordinates must be in decimal degrees. For example, the coordinates for the Washington Monument are latitude 39.485279, longitude -77.619972. Two decimal places are adequate: 39.49, -77.62.
    *	Note that north latitude is positive, south latitude is negative, east longitude is positive, west longitude is negative.
3.	Tools needed: 
    *	Soldering iron
    *	Rosin-core solder
    *	Diagonal cutters 
    *	Hot-melt glue gun
    *	Standard USB 5-volt adapter or power supply.
    *	Micro USB cable. Dollar store varieties are acceptable.
 
### Assemble the kit.
**[Click on any thumbnail to enlarge the photograph.]**
1. Have an activated WeatherBit account and API key as described above.
2. Lay out the parts on a safe working surface.
<img src="/images/Figure 1 - Parts.jpg" width="25%"/>

**The next few instructons use the preinstalled pins of the display unit as a fixture to align the D1 Mini assembly. Carefully follow these instructions. You will install and remove connectors several times.**

3. Place the two long-tailed sockets on the display unit pins. You will not need the short tail sockets or header pins if the are provided in the kit.
<img src="/images/Figure 2 - headers.jpg" width="25%"/>

4.	Carefully set the D1 Mini onto the pins with the silver case toward the back of the display unit. With the D1 Mini in the correct orientation, you will be soldering on the side with the USB connector.
<img src="/images/Figure 6 - replace D1 Mini.png" width="25%"/>

5. Solder each header pin onto the D1 Mini. Carefully inspect your solder joints. Congratulations, you are done soldering.
6. Remove the D1 Mini from the display unit and set aside.
7.	Remove the protective film from the display screen. Place the display into the case with the row of pins RST, A0, D0… toward the top. Engage the holes in the display board with the molded bosses of the case.
8.	While pressing the display into the case to keep it flush, place a dab of hot melt glue on each boss. Hot melt is a good choice because it is removable. You can use other glues at your risk.
<img src="/images/Figure 4 - glue.png" width="25%"/> 

9.	Cut off selected leads from the D1 Mini with diagonal cutters.   
   __MAKE SURE YOU UNDERSTAND THIS STEP BEFORE YOU CUT!__  
    *	Cut these pins: RST, D0, D5, D7, D8, TX, RX, D3, D4, 5V.  
    *	**DO NOT CUT THESE PINS:** D6, 3V3, G, D1, D2, A0.
    * NOTE: D1, D2, and A0 are not used in this kit. D1 and D2 are for I2C sensors, A0 is for analog input. All are reserved for future use.
<img src="/images/Figure 5 - cut leads.png" width="25%"/>

10.	Replace the D1 Mini onto the back of the display unit. Observe the orientation so that pin RST of the D1 Mini engages with RST on the display board. All the pins must match.  
  __THE CORRECT ORIENTATION IS CRUCIAL!__  
  __DOUBLE CHECK THAT ALL PINS ARE CORRECTLY MATED WITH RST TO RST AND 5V to 5V, ETC.__  
    * The USB connector will line up with the hole on the side of the case.
11. Place the leads of the jumper cable onto the DHT11 sensor pins. Note the wire color for each of the pins: +, OUT, and -. The wire colors are different in each kit. As an optional step, you may place a small blob of hot melt glue over the plastic part of the connectors to maintain their orientation if ever removed.
<img src="/images/Figure 7 - DHT11.png" width="25%"/>  

12. Plug a Micro USB cable onto the D1 Mini USB connector through the opening in the side of the case. Feed the jumper cable through the oval slot in the rear case. Connect the jumper wires to the D1 Mini as shown in the table. If the connector is loose, try rotating it 90 degrees on the pin.

|DHT11 |D1 Mini|Your Wire Color|
|:----:|:-----:|:-----:|
| +    | 3V3   |       |
| OUT  | D6    |       |
| -    | G     |       |

<img src="/images/Figure 8 - Final connections.png" width="25%"/> 

13. Snap the rear cover onto the case. Place a dab of hot melt glue on the underside of the DHT11 sensor and place it on the upper right of the case cover.
<img src="/images/Figure 9 - Case.png" width="25%"/>

14. Apply the nameplate on the rear of the case. The step is optional: Apply the decorative keyboard label under the display.
15. The assembly is complete. The next step is to configure the soaftware.

### Software Configuration
These instructions are for a cell phone with Wi-Fi capability. The same method could be used with a Wi Fi connected computer. 
You must be familiar with how to open the Wi-Fi connection settings on your phone, how to open your phone’s web browser, and how to enter an URL into the browser’s address bar.
This procedure will temporarily turn the weather display into a Wi-Fi access point. You will connect your cell phone to the display unit’s Wi-Fi signal. Then use your cell phone’s Internet browser to open a web page hosted by the weather display. After you enter the configuration information into the web page, the weather display will connect to your Wi-Fi network and begin normal operation.  
**Do not connect the display to a power supply until instructed to do so.**
You will need ALL the accounts and information noted in the section "What you will need" above. Do not proceed unless you have ALL this information. 
1.	Have all the configuration information at hand ready to enter:  
    *	Your Wi-Fi name (SSID) and password
    *	Your WeatherBit API key
    *	Your latitude and longitude in decimal degrees
2.	Plug the USB cable into a standard 5-volt adapter or power supply. The display will show a splash screen then brief instructions for configuring the unit will appear.
3.	Open your phone’s Wi-Fi connection settings. Turn off auoto reconnect on your current Wi-Fi connection. After a brief delay *D1Sdisplay* will appear in the list of available networks. Select it and wait for your phone to connect. You may safely ignore any warning that no Internet is available.
4.	Open your phone’s browser. Enter **192.168.4.1** into the address bar. A web page will appear.
5.	Enter the following information:  
    * Your Wi-Fi SSID (you may select this from the available networks shown at the top of the page)
    *	Your Wi-Fi password
    * Your latitude in decimal degrees. Positive for north latitudes, negative for south latitudes.
    *	Your longitude in decimal degrees. Positive for east longitudes, negative for west longitudes.
    *	Select Imperial or Metric units.
    *	~~Check the Show clock box if you wish to have the unit display local and UTC times.~~
    *	Select the duration of each display screen.
6.	The weather display will save the configuration data, connect to your WI-Fi and begin downloading time and weather information.  This may take a minute or so.
7.	Go back to your phone settings and turn on auto reconnect for your regular Wi-Fi connection.
8.	If you ever need to change any parameter, press the reset button on the D1 Mini twice in succession. You may have to do this a few times. The unit goes into configuration mode when the blue LED on the D1 Mini turns on and the configuration reminder screen appears.

<img src="/images/Portal.jpg" width="20%"/>

## Other Display Screens
|Click to enlarge| | | |
|:----:|:----:|:----:|:----:|
|<img src="/images/Splash Screen.jpg"/>|<img src="/images/Config Screen.jpg"/>|<img src="/images/Update Screen.jpg" />|<img src="/images/Data Screen.jpg">|
|Splash Screen|Configuration Screen|Firmware Update Screen| Data Screen |
|<img src="/images/First Weather Screen.jpg"/>|<img src="/images/Second Weather Screen.jpg"/>|<img src="/images/Almanac Screen.jpg">|<img src="/images/Time-Temp Screen.jpg">|
|First Weather Screen|Second Weather Screen|Almanac Screen| Time & Temperature Screen |
