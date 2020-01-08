/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>


#include <iostream>
//#include <algorithm>
//#include <ArduinoJson.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "0000ffe0-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_UUID "0000ffe1-0000-1000-8000-00805f9b34fb"

//#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
//#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
uint32_t value = 0;
using namespace std;
const int potPin = 34;
int adcValue = 0;

class MyServerCallbacks: public BLEServerCallbacks {
   void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      std::cout << "1" << "\n";
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      std::cout << "0" << "\n";
    } 
};

class MyCallBack: public BLECharacteristicCallbacks {

    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      std::cout << "1" << "\n";
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      std::cout << "0" << "\n";
    } 

    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      if ((rxValue.length() > 0) && (rxValue.find ('&') == std::string::npos)) {
          Serial.println("*********");
          std::cout << rxValue << "\n";
      }
    }

}; //end of callback

void setup() {
  delay(5000);
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::init("Long name works now");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                         
                                       );

  pCharacteristic->setValue("Hello World says Neil");
  pCharacteristic->setCallbacks(new MyCallBack());
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
    
    adcValue = analogRead(potPin);
    Serial.printf("*** ADC GPIO34: %d ***\n", adcValue);
    
    pCharacteristic->setValue(adcValue); //BLE Send value
    pCharacteristic->notify(); 
    //pCharacteristic-&gt;indicate(); 
    delay(1000);
}
