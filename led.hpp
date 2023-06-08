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
        currentState = newState;
    }

    /**
     * @brief LEDを点滅させる
     */
    void flash() {
        LedState backup = currentState;
        for (int i = 0; i < FLASH_COUNT; i++) {
            update(LedState::ON);
            delay(INTERVAL_FLASH_ON);
            update(LedState::OFF);
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
        return currentState;
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
    LedState currentState;
};