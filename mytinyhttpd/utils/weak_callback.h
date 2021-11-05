#ifndef MYTINYHTTPD_UTILS_WEAK_CALLBACK_H_
#define MYTINYHTTPD_UTILS_WEAK_CALLBACK_H_

#include <functional>
#include <memory>

namespace mytinyhttpd {

template <typename CLASS, typename... ARGS>
class WeakCallback {
 public:
  WeakCallback(const std::weak_ptr<CLASS>& object,
               const std::function<void(CLASS*, ARGS...)>& function)
      : object_(object), function_(function) {}

  // if object_ exists, call function_, then return true
  bool operator()(ARGS&&... args) const {
    std::shared_ptr<CLASS> ptr(object_.lock());
    if (ptr) {
      function_(ptr.get(), std::forward<ARGS>(args)...);
      return true;
    }
    return false;
  }

 private:
  std::weak_ptr<CLASS> object_;
  std::function<void(CLASS*, ARGS...)> function_;
};

template <typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> MakeWeakCallback(
    const std::shared_ptr<CLASS>& object, void (CLASS::*function)(ARGS...)) {
  return WeakCallback<CLASS, ARGS...>(object, function);
}

template <typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> MakeWeakCallback(
    const std::shared_ptr<CLASS>& object,
    void (CLASS::*function)(ARGS...) const) {
  return WeakCallback<CLASS, ARGS...>(object, function);
}

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_UTILS_WEAK_CALLBACK_H_