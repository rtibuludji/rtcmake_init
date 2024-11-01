/*
 *
 * 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <assert.h>

#include "rtcmake_util.h"
#include "rtcmake_init.h"
#include "rtcmake_prompt.h"

enum {
    BUFSIZE = 128
};

enum ERR {
    ERR_ON_PIPE      = 1,
    ERR_ON_FORK      = 2,
    ERR_ON_FDOPEN    = 3,
    ERR_ON_FGETS     = 4,
    ERR_ON_EXECUTE   = 5,
    ERR_ON_CALL      = 6,

    ERR_CMAKE_FORMAT   = 11,
    ERR_GCC_FORMAT     = 12,
    ERR_CLANG_FORMAT   = 13,
    ERR_MAKE_FORMAT    = 14
};

static const char *const err[] = {
    NULL,
    "pipe return error",                            /* 1 */ 
    "fork return error",                            /* 2 */ 
    "fdopen return error",                          /* 3 */
    "fgets return NULL",                            /* 4 */ 
    "execute command failed or command not found",  /* 5 */ 
    "call C lib return error",                      /* 6 */
    NULL,                                           /* 7 */
    NULL,                                           /* 8 */
    NULL,                                           /* 9 */
    NULL,                                           /* 10 */
    "check cmake: unrecognized version format",     /* 11 */
    "check gcc: unrecognized version format",       /* 12 */
    "check clang: unrecognized version format",     /* 13 */
    "check gnu make: unrecognized version format",  /* 14 */
    NULL
};

int rtcmake_run_command(const char *cmd, const char *arg, char *buf, size_t buf_size)
{
    assert(cmd != NULL && arg != NULL && buf != NULL && buf_size != 0);

    int    pipe_fd[2]   = {0};
    int    errcode      = 0;

    if (pipe(pipe_fd) == -1) {
        return ERR_ON_PIPE;
    }

    pid_t pid = fork();
    if (pid == -1) {
        return ERR_ON_FORK;
    }

    if (pid == 0) {
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[1]);

        execlp(cmd, cmd, arg, (char *)NULL);

        exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
    }
    else {
        FILE  *stream_fd = NULL;
        int    status    = 0;

        close(pipe_fd[1]);

        stream_fd = fdopen(pipe_fd[0], "r");
        if (stream_fd == NULL) {
            close(pipe_fd[0]);
            return ERR_ON_FDOPEN;
        }

        if (fgets(buf, (int)buf_size, stream_fd) == NULL) {
            errcode = ERR_ON_FGETS;
        }

        fclose(stream_fd); // NOLINT(cert-err33-c)
        close(pipe_fd[0]);

        if (waitpid(pid, &status, 0) == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            return ERR_ON_EXECUTE;  // Handle process failure
        }
    }

    return errcode;
}

const char *rtcmake_get_cmake_version(rtcmake_version_t *version)
{
    assert(version != NULL);
    
    char   buf[BUFSIZE]          = {0};
    int    errcode               = 0;

    unsigned int version_length  = 0;
    unsigned int start           = 0;
    unsigned int pos             = 0;

    memset(version, 0x00, sizeof(rtcmake_version_t));    
    if (snprintf(version->id, sizeof(version->id), "cmake") < 0) {
        return err[ERR_ON_CALL];
    }

    errcode = rtcmake_run_command("cmake", "--version", buf, BUFSIZE);
    if (errcode != 0) {
        return err[errcode];
    }

    if (strncmp("cmake version", buf, 13) != 0) { 
        return err[ERR_CMAKE_FORMAT];
    }

    version_length = strlen(buf) - 15; // cmake_version_XXXXXX_ 

    start = 14;
    for (pos = 0; pos < version_length; pos++) {
        if (buf[pos + start] == '.') {
            buf[pos + start] = 0x00;
            pos++;
            break;
        }
    }
    version->major = strtoul(&buf[start], NULL, 10);

    start += pos;
    for (pos = 0; pos < version_length; pos++) {
        if (buf[pos + start] == '.') {
            buf[pos + start] = 0x00;
            pos++;
            break;
        }
    }
    version->minor = strtol(&buf[start], NULL, 10);

    start += pos;
    for (pos = 0; pos < version_length; pos++) {
        if (buf[pos + start] == '.' || buf[pos + start] == 0x0a) {
            buf[pos + start] = 0x00;
            break;
        }
    }
    version->patch = strtol(&buf[start], NULL, 10);
    return NULL;
}

