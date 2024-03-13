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

#include "BluefruitConfig.h"

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

/*=========================================================================
    APPLICATION SETTINGS

    FACTORYRESET_ENABLE     Perform a factory reset when running this sketch
   
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
    -----------------------------------------------------------------------*/
    #define FACTORYRESET_ENABLE        1
    #define MINIMUM_FIRMWARE_VERSION   "0.7.0"
/*=========================================================================*/

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

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
  Serial.println(F("Adafruit Bluefruit EMG Setup"));
  Serial.println(F("-------------------------------------"));

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
  
  if ( !ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    error( F("Callback requires at least 0.7.0") );
  }

  // update service
  ble.reset();

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  Serial.println("Waiting for a connection...");
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
void loop(void)
{  
  voltageRead();
}

// global variable
uint16_t sensorValue = 0;  // stored in 16-bit int
int interval = 1; // in milliseconds
unsigned long previousMillis = 0;

// supporting method to send voltage data from A0
void voltageRead() {
  // Check if it's time to send data
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Update previous time
    
    // Read the voltage at pin A0
    sensorValue = analogRead(A0); 

    ble.print("AT+BLEUARTTX=");

    String hex_string = String(sensorValue, HEX);
    // Pad the string to ensure it's three characters long
    while (hex_string.length() < 3) {
      hex_string = "0" + hex_string;
    }
    ble.println(hex_string);
  }
}
