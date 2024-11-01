/*
 *
 * 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <assert.h>

#include "rtcmake_init.h"

static char errstr[256];

int rtcmake_create_directory() 
{
    assert(projectsetting != NULL);

    char        buf[128] = {0};
    struct stat st       = {0};


    snprintf(buf, sizeof(buf), "./cmake");
    if (stat(buf, &st) == -1) {
        if (mkdir(buf, 0755) == -1) {
            return -1;
        }
    }

    snprintf(buf, sizeof(buf), "./include");
    if (stat(buf, &st) == -1) {
        if (mkdir(buf, 0755) == -1) {
            return -1;
        }
    }

    snprintf(buf, sizeof(buf), "./src");
    if (stat(buf, &st) == -1) {
        if (mkdir(buf, 0755) == -1) {
            return -1;
        }
    }

    snprintf(buf, sizeof(buf), "./test");
    if (stat(buf, &st) == -1) {
        if (mkdir(buf, 0755) == -1) {
            return -1;
        }
    }

    return 0;
}

static int CompilerWarnings_cmake__(const rtcmake_projectsetting_t * const projectsetting) 
{
    FILE       *file;
    char        buf[128] = {0};

    // CompilerWarnings.cmake
    snprintf(buf, sizeof(buf), "./cmake/CompilerWarnings.cmake");
    file = fopen(buf, "w"); 
    if (file == NULL) {
        snprintf(errstr, sizeof(errstr), "invalid generate file %s", buf);
        return -1;
    }

    fprintf(file, "#\n");
    fprintf(file, "# Auto Generated by rtcmake_init\n");
    fprintf(file, "#\n");
    fprintf(file, "# from: https://github.com/cpp-best-practices/cppbestpractices/blob/master/02-Use_the_Tools_Available.md\n");
    fprintf(file, "#\n");
    fprintf(file, "function(set_project_warnings project_name WARNINGS_AS_ERRORS BUILD_HEADERS_ONLY)\n");
    fprintf(file, "    set(MSVC_WARNINGS\n");
    fprintf(file, "        /W4     # Baseline reasonable warnings\n");
    fprintf(file, "        /w14242 # 'identifier': conversion from 'type1' to 'type1', possible loss\n");
    fprintf(file, "                # of data\n");
    fprintf(file, "        /w14254 # 'operator': conversion from 'type1:field_bits' to\n");
    fprintf(file, "                # 'type2:field_bits', possible loss of data\n");
    fprintf(file, "        /w14263 # 'function': member function does not override any base class\n");
    fprintf(file, "                # virtual member function\n");
    fprintf(file, "        /w14265 # 'classname': class has virtual functions, but destructor is not\n");
    fprintf(file, "                # virtual instances of this class may not be destructed correctly\n");
    fprintf(file, "        /w14287 # 'operator': unsigned/negative constant mismatch\n");
    fprintf(file, "        /we4289 # nonstandard extension used: 'variable': loop control variable\n");
    fprintf(file, "                # declared in the for-loop is used outside the for-loop scope\n");
    fprintf(file, "        /w14296 # 'operator': expression is always 'boolean_value'\n");
    fprintf(file, "        /w14311 # 'variable': pointer truncation from 'type1' to 'type2'\n");
    fprintf(file, "        /w14545 # expression before comma evaluates to a function which is missing\n");
    fprintf(file, "                # an argument list\n");
    fprintf(file, "        /w14546 # function call before comma missing argument list\n");
    fprintf(file, "        /w14547 # 'operator': operator before comma has no effect; expected\n");
    fprintf(file, "                # operator with side-effect\n");
    fprintf(file, "        /w14549 # 'operator': operator before comma has no effect; did you intend\n");
    fprintf(file, "                # 'operator'?\n");
    fprintf(file, "        /w14555 # expression has no effect; expected expression with side- effect\n");
    fprintf(file, "        /w14619 # pragma warning: there is no warning number 'number'\n");
    fprintf(file, "        /w14640 # Enable warning on thread un-safe static member initialization\n");
    fprintf(file, "        /w14826 # Conversion from 'type1' to 'type_2' is sign-extended. This may\n");
    fprintf(file, "                # cause unexpected runtime behavior.\n");
    fprintf(file, "        /w14905 # wide string literal cast to 'LPSTR'\n");
    fprintf(file, "        /w14906 # string literal cast to 'LPWSTR'\n");
    fprintf(file, "        /w14928 # illegal copy-initialization; more than one user-defined\n");
    fprintf(file, "                # conversion has been implicitly applied\n");
    fprintf(file, "        /permissive- # standards conformance mode for MSVC compiler.\n");
    fprintf(file, "    )\n");
    fprintf(file, "#\n");
    fprintf(file, "    set(CLANG_WARNINGS\n");
    fprintf(file, "        -Wall\n");
    fprintf(file, "        -Wextra  # reasonable and standard#\n");
    fprintf(file, "        -Wshadow # warn the user if a variable declaration shadows one from a\n");
    fprintf(file, "                 # parent context\n");
    fprintf(file, "        -Wcast-align     # warn for potential performance problem casts\n");
    fprintf(file, "        -Wunused         # warn on anything being unused\n");
    if (strncmp(projectsetting->language, "CXX", 3) == 0) {
        fprintf(file, "        -Woverloaded-virtual # warn if you overload (not override) a virtual function\n");
    }
    fprintf(file, "        -Wpedantic   # warn if non-standard C++ is used\n");
    fprintf(file, "        -Wconversion # warn on type conversions that may lose data\n");
    fprintf(file, "        -Wsign-conversion  # warn on sign conversions\n");
    fprintf(file, "        -Wnull-dereference # warn if a null dereference is detected\n");
    fprintf(file, "        -Wdouble-promotion # warn if float is implicit promoted to double\n");
    fprintf(file, "        -Wformat=2 # warn on security issues around functions that format output\n");
    fprintf(file, "                   # (ie printf)\n");
    if (strncmp(projectsetting->language, "CXX", 3) == 0) {
        fprintf(file, "        -Wimplicit-fallthrough # warn on statements that fallthrough without an explicit annotation\n");
    }
    fprintf(file, "    )\n");
    fprintf(file, "\n");
    fprintf(file, "    if (WARNINGS_AS_ERRORS)\n");
    fprintf(file, "        set(CLANG_WARNINGS ${CLANG_WARNINGS} -Werror)\n");
    fprintf(file, "        set(MSVC_WARNINGS ${MSVC_WARNINGS} /WX)\n");
    fprintf(file, "    endif()\n");
    fprintf(file, "\n");
    fprintf(file, "    set(GCC_WARNINGS\n");
    fprintf(file, "        ${CLANG_WARNINGS}\n");
    fprintf(file, "        -Wmisleading-indentation # warn if indentation implies blocks where blocks\n");
    fprintf(file, "                                 # do not exist\n");
    fprintf(file, "        -Wduplicated-cond        # warn if if / else chain has duplicated conditions\n");
    fprintf(file, "        -Wduplicated-branches    # warn if if / else branches have duplicated code\n");
    fprintf(file, "        -Wlogical-op             # warn about logical operations being used where bitwise were\n");
    fprintf(file, "                                 # probably wanted\n");
    fprintf(file, "         -Wuseless-cast          # warn if you perform a cast to the same type\n");
    fprintf(file, "    )\n");
    fprintf(file, "\n");
    fprintf(file, "    if(MSVC)\n");
    fprintf(file, "        set(PROJECT_WARNINGS ${MSVC_WARNINGS})\n");
    fprintf(file, "    elseif(CMAKE_%s_COMPILER_ID MATCHES \".*Clang\")\n", projectsetting->language);
    fprintf(file, "        set(PROJECT_WARNINGS ${CLANG_WARNINGS})\n");
    fprintf(file, "    elseif(CMAKE_%s_COMPILER_ID STREQUAL \"GNU\")\n", projectsetting->language);
    fprintf(file, "        set(PROJECT_WARNINGS ${GCC_WARNINGS})\n");
    fprintf(file, "    else()\n");
    fprintf(file, "        message(AUTHOR_WARNING \"No compiler warnings set for '${CMAKE_C_COMPILER_ID}' compiler.\")\n");
    fprintf(file, "    endif()\n");
    fprintf(file, "\n");
    fprintf(file, "    if(BUILD_HEADERS_ONLY)\n");
    fprintf(file, "        target_compile_options(${project_name} INTERFACE $<$<COMPILE_LANGUAGE:%s>:${PROJECT_WARNINGS}>)\n", projectsetting->language);
    fprintf(file, "    else()\n");
    fprintf(file, "        target_compile_options(${project_name} PUBLIC $<$<COMPILE_LANGUAGE:%s>:${PROJECT_WARNINGS}>)\n", projectsetting->language);
    fprintf(file, "    endif()\n");
    fprintf(file, "\n");
    fprintf(file, "    if(NOT TARGET ${project_name})\n");
    fprintf(file, "        message(AUTHOR_WARNING \"${project_name} is not a target, thus no compiler warning were added.\")\n");
    fprintf(file, "    endif()\n");
    fprintf(file, "endfunction()\n");

    fflush(file);
    fclose(file);

    return 0;
}

static int InterproceduralOptimization_cmake__() 
{
    assert(projectsetting != NULL);

    FILE       *file;
    char        buf[128] = {0};

    // InterproceduralOptimization.cmake
    snprintf(buf, sizeof(buf), "./cmake/InterproceduralOptimization.cmake");
    file = fopen(buf, "w"); 
    if (file == NULL) {
        snprintf(errstr, sizeof(errstr), "invalid generate file %s", buf);
        return -1;
    }

    fprintf(file, "#\n");
    fprintf(file, "# Auto Generated by rtcmake_init\n");
    fprintf(file, "#\n");
    fprintf(file, "macro(enable_lto)\n");
    fprintf(file, "    if(${PROJECT_NAME}_ENABLE_LTO)\n");
    fprintf(file, "        include(CheckIPOSupported)\n");
    fprintf(file, "        check_ipo_supported(RESULT result OUTPUT output)\n");
    fprintf(file, "        if(result)\n");
    fprintf(file, "            set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)\n");
    fprintf(file, "        else()\n");
    fprintf(file, "            message(FATAL \"IPO is not supported: ${output}.\")\n");
    fprintf(file, "        endif()\n");
    fprintf(file, "    endif()\n");
    fprintf(file, "endmacro()\n");

    fflush(file);
    fclose(file);

    return 0;
}

static int SourcesAndHeaders_cmake__() 
{
    assert(projectsetting != NULL);

    FILE       *file;
    char        buf[128] = {0};

    // SourcesAndHeaders.cmake
    snprintf(buf, sizeof(buf), "./cmake/SourcesAndHeaders.cmake");
    file = fopen(buf, "w"); 
    if (file == NULL) {
        snprintf(errstr, sizeof(errstr), "invalid generate file %s", buf);
        return -1;
    }

    fprintf(file, "#\n");
    fprintf(file, "# Auto Generated by rtcmake_init\n");
    fprintf(file, "#\n");
    fprintf(file, "set(sources\n"); 
    fprintf(file, ")\n");
    fprintf(file, "\n");
    fprintf(file, "set(headers \n");
    fprintf(file, ")\n");

    fflush(file);
    fclose(file);

    return 0;
}

static int StaticAnalyzer_cmake__(const rtcmake_projectsetting_t * const projectsetting) 
{
    assert(projectsetting != NULL);

    FILE       *file;
    char        buf[128] = {0};

    // StaticAnalyzer.cmake
    snprintf(buf, sizeof(buf), "./cmake/StaticAnalyzer.cmake");
    file = fopen(buf, "w"); 
    if (file == NULL) {
        snprintf(errstr, sizeof(errstr), "invalid generate file %s", buf);
        return -1;
    }

    fprintf(file, "#\n");
    fprintf(file, "# Auto Generated by rtcmake_init\n");
    fprintf(file, "#\n");
    fprintf(file, "macro(enable_cppcheck)\n"); 
    fprintf(file, "    if(${PROJECT_NAME}_ENABLE_CPPCHECK)\n");
    fprintf(file, "        find_program(CPPCHECK cppcheck)\n");
    fprintf(file, "        if(CPPCHECK)\n");
    fprintf(file, "            set(CMAKE_C_CPPCHECK ${CPPCHECK}\n");
    fprintf(file, "                --enable=style,performance,warning,portability\n"); 
    fprintf(file, "                --suppress=missingIncludeSystem\n"); 
    fprintf(file, "                --suppress=checkersReport\n"); 
    fprintf(file, "                --check-level=exhaustive\n");
    fprintf(file, "                --inline-suppr\n"); 
    fprintf(file, "                --inconclusive\n");

    if (strncmp(projectsetting->language, "CXX", 3) == 0) {
        fprintf(file, "                --std=c++%s\n", projectsetting->standard);
    }
    else {
        fprintf(file, "                --std=c%s\n", projectsetting->standard);
    }

    fprintf(file, "                --error-exitcode=2\n");
    fprintf(file, "            )\n");
    fprintf(file, "            message(STATUS \"Finished setting up cppcheck.\")\n");
    fprintf(file, "        else()\n");
    fprintf(file, "            message(FATAL_ERROR \"Cppcheck requested but executable not found.\")\n");
    fprintf(file, "        endif()\n");
    fprintf(file, "    endif()\n");
    fprintf(file, "endmacro()\n");
    fprintf(file, "\n");
    fprintf(file, "macro(enable_clang_tidy)\n");
    fprintf(file, "    if(${PROJECT_NAME}_ENABLE_CLANG_TIDY)\n");
    fprintf(file, "        find_program(CLANGTIDY clang-tidy)\n");
    fprintf(file, "        if(CLANGTIDY)\n");
    fprintf(file, "            set(CMAKE_C_CLANG_TIDY ${CLANGTIDY}\n");
    fprintf(file, "                -extra-arg=-Wno-unknown-warning-option\n");
    fprintf(file, "                -extra-arg=-Wno-ignored-optimization-argumen\n");
    fprintf(file, "                -extra-arg=-Wno-unused-command-line-argument\n");
    fprintf(file, "                -p\n");

    if (strncmp(projectsetting->language, "CXX", 3) == 0) {
        fprintf(file, "                -extra-arg=-std=c++%s\n", projectsetting->standard);
    }
    else {
        fprintf(file, "                -extra-arg=-std=c%s\n", projectsetting->standard);
    }
        
    fprintf(file, "            )\n");
    fprintf(file, "            message(STATUS \"Finished setting up clang-tidy\")\n");
    fprintf(file, "        else()\n");
    fprintf(file, "            message(FATAL_ERROR \"clang-tidy requested but executable not found.\")\n");
    fprintf(file, "        endif()\n");
    fprintf(file, "    endif()\n");
    fprintf(file, "endmacro()\n");

    fflush(file);
    fclose(file);

    return 0;
}

static int version_in__(const rtcmake_projectsetting_t * const projectsetting) 
{
    assert(projectsetting != NULL);

    FILE       *file;
    char        buf[128] = {0};

    // version.h.in or version.hpp.in
    if (strncmp(projectsetting->language, "CXX", 3) == 0) {
        snprintf(buf, sizeof(buf), "./cmake/version.hpp.in");
    }
    else {
        snprintf(buf, sizeof(buf), "./cmake/version.h.in");
    }
    file = fopen(buf, "w"); 
    if (file == NULL) {
        snprintf(errstr, sizeof(errstr), "invalid generate file %s", buf);
        return -1;
    }

    fprintf(file, "/*\n");
    fprintf(file, " * Auto generated by rtcmake_init\n");
    fprintf(file, " */\n");
    fprintf(file, "#ifndef @PROJECT_NAME_UPPERCASE@_VERSION_H_\n");
    fprintf(file, "#define @PROJECT_NAME_UPPERCASE@_VERSION_H_\n");
    fprintf(file, "\n");
    fprintf(file, "#define @PROJECT_NAME_UPPERCASE@_VERSION \"@PROJECT_VERSION@\"\n");
    fprintf(file, "\n");
    fprintf(file, "#define @PROJECT_NAME_UPPERCASE@_MAJOR_VERSION @PROJECT_VERSION_MAJOR@\n");
    fprintf(file, "#define @PROJECT_NAME_UPPERCASE@_MINOR_VERSION @PROJECT_VERSION_MINOR@\n");
    fprintf(file, "#define @PROJECT_NAME_UPPERCASE@_PATCH_VERSION @PROJECT_VERSION_PATCH@\n");
    fprintf(file, "\n");
    fprintf(file, "#endif // @PROJECT_NAME_UPPERCASE@_VERSION_H_\n");

    fflush(file);
    fclose(file);

    return 0;
}

