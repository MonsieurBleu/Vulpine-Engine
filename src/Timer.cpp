#include <Timer.hpp>


float BenchTimer::getDelta() const {return deltaS;};

float BenchTimer::getDeltaMS() const {return delta.count();};

float BenchTimer::getElapsedTime() const {return elapsedTime;};

const float* BenchTimer::getElapsedTimeAddr() const {return &elapsedTime;};


void BenchTimer::start()
{
    lastStartTime = clockmicro::now();
}

void BenchTimer::end()
{
    clockmicro::time_point now = clockmicro::now();
    delta = now-lastStartTime;
    lastTimeUpdate = now;
    deltaS = delta.count()*MS_TO_S;
    elapsedTime += deltaS;

    avg = ((avg*avgUpdateCounter)+delta)/(avgUpdateCounter+1);
    avgTotal = ((avgTotal*updateCounter)+delta)/(updateCounter+1);

    if(delta < min || !min.count()) min = delta;
    if(delta > max || !min.count()) max = delta;

    updateCounter ++;
    avgUpdateCounter ++;

    if((now-avgLastUpdate).count() > avgLength)
    {
        avgLastUpdate = now;
        avgUpdateCounter = 0;
        avgLast = avg;
    }
}

void BenchTimer::setAvgLengthMS(int64_t newLength)
{
    avgLength = newLength*NS_TO_MS;
}


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