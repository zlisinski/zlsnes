# Ubuntu repos only include gtest source, not binaries.
# TODO: Make this also work for distros that include binaries.
find_package(Threads REQUIRED)
set(GTEST_SRC "/usr/src/gtest")
include_directories(${GTEST_SRC})
add_library(gtest ${GTEST_SRC}/src/gtest-all.cc)
target_link_libraries(gtest Threads::Threads)

set(TEST_SRC
    AddressModeTest.cpp
    BcdTest.cpp
    CpuTest.cpp
    main.cpp
)

add_executable(test_zlsnes
    ${TEST_SRC}
)
target_link_libraries(test_zlsnes
    gtest
    zlsnes_core
)

add_test(test_zlsnes test_zlsnes)