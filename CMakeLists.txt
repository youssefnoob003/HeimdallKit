cmake_minimum_required(VERSION 3.16)

project(HeimdallKit VERSION 0.1.0 LANGUAGES C)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(HEIM_BUILD_EXAMPLES "Build HeimdallKit examples" ON)
option(HEIM_ENABLE_SANITIZERS "Enable compiler-supported sanitizers" OFF)
option(HEIM_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
set(HEIM_MAX_READ_SAMPLES "10000000" CACHE STRING
    "Maximum number of samples accepted by binary readers")

function(heim_apply_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4)
    if(HEIM_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE /WX)
    endif()
  elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    if(HEIM_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE -Werror)
    endif()
  endif()
endfunction()

function(heim_apply_sanitizers target)
  if(HEIM_ENABLE_SANITIZERS)
    if(MSVC)
      target_compile_options(${target} PRIVATE /fsanitize=address)
      target_link_options(${target} PRIVATE /fsanitize=address)
    elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
      target_compile_options(${target} PRIVATE
        -fsanitize=address,undefined -fno-omit-frame-pointer
      )
      target_link_options(${target} PRIVATE
        -fsanitize=address,undefined -fno-omit-frame-pointer
      )
    else()
      message(FATAL_ERROR "HEIM_ENABLE_SANITIZERS requires GCC, Clang, AppleClang, or MSVC")
    endif()
  endif()
endfunction()

add_library(heim
  # Cross-domain contracts and common helpers.
  src/status/status.c
  src/common/math.c

  # Signal representation and operations.
  src/signal/core.c
  src/signal/generate.c
  src/signal/transform.c
  src/signal/analysis.c
  src/signal/operations.c

  # LTI operations.
  src/lti/convolution.c
  src/lti/properties.c
  src/lti/filter.c

  # File transaction and format implementations.
  src/io/file.c
  src/io/raw.c
  src/io/heim_format.c
)

add_library(HeimdallKit::heim ALIAS heim)

target_include_directories(heim
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/internal
)

if(NOT WIN32)
  target_link_libraries(heim PUBLIC m)
  target_compile_definitions(heim PRIVATE _POSIX_C_SOURCE=200809L)
endif()
heim_apply_warnings(heim)
target_compile_definitions(heim PRIVATE HEIM_MAX_READ_SAMPLES=${HEIM_MAX_READ_SAMPLES})

if(HEIM_ENABLE_SANITIZERS)
  heim_apply_sanitizers(heim)
endif()

include(CTest)

function(heim_add_test name label)
  add_executable(${name} tests/${name}.c)
  target_link_libraries(${name} PRIVATE heim)
  heim_apply_warnings(${name})
  if(MSVC)
    target_compile_options(${name} PRIVATE /UNDEBUG)
  else()
    target_compile_options(${name} PRIVATE -UNDEBUG)
  endif()
  heim_apply_sanitizers(${name})
  add_test(NAME ${name} COMMAND ${name})
  set_tests_properties(${name} PROPERTIES LABELS ${label})
endfunction()

if(BUILD_TESTING)
  heim_add_test(test_signal signal)
  heim_add_test(test_lti lti)
  heim_add_test(test_io io)
  heim_add_test(test_status status)
  heim_add_test(test_status_header status)
endif()

function(heim_add_example name)
  add_executable(${name} examples/${name}.c)
  target_link_libraries(${name} PRIVATE heim)
  heim_apply_warnings(${name})
  heim_apply_sanitizers(${name})
endfunction()

if(HEIM_BUILD_EXAMPLES)
  heim_add_example(signal_basics)
  heim_add_example(lti_filtering)
  heim_add_example(matched_filter)
endif()

install(TARGETS heim
  EXPORT HeimdallKitTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT HeimdallKitTargets
  FILE HeimdallKitTargets.cmake
  NAMESPACE HeimdallKit::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/HeimdallKit
)

configure_package_config_file(
  cmake/HeimdallKitConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/HeimdallKitConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/HeimdallKit
)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/HeimdallKitConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/HeimdallKitConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/HeimdallKitConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/HeimdallKit
)
