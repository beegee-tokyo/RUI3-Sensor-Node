# RUI3-Sensor-Node
RUI3 example code for WisBlock I2C sensor modules.

This code is _**Work in progress**_ and far from finished. But it can give a first start how to use RUI3.     

The code compiles on both RAK4631 and RAK3172 and I test it on both modules.

# Content
- [Hardware supported](#hardware_supported)
- [Software used](#software_used)
- [Packet data format](#packet_data_format)


----

# Hardware supported
| Module | Function | Supported |
| --     | --       | --          |
| [RAK4631-R](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK4631-R/Overview/) ⤴️ | WisBlock Core module with RUI3| ✔ |
| [RAK3372](https://docs.rakwireless.com/Product-Categories/WisDuo/RAK3172-Evaluation-Board/Overview) ⤴️ | WisDuo RAK3172 Evaluation Board with RUI3| ✔ |
| [RAK5005-O](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK5005-O/Overview/) ⤴️ | WisBlock Base board | ✔ |
| [RAK19003](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK19003/Overview/) ⤴️ | WisBlock Mini Base board | ✔ |
| [RAK19001](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK19001/Overview/) ⤴️ | WisBlock Fullsize Base board | ✔ |
| [RAK1901](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK1901/Overview/) ⤴️ | WisBlock Temperature and Humidty Sensor | ✔ |
| [RAK1902](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK1902/Overview/) ⤴️ | WisBlock Barometer Pressure Sensor | ✔ |
| [RAK1903](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK1903/Overview/) ⤴️ | WisBlock Ambient Light Sensor | ✔ |
| [RAK1904](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK1904/Overview/) ⤴️ | WisBlock Acceleration Sensor (used for GNSS solutions) | ✔ |
| [~~RAK1905~~](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK1905/Overview/) ⤴️ | WisBlock 9 DOF sensor | Work in progress |
| [RAK1906](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK1906/Overview/) ⤴️ | WisBlock Environment Sensor | ✔ |
| [~~RAK1910~~](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK1910/Overview/) ⤴️ | WisBlock GNSS Sensor | Work in progress |
| [RAK1921](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK1921/Overview/) ⤴️ | WisBlock OLED display | ✔ |
| [~~RAK12002~~](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK12002/Overview/) ⤴️ | WisBlock RTC module | Work in progress |
| [RAK12003](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK12003/Overview/) ⤴️ | WisBlock FIR sensor | ✔ |
| [~~RAK12004~~](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK12004/Overview/) ⤴️ | WisBlock MQ2 Gas sensor | Work in progress |
| [~~RAK12008~~](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK12008/Overview/) ⤴️ | WisBlock MG812 CO2 Gas sensor | Work in progress |
| [~~RAK12009~~](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK12009/Overview/) ⤴️ | WisBlock MQ3 Alcohol Gas sensor | Work in progress |
| [RAK12010](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK12010/Overview/) ⤴️ | WisBlock Ambient Light sensor | ✔ |
| [~~RAK12014~~](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK12014/Overview/) ⤴️ | WisBlock Laser ToF sensor | Work in progress |
| [~~RAK12019~~](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK12019/Overview/) ⤴️ | WisBlock UV Light sensor | Work in progress |
| [~~RAK12025~~](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK12025/Overview/) ⤴️ | WisBlock Gyroscope sensor | Work in progress |
| [~~RAK12023/RAK12035~~](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK12023/Overview/) ⤴️ | WisBlock Soil Moisture and Temperature sensor | Work in progress |
| [RAK12037](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK12037/Overview/) ⤴️ | WisBlock CO2 sensor | ✔ |
| [RAK12047](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK12047/Overview/) ⤴️ | WisBlock VOC sensor | ✔ |
| [~~RAK12500~~](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK12500/Overview/) ⤴️ | WisBlock GNSS Sensor | Work in progress |
| [~~RAK14002~~](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK14002/Overview/) ⤴️ | WisBlock 3 button touch pad | Work in progress |
| [~~RAK14003~~](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK14003/Overview/) ⤴️ | WisBlock LED bar display | Work in progress |
| [~~RAK14008~~](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK14008/Overview/) ⤴️ | WisBlock Gesture sensor | Work in progress |
| [~~RAK16000~~](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK16000/Overview/) ⤴️ | WisBlock DC current sensor | Work in progress |

## _LIMITATIONS_     
- The RAK1904 module _**MUST**_ be installed in     
    - Slot C of RAK5005-O, RAK19007 or RAK19001
    - Slot A of RAK19003
- The RAK1905 module _**MUST**_ be installed in     
    - Slot C of RAK5005-O, RAK19007 or RAK19001
    - Slot A of RAK19003
- RAK1910 and RAK12500 cannot be used together (both are GNSS location trackers)

----

# Software used
## IDE
- [ArduinoIDE](https://www.arduino.cc/en/software) ⤴️
- [RAK-STM32-RUI](https://github.com/RAKWireless/RAKwireless-Arduino-BSP-Index) ⤴️
- [RAK-nRF52-RUI](https://github.com/RAKWireless/RAKwireless-Arduino-BSP-Index) ⤴️
## LoRaWAN payload creation
- [CayenneLPP](https://registry.platformio.org/libraries/sabas1080/CayenneLPP) ⤴️
- [ArduinoJson](https://registry.platformio.org/libraries/bblanchon/ArduinoJson) ⤴️
## Sensor libraries
- [Adafruit BME680 Library](https://registry.platformio.org/libraries/adafruit/Adafruit%20BME680%20Library) ⤴️
- [Adafruit LIS3DH](https://registry.platformio.org/libraries/adafruit/Adafruit%20LIS3DH) ⤴️
- [Adafruit LPS2X](https://registry.platformio.org/libraries/adafruit/Adafruit%20LPS2X) ⤴️
- [Adafruit MCP23017 Arduino Library](https://registry.platformio.org/libraries/adafruit/Adafruit%20MCP23017%20Arduino%20Library) ⤴️
- [Adafruit Unified Sensor](https://registry.platformio.org/libraries/adafruit/Adafruit%20Unified%20Sensor) ⤴️
- [ClosedCube OPT3001](https://github.com/beegee-tokyo/ClosedCube_OPT3001_Arduino) ⤴️
- [Coulomb Counter](https://registry.platformio.org/libraries/seeed-studio/Grove%20-%20Coulomb%20Counter%20for%203.3V%20to%205V%20LTC2941) ⤴️
- [INA219_WE](https://registry.platformio.org/libraries/wollewald/INA219_WE) ⤴️
- [Melopero RV3028](https://registry.platformio.org/libraries/melopero/Melopero%20RV3028) ⤴️
- [nRF52_OLED](https://registry.platformio.org/libraries/beegee-tokyo/nRF52_OLED) ⤴️
- [Pololu VL53L0X](https://registry.platformio.org/libraries/pololu/VL53L0X) ⤴️
- [RAK12019_LTR390_UV_Light](https://registry.platformio.org/libraries/beegee-tokyo/RAK12019_LTR390_UV_Light) ⤴️
- [RAK12035_SoilMoisture](https://registry.platformio.org/libraries/beegee-tokyo/RAK12035_SoilMoisture) ⤴️
- [RAKwireless CAP1293 Touch Pad library](https://registry.platformio.org/libraries/beegee-tokyo/Arduino%20CAP1293%20Touch%20Pad%20library) ⤴️
- [RAK I3G4250D Gyroscope](https://registry.platformio.org/libraries/beegee-tokyo/RAK%20I3G4250D%20Gyroscope) ⤴️
- [RAKwireless MQx library](https://registry.platformio.org/libraries/rakwireless/RAKwireless%20MQx%20library) ⤴️
- [RAKwireless VEML Light Sensor](https://registry.platformio.org/libraries/rakwireless/RAKwireless%20VEML%20Light%20Sensor) ⤴️
- [RevEng PAJ7620](https://registry.platformio.org/libraries/acrandal/RevEng%20PAJ7620) ⤴️
- [Sensirion Core](https://registry.platformio.org/libraries/sensirion/Sensirion%20Core) ⤴️
- [Sensirion Gas Index Algorithm](https://registry.platformio.org/libraries/sensirion/Sensirion%20Gas%20Index%20Algorithm) ⤴️
- [Sensirion I2C SGP40](https://registry.platformio.org/libraries/sensirion/Sensirion%20I2C%20SGP40) ⤴️
- [SparkFun SHTC3 Humidity and Temperature Sensor Library](https://registry.platformio.org/libraries/sparkfun/SparkFun%20SHTC3%20Humidity%20and%20Temperature%20Sensor%20Library) ⤴️
- [SparkFun u-blox GNSS Arduino Library](https://registry.platformio.org/libraries/sparkfun/SparkFun%20u-blox%20GNSS%20Arduino%20Library) ⤴️
- [TinyGPSPlus](https://registry.platformio.org/libraries/mikalhart/TinyGPSPlus) ⤴️
- [LPS35HW](https://registry.platformio.org/libraries/pilotak/LPS35HW) ⤴️
- [SparkFun SCD30 Arduino Library](https://registry.platformio.org/libraries/sparkfun/SparkFun%20SCD30%20Arduino%20Library) ⤴️
- [SparkFun MLX90632 Noncontact Infrared Temperature Sensor](https://registry.platformio.org/libraries/sparkfun/SparkFun%20MLX90632%20Noncontact%20Infrared%20Temperature%20Sensor) ⤴️
- [Melopero AMG8833](https://registry.platformio.org/libraries/melopero/Melopero%20AMG8833) ⤴️
- [SparkFun ADXL313 Arduino Library](https://registry.platformio.org/libraries/sparkfun/SparkFun%20ADXL313%20Arduino%20Library) ⤴️
- [RAKwireless Storage](https://registry.platformio.org/libraries/beegee-tokyo/RAKwireless%20Storage) ⤴️
- [ArduinoECCX08](https://registry.platformio.org/libraries/arduino-libraries/ArduinoECCX08) ⤴️

----

# Packet data format
The packet data is made compatible with the extended Cayenne LPP encoding from [ElectronicCats/CayenneLPP](https://github.com/ElectronicCats/CayenneLPP) ⤴️.    
The content of the packet depends on the modules installed on the WisBlock Base Board:     

| Data                     | Channel # | Channel ID | Length   | Comment                                           | Required Module   | Decoded Field Name |
| --                       | --        | --         | --       | --                                                | --                | --                 |
| Battery value            | 1         | _**116**_  | 2 bytes  | 0.01 V Unsigned MSB                               | RAK4631           | voltage_1          |
| Humidity                 | 2         | 104        | 1 bytes  | in %RH                                            | RAK1901           | humidity_2         |
| Temperature              | 3         | 103        | 2 bytes  | in °C                                             | RAK1901           | temperature_3      | 
| Barometric Pressure      | 4         | 115        | 2 bytes  | in hPa (mBar)                                     | RAK1902           | barometer_4        |
| Illuminance              | 5         | 101        | 2 bytes  | 1 lux unsigned                                    | RAK1903           | illuminance_5      |
| Humidity 2               | 6         | 104        | 1 bytes  | in %RH                                            | RAK1906           | humidity_6         |
| Temperature 2            | 7         | 103        | 2 bytes  | in °C                                             | RAK1906           | temperature_7      | 
| Barometric Pressure 2    | 8         | 115        | 2 bytes  | in hPa (mBar)                                     | RAK1906           | barometer_8        |
| Gas Resistance 2         | 9         | 2          | 2 bytes  | 0.01 signed (kOhm)                                | RAK1906           | analog_9           |
| GNSS stand. resolution   | 10        | 136        | 9 bytes  | 3 byte lon/lat 0.0001 °, 3 bytes alt 0.01 meter   | RAK1910, RAK12500 | gps_10             |
| GNSS enhanced resolution | 10        | _**137**_  | 11 bytes | 4 byte lon/lat 0.000001 °, 3 bytes alt 0.01 meter | RAK1910, RAK12500 | gps_10             |
| Soil Temperature         | 11        | 103        | 2 bytes  | in °C                                             | RAK12023/RAK12035 | temperature_11     |
| Soil Humidity            | 12        | 104        | 1 bytes  | in %RH                                            | RAK12023/RAK12035 | humidity_12        |
| Soil Humidity Raw        | 13        | 2          | 2 bytes  | 0.01 signed                                       | RAK12023/RAK12035 | analog_in_13       |
| Soil Data Valid          | 14        | 102        | 1 bytes  | bool                                              | RAK12023/RAK12035 | presence_14        |
| Illuminance 2            | 15        | 101        | 2 bytes  | 1 lux unsigned                                    | RAK12010          | illuminance_15     |
| VOC                      | 16        | _**138**_  | 2 bytes  | VOC index                                         | RAK12047          | voc_16             |
| MQ2 Gas                  | 17        | 2          | 2 bytes  | 0.01 signed                                       | RAK12004          | analog_in_17       |
| MQ2 Gas Percentage       | 18        | _**120**_  | 1 bytes  | 1-100% unsigned                                   | RAK12004          | percentage_18      |
| MG812 Gas                | 19        | 2          | 2 bytes  | 0.01 signed                                       | RAK12008          | analog_in_19       |
| MG812 Gas Percentage     | 20        | _**120**_  | 1 bytes  | 1-100% unsigned                                   | RAK12008          | percentage_20      |
| MQ3 Alcohol Gas          | 21        | 2          | 2 bytes  | 0.01 signed                                       | RAK12009          | analog_in_21       |
| MQ3 Alcohol Gas Perc.    | 22        | _**120**_  | 1 bytes  | 1-100% unsigned                                   | RAK12009          | percentage_22      |
| ToF distance             | 23        | 2          | 2 bytes  | 0.01 signed                                       | RAK12014          | analog_in_23       |
| ToF Data Valid           | 24        | 102        | 1 bytes  | bool                                              | RAK12014          | presence_24        |
| Gyro triggered           | 25        | _**134**_  | 6 bytes  | 2 bytes per axis, 0.01 °/s                        | RAK12025          | gyrometer_25       |
| Gesture detected         | 26        | 0          | 1 byte   | 1 byte with id of gesture                         | RAK14008          | digital_in_26      |
| LTR390 UVI value         | 27        | 2          | 2 byte   | 0.01 signed                                       | RAK12019          | analog_in_27       | 
| LTR390 UVS value         | 28        | 101        | 2 bytes  | 1 lux unsigned                                    | RAK12019          | illuminance_28     | 
| INA219 Current           | 29        | 2          | 2 byte   | 0.01 signed                                       | RAK16000          | analog_29          | 
| INA219 Voltage           | 30        | 2          | 2 byte   | 0.01 signed                                       | RAK16000          | analog_30          | 
| INA219 Power             | 31        | 2          | 2 byte   | 0.01 signed                                       | RAK16000          | analog_31          | 
| Touchpad left            | 32        | 102        | 1 bytes  | bool                                              | RAK14002          | presence_32        | 
| Touchpad middle          | 33        | 102        | 1 bytes  | bool                                              | RAK14002          | presence_33        | 
| Touchpad right           | 34        | 102        | 1 bytes  | bool                                              | RAK14002          | presence_34        | 
| SCD30 CO2 concentration  | 35        | 125        | 2 bytes  | 1 ppm unsigned                                    | RAK12037          | 
| SCD30 temperature        | 36        | 103        | 2 bytes  | in °C                                             | RAK12037          | 
| SCD30 humidity           | 37        | 104        | 1 bytes  | in %RH                                            | RAK12037          | 
| MLX90632 sensor temp     | 38        | 103        | 2 bytes  | in °C                                             | RAK12003          |
| MLX90632 object temp     | 39        | 103        | 2 bytes  | in °C                                             | RAK12003          |

### _REMARK_
Channel ID's in cursive are extended format and not supported by standard Cayenne LPP data decoders.

Example decoders for TTN, Chirpstack, Helium and Datacake can be found in the folder [decoders](./decoders) ⤴️
