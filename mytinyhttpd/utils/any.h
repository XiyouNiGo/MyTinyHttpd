#ifndef MYTINYHTTPD_UTILS_ANY_H_
#define MYTINYHTTPD_UTILS_ANY_H_

#include <typeinfo>
#include <utility>

namespace mytinyhttpd {

class Any {
 public:
  Any() : content_(nullptr) {}

  ~Any() { delete content_; }

  const std::type_info &type_info() const {
    return content_ ? content_->type_info() : typeid(void);
  }

  Any(const Any &other)
      : content_(other.content_ ? other.content_->Clone() : nullptr) {}

  Any(Any &&other) noexcept : content_(other.content_) {
    other.content_ = nullptr;
  }

  template <typename ValueType>
  Any(const ValueType &value) : content_(new Holder<ValueType>(value)) {}

  Any &swap(Any &rhs) {
    std::swap(content_, rhs.content_);
    return *this;
  }

  Any &operator=(const Any &rhs) {
    Any(rhs).swap(*this);
    return *this;
  }
  Any &operator=(Any &&rhs) noexcept {
    rhs.swap(*this);
    Any().swap(rhs);
    return *this;
  }
  template <class ValueType>
  Any &operator=(ValueType &&rhs) {
    Any(static_cast<ValueType &&>(rhs)).swap(*this);
    return *this;
  }

  bool empty() const noexcept { return !content_; }

  void clear() noexcept { Any().swap(*this); }

  operator void *() const { return content_; }
  operator const void *() const { return content_; }

  template <typename ValueType>
  bool CopyTo(ValueType &value) const {
    const ValueType *copyable = ToPtr<ValueType>();
    if (copyable) value = *copyable;
    return copyable;
  }

  template <typename ValueType>
  ValueType *ToPtr() const {
    return type_info() == typeid(ValueType)
               ? const_cast<ValueType *>(
                     &((static_cast<Holder<ValueType> *>(content_))->held_))
               : nullptr;
  }

 private:
  class PlaceHolder {
   public:
    virtual ~PlaceHolder() {}
    virtual const std::type_info &type_info() const = 0;
    virtual PlaceHolder *Clone() const = 0;
  };

  template <typename ValueType>
  class Holder : public PlaceHolder {
   public:
    Holder(const ValueType &value) : held_(value) {}

    virtual const std::type_info &type_info() const {
      return typeid(ValueType);
    }

    virtual PlaceHolder *Clone() const { return new Holder(held_); }

    const ValueType held_;
  };

  PlaceHolder *content_;
};

template <typename ValueType>
ValueType AnyCast(const Any &operand) {
  return AnyCast<ValueType>(const_cast<Any &>(operand));
}

template <typename ValueType>
ValueType AnyCast(Any &operand) {
  ValueType *result = operand.ToPtr<ValueType>();
  return result ? *result : throw std::bad_cast();
}

template <typename ValueType>
const ValueType *AnyCast(const Any *operand) {
  return AnyCast<ValueType>(const_cast<Any *>(operand));
}

template <typename ValueType>
ValueType *AnyCast(Any *operand) {
  ValueType *result = operand->ToPtr<ValueType>();
  return result ? result : throw std::bad_cast();
}

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_UTILS_ANY_H_