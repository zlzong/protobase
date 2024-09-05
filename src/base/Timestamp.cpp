#include "Timestamp.h"

#include <sys/time.h>

Timestamp::Timestamp() : m_microSecondsSinceEpoch(0) {

}

Timestamp::Timestamp(int64_t microSecondsSinceEpoch) : m_microSecondsSinceEpoch(microSecondsSinceEpoch) {

}

Timestamp Timestamp::now() {
    timeval tv;
    gettimeofday(&tv, nullptr);
    int64_t seconds = tv.tv_sec;
    return Timestamp(seconds * 1000000 + tv.tv_usec);
}

std::string Timestamp::localeString() const {
    char buf[128] = {0};
  int64_t secondsSinceEpoch = m_microSecondsSinceEpoch / 1000000;
  tm *tm_time = localtime(&secondsSinceEpoch);
    snprintf(buf, 128, "%4d/%02d/%02d %02d:%02d:%02d",
             tm_time->tm_year + 1900,
             tm_time->tm_mon + 1,
             tm_time->tm_mday,
             tm_time->tm_hour,
             tm_time->tm_min,
             tm_time->tm_sec);
    return buf;
}

Timestamp Timestamp::addTime(Timestamp timestamp, double seconds) {
    int64_t delta = static_cast<int64_t>(seconds * 1000000);
    return Timestamp(timestamp.m_microSecondsSinceEpoch + delta);
}
