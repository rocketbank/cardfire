#include <SPI.h>
#include <Adafruit_PN532.h>

#include <USBComposite.h>

USBHID HID;
HIDKeyboard Keyboard(HID);

#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)

Adafruit_PN532 nfc(PN532_SS);

void PrintPAN(const byte * data, const uint32_t numBytes)
{
  uint32_t szPos;
  for (szPos=5; szPos < 13; szPos++)
  {
    // Append leading 0 for small values
    if (data[szPos] <= 0xF) Keyboard.print(F("0"));
    Keyboard.print(data[szPos], HEX);
  }
}

void setup(void) {
  HID.begin(HID_KEYBOARD);
  Keyboard.begin(); // useful to detect host capslock state and LEDs
  
  Serial.begin(115200);
  Serial.println(".");
  
  delay(1000);
 
  
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Keyboard.println("Didn't find PN53x board");
    while (1); // halt
  }
  
  Keyboard.println("Found chip");
  nfc.setPassiveActivationRetries(0xFF);
  nfc.SAMConfig();
  Keyboard.println("Waiting for an ISO14443A card");
}



void loop(void) {
  bool success; 
  uint8_t SELECT_APPLICATION[]    = {0x00, 0xA4, 0x04, 0x00, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x04, 0x10, 0x10, 0x00};  
  uint8_t GET_PROCESSING_OPTIONS[]    = {0x80, 0xA8, 0x00, 0x00, 0x02, 0x83, 0x00, 0x00};  
  uint8_t READ_RECORD[]    = {0x00, 0xB2, 0x01, 0x14, 0x00};  

  uint8_t response[64];
  uint8_t responseLength = sizeof(response);
  Keyboard.println("Hello!");
  success = nfc.inListPassiveTarget();

  if(success) {
          Keyboard.println("Found something!");
          if (nfc.inDataExchange(SELECT_APPLICATION, sizeof(SELECT_APPLICATION), response, &responseLength)) {
              nfc.PrintHexChar(response, responseLength);
          }
          if (nfc.inDataExchange(GET_PROCESSING_OPTIONS, sizeof(GET_PROCESSING_OPTIONS), response, &responseLength)) {
              nfc.PrintHexChar(response, responseLength);
          }
          if (nfc.inDataExchange(READ_RECORD, sizeof(READ_RECORD), response, &responseLength)) {
              nfc.PrintHexChar(response, responseLength);
              Keyboard.println("Yep that's a PAN");
              PrintPAN(response, responseLength);
          }
              delay(1000);
          } else {
              Keyboard.println("Timed out waiting for a card");
          }

}
