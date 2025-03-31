
#ifndef UNTITLED_DROPLETSENSOR_H
#define UNTITLED_DROPLETSENSOR_H

#include "Arduino.h"

class DropletSensor {
public:
    DropletSensor(unsigned int interval, uint8_t samples, int threshold, uint8_t pin);

    ~DropletSensor();

    DropletSensor(const DropletSensor &) = delete;

    DropletSensor &operator=(const DropletSensor &) = delete;

    /**
     * 计时器，每次调用时，最近的液滴间隔计数加1
    */
    void tick();

    /**
     * 读取液滴传感器，并进行触发判断
     */
    void read();

    /**
     * 得到当前液滴平均间隔时间
     * @return  平均液滴间隔时间 (单位：秒)
     */
    double getAverageInterval() const { return average; }

    /**
     * @return  每秒液滴数
     */
    double getDropletsPerSecond() const { return per_second; }

    int getCurrentInterval() const { return current; }

    double getCurrentSeconds() const { return current * sampling_interval / 1000.0; }

private:

    /**
     * 计算平均液滴间隔时间
     */
    void calculate();

    /**
    * 记录本次液滴间隔时间到历史数据中，并准备记录新的液滴间隔时间
    */
    void record();

    const unsigned int sampling_interval; // 取样间隔，单位：ms
    const int trigger_threshold; // 液滴触发阈值
    const uint8_t sensor_pin; // 液滴传感器引脚
    const uint8_t max_samples; // 采样次数，0~255；如果小于1，则设置为1

    unsigned int current; // 当前液滴间隔时间，单位同 sampling_interval
    unsigned int *intervals; // 采样间隔时间，保留指定次数

    bool flag;  // 液滴触发标志位，为false时表示等待液滴触发
    double average; // 平均液滴间隔时间
    double per_second;  // 每秒液滴数
};

#endif //UNTITLED_DROPLETSENSOR_H
