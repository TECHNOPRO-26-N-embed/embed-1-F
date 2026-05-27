# グローバル変数・定数早見表

| No | 名前 | 型 | 初期値 | 説明 |
|:--|:--|:--|:--|:--|
| 1 | `pinSoundSensor` | `const uint8_t` | `ApinA0` | サウンドセンサー（アナログ入力） |
| 2 | `pinDht11` | `const uint8_t` | `Dpin7` | DHT11温湿度センサー |
| 3 | `pinButton` | `const uint8_t` | `Dpin2` | ボタン（INPUT_PULLUP） |
| 4 | `pinMotorIn1` | `const uint8_t` | `Dpin8` | モータードライバ入力1（L293D） |
| 5 | `pinMotorIn2` | `const uint8_t` | `Dpin9` | モータードライバ入力2（L293D） |
| 6 | `pinMotorEn` | `const uint8_t` | `Dpin10` | モータードライバ有効化（L293D、PWM可） |
| 7 | `pinBuzzer` | `const uint8_t` | `Dpin12` | ブザー |
| 8 | `pin7segD1` | `const uint8_t` | `Dpin3` | 7セグメント制御ピン1 |
| 9 | `pin7segD2` | `const uint8_t` | `Dpin4` | 7セグメント制御ピン2 |
| 10 | `pin7segD3` | `const uint8_t` | `Dpin5` | 7セグメント制御ピン3 |
| 11 | `pin7segD4` | `const uint8_t` | `Dpin6` | 7セグメント制御ピン4 |
| 12 | `currentState` | `int` | `0` | 状態管理（0:待機中, 1:モーター作動, 2:機能停止） |
| 13 | `soundStartMillis` | `unsigned long` | `0` | 音の検知が始まった時刻 |
| 14 | `overTempStartMillis` | `unsigned long` | `0` | 温度>=28℃が継続し始めた時刻 |
| 15 | `lastSensorReadMillis` | `unsigned long` | `0` | センサーを最後に読んだ時刻 |
| 16 | `soundDetected` | `bool` | `false` | サウンドセンサーの検知結果 |
| 17 | `soundValue` | `int` | `0` | サウンドセンサーの移動平均値（ノイズ除去後ADC値） |
| 18 | `temperatureC` | `float` | `0.0` | 現在の温度（℃） |
| 19 | `humidityPct` | `float` | `0.0` | 現在の湿度（%） |
| 20 | `buttonPressed` | `bool` | `false` | チャタリング処理後の現在ボタン状態 |
| 21 | `buttonPrevState` | `bool` | `false` | 前回のボタン状態 |
| 22 | `lastDebounceTime` | `unsigned long` | `0` | 前回の判定時刻（ms） |
| 23 | `motorOnTempC` | `float` | `28.0` | モーターON温度しきい値 |
| 24 | `motorOffTempC` | `float` | `26.0` | モーターOFF温度しきい値 |
| 25 | `overTempRequiredMs` | `unsigned long` | `5000` | モーター起動に必要な継続時間（ms） |
| 26 | `sensorReadIntervalMs` | `unsigned long` | `100` | センサー読取り周期（ms） |
| 27 | `abnormalLogFlag` | `bool` | `false` | 異常値検出フラグ |
| 28 | `abnormalLogBuffer` | `char[64]` | `""` | 異常値内容バッファ |