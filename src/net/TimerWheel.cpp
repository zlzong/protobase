#include "TimerWheel.h"
#include "base/Logger.h"

TimerWheel::TimerWheel() : m_wheels(4) {
    m_wheels[kTimingTypeSecond].resize(60);
    m_wheels[kTimingTypeMinute].resize(60);
    m_wheels[kTimingTypeHour].resize(24);
    m_wheels[kTimingTypeDay].resize(30);
}

void TimerWheel::insertNode(uint32_t delay, NodePtr nodePtr) {
    if (delay <= 0) {
        nodePtr.reset();
        return;
    }

    if (delay < secondsPerMinute) {
        insertSecondNode(delay, nodePtr);
    } else if (delay < secondsPerHour) {
        insertMinuteNode(delay, nodePtr);
    } else if (delay < secondsPerDay) {
        insertHourNode(delay, nodePtr);
    } else {
        uint32_t days = delay / secondsPerDay;
        if (days > 30) {
            LOG_ERROR("max timer interval is 30 days");
            return;
        }
        insertDayNode(delay, nodePtr);
    }
}

void TimerWheel::onTime(int64_t now) {
    m_lastTime = now;

    ++m_tick;
    popUp(m_wheels[kTimingTypeSecond]);

    if (m_tick % secondsPerMinute == 0) {
        popUp(m_wheels[kTimingTypeMinute]);
    } else if (m_tick % secondsPerHour == 0) {
        popUp(m_wheels[kTimingTypeHour]);
    } else if (m_tick % secondsPerDay == 0) {
        popUp(m_wheels[kTimingTypeDay]);
    }
}

void TimerWheel::runAfter(int delaySeconds, const TimerTask &task) {
    NodeTaskPtr nodeTaskPtr = std::make_shared<NodeTask>([task]() {
        task();
    });

    insertNode(delaySeconds, nodeTaskPtr);
}

void TimerWheel::runAfter(int delaySeconds, TimerTask &&task) {
    NodeTaskPtr nodeTaskPtr = std::make_shared<NodeTask>([task]() {
        task();
    });

    insertNode(delaySeconds, nodeTaskPtr);
}

void TimerWheel::runEvery(int intervalSeconds, const TimerTask &task) {
    NodeTaskPtr nodeTaskPtr = std::make_shared<NodeTask>([this, intervalSeconds, task]() {
        task();
        runEvery(intervalSeconds, task);
    });

    insertNode(intervalSeconds, nodeTaskPtr);
}

void TimerWheel::runEvery(int intervalSeconds, TimerTask &&task) {
    NodeTaskPtr nodeTaskPtr = std::make_shared<NodeTask>([this, intervalSeconds, task]() {
        task();
        runEvery(intervalSeconds, task);
    });

    insertNode(intervalSeconds, nodeTaskPtr);
}

void TimerWheel::popUp(Wheel &wheel) {
    WheelNode tmp;
    wheel.front().swap(tmp);
    wheel.pop_front();
    wheel.emplace_back();
}

void TimerWheel::insertSecondNode(uint32_t delay, NodePtr nodePtr) {
    m_wheels[kTimingTypeSecond][delay - 1].emplace(nodePtr);
}

void TimerWheel::insertMinuteNode(uint32_t delay, NodePtr nodePtr) {
    uint32_t minute = delay / secondsPerMinute;
    uint32_t second = delay % secondsPerMinute;

    NodeTaskPtr nodeTaskPtr = std::make_shared<NodeTask>([this, second, nodePtr]() {
        insertNode(second, nodePtr);
    });

    m_wheels[kTimingTypeMinute][minute - 1].emplace(nodeTaskPtr);
}

void TimerWheel::insertHourNode(uint32_t delay, NodePtr nodePtr) {
    uint32_t hour = delay / secondsPerHour;
    uint32_t second = delay % secondsPerHour;

    NodeTaskPtr nodeTaskPtr = std::make_shared<NodeTask>([this, second, nodePtr]() {
        insertNode(second, nodePtr);
    });

    m_wheels[kTimingTypeHour][hour - 1].emplace(nodeTaskPtr);
}

void TimerWheel::insertDayNode(uint32_t delay, NodePtr nodePtr) {
    uint32_t day = delay / secondsPerDay;
    uint32_t second = delay % secondsPerDay;

    NodeTaskPtr nodeTaskPtr = std::make_shared<NodeTask>([this, second, nodePtr]() {
        insertNode(second, nodePtr);
    });

    m_wheels[kTimingTypeDay][day - 1].emplace(nodeTaskPtr);
}