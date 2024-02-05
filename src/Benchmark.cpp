#include "Benchmark.hpp"
#include "Globals.hpp"

int BenchmarkMetricDefintion::INDEX_COUNTER = 0;

void Benchmark::addMetric(std::string name, callbackFreq freq, metric_callback callback)
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
    metric.callback = callback;
    metrics.push_back(metric);
    data.push_back(std::deque<BenchmarkMetric>());
}

void Benchmark::printMetricLast(BenchmarkMetricDefintion metric)
{
    BenchmarkMetric d = data[metric.index].back();
    std::cout << metric.name << "[" << ((int)(metric.freq) > 2 ? d.time : d.tick) << "]"
              << " : " << d.value << std::endl;
}

void Benchmark::printMetricAll(BenchmarkMetricDefintion metric)
{
    std::cout << metric.name << " : " << std::endl;
    for (auto d : data[metric.index])
    {
        std::cout << "    " << ((int)(metric.freq) > 2 ? d.time : d.tick) << " : " << d.value << std::endl;
    }
}

void Benchmark::printLast()
{
    for (auto metric : metrics)
    {
        printMetricLast(metric);
    }
}

void Benchmark::printAll()
{
    for (auto metric : metrics)
    {
        printMetricAll(metric);
    }
}

void Benchmark::printMetric(std::string name)
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
            m.value = metric.callback();
            m.tick = tick;
            data[metric.index].push_back(m);
            break;
        }

        case EVERY_100_TICKS:
        {
            if (tick % 100 == 0)
            {
                BenchmarkMetric m;
                m.value = metric.callback();
                m.tick = tick;
                data[metric.index].push_back(m);
            }
            break;
        }

        case EVERY_1000_TICKS:
        {
            if (tick % 1000 == 0)
            {
                BenchmarkMetric m;
                m.value = metric.callback();
                m.tick = tick;
                data[metric.index].push_back(m);
            }
            break;
        }

        case EVERY_SECOND:
        {
            if (time - metric.lastTime > 1.0)
            {
                BenchmarkMetric m;
                m.value = metric.callback();
                m.time = time;
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
                m.value = metric.callback();
                m.time = time;
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
                m.value = metric.callback();
                m.time = time;
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
                m.value = metric.callback();
                m.time = time;
                data[metric.index].push_back(m);
                metric.lastTime = time;
            }
            break;
        }
        }
    }
}