const char *rtcmake_get_gcc_version(rtcmake_version_t *version)
{
    assert(version != NULL);
    
    char   buf[BUFSIZE]          = {0};
    int    errcode               = 0;

    unsigned int version_length  = 0;
    unsigned int start           = 0;
    unsigned int pos             = 0;

    memset(version, 0x00, sizeof(rtcmake_version_t));    
    if (snprintf(version->id, sizeof(version->id), "gcc") < 0) {
        return err[ERR_ON_CALL];
    }

    errcode = rtcmake_run_command("gcc", "--version", buf, BUFSIZE);
    if (errcode != 0) {
        return err[errcode];
    }

    if (strncmp("gcc", buf, 3) != 0) { 
        return err[ERR_GCC_FORMAT];
    }

    errcode = rtcmake_run_command("gcc", "-dumpfullversion", buf, BUFSIZE);
    if (errcode != 0) {
        return err[errcode];
    }

    version_length = strlen(buf);
    start = 0;
    for (pos = 0; pos < version_length; pos++) {
        if (buf[pos + start] == '.') {
            buf[pos + start] = 0x00;
            pos++;
            break;
        }
    }
    version->major = strtoul(&buf[start], NULL, 10);

    start += pos;
    for (pos = 0; pos < version_length; pos++) {
        if (buf[pos + start] == '.') {
            buf[pos + start] = 0x00;
            pos++;
            break;
        }
    }
    version->minor = strtol(&buf[start], NULL, 10);

    start += pos;
    for (pos = 0; pos < version_length; pos++) {
        if (buf[pos + start] == '.' || buf[pos + start] == 0x0a) {
            buf[pos + start] = 0x00;
            break;
        }
    }
    version->patch = strtol(&buf[start], NULL, 10);
    return NULL;
}

const char *rtcmake_get_clang_version(rtcmake_version_t *version)
{
    assert(version != NULL);
    
    char   buf[BUFSIZE]          = {0};
    int    errcode               = 0;

    unsigned int version_length  = 0;
    unsigned int start           = 0;
    unsigned int pos             = 0;

    memset(version, 0x00, sizeof(rtcmake_version_t));    
    if (snprintf(version->id, sizeof(version->id), "clang") < 0) {
        return err[ERR_ON_CALL];
    }

    errcode = rtcmake_run_command("clang", "--version", buf, BUFSIZE);
    if (errcode != 0) {
        return err[errcode];
    }

    if (strncmp("clang", buf, 3) != 0) { 
        return err[ERR_CLANG_FORMAT];
    }

    errcode = rtcmake_run_command("clang", "-dumpversion", buf, BUFSIZE);
    if (errcode != 0) {
        return err[errcode];
    }

    version_length = strlen(buf);
    start = 0;
    for (pos = 0; pos < version_length; pos++) {
        if (buf[pos + start] == '.') {
            buf[pos + start] = 0x00;
            pos++;
            break;
        }
    }
    version->major = strtoul(&buf[start], NULL, 10);

    start += pos;
    for (pos = 0; pos < version_length; pos++) {
        if (buf[pos + start] == '.') {
            buf[pos + start] = 0x00;
            pos++;
            break;
        }
    }
    version->minor = strtol(&buf[start], NULL, 10);

    start += pos;
    for (pos = 0; pos < version_length; pos++) {
        if (buf[pos + start] == '.' || buf[pos + start] == 0x0a) {
            buf[pos + start] = 0x00;
            break;
        }
    }
    version->patch = strtol(&buf[start], NULL, 10);
    return NULL;
}

