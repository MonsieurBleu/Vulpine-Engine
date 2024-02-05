#pragma once
#include <string>
#include <vector>
#include <deque>
#include <functional>
#include <iostream>
#include <fstream>

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

typedef int (*metric_callback)();

struct BenchmarkMetricDefintion
{
    static int INDEX_COUNTER;
    int index = INDEX_COUNTER++;
    std::string name;
    callbackFreq freq;
    metric_callback callback;

    union
    {
        int lastTick = 0;
        float lastTime;
    };
};

struct BenchmarkMetric
{
    int value;
    union
    {
        int tick = 0;
        float time;
    };
};

class Benchmark
{
private:
    std::vector<BenchmarkMetricDefintion> metrics;
    std::vector<std::deque<BenchmarkMetric>> data;

    void printMetricLast(BenchmarkMetricDefintion metric);
    void printMetricAll(BenchmarkMetricDefintion metric);

public:
    Benchmark(){};
    void addMetric(std::string name, callbackFreq freq, metric_callback callback);
    void printLast();
    void printAll();
    void printMetric(std::string name);
    void saveCSV(std::string filename);
    void clearData();
    void clearAll();
    void clearMetric(std::string name);
    void removeMetric(std::string name);

    void tick();
};