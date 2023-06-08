/**
 * @file autolock.cpp
 * @brief Arduinoで動作するオートロック用プログラム
 * @version 0.2
 * @date 2023-06-08
 */
#ifndef AUTOLOCK_MFRC522_H_
#define AUTOLOCK_MFRC522_H_

#include "Adafruit_Fingerprint.h"
#include "AMFRC522Extended.hpp"
#include "servo.hpp"
#include "led.hpp"
#include "ir.hpp"

#endif

/**
 * @brief 認証方法
 */
enum class SecurityMode {
    NfcOnly,
    FingerprintOnly,
    Both
};

/**
 * @brief 認証方法
 */
enum class SecurityType {
    Nfc,
    Fingerprint,
    null
};

/**
 * @brief 動作するモード
 */
SecurityMode SECURITY_MODE = SecurityMode::FingerprintOnly;

/**
 * @brief 各種ピン番号達
 */
const int PIN_IR_SENSOR = 0;
const int PIN_FINGERPRINT_IN  = 3;
const int PIN_FINGERPRINT_OUT = 4;
const int PIN_SERVO = 5;
const int PIN_LED_BLUE = 6;
const int PIN_LED_GREEN = 7;
const int PIN_LED_RED   = 8;
constexpr uint8_t PIN_MFRC_RST = 9;
constexpr uint8_t PIN_MFRC_SS  = 10;

/**
 * @brief PINs of MFRC522
 *
 * 3.3: 3.3V
 * RST: 9
 * GND: GND
 * IRQ: NONE
 * ISO: 12
 * SIM: 11
 * SCK: 13
 * SDA: 10
 */

/**
 * @brief loop関数のインターバル
 */
const int INTERVAL_LOOP = 100;

/**
 * @brief 認証後に点灯するLEDの時間
 */
const int AUTHORIZED_LED_TIME = 5000;

/**
 * @brief ドア閉扉後施錠までの時間
 */
const int TIME_LOCK_AFTER_CLOSED = 5000;

/**
 * @brief 認証有効期限
 */
const int TIME_RESET_TO_COMPLETED_AUTH = 70;

SoftwareSerial _serial(PIN_FINGERPRINT_IN, PIN_FINGERPRINT_OUT);
Adafruit_Fingerprint finger_(&_serial);
MFRC522 mfrc522_(PIN_MFRC_SS, PIN_MFRC_RST);
AMFRC522Extended x522_;
ServoMotor servo_(PIN_SERVO);
Led led_blue_(PIN_LED_BLUE);
Led led_green_(PIN_LED_GREEN);
Led led_red_(PIN_LED_RED);
IR ir_(PIN_IR_SENSOR);

/**
 * @brief LEDを初期化
 */
void initLed() {
    led_blue_.update(LedState::OFF);
    led_red_.update(LedState::ON);
    led_green_.update(LedState::OFF);
}

/**
 * @brief 鍵の状態
 */
bool isOpen = false;

/**
 * @brief 鍵の解施錠
 */
void key() {
    isOpen = !isOpen;
    servo_.attach();
    servo_.rotate(isOpen ? Rotate::Right : Rotate::Default);
}

/**
 * @brief ドア閉扉時に鍵操作
 */
void onDoorClosed() {
    delay(TIME_LOCK_AFTER_CLOSED);
    if (ir_.isDoorClosed()) {
        led_green_.flash();
        key();
        initLed();
    }
}

/**
 * @brief 認証結果のLEDを操作
 *
 * @param auth 認証できたか
 */
void led(bool auth) {
    if (SECURITY_MODE == SecurityMode::Both && auth) {
        led_blue_.update(LedState::OFF);
    }
    led_green_.update(auth ? LedState::ON : LedState::OFF);
    led_red_.update(auth ? LedState::OFF : LedState::ON);

    if (!auth) {
        led_red_.flash();
    }
}

/**
 * @brief Get the Fingerprint Id
 * 
 * @param image 
 * @return uint16_t 
 */
uint16_t getFingerprintId(uint8_t image) {
    uint8_t tz = finger_.image2Tz();
    if (tz != FINGERPRINT_OK) return NULL;

    uint8_t result = finger_.fingerSearch();
    if (result != FINGERPRINT_OK) return NULL;

    return finger_.fingerID;
}

