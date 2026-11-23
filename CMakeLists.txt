# File: /home/kami/motion-guard/CMakeLists.txt

cmake_minimum_required(VERSION 3.20)
project(motion_guard VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "/usr/local/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -O3
        -march=native
        -Wall
        -Wextra
        -Werror=return-type
        -fno-omit-frame-pointer
    )
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    /home/kami/blackbox-sentinel/include
    /usr/local/include
)

find_package(Threads REQUIRED)
find_library(BLACKBOX_LIB blackbox REQUIRED PATHS /usr/local/lib /usr/lib)
find_library(XINFER_LIB xinfer REQUIRED PATHS /usr/local/lib /usr/lib)

message(STATUS "[motion-guard] Linked Blackbox Essential: ${BLACKBOX_LIB}")
message(STATUS "[motion-guard] Linked Xinfer Essential:   ${XINFER_LIB}")

set(LIB_SOURCES
    src/core/tsc_clock.cpp
    src/protocols/ethercat_l2.cpp
    src/protocols/canfd_bus.cpp
    src/protocols/shm_motion.cpp
    src/defense/motion_cbf_gate.cpp
    src/defense/motion_airgap.cpp
    src/pipeline/guard_engine.cpp
    src/protocols/surgical_tsn.cpp
)

add_library(motion_guard SHARED ${LIB_SOURCES})
add_library(motion_guard_static STATIC ${LIB_SOURCES})
set_target_properties(motion_guard_static PROPERTIES OUTPUT_NAME motion_guard)

target_include_directories(motion_guard PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:/home/kami/blackbox-sentinel/include>
    $<BUILD_INTERFACE:/usr/local/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_include_directories(motion_guard_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:/home/kami/blackbox-sentinel/include>
    $<BUILD_INTERFACE:/usr/local/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(motion_guard PUBLIC
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
    Threads::Threads
)

target_link_libraries(motion_guard_static PUBLIC
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
    Threads::Threads
)

add_executable(motion-guard-ctl tools/motion_guard_ctl/main.cpp)
target_link_libraries(motion-guard-ctl PRIVATE
    motion_guard
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
)

add_executable(aryo-motion tools/aryo_motion/main.cpp)
target_link_libraries(aryo-motion PRIVATE
    motion_guard
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
)

add_library(motion_guard_dissector_plugin SHARED
    tools/sentinel_plugin/motion_guard_plugin.cpp
)
target_link_libraries(motion_guard_dissector_plugin PRIVATE
    motion_guard
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
)
set_target_properties(motion_guard_dissector_plugin PROPERTIES
    PREFIX ""
    OUTPUT_NAME "motion_guard_dissector_plugin"
)

add_executable(bench_latency benchmarks/bench_latency.cpp)
target_link_libraries(bench_latency PRIVATE
    motion_guard
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
)

enable_testing()

add_executable(test_ethercat_l2 tests/test_ethercat_l2.cpp)
target_link_libraries(test_ethercat_l2 PRIVATE motion_guard)
add_test(NAME TestEthercatL2 COMMAND test_ethercat_l2)

add_executable(test_canfd_bus tests/test_canfd_bus.cpp)
target_link_libraries(test_canfd_bus PRIVATE motion_guard)
add_test(NAME TestCanfdBus COMMAND test_canfd_bus)

add_executable(test_motion_cbf tests/test_motion_cbf.cpp)
target_link_libraries(test_motion_cbf PRIVATE motion_guard)
add_test(NAME TestMotionCbf COMMAND test_motion_cbf)

add_executable(test_plugin tests/test_sentinel_plugin.cpp)
target_link_libraries(test_plugin PRIVATE
    motion_guard
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
    ${CMAKE_DL_LIBS}
)
add_test(NAME TestSentinelPlugin COMMAND test_plugin)

add_executable(test_motion_tier1_tier2_pipeline tests/test_motion_tier1_tier2_pipeline.cpp)
target_link_libraries(test_motion_tier1_tier2_pipeline PRIVATE
    motion_guard
    -Wl,--no-as-needed
    ${BLACKBOX_LIB}
    ${XINFER_LIB}
    -Wl,--as-needed
    ${CMAKE_DL_LIBS}
)
add_test(NAME TestMotionTier1Tier2Pipeline COMMAND test_motion_tier1_tier2_pipeline)

install(TARGETS motion_guard motion_guard_static motion-guard-ctl aryo-motion motion_guard_dissector_plugin
    EXPORT MotionGuardTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(TARGETS motion_guard_dissector_plugin
    DESTINATION /usr/local/lib/blackbox
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

set(CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/MotionGuard)
configure_package_config_file(
    cmake/MotionGuardConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/MotionGuardConfig.cmake
    INSTALL_DESTINATION ${CONFIG_INSTALL_DIR}
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/MotionGuardConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/MotionGuardConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/MotionGuardConfigVersion.cmake
    DESTINATION ${CONFIG_INSTALL_DIR}
)
install(EXPORT MotionGuardTargets
    FILE MotionGuardTargets.cmake
    NAMESPACE MotionGuard::
    DESTINATION ${CONFIG_INSTALL_DIR}
)

configure_file(
    cmake/motion_guard.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/motion_guard.pc
    @ONLY
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/motion_guard.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
