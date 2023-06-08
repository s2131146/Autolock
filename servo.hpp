/* ファイル一回全部消えたわマジでGit許さない呪呪呪呪呪呪呪 */
#ifndef AUTOLOCK_SERVO_H_
#define AUTOLOCK_SERVO_H_

#include <Servo.h>

#endif

/**
 * @brief 回転方向
 */
enum class Rotate {
    Right,
    Left,
    Default  // 元の位置に戻す
};

/**
 * @brief サーボモーターを扱うクラス
 */
class ServoMotor {
public:
    /**
     * @brief Construct a new Servo Motor object
     * 
     * @param pin PIN number
     */
    ServoMotor(int pin) {
        pin_ = pin;
    }

    /**
     * @brief アタッチ
     */
    void attach() {
        motor_.attach(pin_);
        rotate(Rotate::Default);
    }

    /**
     * @brief 回転
     * 
     * @param direction 
     */
    void rotate(Rotate direction) {
        switch (direction) {
            case Rotate::Default:
            case Rotate::Left:    motor_.write(0);   break;
            case Rotate::Right:   motor_.write(90);  break;
        }
    }

private:
    /**
     * @brief サーボモーターのピン番号
     * 
     */
    int pin_;

    /**
     * @brief サーボモーター
     */
    Servo motor_;
};