/**
 * @brief 
 * 
 */
void initFingerprint() {
    bool on = false;
    Led targets[] = {led_blue_, led_green_, led_red_};
    do {
        for (auto led : targets) {
            led.update(on ? LedState::ON : LedState::OFF);
        }
        finger_.begin(57600);
        on = !on;
    } while (!finger_.verifyPassword());
}

/**
 * @brief 指紋認証
 * 
 * @return true 
 * @return false 
 */
bool authFingerprint(uint8_t image = FINGERPRINT_OK) {
    bool auth = false;
    image = image == FINGERPRINT_OK ? finger_.getImage() : image;
    if (image == FINGERPRINT_OK) {
        uint16_t id = getFingerprintId(image);
        if (id != NULL) {
            auth = true;
        } else {
            led_red_.flash();
        }
    } else if (image != FINGERPRINT_NOFINGER && image != FINGERPRINT_PACKETRECIEVEERR) {
        led_red_.flash();
    }

    return auth;
}

/**
 * @brief Arduino setup
 */
void setup() {
    Serial.begin(9600);
    SPI.begin();
    mfrc522_.PCD_Init();
    initFingerprint();
    initLed();
}

SecurityType authCompleted = SecurityType::null;
int remainLoopCountForResetAuth = -1;

/**
 * @brief Arduino loop
 */
void loop() {
    if (--remainLoopCountForResetAuth == 0) {
        authCompleted = SecurityType::null;
        if (led_green_.current() == LedState::OFF) initLed();
    }

    SecurityType type = SecurityType::null;
    bool canRead = x522_.canReadNfc(&mfrc522_);
    bool auth = false;
    switch (SECURITY_MODE) {
        case SecurityMode::NfcOnly:
            if (!canRead) {
                if (isOpen && ir_.isDoorClosed()) onDoorClosed();
                return;
            };
            auth = x522_.authUid(&mfrc522_.uid);
            break;
        case SecurityMode::FingerprintOnly:
            auth = authFingerprint();
            break;
        case SecurityMode::Both:
            if (authCompleted == SecurityType::Fingerprint) {
                if (!canRead) {
                    if (isOpen && ir_.isDoorClosed()) onDoorClosed();
                    return;
                };
                type = SecurityType::Nfc;
                auth = x522_.authUid(&mfrc522_.uid);
                authCompleted = auth ? SecurityType::null : authCompleted;
            } else if (authCompleted == SecurityType::Nfc) {
                uint8_t image = finger_.getImage();
                if (image == FINGERPRINT_OK) {
                    type = SecurityType::Fingerprint;
                    auth = authFingerprint(image);
                    authCompleted = auth ? SecurityType::null : authCompleted;
                }
            } else {
                Serial.println(canRead);
                if (canRead) {
                    type = SecurityType::Nfc;
                    auth = x522_.authUid(&mfrc522_.uid);
                } else {
                    uint8_t image = finger_.getImage();
                    if (image == FINGERPRINT_OK) {
                        type = SecurityType::Fingerprint;
                        auth = authFingerprint(image);
                    }
                }
                if (auth) {
                    if (type == SecurityType::Fingerprint) {
                        remainLoopCountForResetAuth = TIME_RESET_TO_COMPLETED_AUTH * 7;
                    } else {
                        remainLoopCountForResetAuth = TIME_RESET_TO_COMPLETED_AUTH;
                    }
                    authCompleted = type;
                    led_blue_.update(LedState::ON);
                }
            }
            break;
    }
    
    if (!auth && isOpen && ir_.isDoorClosed()) onDoorClosed();
    if (!auth && !canRead && SECURITY_MODE == SecurityMode::FingerprintOnly) return;

    if (SECURITY_MODE == SecurityMode::Both) {
        if (!auth && (type == SecurityType::Nfc && canRead || type == SecurityType::Fingerprint)) {
            led(auth);
        }
        if (auth && authCompleted == SecurityType::null) {
            led(auth);
            key();
        }
    } else {
        led(auth);        
        if (auth) key();
    }

    delay(INTERVAL_LOOP);
}