#include "mytinyhttpd/utils/program_options.h"

#include <cstddef>
#include <iostream>

using namespace boost::program_options;

namespace mytinyhttpd {

void VerifyExclusiveOption(const variables_map& vm, const char* op) {
  if (vm.count(op) && !vm[op].defaulted() && vm.size() > 1) {
    for (auto i = vm.begin(); i != vm.end(); i++) {
      if (!(*i).second.defaulted() && (*i).first != op) {
        throw std::logic_error(std::string("Exclusive option '") + op + "'");
      }
    }
  }
}

void VerifyConflictingOptions(const variables_map& vm, const char* op1,
                              const char* op2) {
  if (vm.count(op1) && !vm[op1].defaulted() && vm.count(op2) &&
      !vm[op2].defaulted()) {
    throw std::logic_error(std::string("Conflicting options '") + op1 +
                           "' and '" + op2 + "'.");
  }
}

void VerifyOptionDependency(const variables_map& vm, const char* op,
                            const char* required_op) {
  if (vm.count(op) && !vm[op].defaulted()) {
    if (vm.count(required_op) == 0 || vm[required_op].defaulted()) {
      throw std::logic_error(std::string("Option '") + op +
                             "' requires option '" + required_op + "'.");
    }
  }
}

}  // namespace mytinyhttpd