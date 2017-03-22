

Backoff::Backoff(int min, int max, float jitter, int factor)
{
  opts = opts || {};
  _ms = opts.min || 100;
  _max = opts.max || 10000;
  _factor = opts.factor || 2;
  _jitter = opts.jitter > 0 && opts.jitter <= 1 ? opts.jitter : 0;
  _attempts = 0;
}

Backoff::~Backoff()
{

}

float Backoff::getDuration()
{
  var ms = _ms * Math.pow(_factor, _attempts++);
  if (_jitter) {
    var rand =  Math.random();
    var deviation = Math.floor(rand * _jitter * ms);
    ms = (Math.floor(rand * 10) & 1) == 0  ? ms - deviation : ms + deviation;
  }
  return Math.min(ms, _max) | 0;
}

void Backoff::reset()
{
  _attempts = 0;
}

void Backoff::setMin(int min)
{
  _ms = min;
}

void Backoff::setMax(int max)
{
  _max = max;
}

void Backoff::setJitter(float jitter)
{
  _jitter = jitter;
}


