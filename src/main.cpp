
#include <HardwareSerial.h>
#include "Arduino.h"
#include "MsTimer2.h"
#include "U8g2lib.h"

#define SAMPLING_INTERVAL 50 // 取样间隔，单位：ms
#define SAMPLING_TIMES 5 // 取样次数，0~255；
#define TRIGGER_THRESHOLD 500 // 液滴触发阈值

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, 13, 12, 11, 15);  //屏幕初始化

typedef struct {
    bool flag;  // 液滴触发标志位，为false时表示等待液滴触发

    unsigned int current;  // 当前液滴间隔时间，单位同 SAMPLING_INTERVAL 。
    unsigned int intervals[SAMPLING_TIMES];  // 采样间隔时间，保留指定次数

    double average;  // 平均液滴间隔时间
    double per_second;  // 每秒液滴数
} DropletInterval;

/**
 * 计算平均液滴间隔时间
 * @param droplet 液滴间隔时间结构体
 */
void calculate(DropletInterval *droplet) {
    double sum = 0; // 液滴间隔时间总和 (单位：SAMPLING_INTERVAL)
    uint8_t times = 0; // 有效的液滴间隔时间个数
    for (int i = 0; i < SAMPLING_TIMES; i++) {
        if (droplet->intervals[i] == 0) continue; // 如果液滴间隔时间为0，则跳过
        sum += droplet->intervals[i]; // 累加液滴间隔时间
        times++;
    }
    if (times <= 0 || sum <= 0) {
        droplet->average = 0; // 如果没有有效的液滴间隔时间，则平均值为0
        droplet->per_second = 0; // 每秒液滴数为0
    } else {
        droplet->average = sum * SAMPLING_INTERVAL / times / 1000.0; // 平均液滴间隔时间 (单位：秒)
        droplet->per_second = 1.0 / droplet->average; // 每秒液滴数
    }
}

/**
 * 计时器，每次调用时，最近的液滴间隔时间加1，并重新计算平均值
 * @param droplet 液滴间隔时间结构体
 */
void tick(DropletInterval *droplet) {
    if (!droplet->flag) {
        droplet->current++;
    }
}

/**
 * 记录本次液滴间隔时间到历史数据中，并准备记录新的液滴间隔时间
 * @param droplet 液滴间隔时间结构体
 */
void record(DropletInterval *droplet) {
    if (droplet->flag) return; // 仍然处于液滴触发状态，不进行记录操作

    // 记录液滴间隔时间
    for (int i = 0; i < SAMPLING_TIMES - 1; i++) {
        droplet->intervals[i] = droplet->intervals[i + 1];
    }
    droplet->intervals[SAMPLING_TIMES - 1] = droplet->current;
    droplet->current = 0;  // 重置当前液滴间隔时间
    calculate(droplet); // 计算平均液滴间隔时间
}

/**
 * 读取液滴传感器，并进行触发判断
 * @param droplet  液滴间隔时间结构体
 * @param pin     液滴传感器引脚
 */
void read(DropletInterval *droplet, const uint8_t pin) {
    if (analogRead(pin) > TRIGGER_THRESHOLD) {
        droplet->flag = false;
    } else if (analogRead(pin) < TRIGGER_THRESHOLD) {
        record(droplet);
        droplet->flag = true;
    }
}

void display(DropletInterval *droplet, const char str[], const int offset) {
    u8g2.setFont(u8g2_font_ncenB10_tr);
    u8g2.setCursor(0, offset);
    u8g2.print(str);
    u8g2.setCursor(15, offset);
    u8g2.print(droplet->per_second, 2);
    u8g2.setCursor(55, offset);
    u8g2.print("D/s |");
    u8g2.setCursor(95, offset);
    u8g2.print(droplet->current * SAMPLING_INTERVAL / 1000.0, 1);
}

DropletInterval V1 = {true, 0, {0}, 0, 0};  // 液滴1
DropletInterval V2 = {true, 0, {0}, 0, 0};  // 液滴2
DropletInterval V3 = {true, 0, {0}, 0, 0};  // 液滴3

void onTimer() {
    tick(&V1);
    tick(&V2);
    tick(&V3);
}

void setup() {
    Serial.begin(9600);
    MsTimer2::set(SAMPLING_INTERVAL, onTimer);
    MsTimer2::start();
    pinMode(14, OUTPUT);
    u8g2.begin();
}

void loop()  //主函数
{
    u8g2.firstPage();  //帧函数
    //Serial.println(analogRead(A14));
    //digitalWrite(14, HIGH);

    /*液滴检测*/
    read(&V1, A14);
    read(&V2, A13);
    read(&V3, A12);

    /*屏幕显示*/
    do {
        display(&V1, "1:", 15);
        display(&V2, "2:", 35);
        display(&V3, "3:", 55);
    } while (u8g2.nextPage());
}

