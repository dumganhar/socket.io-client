#pragma once

#include "IOTypes.h"

class Emitter
{
public:
    Emitter();
    virtual ~Emitter();

    /**
     * Listen on the given `event` with `fn`.
     *
     * @param {String} event
     * @param {Function} fn
     * @return {Emitter}
     * @api public
     */
    virtual void on(const std::string& eventName, const std::function<void(const Value&)>& fn, int64_t key);
    /**
     * Adds an `event` listener that will be invoked a single
     * time then automatically removed.
     *
     * @param {String} event
     * @param {Function} fn
     * @return {Emitter}
     * @api public
     */
    virtual void once(const std::string& eventName, const std::function<void(const Value&)>& fn, int64_t key);

    /**
     * Remove the given callback for `event` or all
     * registered callbacks.
     *
     * @param {String} event
     * @param {Function} fn
     * @return {Emitter}
     * @api public
     */
    virtual void offAll();
    virtual void off(const std::string& eventName);
    virtual void off(const std::string& eventName, int64_t key);


    /**
     * Emit `event` with the given args.
     *
     * @param {String} event
     * @param {Mixed} ...
     * @return {Emitter}
     */

    virtual void emit(const std::string& eventName, const Value& args);


    struct Callback
    {
        std::function<void(const Value&)> fn;
        int64_t key;

        Callback()
        : fn(nullptr)
        , key(-1)
        {}

        Callback(const std::function<void(const Value&)>& fn_, int64_t key_)
        : fn(fn_)
        , key(key_)
        {}
    };

    /**
     * Return array of callbacks for `event`.
     *
     * @param {String} event
     * @return {Array}
     * @api public
     */

    virtual const std::vector<Callback>& getListeners(const std::string& eventName) const;

    /**
     * Check if this emitter has `event` handlers.
     *
     * @param {String} event
     * @return {Boolean}
     * @api public
     */

    virtual bool hasListeners(const std::string& eventName) const;

private:
    std::unordered_map<std::string, std::vector<Callback>> _callbacks;
};

struct OnObj
{
    std::function<void()> destroy;
};
/**
 * Helper for subscriptions.
 *
 * @param {Object|EventEmitter} obj with `Emitter` mixin or `EventEmitter`
 * @param {String} event name
 * @param {Function} callback
 * @api public
 */

OnObj on(std::shared_ptr<Emitter> obj, const std::string& ev, const std::function<void(const Value&)>& fn, int64_t key);
OnObj on(std::shared_ptr<Emitter> obj, const std::string& ev, const std::function<void(const Value&)>& fn);
