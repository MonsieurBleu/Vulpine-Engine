#pragma once
#include <string>
#include <vector>
#include <deque>
#include <functional>
#include <iostream>
#include <fstream>
#include <limits.h>

enum callbackFreq
{
    EVERY_TICK,
    EVERY_100_TICKS,
    EVERY_1000_TICKS,

    EVERY_SECOND,
    EVERY_10_SECONDS,
    EVERY_MINUTE,
    EVERY_100_MILLISECONDS,
};

typedef int64_t (*metric_callback_int)();
typedef double (*metric_callback_float)();

struct BenchmarkMetricDefintion
{
    static int INDEX_COUNTER;
    int index = INDEX_COUNTER++;
    std::string name;
    callbackFreq freq;
    metric_callback_int callback_int = nullptr;
    metric_callback_float callback_float = nullptr;

    bool isFloat = false;

    float lastTime = 0;
};

struct BenchmarkMetric
{
    union
    {
        int64_t value_int;
        double value_float;
    };

    int tick = 0;
    float time = 0;
};

class Benchmark
{
private:
    std::vector<BenchmarkMetricDefintion> metrics;
    std::vector<std::deque<BenchmarkMetric>> data;

    void printMetricLast(BenchmarkMetricDefintion metric) const;
    void printMetricAll(BenchmarkMetricDefintion metric) const;

public:
    Benchmark(){};
    void addMetric(std::string name, callbackFreq freq, metric_callback_int callback);
    void addMetric(std::string name, callbackFreq freq, metric_callback_float callback);
    void printLast() const;
    void printAll() const;
    void printMetric(std::string name) const;
    void saveCSV() const;
    void saveCSV(std::string filename) const;
    void clearData();
    void clearAll();
    void clearMetric(std::string name);
    void removeMetric(std::string name);

    void tick();
};