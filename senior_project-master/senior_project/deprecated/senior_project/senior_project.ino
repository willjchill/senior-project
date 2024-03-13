/*********************************************************************
 This is an example for our nRF51822 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "Adafruit_BLEGatt.h"

#include "Queue.h"

#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

/*=========================================================================
    APPLICATION SETTINGS

    FACTORYRESET_ENABLE       Perform a factory reset when running this sketch
   
                              Enabling this will put your Bluefruit LE module
                              in a 'known good' state and clear any config
                              data set in previous sketches or projects, so
                              running this at least once is a good idea.
   
                              When deploying your project, however, you will
                              want to disable factory reset by setting this
                              value to 0.  If you are making changes to your
                              Bluefruit LE device via AT commands, and those
                              changes aren't persisting across resets, this
                              is the reason why.  Factory reset will erase
                              the non-volatile memory where config data is
                              stored, setting it back to factory default
                              values.
       
                              Some sketches that require you to bond to a
                              central device (HID mouse, keyboard, etc.)
                              won't work at all with this feature enabled
                              since the factory reset will clear all of the
                              bonding data stored on the chip, meaning the
                              central device won't be able to reconnect.
    MINIMUM_FIRMWARE_VERSION  Minimum firmware version to have some new features
    MODE_LED_BEHAVIOUR        LED activity, valid options are
                              "DISABLE" or "MODE" or "BLEUART" or
                              "HWUART"  or "SPI"  or "MANUAL"
    -----------------------------------------------------------------------*/
    #define FACTORYRESET_ENABLE         1
    #define MINIMUM_FIRMWARE_VERSION    "0.6.6"
    #define MODE_LED_BEHAVIOUR          "MODE"
/*=========================================================================*/

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

Adafruit_BLEGatt gatt(ble);

/* The service information */

int32_t emgServiceId;
int32_t emgMeasureCharId;

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
void setup(void)
{
  while (!Serial);  // required for Flora & Micro
  delay(500);

  Serial.begin(115200);

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  ble.verbose(false);  // debug info is a little annoying after this point!

  // SETTING UP CUSTOM GATT SERVICE, CHAR, PROPERTIES, ETC.
  /////////////////////////////////////////////

  Serial.println(F("Ading a EMG Sensor Service (UUID = 0x1809): "));
  emgServiceId = gatt.addService(0x1809);
  if (emgServiceId == 0) {
    error(F("Could not add EMG service"));
  }

  /* Add the EMG Measurement characteristic which is composed of
   * 1 byte flags + 4 float */
  /* Chars ID for Measurement should be 1 */
  Serial.println(F("Adding the EMG Measurement characteristic (UUID = 0x2A1C): "));
  emgMeasureCharId = gatt.addCharacteristic(0x2A1C, GATT_CHARS_PROPERTIES_INDICATE, 8, 8, BLE_DATATYPE_INTEGER);
  if (emgMeasureCharId == 0) {
    error(F("Could not add EMG characteristic"));
  }

  /* Add the EMG Service to the advertising data (needed for Nordic apps to detect the service) */
  Serial.print(F("Adding EMG Service UUID to the advertising payload: "));
  uint8_t advdata[] { 0x02, 0x01, 0x06, 0x05, 0x02, 0x09, 0x18, 0x0a, 0x18 };
  ble.setAdvData( advdata, sizeof(advdata) );

  /* Reset the device for the new service setting changes to take effect */
  Serial.print(F("Performing a SW reset (service changes require a reset): "));
  ble.reset();

  /////////////////////////////////////////////////////


  /* Wait for connection */
  while (! ble.isConnected()) {
      delay(500);
  }

  Serial.println("Connection successful.");
}

/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/

// global variables
unsigned long previousMillis = 0;  // Store the previous time
const long interval = 10;        // Interval at which to send data (in milliseconds)

// stores all the previous sensor values
Queue<String> db_queue(1000);

void loop(void)
{
  /**************************************************************************/
  // Send data
  // We want to send bytearray of voltage read at A0 to python
  /**************************************************************************/
  
  uint8_t temp = random(0, 100);

  Serial.print(F("Updating Temperature value to "));
  Serial.print(temp);
  Serial.println(F(" Fahrenheit"));

  // TODO temperature is not correct due to Bluetooth use IEEE-11073 format
  gatt.setChar(emgMeasureCharId, temp);

  // read char (is it possible?)
  Serial.print("VALUE: "); 
  Serial.println(gatt.getChar(0));
  Serial.println(gatt.getChar(1));

  /* Delay before next measurement update */
  delay(1000);

  // if(db_queue.count() <= 1000) {
  //   voltageRead();
  // }
  // checkRequest();
  // delay(interval);
}

void checkRequest() {
  if (true) {
    String request = ble.readString(); // IF REQUEST IS "OK" --> RESPOND WITH ELEMENT IN QUEUE
    Serial.print(request + "<- Request 1 ");
    ble.print("AT+READCHAR=");
    ble.println("6e400002-b5a3-f393-e0a9-e50e24dcca9e");
    request = ble.readString(); // IF REQUEST IS "OK" --> RESPOND WITH ELEMENT IN QUEUE
    Serial.print(request + "<- Request 2 ");
    if(true) {
      Serial.print("Currently in queue: ");
      Serial.println(db_queue.count());
      if(db_queue.count() > 0) {
        // sending data via BLE (i.e. RESPONSE)
        String msg = db_queue.pop();
        ble.print("AT+BLEUARTTX=");
        ble.println(msg);
        Serial.println(msg);
      }
    }
  }
}

// supporting method to send voltage data from A0
void voltageRead() {
  // Check if it's time to send data
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Update previous time
    
    // Read the voltage at pin A0
    int sensorValue = analogRead(A0);
  
    // Convert the sensor value to voltage (assuming a 3.3V reference voltage)
    float voltage = sensorValue * (3.3 / 1023.0);
  
    // Send the data with voltage as the label and time as the data
    // Serial.print(voltage);
    // Serial.print('\t');  // Use a tab character to separate the values
    // Serial.println(currentMillis);  // Send time as data (plot for debugging)

    // pushing recent data to our local database
    db_queue.push(String(voltage, 2) + "," + String(currentMillis));
  }
}