#include "Benchmark.hpp"
#include "Globals.hpp"

int BenchmarkMetricDefintion::INDEX_COUNTER = 0;
char *Benchmark::defaultFilename = nullptr;

void Benchmark::addMetric(std::string name, callbackFreq freq, metric_callback_int callback)
{
    // check if name already in use
    for (auto metric : metrics)
    {
        if (metric.name == name)
        {
            std::cout << "Metric with name " << name << " already exists" << std::endl;
            return;
        }
    }

    BenchmarkMetricDefintion metric;
    metric.name = name;
    metric.freq = freq;
    metric.callback_int = callback;
    metric.isFloat = false;
    metrics.push_back(metric);
    data.push_back(std::deque<BenchmarkMetric>());
}

void Benchmark::addMetric(std::string name, callbackFreq freq, metric_callback_float callback)
{
    // check if name already in use
    for (auto metric : metrics)
    {
        if (metric.name == name)
        {
            std::cout << "Metric with name " << name << " already exists" << std::endl;
            return;
        }
    }

    BenchmarkMetricDefintion metric;
    metric.name = name;
    metric.freq = freq;
    metric.callback_float = callback;
    metric.isFloat = true;
    metrics.push_back(metric);
    data.push_back(std::deque<BenchmarkMetric>());
}

void Benchmark::printMetricLast(BenchmarkMetricDefintion metric) const
{
    BenchmarkMetric d = data[metric.index].back();
    std::cout << metric.name << "[" << ((int)(metric.freq) > 2 ? d.time : d.tick) << "]"
              << " : " << (metric.isFloat ? d.value_float : d.value_int) << std::endl;
}

void Benchmark::printMetricAll(BenchmarkMetricDefintion metric) const
{
    std::cout << metric.name << " : " << std::endl;
    for (auto d : data[metric.index])
    {
        std::cout << "    " << ((int)(metric.freq) > 2 ? d.time : d.tick) << " : " << (metric.isFloat ? d.value_float : d.value_int) << std::endl;
    }
}

void Benchmark::printLast() const
{
    for (auto metric : metrics)
    {
        printMetricLast(metric);
    }
}

void Benchmark::printAll() const
{
    for (auto metric : metrics)
    {
        printMetricAll(metric);
    }
}

void Benchmark::printMetric(std::string name) const
{
    for (auto metric : metrics)
    {
        if (metric.name == name)
        {
            printMetricAll(metric);
            return;
        }
    }
    std::cout << "No metric with name " << name << std::endl;
}

void Benchmark::clearData()
{
    for (auto &d : data)
    {
        d.clear();
    }
}

void Benchmark::clearAll()
{
    metrics.clear();
    data.clear();
}

void Benchmark::clearMetric(std::string name)
{
    for (auto &metric : metrics)
    {
        if (metric.name == name)
        {
            data[metric.index].clear();
            return;
        }
    }
    std::cout << "No metric with name " << name << std::endl;
}

void Benchmark::removeMetric(std::string name)
{
    for (auto metric = metrics.begin(); metric != metrics.end(); metric++)
    {
        if (metric->name == name)
        {
            metrics.erase(metric);
            data.erase(data.begin() + metric->index);

            // reindex
            for (auto &m : metrics)
            {
                if (m.index > metric->index)
                {
                    m.index--;
                }
            }

            BenchmarkMetricDefintion::INDEX_COUNTER--;
            return;
        }
    }
    std::cout << "No metric with name " << name << std::endl;
}

void Benchmark::tick()
{
    for (auto &metric : metrics)
    {
        int tick = globals.appTime.getUpdateCounter();
        float time = globals.appTime.getElapsedTime();
        switch (metric.freq)
        {
        case EVERY_TICK:
        {
            BenchmarkMetric m;
            if (metric.isFloat)
                m.value_float = metric.callback_float();
            else
                m.value_int = metric.callback_int();
            m.tick = tick;
            m.time = time;
            data[metric.index].push_back(m);
            break;
        }

        case EVERY_100_TICKS:
        {
            if (tick % 100 == 0)
            {
                BenchmarkMetric m;
                if (metric.isFloat)
                    m.value_float = metric.callback_float();
                else
                    m.value_int = metric.callback_int();
                m.tick = tick;
                m.time = time;
                data[metric.index].push_back(m);
            }
            break;
        }

        case EVERY_1000_TICKS:
        {
            if (tick % 1000 == 0)
            {
                BenchmarkMetric m;
                if (metric.isFloat)
                    m.value_float = metric.callback_float();
                else
                    m.value_int = metric.callback_int();
                m.tick = tick;
                m.time = time;
                data[metric.index].push_back(m);
            }
            break;
        }

        case EVERY_SECOND:
        {
            if (time - metric.lastTime > 1.0)
            {
                BenchmarkMetric m;
                if (metric.isFloat)
                    m.value_float = metric.callback_float();
                else
                    m.value_int = metric.callback_int();
                m.time = time;
                m.tick = tick;
                data[metric.index].push_back(m);
                metric.lastTime = time;
            }
            break;
        }

        case EVERY_10_SECONDS:
        {
            if (time - metric.lastTime > 10.0)
            {
                BenchmarkMetric m;
                if (metric.isFloat)
                    m.value_float = metric.callback_float();
                else
                    m.value_int = metric.callback_int();
                m.time = time;
                m.tick = tick;
                data[metric.index].push_back(m);
                metric.lastTime = time;
            }
            break;
        }

        case EVERY_MINUTE:
        {
            if (time - metric.lastTime > 60.0)
            {
                BenchmarkMetric m;
                if (metric.isFloat)
                    m.value_float = metric.callback_float();
                else
                    m.value_int = metric.callback_int();
                m.time = time;
                m.tick = tick;
                data[metric.index].push_back(m);
                metric.lastTime = time;
            }
            break;
        }

        case EVERY_100_MILLISECONDS:
        {
            if (time - metric.lastTime > 0.1)
            {
                BenchmarkMetric m;
                if (metric.isFloat)
                    m.value_float = metric.callback_float();
                else
                    m.value_int = metric.callback_int();
                m.time = time;
                m.tick = tick;
                data[metric.index].push_back(m);
                metric.lastTime = time;
            }
            break;
        }
        case EVERY_500_MILLISECONDS:
        {
            if (time - metric.lastTime > 0.5)
            {
                BenchmarkMetric m;
                if (metric.isFloat)
                    m.value_float = metric.callback_float();
                else
                    m.value_int = metric.callback_int();
                m.time = time;
                m.tick = tick;
                data[metric.index].push_back(m);
                metric.lastTime = time;
            }
            break;
        }

        case EVERY_250_MILLISECONDS:
        {
            if (time - metric.lastTime > 0.25)
            {
                BenchmarkMetric m;
                if (metric.isFloat)
                    m.value_float = metric.callback_float();
                else
                    m.value_int = metric.callback_int();
                m.time = time;
                m.tick = tick;
                data[metric.index].push_back(m);
                metric.lastTime = time;
            }
            break;
        }
        }
    }
}

