// ==================================
// 全体共通のヘッダファイルのinclude
#include <M5Unified.h>                       // M5Unifiedライブラリ
#include <Arduino.h>                         // Arduinoフレームワークを使用する場合は必ず必要
#include <SD.h>                              // SDカードを使うためのライブラリです。
#include <Update.h>                          // 定義しないとエラーが出るため追加。
#include <Ticker.h>                          // 定義しないとエラーが出るため追加。
#include <M5StackUpdater.h>                  // M5Stack SDUpdaterライブラリ
// ================================== End

// ==================================
// for SystemConfig
#include <Stackchan_system_config.h>          // stack-chanの初期設定ファイルを扱うライブラリ
StackchanSystemConfig system_config;          // (Stackchan_system_config.h) プログラム内で使用するパラメータをYAMLから読み込むクラスを定義
// ================================== End

// ==================================
// for SD
#define SDU_APP_PATH "/stackchan_tester.bin"  // (SDUpdater.h)SDUpdaterで使用する変数
#define TFCARD_CS_PIN 4                       // SDカードスロットのCSPIN番号
// ================================== End

// ==================================
// #defines
// 各要素の使用／不使用を切り替え

#define USE_Avatar                   // Avatarを使用する場合

#define USE_LED                      // LEDを使用する場合(共通)
#define USE_LED_NEKOMIMI             // Necomimi-LED (9 9 straight) を使用する場合

#define USE_LED_GOBOTTOM             // GO Bottomを使用する場合

#define USE_Servo                    // サーボモーターを使用する場合（背面のGPIOから360度サーボモーターを動かす想定）
#define USE_Servo_360_TowerPro       // 使用するサーボモーターのメーカーごとのパラメータ設定。3種から選択。
// #define USE_Servo_360_Feetech360
// #define USE_Servo_360_M5Stack

#define USE_ESPNow                      // ESPNowを使用する場合

// ================================== End

// ==================================
// for Avatar
#ifdef USE_Avatar
  #include <Avatar.h>                          // 顔を表示するためのライブラリ https://github.com/meganetaaan/m5stack-avatar
  using namespace m5avatar;                    // (Avatar.h)avatarのnamespaceを使う宣言（こうするとm5avatar::???と書かなくて済む。)
  Avatar avatar;                               // (Avatar.h)avatarのクラスを定義
  ColorPalette *cp;                            // アバターのカラーパレット用

  // 表情を変更
  void changeExpression(int num) {
    switch(num) {
      case 0:
        avatar.setExpression(Expression::Angry);
        break;
      case 1:
        avatar.setExpression(Expression::Doubt);
        break;
      case 2:
        avatar.setExpression(Expression::Happy);
        break;
      case 3:
        avatar.setExpression(Expression::Neutral);
        break;
      case 4:
        avatar.setExpression(Expression::Sad);
        break;
      case 5:
        avatar.setExpression(Expression::Sleepy);
        break;
      default:
        avatar.setExpression(Expression::Neutral);
    }
  }

  // for 吹き出しテキスト関連
  #include "formatString.hpp"                  // 文字列に変数の値を組み込むために使うライブラリ https://gist.github.com/GOB52/e158b689273569357b04736b78f050d6
  uint32_t mouth_wait = 2000;                  // 通常時のセリフ入れ替え時間（msec）
  uint32_t last_mouth_millis = 0;              // セリフを入れ替えた時間

  const char* lyrics[] = { "BtnA:MoveTo90  ", "BtnB:ServoTest  ", "BtnC:RandomMode  ", "BtnALong:AdjustMode"};  // 通常モード時に表示するセリフ
  const int lyrics_size = sizeof(lyrics) / sizeof(char*);  // セリフの数
  int lyrics_idx = 0;                                      // 表示するセリフ数用の変数

  char speechText[100];  // フォーマットされた文字列を格納するためのバッファ、Avatarの吹き出しに変数を使用する場合に使用
#endif
// ================================== End

