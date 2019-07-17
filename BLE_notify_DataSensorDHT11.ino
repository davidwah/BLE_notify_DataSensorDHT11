/*
   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   A connect hander associated with the server starts a background task that performs notification
   every couple of seconds.
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <DHT.h>


BLEServer* pServer = NULL;
//BLEServer* pServer;
//BLECharacteristic* pCharacteristic = NULL;
BLECharacteristic* pCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

#define DHTPIN 23 // DHT11 data pin
#define DHTTYPE DHT11 // Tipe sensor yang digunakan

DHT dht (DHTPIN, DHTTYPE);

int humidity;
int temperature;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define DHTDATA_CHAR_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" // UUID data Sensor DHT


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};



void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
    // notify changed value
    if (deviceConnected) {

      // Pembacaan Data Sensor - Edit Disini
      humidity = dht.readHumidity ();
      temperature = dht.readTemperature ();
     // test if return is valid, otherwise something is wrong.
      if (isnan (temperature) || isnan (humidity)) {
        Serial.println ("Failed to read from DHT");
        }
        
        else {
            Serial.print ("Humidity:");
            Serial.print (humidity);
            Serial.print ("% \ t");
            Serial.print ("Temperature:");
            Serial.print (temperature);
            Serial.println ("* C");
        }
        
        char humidityString [2];
        char temperatureString [2];
        dtostrf (humidity, 1, 2, humidityString);
        dtostrf (temperature, 1, 2, temperatureString);

        char dhtDataString [16];
        sprintf (dhtDataString, "% d,% d", temperature, humidity);
    
        pCharacteristic-> setValue (dhtDataString);
    
        pCharacteristic-> notify ();  // Send the value to the application!
        Serial.print ("*** Sent Data:");
        Serial.print (dhtDataString);
        Serial.println ("***");
    }
    delay(500);

    
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
    
}
