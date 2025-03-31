
#include <HardwareSerial.h>
#include "Arduino.h"
#include "MsTimer2.h"
#include "U8g2lib.h"
#include "DropletCounter.h"

#define TRIGGER_THRESHOLD 500 // 液滴触发阈值
#define SAMPLING_DURATION 3000   // 取样时长，单位：ms
#define MAX_TIMESTAMPS 100   // 最大时间戳数量，同时代表取样时长内最大可记录液滴数
#define DISPLAY_REFRESH_INTERVAL 500 // 显示屏刷新间隔，单位：ms

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, 13, 12, 11, 15);  //屏幕初始化
DropletCounter V1(MAX_TIMESTAMPS, SAMPLING_DURATION, TRIGGER_THRESHOLD, A14);
DropletCounter V2(MAX_TIMESTAMPS, SAMPLING_DURATION, TRIGGER_THRESHOLD, A13);
DropletCounter V3(MAX_TIMESTAMPS, SAMPLING_DURATION, TRIGGER_THRESHOLD, A12);

/**
 * 显示数据到屏幕
 * @param droplet 液滴结构体指针
 * @param str     显示前缀
 * @param offset  显示行偏移
 */
void draw_data(DropletCounter *droplet, const char str[], int offset) {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.setCursor(0, offset);
    u8g2.print(str);
    u8g2.setCursor(15, offset);
    u8g2.print(droplet->getDropletsPerSecond(), 1); // 保留1位小数
    u8g2.setCursor(55, offset);
    u8g2.print("D/s");
}

void update_display() {
    unsigned long current_millis = millis();
    V1.calculate(current_millis);
    V2.calculate(current_millis);
    V3.calculate(current_millis);

    u8g2.firstPage();
    do {
        draw_data(&V1, "1:", 15);
        draw_data(&V2, "2:", 35);
        draw_data(&V3, "3:", 55);
    } while (u8g2.nextPage());
}

void setup() {
    Serial.begin(9600);
    MsTimer2::set(DISPLAY_REFRESH_INTERVAL, update_display);
    MsTimer2::start();
    u8g2.begin();
    pinMode(14, OUTPUT);
}

void loop() {
    unsigned long current_millis = millis();
    V1.update(current_millis);
    V2.update(current_millis);
    V3.update(current_millis);
}