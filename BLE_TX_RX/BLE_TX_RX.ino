#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <iostream>
#include <sstream>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
const int potPin = 34;
int adcValue = 0;
using namespace std;

#define SERVICE_UUID        "0000ffe0-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_UUID "0000ffe1-0000-1000-8000-00805f9b34fb"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }

};

class MyCallBack: public BLECharacteristicCallbacks {

    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      if ((rxValue.length() > 0) && (rxValue.find ('&') == std::string::npos)) {
          Serial.printf("*** Receive data: %d %d ***\n", rxValue[0], rxValue[1]);
      }
    }

}; //end of callback


void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
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
  pCharacteristic->setCallbacks(new MyCallBack());
  
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
        adcValue = analogRead(potPin);
        Serial.printf("*** ADC GPIO34: %x ***\n", adcValue);
        /*
        std::ostringstream adcValue_str;
        adcValue_str << adcValue;
        std::string converted(adcValue_str.str());
        
        std::string str1 = "20201116 2014 ";
        std::string str2 = adcValue_str.str();
        std::string str = str1 + str2;
        */
        char a[2];
        /*
        a[0] = 20 + 1;
        a[1] = 20 + 1; //year 2020
        a[2] = 01 + 1;
        a[3] = 12 + 1; //date 0112
        a[4] = 21 + 1;
        a[5] = 31 + 1; //time 21:31
        */
        a[0] = (adcValue >> 8) + 1;
        if((adcValue & 0x00ff) == 0xff){
          a[1] = 0xff; //pressure
        }
        else {
          a[1] = (adcValue & 0x00ff) + 1; //pressure
        }
        
        Serial.printf("***  %x  %x***\n", a[0], a[1]);

        pCharacteristic->setValue(a);
        //pCharacteristic->setValue(adcValue);
        pCharacteristic->notify();
        delay(2); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 500ms
    }
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
