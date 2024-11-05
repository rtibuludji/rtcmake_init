/*
 *
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <assert.h>

#include "version.h"
#include "rtcmake_prompt.h"
#include "rtcmake_util.h"
#include "rtcmake_init.h"

int main(int argc, const char *argv[]) 
{
    const char              *err = NULL;
    const char              *project_name;
    rtcmake_projectsetting_t project_setting;

    rtcmake_version_t       cmake_version;

    memset(&project_setting, 0x00, sizeof(project_setting));
    printf("\U0001F4CD RTCMAKE_INIT v%s is going to generate C or C++ project \U0001F4CD\n", RTCMAKE_INIT_VERSION);
    printf("   press CTRL-Q for abort or terminated\n");

    project_name = NULL;
    if (argc >= 2) {
        project_name = argv[1];
    }

    err = rtcmake_get_cmake_version(&cmake_version);
    if (err != NULL) {
        printf("\U0000274C ERR: %s\n", err);
        return 1;
    }

    printf("\U0001F4D6 CMake version %u.%u.%u\n", cmake_version.major, cmake_version.minor, cmake_version.patch);

    RTPROMPT_ERRNO errcode;
    if (rtprompt_init(&errcode) == STATUS_ERR) {
        printf("\U0000274C ERR: Prompt Library initialize failed\n");
        return 1;
    }

    RTPROMPT_STATUS status = rtcmake_projectsetting(&project_setting, project_name, &errcode);
    if (status == STATUS_ERR) {
        printf("\U0000274C ERR: error on get project settings (%d)\n", errcode);

        rtprompt_finalize();
        return 1;
    }
    else if (status == STATUS_OK) {
        char proceed[6];
        rtprompt_yesno_question_t yesno = {
            .prompt          = "Continue to proceed       ",
            .response        = proceed,
            .response_size   = sizeof(proceed),
            .response_length = 0
        };
        
        status = rtprompt_yesno_question(&yesno, 3, &errcode);
        if (status == STATUS_ERR) {
            printf("\U0000274C ERR: error on confirm to proceed %d\n", errcode);

            rtprompt_finalize();
            return 1;
        }
        else if (status == STATUS_OK) {
            const char *errstr;

            if (rtcmake_create_directory() != 0) {
                printf("\U0000274C  ERR: error on create directory\n");

                rtprompt_finalize();
                return 1;
            }

            errstr = rtcmake_create_cmakefiles(&project_setting); 
            if (errstr != NULL) {
                printf("\U0000274C ERR: %s\n", errstr);

                rtprompt_finalize();
                return 1;
            }

            errstr = rtcmake_create_miscfiles();
            if (errstr != NULL) {
                printf("\U0000274C ERR: %s\n", errstr);

                rtprompt_finalize();
                return 1;
            }

            errstr = rtcmake_create_cmakepreset_json(&project_setting, &cmake_version);
            if (errstr != NULL) {
                printf("\U0000274C ERR: %s\n", errstr);

                rtprompt_finalize();
                return 1;
            }

            errstr = rtcmake_create_cmakelists_txt(&project_setting, &cmake_version);
            if (errstr != NULL) {
                printf("\U0000274C ERR: %s\n", errstr);

                rtprompt_finalize();
                return 1;
            }

            printf("\U00002714  DONE: Init Project Success\n");
        }
    }

    rtprompt_finalize();
    return 0;
}


RTPROMPT_STATUS helper_question (void * const prompt, const void * const parameters, uint8_t identation, RTPROMPT_ERRNO *errcode);
RTPROMPT_STATUS helper_compiler (void * const prompt, const void * const parameters, uint8_t identation, RTPROMPT_ERRNO *errcode);
RTPROMPT_STATUS helper_generator(void * const prompt, const void * const parameters, uint8_t identation, RTPROMPT_ERRNO *errcode);
RTPROMPT_STATUS helper_language (void * const prompt, const void * const parameters, uint8_t identation, RTPROMPT_ERRNO *errcode);
RTPROMPT_STATUS helper_standard (void * const prompt, const void * const parameters, uint8_t identation, RTPROMPT_ERRNO *errcode);
RTPROMPT_STATUS helper_target   (void * const prompt, const void * const parameters, uint8_t identation, RTPROMPT_ERRNO *errcode);
RTPROMPT_STATUS helper_yesno    (void * const prompt, const void * const parameters, uint8_t identation, RTPROMPT_ERRNO *errcode);


RTPROMPT_STATUS rtcmake_projectsetting(rtcmake_projectsetting_t * const projectsetting, const char *projectname, RTPROMPT_ERRNO *errcode)
{
    rtprompt_t const prompts[] = {
        { .prompt.question  = { "\U000025B8 Project name            ",  projectname, "^[a-z][a-z0-9_]*$", projectsetting->name,        sizeof(projectsetting->name),        0 }, (const void * const)projectsetting, 3, helper_question   },
        { .prompt.question  = { "\U000025B8 Project description     ",  NULL,        NULL,                projectsetting->description, sizeof(projectsetting->description), 0 }, (const void * const)projectsetting, 3, helper_question   },
        { .prompt.question  = { "\U000025B8 Project version         ",  NULL,        "^[0-9][0-9_.]*$",   projectsetting->version,     sizeof(projectsetting->version),     0 }, (const void * const)projectsetting, 3, helper_question   },
        { .prompt.selection = { "\U000025B8 Generator               ",  NULL, 0, 0, 0,                    projectsetting->generator,   sizeof(projectsetting->generator),   0 }, (const void * const)projectsetting, 3, helper_generator  },
        { .prompt.selection = { "\U000025B8 Compiler                ",  NULL, 0, 0, 0,                    projectsetting->compiler,    sizeof(projectsetting->compiler),    0 }, (const void * const)projectsetting, 3, helper_compiler   },
        { .prompt.selection = { "\U000025B8 Language                ",  NULL, 0, 0, 0,                    projectsetting->language,    sizeof(projectsetting->language),    0 }, (const void * const)projectsetting, 3, helper_language   },
        { .prompt.selection = { "\U000025B8 Standard                ",  NULL, 0, 0, 0,                    projectsetting->standard,    sizeof(projectsetting->standard),    0 }, (const void * const)projectsetting, 3, helper_standard   },
        { .prompt.selection = { "\U000025B8 Target                  ",  NULL, 0, 0, 0,                    projectsetting->target,      sizeof(projectsetting->target),      0 }, (const void * const)projectsetting, 3, helper_target     },
        { .prompt.yesno     = { "\U000025B8 Enable CPPCHECK         ",                                    projectsetting->cppcheck,    sizeof(projectsetting->cppcheck),    0 }, (const void * const)projectsetting, 3, helper_yesno      },
        { .prompt.yesno     = { "\U000025B8 Enable clang-tidy       ",                                    projectsetting->clangtidy,   sizeof(projectsetting->clangtidy),   0 }, (const void * const)projectsetting, 3, helper_yesno      },
    };

    uint8_t prompt_item = sizeof(prompts) / sizeof(rtprompt_t);

    RTPROMPT_STATUS status = STATUS_OK;
    for (uint8_t idx = 0; idx < prompt_item && status == STATUS_OK; ++idx) {
        status = prompts[idx].prompt_fn((void *const) &prompts[idx].prompt.question, 
                                                       prompts[idx].parameters, 
                                                       prompts[idx].identation, 
                                                       errcode);
    }

    return status;
}

// NOLINTBEGIN(misc-unused-parameters)
RTPROMPT_STATUS helper_question(void * const prompt, const void * const parameters, uint8_t identation, RTPROMPT_ERRNO *errcode)
{
    rtprompt_question_t * const question = (rtprompt_question_t * const) prompt;

    return rtprompt_question(question, identation, errcode);
}
// NOLINTEND(misc-unused-parameters)

// NOLINTBEGIN(misc-unused-parameters)
RTPROMPT_STATUS helper_compiler(void * const prompt, const void * const parameters, uint8_t identation, RTPROMPT_ERRNO *errcode)
{
    rtprompt_selection_t *const selection = (rtprompt_selection_t *const)prompt;

    selection->options_size = 2;
    selection->options = (rtprompt_option_t *)malloc(sizeof(rtprompt_option_t) * selection->options_size); 
    if (selection->options == NULL) {
        selection->options_size = 0;
        RETURN_STATUS(errcode, ERRNO_INVALID_ALLOCATE, STATUS_ERR);
    }

    for (uint8_t idx = 0; idx < selection->options_size; ++idx) {
        RTPROMPT_OPTION_INIT(&selection->options[idx]);
    }

    RTPROMPT_STATUS status = rtcmake_get_compilers(selection, errcode);
    if (status == STATUS_OK) {
        status = rtprompt_selection(selection, identation, errcode);
    }

    for (uint8_t idx = 0; idx < selection->options_size; ++idx) {
        RTPROMPT_OPTION_FINALIZE(&selection->options[idx]);
    }

    free(selection->options);
    selection->options      = NULL;
    selection->options_size = 0;
    selection->options_item = 0;
    
    return status;
}
// NOLINTEND(misc-unused-parameters)

// NOLINTBEGIN(misc-unused-parameters)
RTPROMPT_STATUS helper_generator(void * const prompt, const void * const parameters, uint8_t identation, RTPROMPT_ERRNO *errcode)
{
    rtprompt_selection_t *const selection = (rtprompt_selection_t *const)prompt;

    selection->options_size = 2;
    selection->options = (rtprompt_option_t *)malloc(sizeof(rtprompt_option_t) * selection->options_size); 
    if (selection->options == NULL) {
        selection->options_size = 0;
        RETURN_STATUS(errcode, ERRNO_INVALID_ALLOCATE, STATUS_ERR);
    }

    for (uint8_t idx = 0; idx < selection->options_size; ++idx) {
        RTPROMPT_OPTION_INIT(&selection->options[idx]);
    }

    RTPROMPT_STATUS status = rtcmake_get_generators(selection, errcode);
    if (status == STATUS_OK) {
        status = rtprompt_selection(selection, identation, errcode);
    }

    for (uint8_t idx = 0; idx < selection->options_size; ++idx) {
        RTPROMPT_OPTION_FINALIZE(&selection->options[idx]);
    }

    free(selection->options);
    selection->options      = NULL;
    selection->options_size = 0;
    selection->options_item = 0;
    
    return status;
}
// NOLINTEND(misc-unused-parameters)

// NOLINTBEGIN(misc-unused-parameters)
RTPROMPT_STATUS helper_language(void * const prompt, const void * const parameters, uint8_t identation, RTPROMPT_ERRNO *errcode)
{
    rtprompt_selection_t *const selection = (rtprompt_selection_t *const)prompt;

    rtprompt_option_t languages[] = {
        { "C", "C"},
        { "CXX", "C++"}
    };

    selection->options      = languages;
    selection->options_size = 2;
    selection->options_item = 2;

    RTPROMPT_STATUS status = rtprompt_selection(selection, identation, errcode);

    selection->options      = NULL;
    selection->options_size = 0;
    selection->options_item = 0;

    return status;
}
// NOLINTEND(misc-unused-parameters)

RTPROMPT_STATUS helper_standard(void * const prompt, const void * const parameters, uint8_t identation, RTPROMPT_ERRNO *errcode)
{
    rtprompt_selection_t *const            selection      = (rtprompt_selection_t *const)prompt;
    const rtcmake_projectsetting_t * const projectsetting = (const rtcmake_projectsetting_t * const )parameters;

    rtprompt_option_t c_standards[] = {
        { "99", "C99" }, 
        { "11", "C11" },
        { "17", "C17" },
        { "23", "C23" }
    };

    rtprompt_option_t cxx_standards[] = {
        { "98", "C++98" },
        { "11", "C++11" }, 
        { "14", "C++14" },
        { "17", "C++17" },
        { "20", "C++20" },
        { "23", "C++23" }
    };

    if (strncmp(projectsetting->language, "CXX", 3) == 0) {
        selection->options      = cxx_standards;
        selection->options_size = 6;
        selection->options_item = 6;
    }
    else {
        selection->options      = c_standards;
        selection->options_size = 4;
        selection->options_item = 4;
    }

    RTPROMPT_STATUS status = rtprompt_selection(selection, identation, errcode);

    selection->options      = NULL;
    selection->options_size = 0;
    selection->options_item = 0;

    return status;
}

RTPROMPT_STATUS helper_target(void * const prompt, const void * const parameters, uint8_t identation, RTPROMPT_ERRNO *errcode)
{
    rtprompt_selection_t *const            selection      = (rtprompt_selection_t *const)prompt;
    const rtcmake_projectsetting_t * const projectsetting = (const rtcmake_projectsetting_t * const )parameters;

    rtprompt_option_t c_targets[] = {
        { "E", "Executable"},
        { "L", "Static Library"},
        { "S", "Shared Library"}
    };

    rtprompt_option_t cxx_targets[] = {
        { "E", "Executable"},
        { "H", "Header Only Library"},
        { "L", "Static Library"},
        { "S", "Shared Library"}
    };

    if (strncmp(projectsetting->language, "CXX", 3) == 0) {
        selection->options      = cxx_targets;
        selection->options_size = 4;
        selection->options_item = 4;
    }
    else {
        selection->options      = c_targets;
        selection->options_size = 3;
        selection->options_item = 3;
    }

    RTPROMPT_STATUS status = rtprompt_selection(selection, identation, errcode);

    selection->options      = NULL;
    selection->options_size = 0;
    selection->options_item = 0;

    return status;
}

// NOLINTBEGIN(misc-unused-parameters)
RTPROMPT_STATUS helper_yesno(void * const prompt, const void * const parameters, uint8_t identation, RTPROMPT_ERRNO *errcode)
{
    rtprompt_yesno_question_t * const yesno = (rtprompt_yesno_question_t * const)prompt;

    return rtprompt_yesno_question(yesno, identation, errcode);
}
// NOLINTEND(misc-unused-parameters)
