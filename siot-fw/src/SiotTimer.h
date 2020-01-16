#ifndef SIOT_TIMER_H
#define SIOT_TIMER_H

#include "OneWireManager.h"

typedef enum {
    TIMER_STATE_WAITING,
    TIMER_STATE_FIRED
} TimerState;

class SiotTimer {
    OneWireManager* _oneWireManager;
    int _fireDuration;
    int _fireTime;
    int _lastMinutesToday;
    bool _firedToday;
    bool _running;
    TimerState _state;

public:
    SiotTimer(OneWireManager*);

    void setFireDuration(int);
    // time is in minutes into the day
    void setFireTime(int);
    void fire();
    void run();
};

#endif