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
#include "switch.hpp"
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
SecurityMode SECURITY_MODE = SecurityMode::Both;

/**
 * @brief 各種ピン番号達
 */
const int PIN_IR_SENSOR = 0;
const int PIN_SWITCH = 2;
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
const int TIME_RESET_TO_COMPLETED_AUTH = 7000;

SoftwareSerial _serial(PIN_FINGERPRINT_IN, PIN_FINGERPRINT_OUT);
Adafruit_Fingerprint finger_(&_serial);
MFRC522 mfrc522_(PIN_MFRC_SS, PIN_MFRC_RST);
AMFRC522Extended x522_;
ServoMotor servo_(PIN_SERVO);
Switch switch_(PIN_SWITCH);
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
    led_green_.update(auth);
    led_red_.update(!auth);

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

Led all_leds_[] = {led_blue_, led_green_, led_red_};

/**
 * @brief 
 * 
 */
void initFingerprint() {
    bool on = false;
    do {
        Led::update(all_leds_, on);
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
unsigned long startAuthTime = 0;

/**
 * @brief セキュリティ認証
 * 
 * @param canRead 
 * @param type 
 * @return true 
 * @return false 
 */
bool authSecurity(bool canRead, SecurityType* type) {
    bool auth = false;
    SecurityMode mode = inChangingMode ? SecurityMode::Both : SECURITY_MODE;
    switch (mode) {
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
                *type = SecurityType::Nfc;
                auth = x522_.authUid(&mfrc522_.uid);
                authCompleted = auth ? SecurityType::null : authCompleted;
            } else if (authCompleted == SecurityType::Nfc) {
                uint8_t image = finger_.getImage();
                if (image == FINGERPRINT_OK) {
                    *type = SecurityType::Fingerprint;
                    auth = authFingerprint(image);
                    authCompleted = auth ? SecurityType::null : authCompleted;
                }
            } else {
                if (canRead) {
                    *type = SecurityType::Nfc;
                    auth = x522_.authUid(&mfrc522_.uid);
                } else {
                    uint8_t image = finger_.getImage();
                    if (image == FINGERPRINT_OK) {
                        *type = SecurityType::Fingerprint;
                        auth = authFingerprint(image);
                    }
                }
                if (auth) {
                    startAuthTime = millis();
                    authCompleted = *type;
                    led_blue_.update(LedState::ON);
                }
            }
            break;
    }

    return auth;
}

/**
 * @brief 認証方法を変更
 */
void changeSecurityMode() {
    SecurityMode _new;
    switch (SECURITY_MODE) {
        case SecurityMode::NfcOnly: _new = SecurityMode::FingerprintOnly; break;
        case SecurityMode::FingerprintOnly: _new = SecurityMode::Both; break;
        case SecurityMode::Both: _new = SecurityMode::NfcOnly; break;
    }
    SECURITY_MODE = _new;
}

/**
 * @brief 認証開始からの経過時間
 * 
 * @return unsigned long 
 */
unsigned long elapsedAuthTime() {
    return millis() - startAuthTime;
}

void flashOnChangedSecyrityMode() {
    Led leds[] = {led_green_, led_blue_};
    Led::flash(leds);
    Led::update(leds, LedState::ON);
    delay(3000);
    Led::update(leds, LedState::OFF);
}

/**
 * @brief ボタン押下で解錠
 */
void keyIfButtonPressed() {
    if (switch_.shortPressed()) {
        led(true);
        key();
    }
}

/**
 * @brief 認証方法変更待機状態
 */
bool inChangingMode = false;

/**
 * @brief Arduino loop
 */
void loop() {
    if (elapsedAuthTime() >= TIME_RESET_TO_COMPLETED_AUTH) {
        startAuthTime = 0;
        authCompleted = SecurityType::null;
        if (led_green_.current() == LedState::OFF) initLed();
    }
    if (switch_.longPressed()) {
        inChangingMode = true;
        Led leds[] = {led_green_, led_red_};
        Led::flash(leds);
        
        led_green_.update(LedState::ON);
        led_red_.update(LedState::OFF);
        startAuthTime = millis();
    }
    if (inChangingMode) {
        led_green_.invert();
        led_red_.invert();
    }

    SecurityType type = SecurityType::null;
    bool canRead = x522_.canReadNfc(&mfrc522_);
    bool auth = authSecurity(canRead, &type);
    
    if (!auth && isOpen && ir_.isDoorClosed()) onDoorClosed();
    if (!auth && !canRead && SECURITY_MODE == SecurityMode::FingerprintOnly) return;

    if (SECURITY_MODE == SecurityMode::Both) {
        if (!auth && (type == SecurityType::Nfc && canRead || type == SecurityType::Fingerprint)) {
            led(auth);
        }
        if (auth && authCompleted == SecurityType::null) {
            startAuthTime = 0;
            if (inChangingMode) {
                changeSecurityMode();
                flashOnChangedSecyrityMode();
                inChangingMode = false;
            } else {
                led(auth);
                key();
            }
        }
    } else {
        led(auth);        
        if (auth) key();
    }

    delay(INTERVAL_LOOP);
}