const char *rtcmake_get_make_version(rtcmake_version_t *version)
{
    assert(version != NULL);
    
    char   buf[BUFSIZE]          = {0};
    int    errcode               = 0;

    unsigned int version_length  = 0;
    unsigned int start           = 0;
    unsigned int pos             = 0;

    memset(version, 0x00, sizeof(rtcmake_version_t));    
    if (snprintf(version->id, sizeof(version->id), "make") < 0) {
        return err[ERR_ON_CALL];
    }

    errcode = rtcmake_run_command("make", "--version", buf, BUFSIZE);
    if (errcode != 0) {
        return err[errcode];
    }

    if (strncmp("GNU Make", buf, 8) != 0) { 
        return err[ERR_MAKE_FORMAT];
    }

    version_length = strlen(buf) - 9; // GNU_make_

    start = 8;
    for (pos = 0; pos < version_length; pos++) {
        if (buf[pos + start] == '.') {
            buf[pos + start] = 0x00;
            pos++;
            break;
        }
    }
    version->major = strtoul(&buf[start], NULL, 10);

    start += pos;
    for (pos = 0; pos < version_length; pos++) {
        if (buf[pos + start] == '.') {
            buf[pos + start] = 0x00;
            pos++;
            break;
        }
    }
    version->minor = strtol(&buf[start], NULL, 10);

    start += pos;
    for (pos = 0; pos < version_length; pos++) {
        if (buf[pos + start] == '.' || buf[pos + start] == 0x0a) {
            buf[pos + start] = 0x00;
            break;
        }
    }
    version->patch = strtol(&buf[start], NULL, 10);
    return NULL;
}

const char *rtcmake_get_ninja_version(rtcmake_version_t *version)
{
    assert(version != NULL);
    
    char   buf[BUFSIZE]          = {0};
    int    errcode               = 0;

    unsigned int version_length  = 0;
    unsigned int start           = 0;
    unsigned int pos             = 0;

    memset(version, 0x00, sizeof(rtcmake_version_t));    
    if (snprintf(version->id, sizeof(version->id), "ninja") < 0) {
        return err[ERR_ON_CALL];
    }

    errcode = rtcmake_run_command("ninja", "--version", buf, BUFSIZE);
    if (errcode != 0) {
        return err[errcode];
    }

    version_length = strlen(buf);
    
    start = 0;
    for (pos = 0; pos < version_length; pos++) {
        if (buf[pos + start] == '.') {
            buf[pos + start] = 0x00;
            pos++;
            break;
        }
    }
    version->major = strtoul(&buf[start], NULL, 10);

    start += pos;
    for (pos = 0; pos < version_length; pos++) {
        if (buf[pos + start] == '.') {
            buf[pos + start] = 0x00;
            pos++;
            break;
        }
    }
    version->minor = strtol(&buf[start], NULL, 10);

    start += pos;
    for (pos = 0; pos < version_length; pos++) {
        if (buf[pos + start] == '.' || buf[pos + start] == 0x0a) {
            buf[pos + start] = 0x00;
            break;
        }
    }
    version->patch = strtol(&buf[start], NULL, 10);
    return NULL;
}

enum {
    CODE_SIZE   = 16,
    PROMPT_SIZE = 32
};

