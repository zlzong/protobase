#pragma once

#include <cstdint>
#include <memory>
#include <unordered_set>
#include <deque>
#include <utility>
#include <vector>
#include <functional>

enum TimerType {
    kTimingTypeSecond = 0,
    kTimingTypeMinute,
    kTimingTypeHour,
    kTimingTypeDay
};

const int secondsPerMinute = 60;
const int secondsPerHour = 3600;
const int secondsPerDay = 86400;

class NodeTask;

using NodePtr = std::shared_ptr<void>;
using TimerTask = std::function<void()>;
using WheelNode = std::unordered_set<NodePtr>;
using Wheel = std::deque<WheelNode>;
using Wheels = std::vector<Wheel>;
using NodeTaskPtr = std::shared_ptr<NodeTask>;

class NodeTask {
public:
    explicit NodeTask(TimerTask task) : m_task(std::move(task)) {}

    ~NodeTask() {
        if (m_task) {
            m_task();
        }
    }
private:
    TimerTask m_task;
};

class TimerWheel {
public:
    TimerWheel();

    ~TimerWheel() = default;

    void insertNode(uint32_t delay, NodePtr nodePtr);

    void onTime(int64_t now);

    void runAfter(int delaySeconds, const TimerTask &task);

    void runAfter(int delaySeconds, TimerTask &&task);

    void runEvery(int intervalSeconds, const TimerTask &task);

    void runEvery(int intervalSeconds, TimerTask &&task);

private:
    void popUp(Wheel &wheel);

    void insertSecondNode(uint32_t delay, NodePtr nodePtr);

    void insertMinuteNode(uint32_t delay, NodePtr nodePtr);

    void insertHourNode(uint32_t delay, NodePtr nodePtr);

    void insertDayNode(uint32_t delay, NodePtr nodePtr);

private:
    Wheels m_wheels;
    int64_t m_lastTime{0};
    uint64_t m_tick{0};
};
