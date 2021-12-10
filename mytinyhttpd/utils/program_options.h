#ifndef MYTINYHTTPD_UTILS_PROGRAM_OPTIONS_H_
#define MYTINYHTTPD_UTILS_PROGRAM_OPTIONS_H_

#include <boost/program_options.hpp>

namespace mytinyhttpd {

// auxiliary functions for checking input for validity
void VerifyConflictingOptions(const boost::program_options::variables_map& vm,
                              const char* op1, const char* op2);

void VerifyOptionDependency(const boost::program_options::variables_map& vm,
                            const char* op, const char* required_op);

}  // namespace mytinyhttpd

#endif  // !MYTINYHTTPD_UTILS_PROGRAM_OPTIONS_H_