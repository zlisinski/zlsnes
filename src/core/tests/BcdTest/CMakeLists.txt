include_directories(BEFORE
	${CMAKE_CURRENT_SOURCE_DIR}/mock
)
include_directories(
    ../../
)

find_package(Qt5 REQUIRED COMPONENTS Core)

add_executable(BcdTest
    BcdTest.cpp
)

target_link_libraries(BcdTest
    gtest
    gtest_main
    Qt5::Core
)

add_test(NAME BcdTest COMMAND BcdTest)
set_property(TEST BcdTest PROPERTY WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")