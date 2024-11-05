/*
 *
 * 
 */
#ifndef __RTCMAKE_UTIL_H__
#define __RTCMAKE_UTIL_H__

#include <stddef.h>
#include <stdint.h>

#include "rtcmake_init.h"
#include "rtcmake_prompt.h"

const char *rtcmake_get_cmake_version(rtcmake_version_t *version);
const char *rtcmake_get_gcc_version(rtcmake_version_t *version);
const char *rtcmake_get_clang_version(rtcmake_version_t *version);
const char *rtcmake_get_make_version(rtcmake_version_t *version);
const char *rtcmake_get_ninja_version(rtcmake_version_t *version);

RTPROMPT_STATUS rtcmake_get_compilers(rtprompt_selection_t * const selection, RTPROMPT_ERRNO *errcode);
RTPROMPT_STATUS rtcmake_get_generators(rtprompt_selection_t * const selection, RTPROMPT_ERRNO *errcode);

#endif