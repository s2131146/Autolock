#ifndef AUTOLOCK_MFRC522_H_
#define AUTOLOCK_MFRC522_H_

#include "AMFRC522Extended.hpp"

#endif

constexpr uint8_t PIN_RST = 9;
constexpr uint8_t PIN_SS = 10;

MFRC522 mfrc522_(PIN_SS, PIN_RST);
AMFRC522Extended x522_ = AMFRC522Extended();

/**
 * @brief NFC認証
 */
void authorize() {
    String uid = x522_.getUidString(&mfrc522_.uid);
    if (x522_.authUid(&mfrc522_.uid)) {
        Serial.println("Authenicated NFC. UID: " + uid);
    } else {
        Serial.println("Unauthorized NFC. UID: " + uid);
    }
}

void setup() {
    Serial.begin(9600);
    SPI.begin();
    mfrc522_.PCD_Init();
}

void loop() {
    if (!x522_.canReadNfc(&mfrc522_)) return;

    authorize();
    //mfrc522_.PICC_DumpToSerial(&mfrc522_.uid);
}
