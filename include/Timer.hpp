#include <chrono>
typedef std::chrono::high_resolution_clock clockmicro;
typedef std::chrono::duration<float, std::milli> duration;

#define NS_TO_MS 1000000;
#define NS_TO_S  1000000000; 

class BenchTimer
{
    public :

        duration delta;
        duration min;
        duration max;
        duration avgTotal;

        duration avgLast;
        duration avg;
        clockmicro::time_point avgLastUpdate;
        uint64_t avgUpdateCounter = 0;
        int64_t avgLength = 500*NS_TO_MS;

        clockmicro::time_point lastTimeUpdate;
        clockmicro::time_point lastStartTime;

        uint64_t updateCounter = 0;

        void end();
        void start();

        void setAvgLengthMS(int64_t newLength);
};

void BenchTimer::start()
{
    lastStartTime = clockmicro::now();
}

void BenchTimer::end()
{
    clockmicro::time_point now = clockmicro::now();
    delta = now-lastStartTime;
    lastTimeUpdate = now;

    avg = ((avg*avgUpdateCounter)+delta)/(avgUpdateCounter+1);
    avgTotal = ((avgTotal*updateCounter)+delta)/(updateCounter+1);

    if(delta < min || !min.count()) min = delta;
    if(delta > max || !min.count()) max = delta;

    updateCounter ++;
    avgUpdateCounter ++;

    if((now-avgLastUpdate).count() > avgLength)
    {
        std::cout << (now-avgLastUpdate).count()  << " avg update\n";
        avgLastUpdate = now;
        avgUpdateCounter = 0;
        avgLast = avg;
    }
}

void BenchTimer::setAvgLengthMS(int64_t newLength)
{
    avgLength = newLength*NS_TO_MS;
}


#include <iostream>

std::ostream& operator<<(std::ostream& os, BenchTimer e)
{

    os << "delta    : " << e.delta.count() << "\n";
    os << "min      : " << e.min.count() << "\n";
    os << "max      : " << e.max.count() << "\n";
    os << "avgTotal : " << e.avgTotal.count() << "\n";
    os << "avg      : " << e.avgLast.count() << "\n";

    os << "updateCounter  : " << e.updateCounter << "\n";
    os << "lastTimeUpdate : " << e.lastTimeUpdate.time_since_epoch().count() << "\n";

    return os;
}