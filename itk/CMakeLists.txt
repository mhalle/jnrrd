cmake_minimum_required(VERSION 3.16.3)
project(JNRRD)

# Find ITK
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

# Find or fetch nlohmann/json
find_package(nlohmann_json QUIET)

if(NOT nlohmann_json_FOUND)
  message(STATUS "nlohmann/json not found, fetching from GitHub")
  include(FetchContent)
  FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
  )
  FetchContent_MakeAvailable(json)
endif()

# Check for optional compression libraries
find_package(ZLIB REQUIRED)
find_package(BZip2)
find_package(zstd)
find_package(LZ4)

# Configure library with available compression methods
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/jnrrdConfig.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/jnrrdConfig.h
)

# Add include directories
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)

# Set the JNRRD library sources
set(JNRRD_SRC
  itkJNRRDImageIO.cxx
  itkJNRRDImageIOFactory.cxx
)

# Create the JNRRD library
add_library(JNRRD ${JNRRD_SRC})

# Link dependencies
target_link_libraries(JNRRD PUBLIC
  ${ITK_LIBRARIES}
  ZLIB::ZLIB
  nlohmann_json::nlohmann_json
)

# Link optional compression libraries if available
if(BZIP2_FOUND)
  target_link_libraries(JNRRD PUBLIC BZip2::BZip2)
  target_compile_definitions(JNRRD PRIVATE JNRRD_USE_BZ2)
endif()

if(zstd_FOUND)
  target_link_libraries(JNRRD PUBLIC zstd::libzstd_shared)
  target_compile_definitions(JNRRD PRIVATE JNRRD_USE_ZSTD)
endif()

if(LZ4_FOUND)
  target_link_libraries(JNRRD PUBLIC LZ4::LZ4)
  target_compile_definitions(JNRRD PRIVATE JNRRD_USE_LZ4)
endif()

# Install the library
install(TARGETS JNRRD
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)

# Install the headers
install(FILES 
  ${CMAKE_CURRENT_SOURCE_DIR}/itkJNRRDImageIO.h
  ${CMAKE_CURRENT_SOURCE_DIR}/itkJNRRDImageIOFactory.h
  ${CMAKE_CURRENT_BINARY_DIR}/jnrrdConfig.h
  DESTINATION include
)

# Create a simple test program
add_executable(JNRRDTest JNRRDTest.cxx)
target_link_libraries(JNRRDTest JNRRD)