// ==================================
// for LED
#ifdef USE_LED
  #include "FastLED.h"

  bool isLedON = false;  // LEDのON/OFFフラグ

  #ifdef USE_LED_NEKOMIMI
  /**
   * [GPIO Number]
   * BASIC/GRAY/M5GO/FIRE	26
   * CORE2/CORE2 for AWS	26
   * CoreS3	9
   */
    #define LEDS_PIN_NECO 26
    #define NUM_LEDS_NECO 18
    CRGB necomimi_leds[NUM_LEDS_NECO];
    CRGB necomimi_led_table[NUM_LEDS_NECO/2] = {CRGB::Purple,CRGB::MediumPurple,CRGB::Blue,CRGB::Green,CRGB::LimeGreen,CRGB::Yellow,CRGB::DarkOrange,CRGB::OrangeRed,CRGB::Red};
  #endif

  #ifdef USE_LED_GOBOTTOM
    /**
     * [GPIO Number]
     * GO Bottom (BASIC系 GO,Fire)   15
     * GO Bottom2 (CORE2系 AWS)      25
     * GO Bottom3 (CoreS3系 別売のみ)  5
     */
    #if defined(ARDUINO_M5STACK_FIRE) || defined(ARDUINO_M5Stack_Core_ESP32)
      #define LEDS_PIN_GOBTM 15
    #endif
    #ifdef ARDUINO_M5STACK_Core2
      #define LEDS_PIN_GOBTM 25
    #endif
    #ifdef ARDUINO_M5STACK_CORES3
      #define LEDS_PIN_GOBTM 5
    #endif
    #define NUM_LEDS_GOBTM 10
    CRGB gobottom_leds[NUM_LEDS_GOBTM];
    CRGB gobottom_led_table[NUM_LEDS_GOBTM/2] = {CRGB::Purple,CRGB::Blue,CRGB::LimeGreen,CRGB::DarkOrange,CRGB::Red};
  #endif

  void fill_led_buff(CRGB color) {
    // Now turn the LED off, then pause
    for(int i=0;i<NUM_LEDS_NECO;i++) {
      necomimi_leds[i] =  color;
    }
  }

  void clear_led_buff() {
    // Now turn the LED off, then pause
    for(int i=0;i<NUM_LEDS_NECO;i++) {
      necomimi_leds[i] = CRGB::Black;
    }
  }

  void level_led(int level1, int level2) {
    if(level1>NUM_LEDS_NECO/2) level1 = NUM_LEDS_NECO/2;
    if(level2>NUM_LEDS_NECO/2) level2 = NUM_LEDS_NECO/2;
    clear_led_buff();

    // LEDの順番
    //
    // M5GO Bottom2
    //    下 ------------------ 上
    // LED4 LED3 LED2 LED1 LED0
    // LED5 LED6 LED7 LED8 LED9
    //
    // Nekomimi
    // LED0 LED1  LED2  LED3  LED4  LED5  LED6  LED7　LED8
    // LED9 LED10 LED11 LED12 LED13 LED14 LED15 LED16 LED17
    
    // 右LED 
    for(int i=0;i<level1;i++) {
        necomimi_leds[i] = necomimi_led_table[i];
        necomimi_leds[8-i] = necomimi_led_table[i];
    }
    // 左LED
    for(int i=0;i<level2;i++) {
      necomimi_leds[i+NUM_LEDS_NECO/2] = necomimi_led_table[i];
      necomimi_leds[8-i+NUM_LEDS_NECO/2] = necomimi_led_table[i];
    }
    FastLED.show();
  }

  void turn_on_led() {
    #ifdef USE_LED_NEKOMIMI
      level_led(NUM_LEDS_NECO/2, NUM_LEDS_NECO/2);
      FastLED.show();
    #endif
    #ifdef USE_LED_GOBOTTOM
      fill_rainbow_circular(gobottom_leds, NUM_LEDS_GOBTM, 0, 7);
      FastLED.show();
    #endif
    isLedON = true;
  }

  void turn_off_led() {
    #ifdef USE_LED_NEKOMIMI
      for(int i=0;i<NUM_LEDS_NECO;i++) {
        necomimi_leds[i] = CRGB::Black;
      }
      FastLED.show();
    #endif
    #ifdef USE_LED_GOBOTTOM
      fill_solid(gobottom_leds, NUM_LEDS_GOBTM, CRGB::Black);
      FastLED.show();
    #endif
    isLedON = false;
  }

