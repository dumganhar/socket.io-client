#include "Backoff.h"

Backoff::Backoff(long min, long max, float jitter, int factor)
{
    _ms = min;
    _max = max;
    _factor = factor;
    _jitter = jitter;
    _factor = factor;
    _attempts = 0;
}

Backoff::~Backoff()
{

}

long Backoff::getDuration()
{
//  var ms = _ms * Math.pow(_factor, _attempts++);
//  if (_jitter) {
//    var rand =  Math.random();
//    var deviation = Math.floor(rand * _jitter * ms);
//    ms = (Math.floor(rand * 10) & 1) == 0  ? ms - deviation : ms + deviation;
//  }
//  return Math.min(ms, _max) | 0;

    return 0;
}

void Backoff::reset()
{
  _attempts = 0;
}

void Backoff::setMin(long min)
{
  _ms = min;
}

void Backoff::setMax(long max)
{
  _max = max;
}

void Backoff::setJitter(float jitter)
{
  _jitter = jitter;
}


