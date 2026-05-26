#include <DHT.h>
#include <stdio.h>
// =============================================
// ピン定義
// =============================================
const uint8_t pinSoundSensor = A1;
const uint8_t pinDht11 = 7;
const uint8_t pinButton = 2;
const uint8_t pinMotorIn1 = 8;
const uint8_t pinMotorIn2 = 9;
const uint8_t pinMotorEn = 10;
// =============================================
// 状態管理
// =============================================
int currentState = 0;
// =============================================
// タイマー管理
// =============================================
unsigned long soundStartMillis    = 0;   // 音の検知が始まった時刻
unsigned long overTempStartMillis = 0;   // 温度>=28℃が継続し始めた時刻
unsigned long lastSensorReadMillis = 0;  // センサーを最後に読んだ時刻
// =============================================
// センサー・入力値
// =============================================
bool soundDetected   = false;    // サウンドセンサーの検知結果
int soundValue       = 0;        // サウンドセンサーの移動平均値（ノイズ除去後ADC値）
int soundBuffer[5]   = {0, 0, 0, 0, 0}; // サウンドセンサーの過去5回分の値
float temperatureC   = 0.0;      // 現在の温度（℃）
float humidityPct    = 0.0;      // 現在の湿度（%）
bool buttonPressEvent = false;   // 押下イベント（非押下→押下の瞬間のみtrue）
// --- チャタリング対策用 ---
bool buttonPrevState = false;           // 前回のボタン状態
unsigned long lastDebounceTime = 0;     // 前回の判定時刻（ms）
// =============================================
// カウンター・フラグ
// =============================================
float motorOnTempC   = 20.0;//28.0;     // モーターON温度しきい値
float motorOffTempC  = 18.0;//26.0;     // モーターOFF温度しきい値
unsigned long overTempRequiredMs = 5000; // モーター起動に必要な継続時間（ms）
unsigned long sensorReadIntervalMs = 100; // センサー読取り周期（ms）
bool abnormalLogFlag = false;    // 異常値検出フラグ
char abnormalLogBuffer[64] = ""; // 異常値内容バッファ
bool tempStartConditionMet = false;       // 温度起動条件成立フラグ（judgeTemperatureで更新）
bool tempStopConditionMet = false;        // 温度停止条件成立フラグ（judgeTemperatureで更新）
bool soundTimeoutStopConditionMet = false; // 無音60秒停止条件成立フラグ（judgeSoundで更新）

#define SOUND_THRESHOLD 110

#define DHTTYPE DHT11

DHT dht(pinDht11, DHTTYPE);

// =======================================================================================================================
// =======================================================================================================================

bool readButton(unsigned long now){
  bool rawPressed = (digitalRead(pinButton) == LOW); // INPUT_PULLUP: 押下でLOW
  buttonPressEvent = false;

  if (rawPressed != buttonPrevState && now - lastDebounceTime >= 50) {
    if (buttonPrevState == false && rawPressed == true) {
      buttonPressEvent = true;
    }
    buttonPrevState = rawPressed;
    lastDebounceTime = now;
  }
  return buttonPressEvent;
}
// 【処理の流れ】
// 1. 「前回のボタン状態」「前回の判定時刻」を使ってデバウンス判定を行う
// 2. 現在のボタン入力値（digitalRead(pinButton)）を取得する
// 3. 入力が変化しており、かつ 50ms 以上経過していれば確定入力として buttonPressed を更新する
// 4. 前回が非押下（HIGH）かつ今回が押下（LOW）のときのみ buttonPressEvent = true にする
// 5. それ以外は buttonPressEvent = false にする
// 6. buttonPressEvent を戻り値として返す

// 【エラーケース】
// - digitalRead()が異常値を返した場合やピン未接続時は、安全側（非押下=false）として扱う

//  — サウンドセンサー値を取得し音検知結果を更新する
bool detectSound(){
  int nowSound = analogRead(pinSoundSensor);
  if(nowSound>=0 && nowSound<=1023){
    for (int i = 4; i > 0; i--) {
      soundBuffer[i] = soundBuffer[i - 1];
    }
    soundBuffer[0] = nowSound;
    int ValueSum = 0;
    for (int i = 0; i < 5; i++) {
      ValueSum += soundBuffer[i];
    }
    soundValue = ValueSum / 5;
    Serial.print("A1=");
    Serial.print(nowSound);
    Serial.print(" Ave=");
    Serial.println(soundValue);

    if(soundValue >= SOUND_THRESHOLD){
      soundDetected = true;
      Serial.print("soundDetected = true\n");
      return soundDetected;
    }else{
      soundDetected = false;
      return soundDetected;
    }
  }else{
    abnormalLogFlag = true;
    snprintf(abnormalLogBuffer, sizeof(abnormalLogBuffer), "Sound sensor read error: %d", nowSound);
    soundDetected = false;
    return soundDetected;
  }
}
// 【処理の流れ】
// 1. analogRead(pinSoundSensor) でサウンドセンサーの値を取得し、一時変数に格納
// 2. 取得値が0～1023の範囲内か確認し、異常値なら前回値を保持し abnormalLogFlag を立てる
// 3. 5点移動平均を計算し、ノイズを除去した値を soundValue に格納
// 4. 閾値（例: 40dB相当のADC値）と比較し、音を検知したか判定
// 5. 判定結果を soundDetected 変数に反映し、true/false を戻り値として返す

