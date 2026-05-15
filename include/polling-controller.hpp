#ifndef __POLLING_CONTROLLER_HPP__
#define __POLLING_CONTROLLER_HPP__

#include <functional>
#include <chrono>

class PollingController
{
public:
    enum class State : uint8_t { NORMAL, SLOW };

    PollingController(int normalIntervalMs = 3000, int slowIntervalMs = 10000);
    void run(std::function<bool()> func);
    State getState() const;

private:
    static const int FAILURE_THRESHOLD = 3;

    void performPolling(std::function<bool()> func);
    void transition(bool success);
    int getCurrentInterval() const;

    State state;
    int normalInterval;
    int slowInterval;
    int consecutiveFailures;
    std::chrono::steady_clock::time_point lastPollTime;
};

#endif
