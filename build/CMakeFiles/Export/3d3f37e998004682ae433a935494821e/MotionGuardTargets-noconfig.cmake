#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "MotionGuard::motion_guard" for configuration ""
set_property(TARGET MotionGuard::motion_guard APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(MotionGuard::motion_guard PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libmotion_guard.so"
  IMPORTED_SONAME_NOCONFIG "libmotion_guard.so"
  )

list(APPEND _cmake_import_check_targets MotionGuard::motion_guard )
list(APPEND _cmake_import_check_files_for_MotionGuard::motion_guard "${_IMPORT_PREFIX}/lib/libmotion_guard.so" )

# Import target "MotionGuard::motion_guard_static" for configuration ""
set_property(TARGET MotionGuard::motion_guard_static APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(MotionGuard::motion_guard_static PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_NOCONFIG "CXX"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/libmotion_guard.a"
  )

list(APPEND _cmake_import_check_targets MotionGuard::motion_guard_static )
list(APPEND _cmake_import_check_files_for_MotionGuard::motion_guard_static "${_IMPORT_PREFIX}/lib/libmotion_guard.a" )

# Import target "MotionGuard::motion-guard-ctl" for configuration ""
set_property(TARGET MotionGuard::motion-guard-ctl APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(MotionGuard::motion-guard-ctl PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/bin/motion-guard-ctl"
  )

list(APPEND _cmake_import_check_targets MotionGuard::motion-guard-ctl )
list(APPEND _cmake_import_check_files_for_MotionGuard::motion-guard-ctl "${_IMPORT_PREFIX}/bin/motion-guard-ctl" )

# Import target "MotionGuard::aryo-motion" for configuration ""
set_property(TARGET MotionGuard::aryo-motion APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(MotionGuard::aryo-motion PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/bin/aryo-motion"
  )

list(APPEND _cmake_import_check_targets MotionGuard::aryo-motion )
list(APPEND _cmake_import_check_files_for_MotionGuard::aryo-motion "${_IMPORT_PREFIX}/bin/aryo-motion" )

# Import target "MotionGuard::motion_guard_dissector_plugin" for configuration ""
set_property(TARGET MotionGuard::motion_guard_dissector_plugin APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(MotionGuard::motion_guard_dissector_plugin PROPERTIES
  IMPORTED_LINK_DEPENDENT_LIBRARIES_NOCONFIG "MotionGuard::motion_guard"
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/lib/motion_guard_dissector_plugin.so"
  IMPORTED_SONAME_NOCONFIG "motion_guard_dissector_plugin.so"
  )

list(APPEND _cmake_import_check_targets MotionGuard::motion_guard_dissector_plugin )
list(APPEND _cmake_import_check_files_for_MotionGuard::motion_guard_dissector_plugin "${_IMPORT_PREFIX}/lib/motion_guard_dissector_plugin.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
