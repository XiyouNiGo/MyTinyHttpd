#include "mytinyhttpd/utils/weak_callback.h"

#include <gtest/gtest.h>

#include <iostream>
#include <memory>

namespace mytinyhttpd {

class Foo {
 public:
  void Func(int) { std::cout << "Foo::Func is called." << std::endl; }

  void ConstFunc(int) const {
    std::cout << "Foo::ConstFunc is called." << std::endl;
  }
};

TEST(WeakCallbackTest, HaveCallbackTest) {
  std::shared_ptr<Foo> sp_foo(new Foo);
  auto call_func = MakeWeakCallback(sp_foo, &Foo::Func);
  ASSERT_TRUE(call_func(0));
  sp_foo.reset();
  ASSERT_FALSE(call_func(0));
}

TEST(WeakCallbackTest, ConstCallbackTest) {
  std::shared_ptr<Foo> sp_foo(new Foo);
  auto call_func = MakeWeakCallback(sp_foo, &Foo::Func);
  ASSERT_TRUE(call_func(0));
  auto call_constfunc = MakeWeakCallback(sp_foo, &Foo::ConstFunc);
  ASSERT_TRUE(call_constfunc(0));
}

}  // namespace mytinyhttpd