include(cmake/SystemLink.cmake)
include(cmake/LibFuzzer.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)


include(CheckCXXSourceCompiles)


macro(SudokuSearch_supports_sanitizers)
  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)

    message(STATUS "Sanity checking UndefinedBehaviorSanitizer, it should be supported on this platform")
    set(TEST_PROGRAM "int main() { return 0; }")

    # Check if UndefinedBehaviorSanitizer works at link time
    set(CMAKE_REQUIRED_FLAGS "-fsanitize=undefined")
    set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=undefined")
    check_cxx_source_compiles("${TEST_PROGRAM}" HAS_UBSAN_LINK_SUPPORT)

    if(HAS_UBSAN_LINK_SUPPORT)
      message(STATUS "UndefinedBehaviorSanitizer is supported at both compile and link time.")
      set(SUPPORTS_UBSAN ON)
    else()
      message(WARNING "UndefinedBehaviorSanitizer is NOT supported at link time.")
      set(SUPPORTS_UBSAN OFF)
    endif()
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    if (NOT WIN32)
      message(STATUS "Sanity checking AddressSanitizer, it should be supported on this platform")
      set(TEST_PROGRAM "int main() { return 0; }")

      # Check if AddressSanitizer works at link time
      set(CMAKE_REQUIRED_FLAGS "-fsanitize=address")
      set(CMAKE_REQUIRED_LINK_OPTIONS "-fsanitize=address")
      check_cxx_source_compiles("${TEST_PROGRAM}" HAS_ASAN_LINK_SUPPORT)

      if(HAS_ASAN_LINK_SUPPORT)
        message(STATUS "AddressSanitizer is supported at both compile and link time.")
        set(SUPPORTS_ASAN ON)
      else()
        message(WARNING "AddressSanitizer is NOT supported at link time.")
        set(SUPPORTS_ASAN OFF)
      endif()
    else()
      set(SUPPORTS_ASAN ON)
    endif()
  endif()
endmacro()

macro(SudokuSearch_setup_options)
  option(SudokuSearch_ENABLE_HARDENING "Enable hardening" ON)
  option(SudokuSearch_ENABLE_COVERAGE "Enable coverage reporting" OFF)
  cmake_dependent_option(
    SudokuSearch_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to built dependencies"
    ON
    SudokuSearch_ENABLE_HARDENING
    OFF)

  SudokuSearch_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR SudokuSearch_PACKAGING_MAINTAINER_MODE)
    option(SudokuSearch_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(SudokuSearch_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(SudokuSearch_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(SudokuSearch_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(SudokuSearch_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(SudokuSearch_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(SudokuSearch_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(SudokuSearch_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(SudokuSearch_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(SudokuSearch_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(SudokuSearch_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(SudokuSearch_ENABLE_PCH "Enable precompiled headers" OFF)
    option(SudokuSearch_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(SudokuSearch_ENABLE_IPO "Enable IPO/LTO" ON)
    option(SudokuSearch_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
    option(SudokuSearch_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(SudokuSearch_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
    option(SudokuSearch_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(SudokuSearch_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(SudokuSearch_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(SudokuSearch_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(SudokuSearch_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(SudokuSearch_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
    option(SudokuSearch_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
    option(SudokuSearch_ENABLE_PCH "Enable precompiled headers" OFF)
    option(SudokuSearch_ENABLE_CACHE "Enable ccache" ON)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      SudokuSearch_ENABLE_IPO
      SudokuSearch_WARNINGS_AS_ERRORS
      SudokuSearch_ENABLE_USER_LINKER
      SudokuSearch_ENABLE_SANITIZER_ADDRESS
      SudokuSearch_ENABLE_SANITIZER_LEAK
      SudokuSearch_ENABLE_SANITIZER_UNDEFINED
      SudokuSearch_ENABLE_SANITIZER_THREAD
      SudokuSearch_ENABLE_SANITIZER_MEMORY
      SudokuSearch_ENABLE_UNITY_BUILD
      SudokuSearch_ENABLE_CLANG_TIDY
      SudokuSearch_ENABLE_CPPCHECK
      SudokuSearch_ENABLE_COVERAGE
      SudokuSearch_ENABLE_PCH
      SudokuSearch_ENABLE_CACHE)
  endif()

  SudokuSearch_check_libfuzzer_support(LIBFUZZER_SUPPORTED)
  if(LIBFUZZER_SUPPORTED AND (SudokuSearch_ENABLE_SANITIZER_ADDRESS OR SudokuSearch_ENABLE_SANITIZER_THREAD OR SudokuSearch_ENABLE_SANITIZER_UNDEFINED))
    set(DEFAULT_FUZZER ON)
  else()
    set(DEFAULT_FUZZER OFF)
  endif()

  option(SudokuSearch_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})

endmacro()

macro(SudokuSearch_global_options)
  if(SudokuSearch_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    SudokuSearch_enable_ipo()
  endif()

  SudokuSearch_supports_sanitizers()

  if(SudokuSearch_ENABLE_HARDENING AND SudokuSearch_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR SudokuSearch_ENABLE_SANITIZER_UNDEFINED
       OR SudokuSearch_ENABLE_SANITIZER_ADDRESS
       OR SudokuSearch_ENABLE_SANITIZER_THREAD
       OR SudokuSearch_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    message("${SudokuSearch_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${SudokuSearch_ENABLE_SANITIZER_UNDEFINED}")
    SudokuSearch_enable_hardening(SudokuSearch_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()
endmacro()

macro(SudokuSearch_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(SudokuSearch_warnings INTERFACE)
  add_library(SudokuSearch_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  SudokuSearch_set_project_warnings(
    SudokuSearch_warnings
    ${SudokuSearch_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  if(SudokuSearch_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    SudokuSearch_configure_linker(SudokuSearch_options)
  endif()

  include(cmake/Sanitizers.cmake)
  SudokuSearch_enable_sanitizers(
    SudokuSearch_options
    ${SudokuSearch_ENABLE_SANITIZER_ADDRESS}
    ${SudokuSearch_ENABLE_SANITIZER_LEAK}
    ${SudokuSearch_ENABLE_SANITIZER_UNDEFINED}
    ${SudokuSearch_ENABLE_SANITIZER_THREAD}
    ${SudokuSearch_ENABLE_SANITIZER_MEMORY})

  set_target_properties(SudokuSearch_options PROPERTIES UNITY_BUILD ${SudokuSearch_ENABLE_UNITY_BUILD})

  if(SudokuSearch_ENABLE_PCH)
    target_precompile_headers(
      SudokuSearch_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(SudokuSearch_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    SudokuSearch_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(SudokuSearch_ENABLE_CLANG_TIDY)
    SudokuSearch_enable_clang_tidy(SudokuSearch_options ${SudokuSearch_WARNINGS_AS_ERRORS})
  endif()

  if(SudokuSearch_ENABLE_CPPCHECK)
    SudokuSearch_enable_cppcheck(${SudokuSearch_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(SudokuSearch_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    SudokuSearch_enable_coverage(SudokuSearch_options)
  endif()

  if(SudokuSearch_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(SudokuSearch_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

  if(SudokuSearch_ENABLE_HARDENING AND NOT SudokuSearch_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR SudokuSearch_ENABLE_SANITIZER_UNDEFINED
       OR SudokuSearch_ENABLE_SANITIZER_ADDRESS
       OR SudokuSearch_ENABLE_SANITIZER_THREAD
       OR SudokuSearch_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    SudokuSearch_enable_hardening(SudokuSearch_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()

endmacro()
