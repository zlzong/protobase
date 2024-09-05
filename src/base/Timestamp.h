#pragma once

#include <iostream>
#include <string>

class Timestamp {
public:
    Timestamp();

    explicit Timestamp(int64_t microSecondsSinceEpoch);

    static Timestamp now();

    std::string localeString() const;

    static Timestamp addTime(Timestamp timestamp, double seconds);

    static Timestamp invalid() { return {}; }

    bool valid() const { return m_microSecondsSinceEpoch > 0; }

    int64_t microSecondsSinceEpoch() const { return m_microSecondsSinceEpoch; }

    bool operator<(const Timestamp &rhs) const { return m_microSecondsSinceEpoch < rhs.m_microSecondsSinceEpoch; }

    bool operator>(const Timestamp &rhs) const { return m_microSecondsSinceEpoch > rhs.m_microSecondsSinceEpoch; }

    bool operator==(const Timestamp &rhs) const { return m_microSecondsSinceEpoch == rhs.m_microSecondsSinceEpoch; }

private:
    int64_t m_microSecondsSinceEpoch;
};