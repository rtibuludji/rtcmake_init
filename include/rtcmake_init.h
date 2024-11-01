/*
 *
 * 
 */
#ifndef __RTCMAKE_INIT_H__
#define __RTCMAKE_INIT_H__

#include <stddef.h>
#include <stdint.h>

#include "rtcmake_prompt.h"

#define LENGTH_ID 32

typedef struct rtcmake_version_t {
    char       id[LENGTH_ID];
    uint32_t   major;
    uint32_t   minor;
    uint32_t   patch;
} rtcmake_version_t;

typedef struct rtcmake_projectsetting_t {
    char       name[32];
    char       version[16];
    char       description[64];
    char       compiler[16];
    char       generator[16];
    char       language[8];
    char       standard[8];
    char       target[8];
    char       cppcheck[2];
    char       clangtidy[2];
} rtcmake_projectsetting_t;

RTPROMPT_STATUS rtcmake_projectsetting(rtcmake_projectsetting_t * const projectsetting, const char *projectname, RTPROMPT_ERRNO *errcode);

int rtcmake_create_directory();
const char* rtcmake_create_cmakefiles(const rtcmake_projectsetting_t * const projectsetting);
const char* rtcmake_create_miscfiles();
const char* rtcmake_create_cmakepreset_json(const rtcmake_projectsetting_t * const projectsetting, const rtcmake_version_t *cmake_version);
const char* rtcmake_create_cmakelists_txt(const rtcmake_projectsetting_t * const projectsetting, const rtcmake_version_t *cmake_version);

#endif