#endif
// ================================== End

// ==================================
// for Servo
#ifdef USE_Servo
  #include <ServoEasing.hpp>

  /**
   * サーボのPWM値と回転方向の関係（最右列は180サーボの場合の角度）
   * 
   * Tower Pro
   * 時計回り   : 500 - 1500US : 0 - 90度
   * 停止       : 1500US : 90度
   * 反時計周り : 1500 - 2400US : 90 - 180度
   * 
   * Feetech
   * 時計回り   : 700 - 1500US : 0 - 90度
   * 停止       : 1500US : 90度
   * 反時計周り : 1500 - 2300US : 90 - 180度
   * 
   * M5Stack
   * 時計回り   : 500 - 1500US : 0 - 90度
   * 停止       : 1500US : 90度
   * 反時計周り : 1500 - 2500US : 90 - 180度
  */

  ServoEasing servo180;  // 180度サーボ
  ServoEasing servo360;  // 360度サーボ

  #define START_DEGREE_VALUE_SERVO_180  85  // 180度サーボ（Y軸方向）の初期角度(全サーボ共通)

  // サーボの種類毎のPWM幅や初期角度、回転速度のレンジ設定など
  #ifdef USE_Servo_180_TowerPro
    const int MIN_PWM_180 = 500;
    const int MAX_PWM_180 = 2400;
  #endif
  #ifdef USE_Servo_180_Feetech360
    const int MIN_PWM_180 = 700;
    const int MAX_PWM_180 = 2300;
  #endif
  #ifdef USE_Servo_180_M5Stack
    const int MIN_PWM_180 = 500;
    const int MAX_PWM_180 = 2500;
  #endif

  // 使用する360サーボのメーカー
  #define USE_Servo_360_TowerPro
  // #define USE_Servo_360_Feetech360
  // #define USE_Servo_360_M5Stack

  // サーボの種類毎のPWM幅や初期角度、回転速度のレンジ設定など
  #ifdef USE_Servo_360_TowerPro
    const int MIN_PWM_360 = 500;
    const int MAX_PWM_360 = 2400;
    // const int START_DEGREE_VALUE_SERVO_360 = 90;     // 360度サーボの停止位置：仕様では90で停止
    const int START_DEGREE_VALUE_SERVO_360 = 95;        // サーボ個体差で、90度指定で停止しなかった場合値を変えてみる（試作に使用したsg90-hvの場合95付近で停止だった)
    const int SERVO_DEG_RANGE_MAX = 12;
    const int SERVO_DEG_RANGE_MIN = -1 * SERVO_DEG_RANGE_MAX;
  #endif
  #ifdef USE_Servo_360_Feetech360
    const int MIN_PWM_360 = 700;
    const int MAX_PWM_360 = 2300;
    // const int START_DEGREE_VALUE_SERVO_360 = 90;     // 360度サーボの停止位置：仕様では90で停止
    const int START_DEGREE_VALUE_SERVO_360 = 93;        // サーボ個体差で、90度指定で停止しなかった場合値を変えてみる（試作に使用したsg90-hvの場合95付近で停止だった)
    const int SERVO_DEG_RANGE_MAX = 6;
    const int SERVO_DEG_RANGE_MIN = -1 * SERVO_DEG_RANGE_MAX;
  #endif
  #ifdef USE_Servo_360_M5Stack
    const int MIN_PWM_360 = 500;
    const int MAX_PWM_360 = 2500;
    const int START_DEGREE_VALUE_SERVO_360 = 90;         // 360度サーボの停止位置：仕様では90で停止（M5Stack公式は停止のレンジが85～95あたりと広めにとられている様子。手元では個体差なし）
    const int SERVO_DEG_RANGE_MAX = 12;
    const int SERVO_DEG_RANGE_MIN = -1 * SERVO_DEG_RANGE_MAX;
  #endif

  bool isRandomRunning = false;  // サーボ動作のフラグ
  bool isTestRunning = false;    // サーボテスト動作のフラグ
  unsigned long prevTime180 = 0, prevTime360 = 0;
  unsigned long interval180 = 0, interval360 = 0;

  int servo180_angle = 0; // 180サーボの角度用変数
  int servo360_speed = 0; // 360サーボの速度用変数

  // ボード種別毎のピン設定など
  #ifdef ARDUINO_M5STACK_Core2
    // M5Stack Core2用のサーボの設定
    // Port.A X:G33, Y:G32
    // Port.C X:G13, Y:G14
    // スタックチャン基板 X:G19, Y:G27
    #define SERVO_360_PIN 33
    #define SERVO_180_PIN 32
  #endif
  #ifdef ARDUINO_M5STACK_FIRE 
    // M5Stack Fireの場合はPort.A(X:G22, Y:G21)のみです。
    // I2Cと同時利用は不可
    #define SERVO_360_PIN 22
    #define SERVO_180_PIN 21
  #endif
  #ifdef ARDUINO_M5Stack_Core_ESP32 
    // M5Stack Basic/Gray/Go用の設定
    // Port.A X:G22, Y:G21
    // Port.C X:G16, Y:G17
    // スタックチャン基板 X:G5, Y:G2
    #define SERVO_360_PIN 22
    #define SERVO_180_PIN 21
  #endif
  #ifdef ARDUINO_M5STACK_CORES3
    // M5Stack CoreS3用の設定 ※暫定的にplatformio.iniにARDUINO_M5STACK_CORES3を定義しています。
    // Port.A X:G1 Y:G2
    // Port.B X:G8 Y:G9
    // Port.C X:18 Y:17
    #define SERVO_360_PIN 18 
    #define SERVO_180_PIN 17
    #include <gob_unifiedButton.hpp> // 2023/5/12現在 M5UnifiedにBtnA等がないのでGobさんのライブラリを使用
    goblib::UnifiedButton unifiedButton;
  #endif

  bool core_port_a = false;         // Core1のPortAを使っているかどうか
  // ================================== End

  // ==================================
  // for Servo Running Mode

  // サーボのタイマーを初期化する関数
  void initializeServoTimers() {
    unsigned long now = millis();
    prevTime180 = now;
    prevTime360 = now;
    interval180 = random(0, 101);  // ボタンを押した直後はすぐに動作するように初期値として短い時間を設定
    interval360 = random(0, 101);
  }

  // ランダムモード開始時にタイマーをリセット
  void startRandomMode() {
    isRandomRunning = true;
    initializeServoTimers();  // タイマー初期化
  }

  // テストモード開始時にタイマーをリセット
  void startTestMode() {
    isTestRunning = true;
    initializeServoTimers();  // タイマー初期化
  }

  // ランダムモード
  void servoRandomRunningMode(unsigned long currentMillis) {

    // === 180°サーボの動作 (5秒〜10秒間隔) ===
    if (currentMillis - prevTime180 >= interval180) {
      prevTime180 = currentMillis;
      interval180 = random(5000, 10001); // 5秒〜10秒のランダム間隔

      // 90 ～ 50 度の間の角度（初期位置は85）
      int rand_speed_offset_180 = random(-5, 36);
      servo180.startEaseTo(START_DEGREE_VALUE_SERVO_180 - rand_speed_offset_180);

      // 顔の向きが変わるタイミングで表情も変化させる
      changeExpression((rand_speed_offset_180 + 5) % 7);
    }

    // === 360°サーボの動作 (7秒〜30秒間隔) ===
    if (currentMillis - prevTime360 >= interval360) {
      prevTime360 = currentMillis;
      interval360 = random(7000, 30001); // 7秒〜30秒のランダム間隔

      // 66 ～ 114 度が示す速度（初期位置は90、2度刻み）
      int rand_speed_offset_360 = random(SERVO_DEG_RANGE_MIN, SERVO_DEG_RANGE_MAX)* 2;
      servo360.startEaseTo(START_DEGREE_VALUE_SERVO_360 + rand_speed_offset_360);
    }
  }

  // テストモード
  int count_180 = 0;
  int count_360 = 0;
  void servoTestRunningMode(unsigned long currentMillis) {

    // === 180°サーボの動作 (3秒間隔) ===
    if (currentMillis - prevTime180 >= interval180) {
      prevTime180 = currentMillis;
      interval180 = 3000; // 3秒間隔固定
      servo180.startEaseTo(START_DEGREE_VALUE_SERVO_180 - (count_180 % 8) * 5);
      count_180 = (count_180 + 1) % 99;
    }

    // === 360°サーボの動作 (5秒間隔) ===
    if (currentMillis - prevTime360 >= interval360) {
      prevTime360 = currentMillis;
      interval360 = 5000; // 5秒間隔固定

      // 60 ～ 120 度が示す速度（初期位置は90）
      servo360.startEaseTo(START_DEGREE_VALUE_SERVO_360 + (count_360 % 7) * 10 - 30);
      count_360 = (count_360 + 1) % 99;
    }
  }
  // ================================== End
