# CMake generated Testfile for 
# Source directory: /home/kami/motion-guard
# Build directory: /home/kami/motion-guard/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[TestEthercatL2]=] "/home/kami/motion-guard/build/test_ethercat_l2")
set_tests_properties([=[TestEthercatL2]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/kami/motion-guard/CMakeLists.txt;132;add_test;/home/kami/motion-guard/CMakeLists.txt;0;")
add_test([=[TestCanfdBus]=] "/home/kami/motion-guard/build/test_canfd_bus")
set_tests_properties([=[TestCanfdBus]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/kami/motion-guard/CMakeLists.txt;136;add_test;/home/kami/motion-guard/CMakeLists.txt;0;")
add_test([=[TestMotionCbf]=] "/home/kami/motion-guard/build/test_motion_cbf")
set_tests_properties([=[TestMotionCbf]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/kami/motion-guard/CMakeLists.txt;140;add_test;/home/kami/motion-guard/CMakeLists.txt;0;")
add_test([=[TestSentinelPlugin]=] "/home/kami/motion-guard/build/test_plugin")
set_tests_properties([=[TestSentinelPlugin]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/kami/motion-guard/CMakeLists.txt;151;add_test;/home/kami/motion-guard/CMakeLists.txt;0;")
add_test([=[TestMotionTier1Tier2Pipeline]=] "/home/kami/motion-guard/build/test_motion_tier1_tier2_pipeline")
set_tests_properties([=[TestMotionTier1Tier2Pipeline]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/kami/motion-guard/CMakeLists.txt;162;add_test;/home/kami/motion-guard/CMakeLists.txt;0;")
