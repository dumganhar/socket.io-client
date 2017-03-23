#pragma once

class Backoff
{
public:

    /**
     * Initialize backoff timer with `opts`.
     *
     * - `min` initial timeout in milliseconds [100]
     * - `max` max timeout [10000]
     * - `jitter` [0]
     * - `factor` [2]
     *
     * @param {Object} opts
     * @api public
     */

    Backoff(long min, long max, float jitter, int factor);

    ~Backoff();

    /**
     * Return the backoff duration.
     *
     * @return {Number}
     * @api public
     */

    long getDuration();

    /**
     * Reset the number of attempts.
     *
     * @api public
     */

    void reset();

    /**
     * Set the minimum duration
     *
     * @api public
     */

    void setMin(long min);

    /**
     * Set the maximum duration
     *
     * @api public
     */

    void setMax(long max);

    /**
     * Set the jitter
     *
     * @api public
     */

    void setJitter(float jitter);

private:
    long _ms;
    long _max;
    float _jitter;
    int _factor;
    int _attempts;
};