#endif
// ================================== End

// ==================================
// for ESPNow
#ifdef USE_ESPNow
#include <WiFi.h>  // ESPNOWを使う場合はWiFiも必要
#include <esp_now.h> // ESPNOW本体

// ESP-NOW受信時に呼ばれる関数
void OnDataReceived(const uint8_t *mac_addr, const uint8_t *data, int data_len) {

  M5.Speaker.tone(1000, 100);
  delay(200);
  M5.Speaker.tone(1500, 100);
  delay(200);
  M5.Speaker.tone(2000, 100);
  delay(200);
  M5.Speaker.tone(2500, 100);

  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

  Serial.print("Last Packet Recv from: "); Serial.println(macStr);
  Serial.print("Last Packet Recv Data: "); Serial.println(*data);

  if (data[0] == 1) {
    if (!isRandomRunning || !isLedON) {
      #ifdef USE_Servo
        startRandomMode();
      #endif
      turn_on_led();
    } else {
      #ifdef USE_Servo
        servo360.startEaseTo(START_DEGREE_VALUE_SERVO_360);  // 360°サーボを停止
        isRandomRunning = false;
      #endif
      #ifdef USE_LED
        turn_off_led();
      #endif
    }
  }
}
#endif
// ================================== End

void setup() {
  // 設定用の情報を抽出
  auto cfg = M5.config();
  // M5Stackをcfgの設定で初期化
  M5.begin(cfg);

  M5.Speaker.setVolume(100);

#ifdef ARDUINO_M5STACK_FIRE 
  // M5Stack Fireの場合、Port.Aを使う場合は内部I2CをOffにする必要がある。
  M5.In_I2C.release();
#endif
#ifdef ARDUINO_M5STACK_CORES3 // CORES3のときに有効になります。
  // 画面上のタッチパネルを3分割してBtnA, B, Cとして使う設定。
  unifiedButton.begin(&M5.Display, goblib::UnifiedButton::appearance_t::transparent_all);
#endif

  // ログ設定
  M5.Log.setLogLevel(m5::log_target_display, ESP_LOG_NONE);    // M5Unifiedのログ初期化（画面には表示しない。)
  M5.Log.setLogLevel(m5::log_target_serial, ESP_LOG_INFO);     // M5Unifiedのログ初期化（シリアルモニターにESP_LOG_INFOのレベルのみ表示する)
  M5.Log.setEnableColor(m5::log_target_serial, false);         // M5Unifiedのログ初期化（ログをカラー化しない。）
  M5_LOGI("Hello World");                                      // logにHello Worldと表示
  SD.begin(GPIO_NUM_4, SPI, 25000000);                         // SDカードの初期化
  delay(2000);                                                 // SDカードの初期化を少し待ちます。

  system_config.loadConfig(SD, "");                            // SDカードから初期設定ファイルを読み込む
  if (M5.getBoard() == m5::board_t::board_M5Stack) {           // Core1かどうかの判断
    if (system_config.getServoInfo(AXIS_X)->pin == 22) {       // サーボのGPIOが22であるか確認（本当は21も確認してもいいかもしれないが省略）
      // M5Stack Coreの場合、Port.Aを使う場合は内部I2CをOffにする必要がある。バッテリー表示は不可。
      M5_LOGI("I2CRelease");              // ログに出力
      avatar.setBatteryIcon(false);       // avatarのバッテリーアイコンを表示しないモードに設定
      M5.In_I2C.release();                // I2Cを使わないように設定(IMUやIP5306との通信をしないようにします。)※設定しないとサーボの動きがおかしくなります。
      core_port_a = true;                 // Core1でポートAを使用しているフラグをtrueに設定
    }
  } else {
    avatar.setBatteryIcon(true);          // Core2以降の場合は、バッテリーアイコンを表示する。
  }

  #ifdef USE_Avatar
    // ---------------------------------------------------------------
    // Avatarの初期化
    // ---------------------------------------------------------------
    // 顔の色変更
    // TFT_WHITEなど既定の色が多数用意されている
    // オリジナルの色は以下のコードで定義可能
    // uint16_t customColor = 0;
    // customColor = M5.Lcd.color565(255,140,50);
    cp = new ColorPalette();
    cp->set(COLOR_PRIMARY, TFT_WHITE);
    cp->set(COLOR_BACKGROUND, TFT_BLACK);
    avatar.setColorPalette(*cp);
    avatar.init(8);
    avatar.setBatteryIcon(false);                       // バッテリーアイコンの表示／非表示
    avatar.setSpeechFont(&fonts::lgfxJapanGothicP_12);  // フォントの指定
    last_mouth_millis = millis();    // loop内で使用するのですが、処理を止めずにタイマーを実行するための変数です。一定時間で口を開くのとセリフを切り替えるのに利用します。
  #endif
  
  // ---------------------------------------------------------------
  // LEDの初期化
  // ---------------------------------------------------------------
  #ifdef USE_LED
    #ifdef USE_LED_NEKOMIMI
      FastLED.addLeds<SK6812, LEDS_PIN_NECO, GRB>(necomimi_leds, NUM_LEDS_NECO);  // Necomimi LED
    #endif
    #ifdef USE_LED_GOBOTTOM
      FastLED.addLeds<NEOPIXEL, LEDS_PIN_GOBTM>(gobottom_leds, NUM_LEDS_GOBTM);   // GoBottom LED
    #endif
    FastLED.setBrightness(10);
    turn_on_led();
    delay(3000);
    turn_off_led();
  #endif

  // ---------------------------------------------------------------
  // servoの初期化
  // ---------------------------------------------------------------
  #ifdef USE_Servo
    M5_LOGI("attach servo");

    ESP32PWM::allocateTimer(0); // ESP32Servoはタイマーを割り当てる必要がある
    ESP32PWM::allocateTimer(1);

    servo180.setPeriodHertz(50);  // サーボ用のPWMを50Hzに設定
    servo360.setPeriodHertz(50);
    
    servo180.attach(SERVO_180_PIN);
    servo360.attach(SERVO_360_PIN);
    
    servo180.setEasingType(EASE_QUADRATIC_IN_OUT);       // 動きの最初と終わりがスムーズになる
    servo360.setEasingType(EASE_QUADRATIC_IN_OUT);       // 一定の速度で動かす場合は EASE_LINEAR に変更

    setSpeedForAllServos(60);

    servo180.startEaseTo(START_DEGREE_VALUE_SERVO_180);  // 180°サーボを初期位置にセット
    servo360.startEaseTo(START_DEGREE_VALUE_SERVO_360);  // 360°サーボを停止状態にセット

    M5.Power.setExtOutput(!system_config.getUseTakaoBase());       // 設定ファイルのTakaoBaseがtrueの場合は、Groveポートの5V出力をONにする。
    M5_LOGI("ServoType: %d\n", system_config.getServoType());      // サーボのタイプをログに出力

    // ランダム動作用の変数初期化、個体差を出すためMACアドレスを使用する
    uint8_t mac[6];
    esp_efuse_mac_get_default(mac);
    uint32_t seed = mac[0] | (mac[1] << 8) | (mac[2] << 16) | (mac[3] << 24);
    randomSeed(seed);
    interval180 = random(5000, 10001); // 5秒〜10秒のランダム間隔
    interval360 = random(7000, 30001); // 7秒〜30秒のランダム間隔
  #endif

  // ---------------------------------------------------------------
  // ESPNowの初期化
  // ---------------------------------------------------------------
  #ifdef USE_ESPNow
    // WiFi初期化
    WiFi.mode(WIFI_STA);

    // ESP-NOWの初期化(出来なければリセットして繰り返し)
    if (esp_now_init() != ESP_OK) {
      return;
    }

    // ESP-NOW受信時に呼ばれる関数の登録
    esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataReceived));
  #endif

}

