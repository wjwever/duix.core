function(download_websocketpp)
  include(FetchContent)

  set(websocketpp_URL   "https://github.com/zaphoyd/websocketpp/archive/refs/tags/0.8.2.tar.gz")
  #set(websocketpp_URL2  "https://github.com/libsndfile/websocketpp")
  #set(websocketpp_HASH "SHA256=37c1aa230b00fe062791d800d8fc50aa3de215918d3dce6440699e67275d859e")

  #set(websocketpp_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  #set(websocketpp_BUILD_PYTHON OFF CACHE BOOL "" FORCE)
  #set(websocketpp_ENABLE_CHECK OFF CACHE BOOL "" FORCE)

  # If you don't have access to the Internet,
  # please pre-download websocketpp
  #set(possible_file_locations
  #  $ENV{HOME}/Downloads/websocketpp-1.21.1.tar.gz
  #  ${CMAKE_SOURCE_DIR}/websocketpp-1.21.1.tar.gz
  #  ${CMAKE_BINARY_DIR}/websocketpp-1.21.1.tar.gz
  #  /tmp/websocketpp-1.21.1.tar.gz
  #  /star-fj/fangjun/download/github/websocketpp-1.21.1.tar.gz
  #)

  #foreach(f IN LISTS possible_file_locations)
  #  if(EXISTS ${f})
  #    set(websocketpp_URL  "${f}")
  #    file(TO_CMAKE_PATH "${websocketpp_URL}" websocketpp_URL)
  #    message(STATUS "Found local downloaded websocketpp: ${websocketpp_URL}")
  #    set(websocketpp_URL2 )
  #    break()
  #  endif()
  #endforeach()

  FetchContent_Declare(websocketpp
    URL ${websocketpp_URL}
  )

  FetchContent_GetProperties(websocketpp)
  if(NOT websocketpp_POPULATED)
    message(STATUS "Downloading websocketpp from ${websocketpp_URL}")
    FetchContent_Populate(websocketpp)
  endif()
  message(STATUS "websocketpp is downloaded to ${websocketpp_SOURCE_DIR}")
  message(STATUS "websocketpp's binary dir is ${websocketpp_BINARY_DIR}")
  include_directories(${websocketpp_SOURCE_DIR})

  #if(BUILD_SHARED_LIBS)
  #  set(_build_shared_libs_bak ${BUILD_SHARED_LIBS})
  #  set(BUILD_SHARED_LIBS OFF)
  #endif()

  #add_subdirectory(${websocketpp_SOURCE_DIR} ${websocketpp_BINARY_DIR} EXCLUDE_FROM_ALL)

  #if(_build_shared_libs_bak)
  #  set_target_properties(websocketpp-core
  #    PROPERTIES
  #      POSITION_INDEPENDENT_CODE ON
  #      C_VISIBILITY_PRESET hidden
  #      CXX_VISIBILITY_PRESET hidden
  #  )
  #  set(BUILD_SHARED_LIBS ON)
  #endif()

endfunction()

download_websocketpp()
