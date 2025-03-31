
#include <HardwareSerial.h>
#include "Arduino.h"
#include "MsTimer2.h"
#include "U8g2lib.h"
#include "DropletCounter.h"

#define TRIGGER_THRESHOLD 500 // 液滴触发阈值
#define SAMPLING_DURATION 3000   // 取样时长，单位：ms
#define MAX_TIMESTAMPS 100   // 最大时间戳数量，同时代表取样时长内最大可记录液滴数
#define DISPLAY_REFRESH_INTERVAL 500 // 显示屏刷新间隔，单位：ms

unsigned long current_millis;
U8G2_ST7920_128X64_F_SW_SPI lcd(U8G2_R0, 13, 12, 11, 15);  //屏幕初始化
DropletCounter sensor1(MAX_TIMESTAMPS, SAMPLING_DURATION, TRIGGER_THRESHOLD, A14);
DropletCounter sensor2(MAX_TIMESTAMPS, SAMPLING_DURATION, TRIGGER_THRESHOLD, A13);
DropletCounter sensor3(MAX_TIMESTAMPS, SAMPLING_DURATION, TRIGGER_THRESHOLD, A12);

/**
 * 显示数据到屏幕
 * @param droplet  液滴结构体指针
 * @param str      显示前缀
 * @param yOffset  显示行像素偏移量
 */
void draw_data_line(DropletCounter *droplet, const char str[], const int yOffset) {
    lcd.setCursor(0, yOffset);
    lcd.print(str);
    lcd.setCursor(15, yOffset);
    lcd.print(droplet->getDropletsPerSecond(), 1); // 保留1位小数
    lcd.setCursor(55, yOffset);
    lcd.print("D/s");
}

void update_display() {
    sensor1.calculate(current_millis);
    sensor2.calculate(current_millis);
    sensor3.calculate(current_millis);

    lcd.firstPage();
    do {
        lcd.setFont(u8g2_font_ncenB10_tr);
        draw_data_line(&sensor1, "1:", 15);
        draw_data_line(&sensor2, "2:", 35);
        draw_data_line(&sensor3, "3:", 55);
    } while (lcd.nextPage());
}

void setup() {
    Serial.begin(9600);
    MsTimer2::set(DISPLAY_REFRESH_INTERVAL, update_display);
    MsTimer2::start();
    lcd.begin();
    pinMode(14, OUTPUT);
}

void loop() {
    current_millis = millis();
    sensor1.update(current_millis);
    sensor2.update(current_millis);
    sensor3.update(current_millis);
}