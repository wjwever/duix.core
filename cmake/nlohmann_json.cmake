function(download_json)
  include(FetchContent)

  set(json_URL   "https://github.com/nlohmann/json/archive/refs/tags/v3.12.0.tar.gz")
  #set(json_URL2  "https://github.com/libsndfile/json")
  #set(json_HASH "SHA256=37c1aa230b00fe062791d800d8fc50aa3de215918d3dce6440699e67275d859e")

  #set(json_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  #set(json_BUILD_PYTHON OFF CACHE BOOL "" FORCE)
  #set(json_ENABLE_CHECK OFF CACHE BOOL "" FORCE)

  # If you don't have access to the Internet,
  # please pre-download json
  #set(possible_file_locations
  #  $ENV{HOME}/Downloads/json-1.21.1.tar.gz
  #  ${CMAKE_SOURCE_DIR}/json-1.21.1.tar.gz
  #  ${CMAKE_BINARY_DIR}/json-1.21.1.tar.gz
  #  /tmp/json-1.21.1.tar.gz
  #  /star-fj/fangjun/download/github/json-1.21.1.tar.gz
  #)

  #foreach(f IN LISTS possible_file_locations)
  #  if(EXISTS ${f})
  #    set(json_URL  "${f}")
  #    file(TO_CMAKE_PATH "${json_URL}" json_URL)
  #    message(STATUS "Found local downloaded json: ${json_URL}")
  #    set(json_URL2 )
  #    break()
  #  endif()
  #endforeach()

  FetchContent_Declare(json
    URL ${json_URL}
  )

  FetchContent_GetProperties(json)
  if(NOT json_POPULATED)
    message(STATUS "Downloading json from ${json_URL}")
    FetchContent_Populate(json)
  endif()
  message(STATUS "json is downloaded to ${json_SOURCE_DIR}")
  message(STATUS "json's binary dir is ${json_BINARY_DIR}")
  include_directories(${json_SOURCE_DIR}/include)

  #if(BUILD_SHARED_LIBS)
  #  set(_build_shared_libs_bak ${BUILD_SHARED_LIBS})
  #  set(BUILD_SHARED_LIBS OFF)
  #endif()

  #add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)

  #if(_build_shared_libs_bak)
  #  set_target_properties(json-core
  #    PROPERTIES
  #      POSITION_INDEPENDENT_CODE ON
  #      C_VISIBILITY_PRESET hidden
  #      CXX_VISIBILITY_PRESET hidden
  #  )
  #  set(BUILD_SHARED_LIBS ON)
  #endif()

endfunction()

download_json()
