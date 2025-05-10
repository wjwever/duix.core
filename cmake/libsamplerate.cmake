function(download_libsamplerate)
  include(FetchContent)

  set(libsamplerate_URL   "https://github.com/libsndfile/libsamplerate/archive/refs/tags/0.2.2.tar.gz")
  #set(libsamplerate_URL2  "https://github.com/libsndfile/libsamplerate")
  #set(libsamplerate_HASH "SHA256=37c1aa230b00fe062791d800d8fc50aa3de215918d3dce6440699e67275d859e")

  #set(libsamplerate_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  #set(libsamplerate_BUILD_PYTHON OFF CACHE BOOL "" FORCE)
  #set(libsamplerate_ENABLE_CHECK OFF CACHE BOOL "" FORCE)

  # If you don't have access to the Internet,
  # please pre-download libsamplerate
  #set(possible_file_locations
  #  $ENV{HOME}/Downloads/libsamplerate-1.21.1.tar.gz
  #  ${CMAKE_SOURCE_DIR}/libsamplerate-1.21.1.tar.gz
  #  ${CMAKE_BINARY_DIR}/libsamplerate-1.21.1.tar.gz
  #  /tmp/libsamplerate-1.21.1.tar.gz
  #  /star-fj/fangjun/download/github/libsamplerate-1.21.1.tar.gz
  #)

  #foreach(f IN LISTS possible_file_locations)
  #  if(EXISTS ${f})
  #    set(libsamplerate_URL  "${f}")
  #    file(TO_CMAKE_PATH "${libsamplerate_URL}" libsamplerate_URL)
  #    message(STATUS "Found local downloaded libsamplerate: ${libsamplerate_URL}")
  #    set(libsamplerate_URL2 )
  #    break()
  #  endif()
  #endforeach()

  FetchContent_Declare(libsamplerate
    URL ${libsamplerate_URL}
  )

  FetchContent_GetProperties(libsamplerate)
  if(NOT libsamplerate_POPULATED)
    message(STATUS "Downloading libsamplerate from ${libsamplerate_URL}")
    FetchContent_Populate(libsamplerate)
  endif()
  message(STATUS "libsamplerate is downloaded to ${libsamplerate_SOURCE_DIR}")
  message(STATUS "libsamplerate's binary dir is ${libsamplerate_BINARY_DIR}")

  #if(BUILD_SHARED_LIBS)
  #  set(_build_shared_libs_bak ${BUILD_SHARED_LIBS})
  #  set(BUILD_SHARED_LIBS OFF)
  #endif()

  add_subdirectory(${libsamplerate_SOURCE_DIR} ${libsamplerate_BINARY_DIR} EXCLUDE_FROM_ALL)

  #if(_build_shared_libs_bak)
  #  set_target_properties(libsamplerate-core
  #    PROPERTIES
  #      POSITION_INDEPENDENT_CODE ON
  #      C_VISIBILITY_PRESET hidden
  #      CXX_VISIBILITY_PRESET hidden
  #  )
  #  set(BUILD_SHARED_LIBS ON)
  #endif()

  target_include_directories(samplerate
    INTERFACE
      ${libsamplerate_SOURCE_DIR}/
  )

  if(NOT BUILD_SHARED_LIBS)
    install(TARGETS samplerate DESTINATION lib)
  endif()
endfunction()

download_libsamplerate()
