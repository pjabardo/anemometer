# envconds
Temperature, Pressure and Humidity measurement board and software

The objective of this project is to develop a board that reads environmental conditions including
 * Temperature using DS18B20 sensor
 * Pressure using the BMP280 sensor
 * Humidity using the DHT22 sensor

In the future, more sensors could be added.

The board uses a ESP32 microcontroller, in particular the Devkit v1 board with 30 pins. Embeded programming is done using arduino framework.

The folder `kicad` contains the schematics of the board.

For now a simple communication protocol over the serial line is used. In the future MQTT should be used.

To read a value, user should send the command `*XY` where `X` refers to the sensor and `Y` the variable being read. The following variables can be read:

 * `HH` - Humidity from the DHT22
 * `HT` - Temperature from the DHT22
 * `PP` - Atmospheric pressure from the BMP280
 * `PT` - Temperature from the BMP280
 * `Tx' - where x is the temperature sensor.

 To read the atmospheric pressure, the command `*PP` should be sent to the microcontroller. The response is sent as a line of text containing the value as an ASCII string.

