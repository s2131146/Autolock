class IR {
public:
    /**
     * @brief Construct a new IR object
     * 
     * @param pin 
     */
    IR::IR(int pin) {
        pin_ = pin;
    }

    /**
     * @brief 距離を取得
     * 
     * @return double [cm]
     */
    double distance() {
        int data = analogRead(pin_);
        double out = 5.0 * data / 1023;
        return 26.549 * pow(out, -1.2091);
    }

    /**
     * @brief ドアが閉まっているか
     * 
     * @return true 
     * @return false 
     */
    bool isDoorClosed() {
        return distance() < THRESHOLD_DOOR_CLOSED;
    }

private:
    /**
     * @brief 赤外線距離センサーのドア閉扉検知閾値
     */
    const double THRESHOLD_DOOR_CLOSED = 15.0;

    /**
     * @brief IRピン番号
     */
    int pin_;
};