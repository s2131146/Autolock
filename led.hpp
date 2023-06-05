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
    }

    /**
     * @brief LEDの状態を更新
     * 
     * @param newState 
     */
    void update(LedState newState) {
        switch (newState) {
            case LedState::ON:  digitalWrite(pin_, HIGH); break;
            case LedState::OFF: digitalWrite(pin_, LOW);  break;;
        }
    }

    /**
     * @brief LEDを点滅させる
     */
    void flash(bool afterOn = true) {
        for (int i = 0; i < FLASH_COUNT; i++) {
            update(LedState::ON);
            delay(INTERVAL_FLASH_ON);
            update(LedState::OFF);
            delay(INTERVAL_FLASH_OFF);
        }
        update(afterOn ? LedState::ON : LedState::OFF);
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
    const int FLASH_COUNT = 3;

    /**
     * @brief LEDピン番号
     */
    int pin_;
};