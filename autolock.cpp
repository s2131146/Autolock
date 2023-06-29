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
#include "array.hpp"
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
SecurityMode SECURITY_MODE = SecurityMode::NfcOnly;

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
const int TIME_RESET_TO_COMPLETED_AUTH = 10000;

/**
 * @brief Constraints of fingerprint sensor
 */
const uint32_t FINGERPRINT_SENSOR_BAUD = 57600;
const int FINGERPRINT_SENSOR_INIT_FAIL_LIMIT = 6;
const int INTERVAL_FINGERPRINT_SENSOR_ERR_LED = 100;

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

const int SIZE_ALL_LED = 3;
Led all_leds_[] = {led_blue_, led_green_, led_red_};

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
    servo_.rotate(isOpen ? Rotate::Left : Rotate::Right);
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

/**
 * @brief 指紋認証センサーの接続に失敗したときのLEDを表示
 */
void fsInitializaionFailedLedAnim() {
    for (int i = 0; i < 7; i++) {
        for (int k = 0; k <= SIZE_ALL_LED; k++) {
            Led::update(all_leds_, SIZE_ALL_LED, LedState::OFF);
            all_leds_[0].update(LedState::ON);
            shift<Led, SIZE_ALL_LED>(all_leds_);
            delay(INTERVAL_FINGERPRINT_SENSOR_ERR_LED);
        }
    }
    initLed();
}

bool isFSErr = false;

/**
 * @brief 指紋認証センサーを初期化
 */
void initFingerprint() {
    bool on = true;
    int tryCount = 0;
    do {
        Led::update(all_leds_, SIZE_ALL_LED, on);
        finger_.begin(FINGERPRINT_SENSOR_BAUD);
        on = !on;
        if (++tryCount == FINGERPRINT_SENSOR_INIT_FAIL_LIMIT) {
            fsInitializaionFailedLedAnim();
            SECURITY_MODE = SecurityMode::NfcOnly;
            isFSErr = true;
            break;
        }
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
    SPI.begin();
    mfrc522_.PCD_Init();
    initFingerprint();
    initLed();
}

SecurityType authCompleted = SecurityType::null;
unsigned long startAuthTime = 0;

/**
 * @brief 認証方法変更待機状態
 */
bool inChangingMode = false;

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
                return false;
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
                    return false;
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

unsigned long startTimeForDelay = 0;

/**
 * @brief 遅延のための経過時間
 * 
 * @return unsigned long 
 */
unsigned long elapsedDelayTime() {
    return millis() - startTimeForDelay;
}

void flashOnChangedSecyrityMode() {
    Led leds[] = {led_green_, led_blue_};
    Led::flash(leds, 2);
    Led::update(leds, SIZE_ALL_LED, LedState::ON);
    delay(3000);
    Led::update(leds, SIZE_ALL_LED, LedState::OFF);
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

Led fsErrLed[] = {led_blue_, led_green_};

/**
 * @brief Arduino loop
 */
void loop() {
    keyIfButtonPressed();

    //if (!isFSErr && SECURITY_MODE != SecurityMode::NfcOnly)

    /**
     * @brief delay関数で遅延をかけるとボタン押下処理の
     * レスポンスが悪くなるため，時間差分で遅延をかける
     */
    if (elapsedDelayTime() < INTERVAL_LOOP) {
        startTimeForDelay = millis();
        return;
    }
    if (elapsedAuthTime() >= TIME_RESET_TO_COMPLETED_AUTH) {
        startAuthTime = 0;
        authCompleted = SecurityType::null;
        if (led_green_.current() == LedState::OFF || inChangingMode) initLed();
        inChangingMode = false;
    }
    if (switch_.longPressed()) {
        inChangingMode = true;
        Led leds[] = {led_green_, led_red_};
        Led::flash(leds, 2);
        
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

    /*if (canRead) {
        mfrc522_.PICC_DumpToSerial(&mfrc522_.uid);
    }*/
    
    if (!auth && isOpen && ir_.isDoorClosed()) onDoorClosed();
    if (!auth && !canRead && SECURITY_MODE != SecurityMode::FingerprintOnly) return;

    if (!auth && (type == SecurityType::Nfc && canRead)) {
        led(auth);
    }
    if (SECURITY_MODE == SecurityMode::Both || inChangingMode) {
        if (auth && authCompleted == SecurityType::null) {
            startAuthTime = 0;
            if (inChangingMode) {
                changeSecurityMode();
                flashOnChangedSecyrityMode();
                inChangingMode = false;
                initLed();
            } else {
                led(auth);
                key();
            }
        }
    } else {
        if (auth || SECURITY_MODE != SecurityMode::FingerprintOnly) led(auth);        
        if (auth) key();
    }
}