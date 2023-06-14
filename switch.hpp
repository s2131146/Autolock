/**
 * @brief スイッチ
 */
class Switch {
public:
    /**
     * @brief Construct a new Switch object
     * 
     * @param pin 
     */
    Switch(int pin) {
        pin_ = pin;
        pinMode(pin, INPUT_PULLUP);
    }

    /**
     * @brief スイッチが押されているか
     * 
     * @return bool 
     */
    bool isPressing() {
        return digitalRead(pin_) == LOW;
    }

    /**
     * @brief 短く押されたか判定，毎ループ呼び出し
     * 
     * @return true 
     * @return false 
     */
    bool shortPressed() {
        if (isPressing() && start_pressing_s_ == 0) start_pressing_s_ = millis();
        if (start_pressing_s_ != 0 && !isPressing()) {
            if ((millis() - start_pressing_s_) < SHORT_PRESS_THRESHOLD) {
                start_pressing_s_ = 0;
                return true;
            }
            start_pressing_s_ = 0;
        }

        return false;
    }

    /**
     * @brief 長押しされたか判定，毎ループ時呼び出し
     * 
     * @return true 
     * @return false 
     */
    bool longPressed() {
        if (isPressing() && start_pressing_l_ == 0) start_pressing_l_ = millis();
        if (start_pressing_l_ != 0 && !isPressing()) start_pressing_l_ = 0;
        if ((start_pressing_l_ != 0) && (millis() - start_pressing_l_) > LONG_PRESS_THRESHOLD) {
            start_pressing_l_ = 0;
            return true;
        }

        return false;
    }

private:
    /**
     * @brief 長押し判定のしきい値
     */
    const int LONG_PRESS_THRESHOLD = 5000;
    
    /**
     * @brief 押下判定のしきい値
     */
    const int SHORT_PRESS_THRESHOLD = 700;

    /**
     * @brief Switchピン番号
     */
    int pin_;

    /**
     * @brief ボタン押下開始時刻 (ロング判定用)
     */
    unsigned long start_pressing_l_ = 0;

    /**
     * @brief ボタン押下開始時刻 (ショート判定用)
     * 
     */
    unsigned long start_pressing_s_ = 0;
};