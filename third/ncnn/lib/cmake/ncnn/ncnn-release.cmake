#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ncnn" for configuration "Release"
set_property(TARGET ncnn APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(ncnn PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libncnn.so.1.0.20241226"
  IMPORTED_SONAME_RELEASE "libncnn.so.1"
  )

list(APPEND _cmake_import_check_targets ncnn )
list(APPEND _cmake_import_check_files_for_ncnn "${_IMPORT_PREFIX}/lib/libncnn.so.1.0.20241226" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
