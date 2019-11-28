#include "SiotTimer.h"

SiotTimer::SiotTimer(OneWireManager* oneWireManager)
    : _oneWireManager(oneWireManager)
    , _fireDuration(0)
    , _fireTime(0)
    , _lastMinutesToday(0)
    , _firedToday(false)
    , _running(false)
{
}

void SiotTimer::setFireDuration(int fireDuration)
{
    _fireDuration = fireDuration;
}

void SiotTimer::setFireTime(int time)
{
    _fireTime = time;
    _firedToday = false;
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

void SiotTimer::run()
{
    int now = Time.now();
    int minutesToday = Time.hour(now) * 60 + Time.minute(now);
    if (minutesToday < _lastMinutesToday) {
        _firedToday = false;
    }

    //Serial.printf("firetime: %i, minutes today: %i, fired: %i\n", _fireTime, minutesToday, _firedToday);

    if (!_firedToday && minutesToday >= _fireTime) {
        fire();
        _firedToday = true;
    } else if (!_running) {
        _oneWireManager->setGpio(true, true);
    }
}