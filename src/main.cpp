
#include <HardwareSerial.h>
#include "Arduino.h"
#include "MsTimer2.h"
#include "U8g2lib.h"

#define TRIGGER_THRESHOLD 500 // 液滴触发阈值
#define SAMPLING_INTERVAL 50 // 取样间隔，单位：ms
#define SAMPLING_DURATION 3000   // 取样时长，单位：ms
#define MAX_TIMESTAMPS 100   // 最大时间戳数量，同时代表取样时长内最大可记录液滴数

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, 13, 12, 11, 15);  //屏幕初始化

typedef struct {
    bool flag;                // 触发标志位，防止重复记录
    unsigned long timestamps[MAX_TIMESTAMPS]; // 时间戳数组
    int count;                // 有效时间戳数量
    double average;        // 每秒液滴数
} DropletInterval;

/**
 * 记录液滴时间戳并清理过期数据
 * @param droplet 液滴结构体指针
 */
void droplet_record(const unsigned long current_millis, DropletInterval *droplet) {
    if (droplet->flag) return; // 防止重复触发

    // 清理超过1秒的旧数据
    int newCount = 0;
    for (int i = 0; i < droplet->count; i++) {
        if (current_millis - droplet->timestamps[i] <= SAMPLING_DURATION) {
            droplet->timestamps[newCount++] = droplet->timestamps[i];
        }
    }
    droplet->count = newCount;

    // 添加新时间戳
    if (droplet->count < MAX_TIMESTAMPS) {
        droplet->timestamps[droplet->count++] = current_millis;
    } else {
        // 数组满时覆盖最旧数据（已按时间排序，直接覆盖首元素）
        memmove(droplet->timestamps, droplet->timestamps + 1, sizeof(unsigned long) * (MAX_TIMESTAMPS - 1));
        droplet->timestamps[MAX_TIMESTAMPS - 1] = current_millis;
    }
}

void droplet_calculate(const unsigned long current_millis, DropletInterval *droplet) {
    int count = 0;

    // 计算过去1秒内的液滴数
    for (int i = 0; i < droplet->count; i++) {
        if (current_millis - droplet->timestamps[i] <= SAMPLING_DURATION) {
            count++; // 满足条件的液滴数
        }
    }

    // 更新每秒液滴数
    droplet->average = (double) count / (SAMPLING_DURATION / 1000.0);
}

/**
 * 读取传感器并更新状态
 * @param droplet 液滴结构体指针
 * @param pin     传感器引脚
 */
void sensor_read(const unsigned long current_millis, DropletInterval *droplet, const uint8_t pin) {
    int val = analogRead(pin);
    if (val > TRIGGER_THRESHOLD) {
        droplet->flag = false; // 重置触发标志
    } else if (!droplet->flag) {
        droplet_record(current_millis, droplet);       // 首次触发时记录
        droplet->flag = true;  // 标记为已触发
    }
}

/**
 * 显示数据到屏幕
 * @param droplet 液滴结构体指针
 * @param str     显示前缀
 * @param offset  显示行偏移
 */
void draw_data(DropletInterval *droplet, const char str[], int offset) {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.setCursor(0, offset);
    u8g2.print(str);
    u8g2.setCursor(15, offset);
    u8g2.print(droplet->average, 1); // 保留1位小数
    u8g2.setCursor(55, offset);
    u8g2.print("D/s");
}

DropletInterval V1 = {true, {0}, 0, 0}; // 初始化三个传感器
DropletInterval V2 = {true, {0}, 0, 0};
DropletInterval V3 = {true, {0}, 0, 0};

void update_display() {
    unsigned long current_millis = millis();

    droplet_calculate(current_millis, &V1);
    droplet_calculate(current_millis, &V2);
    droplet_calculate(current_millis, &V3);

    u8g2.firstPage();
    do {
        draw_data(&V1, "1:", 15);
        draw_data(&V2, "2:", 35);
        draw_data(&V3, "3:", 55);
    } while (u8g2.nextPage());
}

void setup() {
    Serial.begin(9600);
    MsTimer2::set(500, update_display);
    MsTimer2::start();
    u8g2.begin();
    pinMode(14, OUTPUT);
}

void loop() {
    unsigned long current_millis = millis();
    sensor_read(current_millis, &V1, A14);
    sensor_read(current_millis, &V2, A13);
    sensor_read(current_millis, &V3, A12);
}