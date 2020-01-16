#include "SiotTimer.h"
#include <Particle.h>

SiotTimer::SiotTimer(OneWireManager* oneWireManager)
    : _oneWireManager(oneWireManager)
    , _fireDuration(0)
    , _fireTime(0)
    , _state(TIMER_STATE_WAITING)
    , _running(false)
    , _lastMinutesToday(0)
{
}

void SiotTimer::setFireDuration(int fireDuration)
{
    _fireDuration = fireDuration;
}

// time is minutes into the day in UTC
void SiotTimer::setFireTime(int time)
{
    _fireTime = time;
    _state = TIMER_STATE_WAITING;
}

void SiotTimer::fire()
{
    Serial.println("fire timer");
    _running = true;
    _oneWireManager->setGpio(false, false);
    delay(_fireDuration * 1000);
    _oneWireManager->setGpio(true, true);
    _running = false;
    Serial.println("fire timer done");
}

// Time on device is in UTC
void SiotTimer::run()
{
    int now = Time.now();
    int minutesToday = Time.hour(now) * 60 + Time.minute(now);

    //Serial.printf("firetime: %i, minutes today: %i, _lastMinutesToday: %i, state: %i\n", _fireTime, minutesToday, _lastMinutesToday, _state);

    switch (_state) {
    case TIMER_STATE_WAITING:
        if (minutesToday == _fireTime) {
            fire();
            _state = TIMER_STATE_FIRED;
        }
        break;
    case TIMER_STATE_FIRED:
        // check if timer rolled over
        if (minutesToday < _lastMinutesToday) {
            _state = TIMER_STATE_WAITING;
        }
        break;
    default:
        Serial.printf("Error: unknown timer state: %i\n", _state);
    }

    _lastMinutesToday = minutesToday;
}