#include "Event.h"

Event::Event(EventLoop *eventLoop) {

}

Event::Event(EventLoop *eventLoop, int fd) {

}

bool Event::enableReading() {
    return false;
}

bool Event::disableReading() {
    return false;
}

bool Event::enableWriting() {
    return false;
}

bool Event::disableWriting() {
    return false;
}

int Event::getFd() const {
    return 0;
}

void Event::close() {

}
