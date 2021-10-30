#!/bin/bash
cmake-format -i CMakeLists.txt
find mytinyhttpd/ -regex '.*\.\(cc\|h\)' -exec clang-format -i { } \;