const char *rtcmake_create_cmakefiles(const rtcmake_projectsetting_t * const projectsetting) 
{
    if (CompilerWarnings_cmake__(projectsetting) != 0) {
        return errstr;
    }

    if (InterproceduralOptimization_cmake__() != 0) {
        return errstr;
    }

    if (SourcesAndHeaders_cmake__() != 0) {
        return errstr;
    }

    if (StaticAnalyzer_cmake__(projectsetting) != 0) {
        return errstr;
    }

    if (version_in__(projectsetting) != 0) {
        return errstr;
    }
    
    return NULL;
}

static int clang_tidy__() 
{
    assert(projectsetting != NULL);

    FILE       *file;
    char        buf[128] = {0};

    // .clang-tidy
    snprintf(buf, sizeof(buf), "./.clang-tidy");
    file = fopen(buf, "w"); 
    if (file == NULL) {
        snprintf(errstr, sizeof(errstr), "invalid generate file %s", buf);
        return -1;
    }

    fprintf(file, "---\n");
    fprintf(file, "Checks: '\n");
    fprintf(file, "         clang-diagnostic-*,\n");
    fprintf(file, "         clang-analyzer-*,\n");
    fprintf(file, "         readability-*,\n");
    fprintf(file, "         modernize-*,\n");
    fprintf(file, "         bugprone-*,\n");
    fprintf(file, "         misc-*,\n");
    fprintf(file, "         google-runtime-int,\n");
    fprintf(file, "         llvm-header-guard,\n");
    fprintf(file, "         fuchsia-restrict-system-includes,\n");
    fprintf(file, "         -clang-analyzer-valist.Uninitialized,\n");
    fprintf(file, "         -clang-analyzer-security.insecureAPI.rand,\n");
    fprintf(file, "         -clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling,\n");
    fprintf(file, "         -clang-analyzer-alpha.*,\n");
    fprintf(file, "         -readability-magic-numbers,\n");
    fprintf(file, "         -readability-non-const-parameter,\n");
    fprintf(file, "         -readability-avoid-const-params-in-decls,\n");
    fprintf(file, "         -readability-else-after-return,\n");
    fprintf(file, "         -readability-isolate-declaration,\n");
    fprintf(file, "         -readability-uppercase-literal-suffix,\n");
    fprintf(file, "         -readability-function-cognitive-complexity,\n");
    fprintf(file, "         -readability-identifier-length,\n");
    fprintf(file, "         -bugprone-sizeof-expression,\n");
    fprintf(file, "         -bugprone-easily-swappable-parameters,\n");
    fprintf(file, "         -misc-no-recursion,\n");
    fprintf(file, "        '\n");
    fprintf(file, "CheckOptions: [{ key: misc-non-private-member-variables-in-classes, value: IgnoreClassesWithAllMemberVariablesBeingPublic }]\n");
    fprintf(file, "WarningsAsErrors: '*'\n");
    fprintf(file, "HeaderFilterRegex: ''\n");
    fprintf(file, "FormatStyle: none\n");
    fprintf(file, "\n");

    fflush(file);
    fclose(file);

    return 0;
}

const char* rtcmake_create_miscfiles()
{
    if (clang_tidy__() != 0) {
        return errstr;
    }

    return NULL;
}
