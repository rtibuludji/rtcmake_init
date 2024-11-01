#
#
#
macro(enable_lto)
    if(${PROJECT_NAME}_ENABLE_LTO)
        include(CheckIPOSupported)
        check_ipo_supported(RESULT result OUTPUT output)
        if(result)
            set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
        else()
            message(FATAL "IPO is not supported: ${output}.")
        endif()
    endif()
endmacro()
