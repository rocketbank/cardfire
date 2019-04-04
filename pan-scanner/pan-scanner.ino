// #define DEBUG

// Beeper type
// #define ACTIVE

// PAN keyboard output mode
#define LAST4

#define PN532_SCK (2)
#define PN532_MOSI (3)
#define PN532_SS (4)
#define PN532_MISO (5)
#define LAST4

#define PN532_SCK (2)
#define PN532_MOSI (3)
#define PN532_SS (4)
#define PN532_MISO (5)

#ifdef DEBUG
#define MIFAREDEBUG
#define PN532DEBUG
#endif

#include <SPI.h>
#include <Adafruit_PN532.h>

#include <USBComposite.h>

USBHID HID;
HIDKeyboard Keyboard(HID);

Adafruit_PN532 nfc(PN532_SS);

void PrintPAN(const byte *data)
{
  uint32_t szPos = 5;
#ifdef LAST4
  szPos += (16 - 4) / 2;
#endif
  for (; szPos < 13; szPos++) // 8 итераций по 2 PAN-цифры в каждом числе за раз
  {
    // Append leading 0 for small values
    if (data[szPos] <= 0xF)
      Keyboard.print(F("0"));
    Keyboard.print(data[szPos], HEX);
  }
}

void Beep(unsigned int duration)
{
#ifdef ACTIVE
  digitalWrite(PB1, HIGH);
  delay(duration);
  digitalWrite(PB1, LOW);
#endif
#ifndef ACTIVE
  tone(PB1, 2400);
  delay(duration);
  analogWrite(PB1, 0);
#endif
}

// short-short
void BeepReady()
{
  Beep(100u);
  delay(100);
  Beep(100u);
}

// long-short-long
void BeepError()
{
  Beep(100u);
  delay(500);
  Beep(100u);
  delay(100);
  Beep(1000u);
}

void Debug(const char * str)
{
#ifdef DEBUG
  Keyboard.println(str);
  Serial.println(str);
#endif
}

void setup(void)
{
  Serial.begin(115200);

#ifdef DEBUG
  Serial.println("initializing...");
#endif

  HID.begin(HID_KEYBOARD);
  Keyboard.begin(); // useful to detect host capslock state and LEDs
  pinMode(PB1, OUTPUT); // BEEP_PIN
  //delay(1000);

  Debug("keyboard had initialized");
  nfc.begin();

  delay(1000);

  Debug("All (nfc, keyboard, serial, pins) had initialized");

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata)
  {
    Debug("Didn't find PN53x board");
    BeepError();
    while (1); // halt
  }

  Debug("Found chip");

  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  // 0xFF to wait forever, 0x00..0xFE to timeout after mxRetries
  nfc.setPassiveActivationRetries(0xFF);

  nfc.SAMConfig();
  // Keyboard.println("Waiting for an ISO14443A card");

  Debug("ready");

  BeepReady();
}

void loop(void)
{
  bool success;
  uint8_t SELECT_APPLICATION[] = {0x00, 0xA4, 0x04, 0x00, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x04, 0x10, 0x10, 0x00};
  uint8_t GET_PROCESSING_OPTIONS[] = {0x80, 0xA8, 0x00, 0x00, 0x02, 0x83, 0x00, 0x00};
  uint8_t READ_RECORD[] = {0x00, 0xB2, 0x01, 0x14, 0x00};

  uint8_t response[64];
  uint8_t responseLength = sizeof(response);

  Debug(".");

  success = nfc.inListPassiveTarget();

  if (success)
  {
    Debug("Found something!");

    if (nfc.inDataExchange(SELECT_APPLICATION, sizeof(SELECT_APPLICATION), response, &responseLength))
    {
      //nfc.PrintHexChar(response, responseLength);
    }
    if (nfc.inDataExchange(GET_PROCESSING_OPTIONS, sizeof(GET_PROCESSING_OPTIONS), response, &responseLength))
    {
      //nfc.PrintHexChar(response, responseLength);
    }
    if (nfc.inDataExchange(READ_RECORD, sizeof(READ_RECORD), response, &responseLength))
    {
      //nfc.PrintHexChar(response, responseLength); // Ничего не печатает
      PrintPAN(response);
      Keyboard.println("");
      Beep(100u);
      // delay(1000);
    }
  }

  else
  {
    //              Keyboard.println("Timed out waiting for a card");
  }
}
