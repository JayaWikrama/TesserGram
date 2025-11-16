#ifndef __POLLING_CONTROLLER_HPP__
#define __POLLING_CONTROLLER_HPP__

#include <functional>
#include <chrono>

class PollingController
{
private:
    void performPolling(std::function<bool()> func);
    int getCurrentInterval() const;

    int normalInterval;
    int slowInterval;
    int consecutiveFailures;
    std::chrono::steady_clock::time_point lastPollTime;

public:
    PollingController(int normalIntervalMs = 3000, int slowIntervalMs = 10000);
    void run(std::function<bool()> func);
};

#endif
