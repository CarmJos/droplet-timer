#include "DropletCounter.h"
#include <Arduino.h>

DropletCounter::DropletCounter(
        uint8_t max_records,
        unsigned long sampling_duration,
        int threshold,
        uint8_t pin
) : sampling_duration(sampling_duration),
    trigger_threshold(threshold),
    sensor_pin(pin),
    max_records(max_records),
    flag(false),
    count(0),
    average(0) {
    timestamps = new unsigned long[max_records]();
}

DropletCounter::~DropletCounter() {
    delete[] timestamps;
}

void DropletCounter::update(unsigned long current_millis) {
    if (analogRead(sensor_pin) > trigger_threshold) /*消抖*/ {
        flag = false;
    }
    if (analogRead(sensor_pin) < trigger_threshold) /*消抖*/ {
        record(current_millis);
        flag = true;
    }
}

void DropletCounter::record(unsigned long current_millis) {
    if (flag) return; // 防止重复触发

    // 清理超过1秒的旧数据
    int new_count = 0;
    for (int i = 0; i < count; i++) {
        if (current_millis - timestamps[i] <= sampling_duration) {
            timestamps[new_count++] = timestamps[i];
        }
    }
    count = new_count;

    // 添加新记录
    if (count < max_records) {
        timestamps[count++] = current_millis;
    } else {
        // 数组满时覆盖最旧数据（已按时间排序，直接覆盖首元素）
        memmove(timestamps, timestamps + 1, sizeof(unsigned long) * (max_records - 1));
        timestamps[max_records - 1] = current_millis;
    }
}

void DropletCounter::calculate(unsigned long current_millis) {
    int valid_count = 0;
    for (int i = 0; i < count; i++) {    // 计算过去1秒内的液滴数
        if (current_millis - timestamps[i] <= sampling_duration) {
            valid_count++;
        }
    }
    average = (double) valid_count / (sampling_duration / 1000.0);
}