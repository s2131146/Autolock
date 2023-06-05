/**
 * @file autolock.cpp
 * @brief Arduinoで動作するオートロック用プログラム
 * @version 0.2
 * @date 2023-06-03
 */
#ifndef AUTOLOCK_MFRC522_H_
#define AUTOLOCK_MFRC522_H_

#include "AMFRC522Extended.hpp"
#include "servo.hpp"
#include "led.hpp"

#endif

/**
 * @brief 各種ピン番号達
 */
const int PIN_LED_RED   = 6;
const int PIN_LED_GREEN = 7;
const int PIN_SERVO     = 8;
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

MFRC522 mfrc522_(PIN_MFRC_SS, PIN_MFRC_RST);
AMFRC522Extended x522_ = AMFRC522Extended();
ServoMotor servo_ = ServoMotor(PIN_SERVO);
Led led_green_ = Led(PIN_LED_GREEN);
Led led_red_   = Led(PIN_LED_RED);

/**
 * @brief LEDを初期化
 */
void initLed() {
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
    servo_.rotate(isOpen ? Rotate::Right : Rotate::Left);
}

/**
 * @brief 認証結果のLEDを操作
 * 
 * @param auth 認証できたか
 */
void led(bool auth) {
    led_green_.update(auth ? LedState::ON : LedState::OFF);
    led_red_.update(auth ? LedState::OFF : LedState::ON);

    if (auth) {
        delay(AUTHORIZED_LED_TIME);
        initLed();
    } else {
        led_red_.flash();
    }
}

/**
 * @brief Arduino setup
 */
void setup() {
    Serial.begin(9600);
    SPI.begin();
    mfrc522_.PCD_Init();
    initLed();
}

/**
 * @brief Arduino loop
 */
void loop() {
    if (!x522_.canReadNfc(&mfrc522_)) return;
    
    bool auth = x522_.authUid(&mfrc522_.uid);
    if (auth) {
        key();
    }
    led(auth);
    
    delay(INTERVAL_LOOP);
}