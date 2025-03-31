#ifndef DROPLET_COUNTER_H
#define DROPLET_COUNTER_H

#include "Arduino.h"

class DropletCounter {
public:
    /**
     * @param max_records 最大记录数
     * @param sampling_duration 采样时长(ms)
     * @param threshold 触发阈值
     * @param pin 传感器引脚
     */
    DropletCounter(uint8_t max_records,
                   unsigned long sampling_duration,
                   int threshold,
                   uint8_t pin);

    ~DropletCounter();

    DropletCounter(const DropletCounter &) = delete;     // 禁用拷贝

    DropletCounter &operator=(const DropletCounter &) = delete;

    /**
    * 读取传感器并更新状态
    * @param current_millis 当前时间戳
    */
    void update(unsigned long current_millis);

    void calculate(unsigned long current_millis);

    double getDropletsPerSecond() const { return average; }

    int getValidCount() const { return count; }

private:
    /**
    * 记录液滴时间戳并清理过期数据
    * @param current_millis 当前时间戳
    */
    void record(unsigned long current_millis);

    const unsigned long sampling_duration;
    const int trigger_threshold;
    const uint8_t sensor_pin;
    const uint8_t max_records;

    bool flag;
    unsigned long *timestamps; // 动态数组
    int count;
    double average;
};

#endif


