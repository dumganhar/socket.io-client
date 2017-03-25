#include "Emitter.h"
#include "IOUtils.h"

Emitter::Emitter()
{

}

Emitter::~Emitter()
{

}

void Emitter::on(const std::string& eventName, const std::function<void(const Value&)>& fn, int64_t key)
{
  Callback cb(fn, key);
  _callbacks[eventName].push_back(std::move(cb));
}


void Emitter::once(const std::string& eventName, const std::function<void(const Value&)>& fn, int64_t key)
{
  auto cb = [=](const Value& args) {
    off(eventName, key);
    fn(args);
  };

  on(eventName, cb, key);
}

void Emitter::offAll()
{
    _callbacks.clear();
}

void Emitter::off(const std::string& eventName)
{
    auto iter = _callbacks.find(eventName);
    if (iter != _callbacks.end())
    {
        _callbacks.erase(iter);
    }
}

void Emitter::off(const std::string& eventName, int64_t key)
{
  // specific event
  auto iter = _callbacks.find(eventName);
  if (iter == _callbacks.end())
  {
      return;
  }
  auto& callbacks = iter->second;

  // remove specific handler
  for (auto iter = callbacks.begin(); iter != callbacks.end(); ++iter)
  {
      auto& cb = *iter;
      if (cb.key == key)
      {
        callbacks.erase(iter);
        break;
      }
  }
}

void Emitter::emit(const std::string& eventName, const Value& args)
{
  auto iter = _callbacks.find(eventName);
  if (iter != _callbacks.end())
  {
    std::vector<Callback> copied = iter->second;
    for (const auto& cb : copied)
    {
      cb.fn(args);
    }
  }
}

const std::vector<Emitter::Callback>& Emitter::getListeners(const std::string& eventName) const
{
    return _callbacks.at(eventName);
}

bool Emitter::hasListeners(const std::string& eventName) const
{
    return _callbacks.find(eventName) != _callbacks.end();
}

OnObj on(std::shared_ptr<Emitter> obj, const std::string& ev, const std::function<void(const Value&)>& fn, int64_t key)
{
  obj->on(ev, fn, key);
  OnObj onObj;
  onObj.destroy = [obj, ev, fn, key](){
    obj->off(ev, key);
  };
  return onObj;
}

OnObj on(std::shared_ptr<Emitter> obj, const std::string& ev, const std::function<void(const Value&)>& fn)
{
  int64_t key = grabListenerId();
  return on(obj, ev, fn, key);
}
