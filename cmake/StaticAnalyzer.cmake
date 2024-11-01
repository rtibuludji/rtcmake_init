
macro(enable_cppcheck) 
    if(${PROJECT_NAME}_ENABLE_CPPCHECK)
        find_program(CPPCHECK cppcheck)
        if(CPPCHECK)
            set(CMAKE_C_CPPCHECK ${CPPCHECK}
                --enable=style,performance,warning,portability 
                --suppress=missingIncludeSystem 
                --suppress=checkersReport 
                --check-level=exhaustive
                --inline-suppr 
                --inconclusive
                --std=c17
                --error-exitcode=2
            )
            message(STATUS "Finished setting up cppcheck.")
        else()
            message(FATAL_ERROR "Cppcheck requested but executable not found.")
        endif()
    endif()
endmacro()

macro(enable_clang_tidy)
    if(${PROJECT_NAME}_ENABLE_CLANG_TIDY)
        find_program(CLANGTIDY clang-tidy)
        if(CLANGTIDY)
            set(CMAKE_C_CLANG_TIDY ${CLANGTIDY}
                -extra-arg=-Wno-unknown-warning-option
                -extra-arg=-Wno-ignored-optimization-argumen
                -extra-arg=-Wno-unused-command-line-argument
                -p
                -extra-arg=-std=c17
            )
            message(STATUS "Finished setting up clang-tidy")
        else()
            message(FATAL_ERROR "clang-tidy requested but executable not found.")
        endif()
    endif()
endmacro()