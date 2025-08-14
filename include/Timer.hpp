#ifndef TIMER_HPP
#define TIMER_HPP

#include <iostream>
#include <chrono>
typedef std::chrono::high_resolution_clock clockmicro;
typedef std::chrono::duration<float, std::milli> duration;

#define NS_TO_MS 1000000;
#define NS_TO_S 1000000000;
#define MS_TO_S 0.001;

#define NAMED_TIMER(x) BenchTimer x(#x);

class FastUI_valueMenu;

class BenchTimer
{
    friend std::ostream &operator<<(std::ostream &os, BenchTimer e);

private:
    bool paused = false;
    bool resumePending = false;

    duration delta    = duration(0);
    duration min      = duration(1e9);
    duration max      = duration(0);
    duration avgTotal = duration(0);

    duration avgLast;
    duration avg;
    clockmicro::time_point avgLastUpdate;
    uint64_t avgUpdateCounter = 0;
    int64_t avgLength = 500 * NS_TO_MS;

    clockmicro::time_point lastTimeUpdate;
    clockmicro::time_point lastStartTime;

    uint64_t updateCounter = 0;
    uint32_t holdCounter = 0;

    float elapsedTime = 0.0;
    float deltaS = 0.0;

    duration holdDeltaBuff = duration(0);

public:

    std::string name;

    BenchTimer();
    BenchTimer(std::string name);

    float speed = 1.0;

    void stop();
    void hold();
    void start();

    void toggle();
    void resume();
    void pause();
    bool isPaused();


    void setAvgLengthMS(int64_t newLength);

    /**
     * @brief Get the Delta time in secondes
     */
    float getDelta() const;
    /**
     * @brief Get the Delta time in milisecondes
     */
    float getDeltaMS() const;
    /**
     * @brief Get the Elapsed time in secondes
     */
    float getElapsedTime() const;

    const float *getElapsedTimeAddr() const;

    uint64_t getUpdateCounter() { return updateCounter; };

    // std::ostream& operator<<(std::ostream& os);

    void setMenu(FastUI_valueMenu &menu);
    void setMenuConst(FastUI_valueMenu &menu) const;

    duration getLastAvg() const
    {
        return avgLast;
    };

    duration getMax() const
    {
        return max;
    };

    void reset()
    {
        delta    = duration(0);
        min      = duration(1e9);
        max      = duration(0);
        avgTotal = duration(0);

        updateCounter = 0;
        elapsedTime = 0;
    };
};

std::ostream &operator<<(std::ostream &os, BenchTimer e);

class LimitTimer
{
private:
    bool activated;
    clockmicro::time_point lastStartTime;

public:
    float freq;

    LimitTimer(float freq = 60.f);
    void start();
    void waitForEnd();

    void activate();
    void deactivate();
    void toggle();
    bool isActivated();

    void setMenu(FastUI_valueMenu &menu);
};

class TickTimer
{

private :
    // bool activated;
    clockmicro::time_point lasTickTime;

public : 

    void tick();
    float timeSinceLastTickMS();
};


#endif