void Benchmark::saveCSV() const
{
    if (defaultFilename == nullptr)
    {

        // get unix timestamp
        std::time_t t = std::time(0);
        std::tm *now = std::localtime(&t);
        std::string filename = "benchmark_" +
                               std::to_string(now->tm_year + 1900) + "-" +
                               std::to_string(now->tm_mon + 1) + "-" +
                               std::to_string(now->tm_mday) + "_" +
                               std::to_string(now->tm_hour) + "-" +
                               std::to_string(now->tm_min) + "-" +
                               std::to_string(now->tm_sec) + ".csv";

        saveCSV(filename);
    }
    else
    {
        saveCSV(std::string(defaultFilename));
    }
}

void Benchmark::saveCSV(std::string filename) const
{
    std::ofstream file;
    file.open(filename, std::ios::out | std::ios::trunc);
    file << "tick,time,";
    for (unsigned int i = 0; i < metrics.size() - 1; i++)
    {
        file << metrics[i].name << ",";
    }
    file << metrics[metrics.size() - 1].name << std::endl;

    int startTick = INT_MAX;
    for (auto &metric : metrics)
    {
        if (data[metric.index].size() > 0)
        {
            if (data[metric.index].front().tick < startTick)
            {
                startTick = data[metric.index].front().tick;
            }
        }
    }

    int maxTick = 0;
    for (auto &metric : metrics)
    {
        if (data[metric.index].size() > 0)
        {
            if (data[metric.index].back().tick > maxTick)
            {
                maxTick = data[metric.index].back().tick;
            }
        }
    }

    std::vector<int> indices(metrics.size(), 0);

    for (int tick = startTick; tick <= maxTick; tick++)
    {
        // check if any metric has a value at this tick
        bool hasValue = false;
        float time;
        // this is going to be slow, but whatever
        for (auto &metric : metrics)
        {
            if (data[metric.index].size() > 0 && indices[metric.index] < (int)data[metric.index].size())
            {
                if (data[metric.index][indices[metric.index]].tick == tick)
                {
                    hasValue = true;
                    time = data[metric.index][indices[metric.index]].time;
                    break;
                }
            }
        }

        if (hasValue)
        {
            file << tick << ",";
            file << time << ",";
            for (unsigned int i = 0; i < metrics.size() - 1; i++)
            {
                if ((int)data[metrics[i].index].size() > indices[metrics[i].index])
                {
                    if (data[metrics[i].index][indices[metrics[i].index]].tick == tick)
                    {
                        file << (metrics[i].isFloat ? data[metrics[i].index][indices[metrics[i].index]].value_float : data[metrics[i].index][indices[metrics[i].index]].value_int) << ",";
                        indices[metrics[i].index]++;
                    }
                    else
                    {
                        file << ",";
                    }
                }
                else
                {
                    file << ",";
                }
            }

            // redo for last metric to avoid trailing comma
            if ((int)data[metrics[metrics.size() - 1].index].size() > indices[metrics[metrics.size() - 1].index])
            {
                if (data[metrics[metrics.size() - 1].index][indices[metrics[metrics.size() - 1].index]].tick == tick)
                {
                    file << (metrics[metrics.size() - 1].isFloat ? data[metrics[metrics.size() - 1].index][indices[metrics[metrics.size() - 1].index]].value_float : data[metrics[metrics.size() - 1].index][indices[metrics[metrics.size() - 1].index]].value_int) << std::endl;
                    indices[metrics[metrics.size() - 1].index]++;
                }
                else
                {
                    file << std::endl;
                }
            }
            else
            {
                file << std::endl;
            }

            // file << std::endl;
        }
    }
}