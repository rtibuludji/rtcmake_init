cmake_minimum_required(VERSION 3.30.3)
project(rtcmake_init
               VERSION 1.0.0
               LANGUAGES C
               DESCRIPTION "")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWERCASE)
string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPERCASE)

#
# Prevent building in the source directory
#
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

message(STATUS "Started CMake Configuration for ${PROJECT_NAME} v${PROJECT_VERSION}-${CMAKE_BUILD_TYPE}...")

#
# Project Options
#
option(${PROJECT_NAME}_BUILD_EXECUTABLE "Build the project as an executable, rather than a library." ON)
option(${PROJECT_NAME}_BUILD_STATIC "Build the project as a static library." OFF)
option(${PROJECT_NAME}_BUILD_SHARED "Build the project as a shared library." OFF)
option(${PROJECT_NAME}_BUILD_HEADERS_ONLY "Build the project as a header-only library." OFF)

option(${PROJECT_NAME}_WARNINGS_AS_ERRORS "Treat compiler warnings as errors." ON)

option(${PROJECT_NAME}_ENABLE_LTO "Enable Interprocedural Optimization, aka Link Time Optimization (LTO)." ON)

#
# Static analyzers
#
option(${PROJECT_NAME}_ENABLE_CLANG_TIDY "Enable static analysis with Clang-Tidy." ON)
option(${PROJECT_NAME}_ENABLE_CPPCHECK "Enable static analysis with Cppcheck." ON)

include(CompilerWarnings)
include(InterproceduralOptimization)
include(StaticAnalyzer)

enable_lto()
enable_cppcheck()
enable_clang_tidy()

#
# Generate compile_commands.json to make it easier to work with clang based tools
#
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#
# Configure Target
#
include(SourcesAndHeaders)
if(${PROJECT_NAME}_BUILD_HEADERS_ONLY)
    add_library(${PROJECT_NAME} INTERFACE)
    target_compile_features(${PROJECT_NAME} INTERFACE c_std_17)
else()
    if(${PROJECT_NAME}_BUILD_EXECUTABLE)
        add_executable(${PROJECT_NAME} ${headers} ${sources})
    elseif(${PROJECT_NAME}_BUILD_STATIC)
        add_library(${PROJECT_NAME} STATIC ${headers} ${sources})
    elseif(${PROJECT_NAME}_BUILD_SHARED)
        add_library(${PROJECT_NAME} SHARED ${headers} ${sources})
    endif()

    target_compile_features(${PROJECT_NAME} PUBLIC c_std_17)
endif()
set_project_warnings(${PROJECT_NAME} ${PROJECT_NAME}_WARNINGS_AS_ERRORS ${PROJECT_NAME}_BUILD_HEADERS_ONLY)

#
# Enhance error reporting and compiler messages
#
# if(CMAKE_C_COMPILER_ID MATCHES ".*Clang")
#     target_compile_options(${PROJECT_NAME} PUBLIC -fcolor-diagnostics)
# elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
#     target_compile_options(${PROJECT_NAME} PUBLIC -fdiagnostics-color=always)
# else()
#     message(STATUS "No colored compiler diagnostic set for '${CMAKE_C_COMPILER_ID}' compiler.")
# endif()

message(STATUS "Finished setting up target.")

#
# Configure Include Directories
#
if(${PROJECT_NAME}_BUILD_HEADERS_ONLY)
    target_include_directories(
        ${PROJECT_NAME}
        INTERFACE
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )
else()
    target_include_directories(
        ${PROJECT_NAME}
        PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
endif()
message(STATUS "Finished setting up include directories.")

#
# Provide alias to library for
#
if(${PROJECT_NAME}_BUILD_EXECUTABLE)
    add_executable(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
else()
    add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
endif()

#
# Install Package
#
include(GNUInstallDirs)
install(
    TARGETS
        ${PROJECT_NAME}
    EXPORT
        ${PROJECT_NAME}Targets
    LIBRARY DESTINATION
        ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION
        ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION
        ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION
        include
    PUBLIC_HEADER DESTINATION
        include
)

if(NOT ${PROJECT_NAME}_BUILD_EXECUTABLE)
    install(
        EXPORT
            ${PROJECT_NAME}Targets
        FILE
            ${PROJECT_NAME}Targets.cmake
        NAMESPACE
            ${PROJECT_NAME}::
        DESTINATION
            ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )
endif()

#
# Add version header
#
configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/version.h.in
  ${PROJECT_SOURCE_DIR}/include/version.h
  @ONLY
)

#
# Install the `include` directory
#
if(NOT ${PROJECT_NAME}_BUILD_EXECUTABLE)
    install(
    DIRECTORY
        include/${PROJECT_NAME}
    DESTINATION
        include
    )
endif()
