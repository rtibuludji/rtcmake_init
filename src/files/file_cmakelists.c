/*
 *
 * 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "rtcmake_init.h"

static char errstr[256];

const char* rtcmake_create_cmakelists_txt(const rtcmake_projectsetting_t * const projectsetting, const rtcmake_version_t *cmake_version) 
{
    assert(projectsetting != NULL);

    FILE       *file;
    char        buf[128] = {0};

    // CMakeLists.txt
    snprintf(buf, sizeof(buf), "./CMakeLists.txt");
    file = fopen(buf, "w"); 
    if (file == NULL) {
        snprintf(errstr, sizeof(errstr), "invalid generate file %s", buf);
        return errstr;
    }

    fprintf(file, "cmake_minimum_required(VERSION %u.%u.%u)\n", cmake_version->major, cmake_version->minor, cmake_version->patch);
    fprintf(file, "project(%s\n", projectsetting->name);
    fprintf(file, "               VERSION %s\n", projectsetting->version);
    fprintf(file, "               LANGUAGES %s\n", projectsetting->language);
    fprintf(file, "               DESCRIPTION \"%s\")\n", projectsetting->description);
    fprintf(file, "\n");
    fprintf(file, "list(APPEND CMAKE_MODULE_PATH \"${CMAKE_CURRENT_SOURCE_DIR}/cmake\")\n");
    fprintf(file, "\n");
    fprintf(file, "string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWERCASE)\n");
    fprintf(file, "string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPERCASE)\n");
    fprintf(file, "\n");
    fprintf(file, "#\n");
    fprintf(file, "# Prevent building in the source directory\n");
    fprintf(file, "#\n");
    fprintf(file, "if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)\n");
    fprintf(file, "    message(FATAL_ERROR \"In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.\")\n");
    fprintf(file, "endif()\n");
    fprintf(file, "\n");
    fprintf(file, "message(STATUS \"Started CMake Configuration for ${PROJECT_NAME} v${PROJECT_VERSION}-${CMAKE_BUILD_TYPE}...\")\n");
    fprintf(file, "\n");
    fprintf(file, "#\n");
    fprintf(file, "# Project Options\n");
    fprintf(file, "#\n");
    fprintf(file, "option(${PROJECT_NAME}_BUILD_EXECUTABLE \"Build the project as an executable, rather than a library.\" %s)\n", projectsetting->target[0] == 'E' ? "ON":"OFF");
    fprintf(file, "option(${PROJECT_NAME}_BUILD_STATIC \"Build the project as a static library.\" %s)\n", projectsetting->target[0] == 'L' ? "ON":"OFF");
    fprintf(file, "option(${PROJECT_NAME}_BUILD_SHARED \"Build the project as a shared library.\" %s)\n", projectsetting->target[0] == 'S' ? "ON":"OFF");
    fprintf(file, "option(${PROJECT_NAME}_BUILD_HEADERS_ONLY \"Build the project as a header-only library.\" %s)\n", projectsetting->target[0] == 'H' ? "ON":"OFF");
    fprintf(file, "\n");
    fprintf(file, "option(${PROJECT_NAME}_WARNINGS_AS_ERRORS \"Treat compiler warnings as errors.\" ON)\n");
    fprintf(file, "\n");
    fprintf(file, "option(${PROJECT_NAME}_ENABLE_LTO \"Enable Interprocedural Optimization, aka Link Time Optimization (LTO).\" ON)\n");
    fprintf(file, "\n");
    fprintf(file, "#\n");
    fprintf(file, "# Static analyzers\n");
    fprintf(file, "#\n");
    fprintf(file, "option(${PROJECT_NAME}_ENABLE_CLANG_TIDY \"Enable static analysis with Clang-Tidy.\" %s)\n", projectsetting->clangtidy[0] == 'Y' ? "ON":"OFF");
    fprintf(file, "option(${PROJECT_NAME}_ENABLE_CPPCHECK \"Enable static analysis with Cppcheck.\" %s)\n", projectsetting->clangtidy[0] == 'Y' ? "ON":"OFF");
    fprintf(file, "\n");
    fprintf(file, "include(CompilerWarnings)\n");
    fprintf(file, "include(InterproceduralOptimization)\n");
    fprintf(file, "include(StaticAnalyzer)\n");
    fprintf(file, "\n");
    fprintf(file, "enable_lto()\n");
    fprintf(file, "enable_cppcheck()\n");
    fprintf(file, "enable_clang_tidy()\n");
    fprintf(file, "\n");
    fprintf(file, "#\n");
    fprintf(file, "# Generate compile_commands.json to make it easier to work with clang based tools\n");
    fprintf(file, "#\n");
    fprintf(file, "set(CMAKE_EXPORT_COMPILE_COMMANDS ON)\n");
    fprintf(file, "\n");
    fprintf(file, "#\n");
    fprintf(file, "# Configure Target\n");
    fprintf(file, "#\n");
    fprintf(file, "include(SourcesAndHeaders)\n");
    fprintf(file, "if(${PROJECT_NAME}_BUILD_HEADERS_ONLY)\n");
    fprintf(file, "    add_library(${PROJECT_NAME} INTERFACE)\n");

    if (strncmp(projectsetting->language, "CXX", 3) == 0) {
        fprintf(file, "    target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_%s)\n", projectsetting->standard);
    }
    else {
        fprintf(file, "    target_compile_features(${PROJECT_NAME} INTERFACE c_std_%s)\n", projectsetting->standard);
    }

    fprintf(file, "else()\n");
    fprintf(file, "    if(${PROJECT_NAME}_BUILD_EXECUTABLE)\n");
    fprintf(file, "        add_executable(${PROJECT_NAME} ${headers} ${sources})\n");
    fprintf(file, "    elseif(${PROJECT_NAME}_BUILD_STATIC)\n");
    fprintf(file, "        add_library(${PROJECT_NAME} STATIC ${headers} ${sources})\n");
    fprintf(file, "    elseif(${PROJECT_NAME}_BUILD_SHARED)\n");
    fprintf(file, "        add_library(${PROJECT_NAME} SHARED ${headers} ${sources})\n");
    fprintf(file, "    endif()\n");
    fprintf(file, "\n");

    if (strncmp(projectsetting->language, "CXX", 3) == 0) {
        fprintf(file, "    target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_%s)\n", projectsetting->standard);
    }
    else {
        fprintf(file, "    target_compile_features(${PROJECT_NAME} PUBLIC c_std_%s)\n", projectsetting->standard);
    }

    fprintf(file, "endif()\n");
    fprintf(file, "set_project_warnings(${PROJECT_NAME} ${PROJECT_NAME}_WARNINGS_AS_ERRORS ${PROJECT_NAME}_BUILD_HEADERS_ONLY)\n");
    fprintf(file, "\n");
    fprintf(file, "#\n");
    fprintf(file, "# Enhance error reporting and compiler messages\n");
    fprintf(file, "#\n");
    fprintf(file, "if(CMAKE_%s_COMPILER_ID MATCHES \".*Clang\")\n", projectsetting->language);
    fprintf(file, "    target_compile_options(${PROJECT_NAME} PUBLIC -fcolor-diagnostics)\n");
    fprintf(file, "elseif(CMAKE_%s_COMPILER_ID STREQUAL \"GNU\")\n", projectsetting->language);
    fprintf(file, "    target_compile_options(${PROJECT_NAME} PUBLIC -fdiagnostics-color=always)\n");
    fprintf(file, "else()\n");
    fprintf(file, "    message(STATUS \"No colored compiler diagnostic set for '${CMAKE_%s_COMPILER_ID}' compiler.\")\n", projectsetting->language);
    fprintf(file, "endif()\n");
    fprintf(file, "\n");
    fprintf(file, "message(STATUS \"Finished setting up target.\")\n");
    fprintf(file, "\n");
    fprintf(file, "#\n");
    fprintf(file, "# Configure Include Directories\n");
    fprintf(file, "#\n");
    fprintf(file, "if(${PROJECT_NAME}_BUILD_HEADERS_ONLY)\n");
    fprintf(file, "    target_include_directories(\n");
    fprintf(file, "        ${PROJECT_NAME}\n");
    fprintf(file, "        INTERFACE\n");
    fprintf(file, "            $<INSTALL_INTERFACE:include>\n");
    fprintf(file, "            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>\n");
    fprintf(file, "    )\n");
    fprintf(file, "else()\n");
    fprintf(file, "    target_include_directories(\n");
    fprintf(file, "        ${PROJECT_NAME}\n");
    fprintf(file, "        PUBLIC\n");
    fprintf(file, "            $<INSTALL_INTERFACE:include>\n");
    fprintf(file, "            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>\n");
    fprintf(file, "        PRIVATE\n");
    fprintf(file, "            ${CMAKE_CURRENT_SOURCE_DIR}/src\n");
    fprintf(file, "    )\n");
    fprintf(file, "endif()\n");
    fprintf(file, "message(STATUS \"Finished setting up include directories.\")\n");
    fprintf(file, "\n");
    fprintf(file, "#\n");
    fprintf(file, "# Provide alias to library for\n");
    fprintf(file, "#\n");
    fprintf(file, "if(${PROJECT_NAME}_BUILD_EXECUTABLE)\n");
    fprintf(file, "    add_executable(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})\n");
    fprintf(file, "else()\n");
    fprintf(file, "    add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})\n");
    fprintf(file, "endif()\n");
    fprintf(file, "\n");
    fprintf(file, "#\n");
    fprintf(file, "# Install Package\n");
    fprintf(file, "#\n");
    fprintf(file, "include(GNUInstallDirs)\n");
    fprintf(file, "install(\n");
    fprintf(file, "    TARGETS\n");
    fprintf(file, "        ${PROJECT_NAME}\n");
    fprintf(file, "    EXPORT\n");
    fprintf(file, "        ${PROJECT_NAME}Targets\n");
    fprintf(file, "    LIBRARY DESTINATION\n");
    fprintf(file, "        ${CMAKE_INSTALL_LIBDIR}\n");
    fprintf(file, "    RUNTIME DESTINATION\n");
    fprintf(file, "        ${CMAKE_INSTALL_BINDIR}\n");
    fprintf(file, "    ARCHIVE DESTINATION\n");
    fprintf(file, "        ${CMAKE_INSTALL_LIBDIR}\n");
    fprintf(file, "    INCLUDES DESTINATION\n");
    fprintf(file, "        include\n");
    fprintf(file, "    PUBLIC_HEADER DESTINATION\n");
    fprintf(file, "        include\n");
    fprintf(file, ")\n");
    fprintf(file, "\n");
    fprintf(file, "if(NOT ${PROJECT_NAME}_BUILD_EXECUTABLE)\n");
    fprintf(file, "    install(\n");
    fprintf(file, "        EXPORT\n");
    fprintf(file, "            ${PROJECT_NAME}Targets\n");
    fprintf(file, "        FILE\n");
    fprintf(file, "            ${PROJECT_NAME}Targets.cmake\n");
    fprintf(file, "        NAMESPACE\n");
    fprintf(file, "            ${PROJECT_NAME}::\n");
    fprintf(file, "        DESTINATION\n");
    fprintf(file, "            ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}\n");
    fprintf(file, "    )\n");
    fprintf(file, "endif()\n");
    fprintf(file, "\n");
    fprintf(file, "#\n");
    fprintf(file, "# Add version header\n");
    fprintf(file, "#\n");

    if (strncmp(projectsetting->language, "CXX", 3) == 0) {
        fprintf(file, "configure_file(\n");
        fprintf(file, "  ${CMAKE_CURRENT_LIST_DIR}/cmake/version.hpp.in\n");
        fprintf(file, "  ${PROJECT_SOURCE_DIR}/include/version.hpp\n");
        fprintf(file, "  @ONLY\n");
        fprintf(file, ")\n");
    }
    else {
        fprintf(file, "configure_file(\n");
        fprintf(file, "  ${CMAKE_CURRENT_LIST_DIR}/cmake/version.h.in\n");
        fprintf(file, "  ${PROJECT_SOURCE_DIR}/include/version.h\n");
        fprintf(file, "  @ONLY\n");
        fprintf(file, ")\n");
    }

    fprintf(file, "\n");
    fprintf(file, "#\n");
    fprintf(file, "# Install the `include` directory\n");
    fprintf(file, "#\n");
    fprintf(file, "if(NOT ${PROJECT_NAME}_BUILD_EXECUTABLE)\n");
    fprintf(file, "    install(\n");
    fprintf(file, "      DIRECTORY\n");
    fprintf(file, "        include/${PROJECT_NAME}\n");
    fprintf(file, "      DESTINATION\n");
    fprintf(file, "        include\n");
    fprintf(file, ")\n");
    fprintf(file, "endif()\n");

    fflush(file);
    fclose(file);

    return NULL;
}