// 【エラーケース】
// - analogRead()が範囲外の値を返した場合は、前回正常値を保持し、abnormalLogBufferに内容を記録


// — 音の有無と継続時間を判定し状態遷移に反映する
void judgeSound(unsigned long now){
  if(soundDetected==true){
    soundStartMillis=0;
  }else if(soundDetected == false && soundStartMillis == 0){
    soundStartMillis = now;
  }

  if(soundDetected == false && now - soundStartMillis >= 60000){
    soundTimeoutStopConditionMet = true;
  }else{
    soundTimeoutStopConditionMet = false;
  }
}
// 【処理の流れ】
// 1. soundTimeoutStopConditionMet を false で初期化
// 2. soundDetected が true の場合：soundStartMillis を 0 にリセット（無検知タイマをリセット）
// 3. soundDetected が false かつ soundStartMillis == 0 の場合：soundStartMillis = now（引数）を記録
// 4. soundDetected が false のまま now - soundStartMillis >= 60000ms（60秒）なら soundTimeoutStopConditionMet = true
// 5. 状態遷移用フラグ soundTimeoutStopConditionMet を更新して終了

// 【エラーケース】
// - soundDetected の値が不定や異常な場合は、安全側（無検知扱い）として soundTimeoutStopConditionMet = true にする

// — DHT11から温度を取得してtemperatureCを更新する
float readTemperature(){
  float nowTemperature = dht.readTemperature();
  Serial.print("T = ");
  Serial.println(temperatureC);
  Serial.print("\n");
  if(isfinite(nowTemperature)&&(nowTemperature>=0 && nowTemperature<=50)){
      temperatureC = nowTemperature;
      Serial.print("温度の閾値を超えました\n");
      return temperatureC;
  }else{
    abnormalLogFlag=true;
    snprintf(abnormalLogBuffer, sizeof(abnormalLogBuffer), "DHT11 read error: %.2f", nowTemperature);
    return temperatureC;
  }
}
// 【処理の流れ】
// 1. DHT11ライブラリの readTemperature() で温度値を取得し、一時変数に格納
// 2. 取得値が isfinite() で有限かつ0～50℃の範囲内か判定。異常値なら前回値を保持し abnormalLogFlag を立てる
// 3. 正常値なら temperatureC 変数を更新
// 4. 取得した温度値を戻り値として返す

// 【エラーケース】
// - 取得値がNaNや範囲外の場合は、前回正常値を保持し、abnormalLogBufferに内容を記録

// — 温度しきい値を判定して起動・停止条件を決定する
void judgeTemperature(unsigned long now){
  tempStartConditionMet = false;
  tempStopConditionMet = false;

  if(temperatureC >= motorOnTempC){
    if(overTempStartMillis == 0){
      overTempStartMillis = now;
    }
    if(now - overTempStartMillis >= overTempRequiredMs){
      tempStartConditionMet = true;
    }
  }else{
    overTempStartMillis = 0;
  }

  if(temperatureC <= motorOffTempC){
    tempStopConditionMet = true;
  }else{
    tempStopConditionMet = false;
  }
}
// 【処理の流れ】
// 1. tempStartConditionMet = false、tempStopConditionMet = false で初期化
// 2. temperatureC >= motorOnTempC（28℃以上）の場合：
//    - overTempStartMillis が 0 なら overTempStartMillis = now（引数）を記録
//    - now - overTempStartMillis >= overTempRequiredMs（5000ms）なら tempStartConditionMet = true
// 3. temperatureC < motorOnTempC の場合：overTempStartMillis = 0 にリセット（継続時間をリセット）
// 4. temperatureC <= motorOffTempC（26℃以下）の場合：tempStopConditionMet = true
// 5. 状態遷移用フラグ tempStartConditionMet / tempStopConditionMet を更新して終了

// 【エラーケース】
// - temperatureC が異常値や不定の場合は、安全側（ファン停止）として扱う



// ### `readHumidity()` — DHT11から湿度を取得してhumidityPctを更新する
// **basic_design.md 2-2 との対応：** DHT11から湿度を取得しhumidityPct(変数)を更新

// **引数：**
// - なし

// **戻り値：**
// - `float`：取得した湿度（%）