RTPROMPT_STATUS rtcmake_get_compilers(rtprompt_selection_t * const selection, RTPROMPT_ERRNO *errcode)
{
    assert(selection != NULL);
    assert(selection->options != NULL);
    assert(selection->options_size >= 2);

    rtcmake_version_t   compiler_version;
    const char         *errd = NULL;

    selection->options_item = 0;

    /* check GCC */
    errd = rtcmake_get_gcc_version(&compiler_version);
    if (errd == NULL) {
        char *code   = (char *)malloc(CODE_SIZE);
        if (code == NULL) {
            RETURN_STATUS(errcode, ERRNO_INVALID_ALLOCATE, STATUS_ERR);
        }

        char *prompt = (char *)malloc(PROMPT_SIZE);
        if (prompt == NULL) {
            free(code);
            RETURN_STATUS(errcode, ERRNO_INVALID_ALLOCATE, STATUS_ERR);
        }

        memset(code,   0x00, CODE_SIZE);
        memset(prompt, 0x00, PROMPT_SIZE);

        snprintf(code,   CODE_SIZE,   "gcc,g++");
        snprintf(prompt, PROMPT_SIZE, "GCC %u.%u.%u", compiler_version.major, compiler_version.minor, compiler_version.patch);

        selection->options[selection->options_item].code   = code;
        selection->options[selection->options_item].prompt = prompt;

        selection->options_item++;
    }

    /* check CLANG */
    errd = rtcmake_get_clang_version(&compiler_version);
    if (errd == NULL) {
        char *code   = (char *)malloc(CODE_SIZE);
        if (code == NULL) {
            RETURN_STATUS(errcode, ERRNO_INVALID_ALLOCATE, STATUS_ERR);
        }

        char *prompt = (char *)malloc(PROMPT_SIZE);
        if (prompt == NULL) {
            free(code);
            RETURN_STATUS(errcode, ERRNO_INVALID_ALLOCATE, STATUS_ERR);
        }

        memset(code,   0x00, CODE_SIZE);
        memset(prompt, 0x00, PROMPT_SIZE);

        snprintf(code,   CODE_SIZE,   "clang,clang++");
        snprintf(prompt, PROMPT_SIZE, "Clang %u.%u.%u", compiler_version.major, compiler_version.minor, compiler_version.patch);

        selection->options[selection->options_item].code   = code;
        selection->options[selection->options_item].prompt = prompt;

        selection->options_item++;
    }

    RETURN_STATUS(errcode, ERRNO_NO_ERROR, STATUS_OK);
}

RTPROMPT_STATUS rtcmake_get_generators(rtprompt_selection_t * const selection, RTPROMPT_ERRNO *errcode)
{
    assert(selection != NULL);
    assert(selection->options != NULL);
    assert(selection->options_size >= 2);

    rtcmake_version_t   compiler_version;
    const char         *errd = NULL;

    selection->options_item = 0;

    /* check Make */
    errd = rtcmake_get_make_version(&compiler_version);
    if (errd == NULL) {
        char *code   = (char *)malloc(CODE_SIZE);
        if (code == NULL) {
            RETURN_STATUS(errcode, ERRNO_INVALID_ALLOCATE, STATUS_ERR);
        }

        char *prompt = (char *)malloc(PROMPT_SIZE);
        if (prompt == NULL) {
            free(code);
            RETURN_STATUS(errcode, ERRNO_INVALID_ALLOCATE, STATUS_ERR);
        }

        memset(code,   0x00, CODE_SIZE);
        memset(prompt, 0x00, PROMPT_SIZE);

        snprintf(code,   CODE_SIZE,   "Unix Makefiles");
        snprintf(prompt, PROMPT_SIZE, "Unix Makefiles");

        selection->options[selection->options_item].code   = code;
        selection->options[selection->options_item].prompt = prompt;

        selection->options_item++;
    }

    /* check Ninja */
    errd = rtcmake_get_ninja_version(&compiler_version);
    if (errd == NULL) {
        char *code   = (char *)malloc(CODE_SIZE);
        if (code == NULL) {
            RETURN_STATUS(errcode, ERRNO_INVALID_ALLOCATE, STATUS_ERR);
        }

        char *prompt = (char *)malloc(PROMPT_SIZE);
        if (prompt == NULL) {
            free(code);
            RETURN_STATUS(errcode, ERRNO_INVALID_ALLOCATE, STATUS_ERR);
        }

        memset(code,   0x00, CODE_SIZE);
        memset(prompt, 0x00, PROMPT_SIZE);

        snprintf(code,   CODE_SIZE,   "Ninja");
        snprintf(prompt, PROMPT_SIZE, "Ninja %u.%u.%u", compiler_version.major, compiler_version.minor, compiler_version.patch);

        selection->options[selection->options_item].code   = code;
        selection->options[selection->options_item].prompt = prompt;

        selection->options_item++;
    }

    RETURN_STATUS(errcode, ERRNO_NO_ERROR, STATUS_OK);
}