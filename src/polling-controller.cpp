#include "polling-controller.hpp"

PollingController::PollingController(int normalIntervalMs, int slowIntervalMs) : lastPollTime(std::chrono::steady_clock::now())
{
    this->normalInterval = normalIntervalMs;
    this->slowInterval = slowIntervalMs;
    this->consecutiveFailures = 0;
}

void PollingController::run(std::function<bool()> func)
{
    auto now = std::chrono::steady_clock::now();
    int interval = getCurrentInterval();

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastPollTime).count();

    if (elapsed >= interval)
    {
        performPolling(func);
        lastPollTime = std::chrono::steady_clock::now();
    }
}

void PollingController::performPolling(std::function<bool()> func)
{
    bool result = func();

    if (!result && consecutiveFailures < 3)
    {
        consecutiveFailures++;
    }
    else
    {
        consecutiveFailures = 0;
    }
}

int PollingController::getCurrentInterval() const
{
    return (consecutiveFailures == 3) ? slowInterval : normalInterval;
}
