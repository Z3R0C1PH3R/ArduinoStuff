#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BluetoothSerial.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


//#define USE_PIN // Uncomment this to use PIN during pairing. The pin is specified on the line below
const char *pin = "1234"; // Change this to more secure PIN.

String device_name = "ESP32-BT-Slave";

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;


BLEServer* pServer = NULL;
BLECharacteristic* pSensorCharacteristic = NULL;
BLECharacteristic* pLedCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

const int ledPin = LED_BUILTIN; // Use the appropriate GPIO pin for your setup

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "19b10000-e8f2-537e-4f6c-d104768a1214"
#define SENSOR_CHARACTERISTIC_UUID "19b10001-e8f2-537e-4f6c-d104768a1214"
#define LED_CHARACTERISTIC_UUID "19b10002-e8f2-537e-4f6c-d104768a1214"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      display.println("Device Connected.");
      display.display();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      display.println("Device disconnected.");
      display.display();
    }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pLedCharacteristic) {
        std::string value = pLedCharacteristic->getValue();
        if (value.length() > 0) {
            display.print("Characteristic event, written: ");
            display.println(static_cast<int>(value[0])); // Print the integer value
            display.display();
            int receivedValue = static_cast<int>(value[0]);
            if (receivedValue == 1) {
                digitalWrite(ledPin, HIGH);
            } else {
                digitalWrite(ledPin, LOW);
            }
        }
    }
};

void sendBT(){
  SerialBT.begin(device_name); //Bluetooth device name
  display.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str());
  display.display();
  //Serial.printf("The device with name \"%s\" and MAC address %s is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str(), SerialBT.getMacString()); // Use this after the MAC method is implemented
  #ifdef USE_PIN
    SerialBT.setPin(pin);
    Serial.println("Using PIN");
  #endif
  
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
    Serial.write(SerialBT.read());
  }
  delay(20);
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  analogReadResolution(10);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay(); // Clear display buffer
  // display.startscrollleft(0x00, 0x01);
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Draw 'inverse' text
  display.setTextSize(1);


  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pSensorCharacteristic = pService->createCharacteristic(
                      SENSOR_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // Create the ON button Characteristic
  pLedCharacteristic = pService->createCharacteristic(
                      LED_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  // Register the callback for the ON button characteristic
  pLedCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pSensorCharacteristic->addDescriptor(new BLE2902());
  pLedCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  display.println("Waiting a client connection to notify...");
  display.display();
}

void loop() {
    // notify changed value
    if (deviceConnected) {
        pSensorCharacteristic->setValue(String(value).c_str());
        pSensorCharacteristic->notify();
        value++;
        Serial.print("New value notified: ");
        Serial.println(value);
        delay(3000); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        display.println("Device disconnected.");
        display.display();
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
        display.println("Device Connected.");
        display.display();
    }
}