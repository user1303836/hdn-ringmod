include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.12.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(Catch2)

add_executable(HdnRingmodTests
    TestOscillator.cpp
    TestYinPitchDetector.cpp
    TestPitchSmoother.cpp
    TestParameters.cpp
)

target_sources(HdnRingmodTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../source/dsp/Oscillator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../source/dsp/YinPitchDetector.cpp
)

target_include_directories(HdnRingmodTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../source
)

target_link_libraries(HdnRingmodTests PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(HdnRingmodTests)