// ```
// 【処理の流れ】
// 1. DHT11ライブラリの readHumidity() で湿度値を取得し、一時変数に格納
// 2. 取得値が isfinite() で有限かつ0～100%の範囲内か判定。異常値なら前回値を保持し abnormalLogFlag を立てる
// 3. 正常値なら humidityPct 変数を更新
// 4. 取得した湿度値を戻り値として返す

// 【エラーケース】
// - 取得値がNaNや範囲外の場合は、前回正常値を保持し、abnormalLogBufferに内容を記録
// ```

// ---

// — 現在状態に応じてファンのON/OFFを切り替える
void controlFan(){
  if(currentState==1){
    digitalWrite(pinMotorIn1, HIGH);
    digitalWrite(pinMotorIn2, LOW);
    analogWrite(pinMotorEn, 200);
  }else{
    digitalWrite(pinMotorIn1, LOW);
    digitalWrite(pinMotorIn2, LOW);
    analogWrite(pinMotorEn, 0);
  }
}
// 【処理の流れ】
// 1. currentState が 1（モーター作動）の場合：
//    - pinMotorIn1, pinMotorIn2, pinMotorEn をON側（例: In1=HIGH, In2=LOW, En=PWM値）に設定しファンを回す
// 2. currentState が 0または2（待機/機能停止）の場合：
//    - pinMotorIn1, pinMotorIn2, pinMotorEn をOFF側（例: In1=LOW, In2=LOW, En=0）に設定しファンを停止
// 3. 状態遷移や安全側制御が必要な場合は、常にOFF側を優先

// 【エラーケース】
// - currentStateやピン出力が不定・異常な場合は、安全側（ファンOFF・出力OFF）として全ピンLOW/En=0に設定

// — ボタン押下時に機能停止状態への遷移を処理する
void handleStop(){
  if((currentState==1 || currentState==0)){
    currentState=2;
    controlFan();
    return;
  }else if(currentState==2){
    currentState=0;
    return;
  }
}

// 【処理の流れ】
// 1. 前提条件：buttonPressEvent == true のときに loop() から呼び出す
// 2. currentState=2（機能停止）へ遷移
// 3. controlFan()でファンをOFF
// 4. 必要に応じてブザーや7セグメント表示も停止状態にする

// 【エラーケース】
// - 異常な値が来た場合：
//    - currentStateやbuttonPressEventが不定の場合も、安全側（ファンOFF・出力OFF）を優先
//    - 異常値検出時はabnormalLogBufferに内容を記録し、abnormalLogFlagを立てる

// — 異常値をシリアルにログ出力する
void logAbnormalValues(){
  if(abnormalLogFlag==true){
    Serial.println("エラーログ:");
    Serial.println(abnormalLogBuffer);
  }
  abnormalLogFlag = false;
  abnormalLogBuffer[0] = '\0';
}

// 【処理の流れ】
// 1. abnormalLogFlag が true かどうか判定
// 2. true の場合、abnormalLogBuffer の内容を Serial.println() でシリアル出力
// 3. ログ出力後、abnormalLogFlag を false にリセット
// 4. abnormalLogBuffer の内容もクリア（空文字にする）

// 【エラーケース】
// - 異常な値が来た場合：シリアル通信が未初期化の場合やバッファ内容が不正な場合は、何もせず安全側（リセットのみ）とする


void setup() {
  pinMode(pinButton,INPUT_PULLUP);
  pinMode(pinSoundSensor, INPUT);
  pinMode(pinDht11, INPUT);
  pinMode(pinMotorIn1, OUTPUT);
  pinMode(pinMotorIn2, OUTPUT);
  pinMode(pinMotorEn, OUTPUT);

  Serial.begin(9600);
  dht.begin();

  digitalWrite(pinMotorIn1, LOW);
  digitalWrite(pinMotorIn2, LOW);
  currentState = 0;
  soundStartMillis = 0;
  overTempStartMillis = 0;
  lastSensorReadMillis = millis();

}

void loop() {
  unsigned long now = millis();
  readButton(now);
  if((now - lastSensorReadMillis) >= sensorReadIntervalMs){
    detectSound();
    readTemperature();
    lastSensorReadMillis = now;
    judgeSound(now);
    judgeTemperature(now);
  }
  
  if(abnormalLogFlag==true){
    logAbnormalValues();
  }

  if(currentState == 0){
    if(buttonPressEvent == true){handleStop(); return;}
    if(soundDetected == true && tempStartConditionMet == true){
      currentState = 1;
      controlFan();
      return;
    }
  }else if(currentState == 1){
    if(buttonPressEvent == true){handleStop(); return;}
    if(tempStopConditionMet == true || soundTimeoutStopConditionMet == true){
      currentState = 0;
      controlFan();
      return;
    }
  }else{
    currentState = 2;
    controlFan();
    if(buttonPressEvent == true){
      currentState = 0;
    }
    return;
  }
}