void loop() {
  M5.update();

  // === ボタンAが押されたらテスト動作モードの開始/停止を切り替え ===
  if (M5.BtnA.wasPressed()) {
    M5.Speaker.tone(1500, 200);
    avatar.setExpression(Expression::Happy);
    avatar.setSpeechText("テストモード");
    if (!isTestRunning) {
      #ifdef USE_LED
        turn_on_led();
      #endif
      startTestMode();
    } else {
      #ifdef USE_LED
        turn_off_led();
      #endif
      isTestRunning = false;
      isRandomRunning = false;  // ランダムモードのフラグは強制終了
      servo180.startEaseTo(START_DEGREE_VALUE_SERVO_180);  // 180°サーボを初期位置に戻す
      servo360.startEaseTo(START_DEGREE_VALUE_SERVO_360);  // 360°サーボを停止
      avatar.setExpression(Expression::Neutral);
      avatar.setSpeechText("");
    }
  }

  // === 強制停止 ===
  if (M5.BtnB.wasPressed()) {
    M5.Speaker.tone(2500, 200);
    avatar.setSpeechText("停止");
    isTestRunning = false;
    isRandomRunning = false;  // ランダムモードのフラグは強制終了
    servo180.startEaseTo(START_DEGREE_VALUE_SERVO_180);  // 180°サーボを初期位置に戻す
    servo360.startEaseTo(START_DEGREE_VALUE_SERVO_360);  // 360°サーボを停止
    avatar.setExpression(Expression::Neutral);
    delay(1500);
    avatar.setSpeechText("");

    #ifdef USE_LED
      turn_off_led();
    #endif
  }

  // === ボタンCが押されたらランダム動作モードの開始/停止を切り替え ===
  if (M5.BtnC.wasPressed()) {
    M5.Speaker.tone(1000, 200);
    avatar.setSpeechText("");
    if (!isRandomRunning) {
      #ifdef USE_LED
        turn_on_led();
      #endif
      startRandomMode();
    } else {
      #ifdef USE_LED
        turn_off_led();
      #endif
      isRandomRunning = false;
      isTestRunning = false;  // テストモードのフラグは強制終了
      servo180.startEaseTo(START_DEGREE_VALUE_SERVO_180);  // 180°サーボを初期位置に戻す
      servo360.startEaseTo(START_DEGREE_VALUE_SERVO_360);  // 360°サーボを停止
      avatar.setExpression(Expression::Neutral);
      avatar.setSpeechText("");
    }

  }
  
#ifdef USE_Servo
  if (!isRandomRunning) return;  // 停止中なら何もしない
  unsigned long currentMillis = millis();
  // ランダム動作モード（デフォルト）
  if (isRandomRunning) servoRandomRunningMode(currentMillis);
  // ★テストモード（段階的に回転速度を変えるデモ）を動かしたい場合はこちらを使用
  // if (isRandomRunning) servoTestRunningMode(currentMillis);
#endif

}
