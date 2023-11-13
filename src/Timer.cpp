#include <Timer.hpp>
#include <Utils.hpp>

BenchTimer::BenchTimer(){};
BenchTimer::BenchTimer(std::string name) : name(name){};

float BenchTimer::getDelta() const {return deltaS;};

float BenchTimer::getDeltaMS() const {return delta.count();};

float BenchTimer::getElapsedTime() const {return elapsedTime;};

const float* BenchTimer::getElapsedTimeAddr() const {return &elapsedTime;};

void BenchTimer::toggle()
{
    if(paused)
        resume();
    else
        pause();
}

void BenchTimer::resume()
{
    resumePending = true;
}

void BenchTimer::pause()
{
    resumePending = false;
    paused = true;
}

bool BenchTimer::isPaused(){return paused;};

void BenchTimer::start()
{
    if(paused)
    {
        if(resumePending)
        {
            paused = false;
            resumePending = false;
        }
        else return;
    }
    lastStartTime = clockmicro::now();
}

void BenchTimer::end()
{
    if(paused) return;

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

    os << TERMINAL_INFO << "Timer : " << "\n";
    os << TERMINAL_INFO <<"\tname     : " << TERMINAL_NOTIF << e.name << "\n";
    os << TERMINAL_INFO <<"\tdelta    : " << TERMINAL_NOTIF <<e.delta.count() << "\n";
    os << TERMINAL_INFO <<"\tmin      : " << TERMINAL_NOTIF <<e.min.count() << "\n";
    os << TERMINAL_INFO <<"\tmax      : " << TERMINAL_NOTIF <<e.max.count() << "\n";
    os << TERMINAL_INFO <<"\tavgTotal : " << TERMINAL_NOTIF <<e.avgTotal.count() << "\n";
    os << TERMINAL_INFO <<"\tavg      : " << TERMINAL_NOTIF <<e.avgLast.count() << "\n";

    os << TERMINAL_INFO <<"\tupdateCounter  : " << TERMINAL_NOTIF << e.updateCounter << "\n";
    os << TERMINAL_INFO <<"\tlastTimeUpdate : " << TERMINAL_NOTIF << e.lastTimeUpdate.time_since_epoch().count() << "\n";

    return os;
}