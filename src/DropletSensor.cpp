#include "DropletSensor.h"
#include <Arduino.h> // 包含 analogRead 的声明

DropletSensor::DropletSensor(
        unsigned int interval,
        uint8_t samples,
        int threshold,
        uint8_t pin
) : sampling_interval(interval),
    trigger_threshold(threshold),
    sensor_pin(pin),
    max_samples(samples > 0 ? samples : 1),
    current(0),
    flag(true),
    average(0),
    per_second(0) {
    intervals = new unsigned int[max_samples](); // 动态分配并初始化为0
}

DropletSensor::~DropletSensor() {
    delete[] intervals;
}

void DropletSensor::tick() {
    if (!flag) ++current;
}

void DropletSensor::read() {
    if (analogRead(sensor_pin) > trigger_threshold) /*消抖*/ {
        flag = false;
    }
    if (analogRead(sensor_pin) < trigger_threshold) /*消抖*/ {
        record();
        flag = true;
    }
}

void DropletSensor::calculate() {
    double sum = 0;
    uint8_t valid_count = 0;

    for (uint8_t i = 0; i < max_samples; ++i) {
        if (intervals[i] > 0) {
            sum += intervals[i];
            ++valid_count;
        }
    }

    if (valid_count > 0 && sum > 0) {
        average = (sum * sampling_interval) / (valid_count * 1000.0);
        per_second = 1.0 / average;
    } else {
        average = 0;
        per_second = 0;
    }

}

void DropletSensor::record() {
    if (flag) return;
    for (uint8_t i = 0; i < max_samples - 1; ++i) {
        intervals[i] = intervals[i + 1];
    }
    intervals[max_samples - 1] = current;
    current = 0;
    calculate();
}