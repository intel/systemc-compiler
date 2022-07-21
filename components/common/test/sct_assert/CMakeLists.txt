#
# sct_property in thread property test
#

cmake_minimum_required(VERSION 3.12)
 
set(CMAKE_CXX_STANDARD 14)

set(SYSTEMC_INCLUDE $ENV{SYSTEMC_HOME}/include)
set(SYSTEMC_LIBRARY $ENV{SYSTEMC_HOME}/lib-linux64)

set(SCT_COMMON $ENV{SCT_HOME}/components/common)

link_directories(${SYSTEMC_LIBRARY})
include_directories(${SYSTEMC_INCLUDE} ${SCT_COMMON}/sctcommon)

add_executable (sct_assert_test
    ${SCT_COMMON}/sctcommon/sct_property.cpp
    ${SCT_COMMON}/test/sct_assert/sc_main.cpp
)

target_link_libraries(sct_assert_test systemc)


