#include <stdio.h>
#include <stdbool.h>
#include "music_player.h"

// ハードウェア制御用の関数プロトタイプ（実装は別途）
void fan_on();
void fan_off();
void led_on();
void led_off();
bool detect_sound();
void delay_ms(int ms);

int main(void) {
	while (1) {
		if (detect_sound()) {
			// 音を検知したらファンON、LED ON、音楽再生
			fan_on();
			led_on();
			play_music(0); // 0番の音楽を再生（複数ある場合は引数で切替）
			delay_ms(5000); // 5秒間動作
			fan_off();
			led_off();
		}
		delay_ms(100); // 音検知の間隔
	}
	return 0;
}
