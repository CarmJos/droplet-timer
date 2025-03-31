
#include <HardwareSerial.h>
#include "Arduino.h"
#include "MsTimer2.h"
#include "U8g2lib.h"
#include "DropletSensor.h"

#define SAMPLING_INTERVAL 50 // 取样间隔，单位：ms
#define SAMPLING_TIMES 5 // 取样次数，0~255；
#define TRIGGER_THRESHOLD 500 // 液滴触发阈值

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, 13, 12, 11, 15);  //屏幕初始化

DropletSensor V1(SAMPLING_INTERVAL, SAMPLING_TIMES, TRIGGER_THRESHOLD, A14);
DropletSensor V2(SAMPLING_INTERVAL, SAMPLING_TIMES, TRIGGER_THRESHOLD, A13);
DropletSensor V3(SAMPLING_INTERVAL, SAMPLING_TIMES, TRIGGER_THRESHOLD, A12);

void display(DropletSensor *droplet, const char str[], const int offset) {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.setCursor(0, offset);
    u8g2.print(str);
    u8g2.setCursor(15, offset);
    u8g2.print(droplet->getDropletsPerSecond(), 2);
    u8g2.setCursor(55, offset);
    u8g2.print("D/s |");
    u8g2.setCursor(95, offset);
    u8g2.print(droplet->getCurrentSeconds(), 1);
}

void onTimer() {
    V1.tick();
    V2.tick();
    V3.tick();
}

void setup() {
    Serial.begin(9600);
    MsTimer2::set(SAMPLING_INTERVAL, onTimer);
    MsTimer2::start();
    pinMode(14, OUTPUT);
    u8g2.begin();
}

void loop() {
    u8g2.firstPage();  //帧函数

    V1.read();
    V2.read();
    V3.read();

    do {
        display(&V1, "1:", 15);
        display(&V2, "2:", 35);
        display(&V3, "3:", 55);
    } while (u8g2.nextPage());
}

