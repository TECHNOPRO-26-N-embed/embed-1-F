# 関数早見表

| 関数名 | 概要 | 引数 | 入力 | 出力 |
|:--|:--|:--|:--|:--|
| `setup()` | 初期化処理を行う | なし | なし | なし |
| `loop()` | メインループ処理 | なし | なし | なし |
| `readButton()` | ボタン入力を読み取り、押下イベントを更新 | `unsigned long now` | 現在時刻 | 押下イベントフラグ |
| `detectSound()` | 音センサーの値を読み取り、音検知フラグを更新 | なし | なし | 音検知フラグ |
| `judgeSound()` | 音検知フラグを基に停止条件を判定 | `unsigned long now` | 現在時刻 | 停止条件フラグ |
| `readTemperature()` | 温度センサーの値を読み取る | なし | なし | 温度値 |
| `judgeTemperature()` | 温度値を基に開始・停止条件を判定 | `unsigned long now` | 現在時刻 | 条件フラグ |
| `controlFan()` | モーターの制御を行う | `int command` | 制御指示 | なし |
| `handleStop()` | 停止処理を行う | なし | なし | なし |
| `displayTemperature()` | 温度を表示する | `float temperatureC` | 温度値 | なし |
| `controlAlert()` | アラートを制御する | `bool alertCondition` | アラート条件 | なし |
| `logAbnormalValues()` | 異常値をログに記録する | `const char* message` | 異常値 | なし |
