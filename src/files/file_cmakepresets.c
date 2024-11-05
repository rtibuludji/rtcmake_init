/*
 *
 * 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "rtcmake_init.h"

static char compiler_id[20];

int getcompiler__(const char *compilers, const char *language) 
{
    char  compilers_str[16];

    const char *language_1 = NULL;
    const char *language_2 = NULL;

    assert(compilers != NULL);

    memset(compiler_id,   0x00, sizeof(compiler_id));
    memset(compilers_str, 0x00, sizeof(compilers_str));
    memcpy(compilers_str, compilers, strlen(compilers) > sizeof(compilers_str) ? 15 : strlen(compilers));

    language_1 = &compilers_str[0];
    for (int index = 1; index < strlen(compilers_str) - 1; ++index) {
        if (compilers_str[index] == ',') {
            compilers_str[index] = 0x00;
            language_2           = &compilers_str[index + 1];
            break;
        }
    }

    if (language_2 == NULL) {
        language_2 = language_1;
    }

    if (strncmp(language, "CXX", 3) == 0) {
        snprintf(compiler_id, sizeof(compiler_id), "%s", language_2);
    }
    else {
        snprintf(compiler_id, sizeof(compiler_id), "%s", language_1);
    }

    return 0;
}

static char errstr[256];

const char* rtcmake_create_cmakepreset_json(const rtcmake_projectsetting_t * const projectsetting, const rtcmake_version_t *cmake_version) 
{
    assert(projectsetting != NULL);

    FILE       *file;
    char        buf[128] = {0};

    getcompiler__(projectsetting->compiler, projectsetting->language);

    // CMakePresets.json
    snprintf(buf, sizeof(buf), "./CMakePresets.json");
    file = fopen(buf, "w"); 
    if (file == NULL) {
        snprintf(errstr, sizeof(errstr), "invalid generate file %s", buf);
        return errstr;
    }
    
    fprintf(file, "{\n");
    fprintf(file, "    \"version\": 8,\n");
    fprintf(file, "    \"cmakeMinimumRequired\": {\n");
    fprintf(file, "        \"major\": %u,\n", cmake_version->major);
    fprintf(file, "        \"minor\": %u,\n", cmake_version->minor);
    fprintf(file, "        \"patch\": %u\n",  cmake_version->patch);
    fprintf(file, "    },\n");
    fprintf(file, "    \"configurePresets\": [\n");
    fprintf(file, "        {\n");
    fprintf(file, "            \"name\": \"%s\",\n", projectsetting->name);
    fprintf(file, "            \"hidden\": true,\n");
    fprintf(file, "            \"generator\": \"%s\",\n", projectsetting->generator);
    fprintf(file, "            \"binaryDir\": \"${sourceDir}/.target/${presetName}/build\",\n");
    fprintf(file, "            \"installDir\": \"${sourceDir}/.target/${presetName}/out\",\n");
    fprintf(file, "            \"cacheVariables\": {\n");
    fprintf(file, "                \"CMAKE_%s_COMPILER\": \"%s\",\n", projectsetting->language, compiler_id);
    fprintf(file, "                \"CMAKE_%s_STANDARD\": \"%s\",\n", projectsetting->language, projectsetting->standard);
    fprintf(file, "                \"CMAKE_%s_STANDARD_REQUIRED\": \"YES\",\n", projectsetting->language);
    fprintf(file, "                \"CMAKE_%s_EXTENSIONS\": \"OFF\"\n", projectsetting->language);
    fprintf(file, "            }\n");
    fprintf(file, "        },\n");
    fprintf(file, "        {\n");
    fprintf(file, "            \"name\": \"debug\",\n");
    fprintf(file, "            \"displayName\": \"Debug\",\n");
    fprintf(file, "            \"inherits\": \"%s\",\n", projectsetting->name);
    fprintf(file, "            \"cacheVariables\": {\n");
    fprintf(file, "                \"CMAKE_BUILD_TYPE\": \"Debug\"\n");
    fprintf(file, "            }\n");
    fprintf(file, "        },\n");
    fprintf(file, "        {\n");
    fprintf(file, "            \"name\": \"release\",\n");
    fprintf(file, "            \"displayName\": \"Release\",\n");
    fprintf(file, "            \"inherits\": \"%s\",\n", projectsetting->name);
    fprintf(file, "            \"cacheVariables\": {\n");
    fprintf(file, "                \"CMAKE_BUILD_TYPE\": \"Release\"\n");
    fprintf(file, "            }\n");
    fprintf(file, "        }\n");
    fprintf(file, "    ],\n");
    fprintf(file, "    \"buildPresets\": [\n");
    fprintf(file, "        {\n");
    fprintf(file, "            \"name\": \"debug\",\n");
    fprintf(file, "            \"targets\": [\"%s\",\"install\"],\n", projectsetting->name);
    fprintf(file, "            \"configurePreset\": \"debug\",\n");
    fprintf(file, "            \"configuration\": \"Debug\"\n");
    fprintf(file, "        },\n");
    fprintf(file, "        {\n");
    fprintf(file, "            \"name\": \"release\",\n");
    fprintf(file, "            \"targets\": [\"%s\",\"install\"],\n", projectsetting->name);
    fprintf(file, "            \"configurePreset\": \"release\",\n");
    fprintf(file, "            \"configuration\": \"Release\"\n");
    fprintf(file, "        }\n");
    fprintf(file, "    ]\n");
    fprintf(file, "}\n");


    fflush(file);
    fclose(file);

    return NULL;
}
