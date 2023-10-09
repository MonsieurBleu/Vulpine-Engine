#ifndef TIMER_HPP
#define TIMER_HPP

#include <iostream>
#include <chrono>
typedef std::chrono::high_resolution_clock clockmicro;
typedef std::chrono::duration<float, std::milli> duration;

#define NS_TO_MS 1000000;
#define NS_TO_S  1000000000; 
#define MS_TO_S  0.001;

class BenchTimer
{
    friend std::ostream& operator<<(std::ostream& os, BenchTimer e);

    private : 

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

        float elapsedTime = 0.0;
        float deltaS = 0.0;

        std::string name;

    public :

        BenchTimer();
        BenchTimer(std::string name);

        void end();
        void start();

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

        const float* getElapsedTimeAddr() const;

        std::ostream& operator<<(std::ostream& os);
};

std::ostream& operator<<(std::ostream& os, BenchTimer e);

#endif