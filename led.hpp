#ifndef AUTOLOCK_LED_H_
#define AUTOLOCK_LED_H_

#include "array.hpp"

#endif

/**
 * @brief LEDの状態
 */
enum class LedState {
    ON,
    OFF
};

/**
 * @brief LEDを扱うクラス
 */
class Led {
public:
    /**
     * @brief Construct a new Led object
     * 
     * @param pin PIN number
     */
    Led(int pin) {
        pin_ = pin;
        pinMode(pin, OUTPUT);
        Serial.println(pin);
    }

    /**
     * @brief LEDの状態を更新
     * 
     * @param newState 
     */
    void update(LedState newState) {
        switch (newState) {
            case LedState::ON:  digitalWrite(pin_, HIGH); break;
            case LedState::OFF: digitalWrite(pin_, LOW);  break;
        }
        current_state_ = newState;
    }

    /**
     * @brief LEDの状態を更新
     * 
     * @param on LEDをON
     */
    void update(bool on) {
        update(on ? LedState::ON : LedState::OFF);
    }

    /**
     * @brief LEDの状態を更新
     * 
     * @param leds 
     * @param state 
     */
    void update(Led leds[], LedState state) {
        for (int i = 0; i < array_size(leds); ++i) {
            leds[i].update(state);
        }
    }

    /**
     * @brief LEDの状態を更新
     * 
     * @param leds 
     * @param on
     */
    void update(Led leds[], bool on) {
        update(leds, on ? LedState::ON : LedState::OFF);
    }

    /**
     * @brief LEDを点滅させる
     */
    void flash() {
        LedState backup = current_state_;
        for (int i = 0; i < FLASH_COUNT; i++) {
            update(LedState::ON);
            delay(INTERVAL_FLASH_ON);
            update(LedState::OFF);
            delay(INTERVAL_FLASH_OFF);
        }
        update(backup);
    }

   /**
    * @brief LEDを点滅させる
    * 
    * @param leds 
    */
    void flash(Led leds[]) {
        LedState backup = current_state_;
        for (int i = 0; i < FLASH_COUNT; i++) {
            update(leds, LedState::ON);
            delay(INTERVAL_FLASH_ON);
            update(leds, LedState::OFF);
            delay(INTERVAL_FLASH_OFF);
        }
        update(backup);
    }

    /**
     * @brief 現在の状態を取得
     * 
     * @return LedState 
     */
    LedState current() {
        return current_state_;
    }

    /**
     * @brief 反転
     */
    void invert() {
        LedState newState = current_state_ == LedState::ON ? LedState::OFF : LedState::ON;
        update(newState);
    }

private:
    /**
     * @brief フラッシュ時の間隔
     */
    const int INTERVAL_FLASH_ON  = 500;
    const int INTERVAL_FLASH_OFF = 100;

    /**
     * @brief フラッシュする回数
     */
    const int FLASH_COUNT = 4;

    /**
     * @brief LEDピン番号
     */
    int pin_;

    /**
     * @brief 現在の状態
     */
    LedState current_state_;
};