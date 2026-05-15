#include "polling-controller.hpp"

PollingController::PollingController(int normalIntervalMs, int slowIntervalMs)
    : state(State::NORMAL),
      normalInterval(normalIntervalMs),
      slowInterval(slowIntervalMs),
      consecutiveFailures(0),
      lastPollTime(std::chrono::steady_clock::now())
{
}

void PollingController::run(std::function<bool()> func)
{
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastPollTime).count();

    if (elapsed >= getCurrentInterval())
    {
        performPolling(func);
        lastPollTime = std::chrono::steady_clock::now();
    }
}

void PollingController::performPolling(std::function<bool()> func)
{
    transition(func());
}

void PollingController::transition(bool success)
{
    switch (state)
    {
    case State::NORMAL:
        if (success)
        {
            consecutiveFailures = 0;
        }
        else if (++consecutiveFailures >= FAILURE_THRESHOLD)
        {
            state = State::SLOW;
        }
        break;

    case State::SLOW:
        if (success)
        {
            state = State::NORMAL;
            consecutiveFailures = 0;
        }
        // failure in SLOW: remain in SLOW, counter unchanged
        break;
    }
}

int PollingController::getCurrentInterval() const
{
    return (state == State::SLOW) ? slowInterval : normalInterval;
}

PollingController::State PollingController::getState() const
{
    return state;
}
