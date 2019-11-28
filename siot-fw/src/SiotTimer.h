#ifndef SIOT_TIMER_H
#define SIOT_TIMER_H

#include "OneWireManager.h"

class SiotTimer {
    OneWireManager* _oneWireManager;
    int _fireDuration;
    int _fireTime;
    int _lastMinutesToday;
    bool _firedToday;
    bool _running;

public:
    SiotTimer(OneWireManager*);

    void setFireDuration(int);
    // time is in minutes into the day
    void setFireTime(int);
    void fire();
    void run();
};

#endif