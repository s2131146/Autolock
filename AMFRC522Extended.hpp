#ifndef AUTOLOCK_H_
#define AUTOLOCK_H_

#include <SPI.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>

#include "Arduino.h"

const char* kNfc_Auth_Uid = "60 68 1e 1e";  // 認証可能なUID

class AMFRC522Extended {
public:
    /**
     * @brief Uid構造体からUidを文字列で取得
     * 
     * @param uid 
     * @param str 
     * @return string 
     */
    String getUidString(MFRC522::Uid *uid) {
        String result = "";

        for (byte i = 0; i < uid->size; i++) {
            result.concat(uid->uidByte[i] < 0x10 ? " 0" : " ");
            result.concat(String(uid->uidByte[i], HEX));
        }

        result.trim();

        return result;
    }

    /**
     * @brief UIDを認証
     * 
     * @param uid UID
     * @return true 認証成功
     * @return false 認証失敗
     */
    bool authUid(MFRC522::Uid *uid) {
        return getUidString(uid) == kNfc_Auth_Uid;
    }

    /**
     * @brief カードが読み取れるか
     * 
     * @param mfrc 
     * @return true 
     * @return false 
     */
    bool canReadNfc(MFRC522* mfrc) {
        return (*mfrc).PICC_IsNewCardPresent() && (*mfrc).PICC_ReadCardSerial();
    }
};

void setup();
void loop();

#endif