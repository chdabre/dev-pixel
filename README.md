# dev-pixel
A Small, Web-Controlled 8x8 Matrix LED Display with esp266 &amp; MAX7219

Web Interface: [Live Demo](http://imakethings.ch/pixel)

## Hardware Setup
Tested on WeMos D1 mini with esp8266. Should work on any other esp8266-based board too.

- Connect a MAX72xx based display driver with an 8x8 LED Matrix to the SPI bus on the esp8266.
- Make sure to set the correct CS Pin in the Arduino code.

**Arduino Dependencies**
- [Adafruit_GFX Library](https://github.com/adafruit/Adafruit-GFX-Library)
- [MAX72xxPanel Library](https://github.com/markruys/arduino-Max72xxPanel)
- [WifiManager Library](https://github.com/tzapu/WiFiManager)
