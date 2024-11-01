/*
 *
 * 
 */
#ifndef __RTCMAKE_PROMPT_H__
#define __RTCMAKE_PROMPT_H__

#include <stddef.h>
#include <stdint.h>

#include <termios.h>

#define SET_ERROR(errcode, error) \
    if ((errcode) != NULL) { \
        (*errcode) = (error); \
    } \

#define RETURN_STATUS(errcode, error, returncode) \
    SET_ERROR(errcode, error); \
    return (returncode);


#define SET_ERRNO(errcode, error) \
    if ((errcode) != NULL) { \
        (*errcode) = (error); \
    } \

#define PROMPT_API extern

typedef enum RTPROMPT_STATUS {
    STATUS_ERR   = -1,
    STATUS_EXIT  =  0,
    STATUS_OK    =  1
} RTPROMPT_STATUS;

typedef enum RTPROMPT_ERRNO {
    ERRNO_NO_ERROR,
    ERRNO_INVALID_ARGS,
    ERRNO_INVALID_RETURN,
    ERRNO_INVALID_ALLOCATE,
    ERRNO_IO,
    ERRNO_IO_WRITE,
    ERRNO_IO_READ,
    ERRNO_TERM_NOT_INIT,
    ERRNO_TERM_NOTTY,
    ERRNO_REGEX_COMPILE,
    ERRNO_REGEX_EXECUTE
} RTPROMPT_ERRNO;

typedef enum RTPROMPT_KEYTYPE {
    KEY_NONE,
    KEY_ERROR,
    KEY_CHAR,
    KEY_CTRL,
    KEY_ESC,
    KEY_ENTER,
    KEY_BACKSPACE,
    KEY_INS,
    KEY_DEL,
    KEY_UP,
    KEY_DOWN,
    KEY_RIGHT,
    KEY_LEFT,
    KEY_HOME,
    KEY_END,
} RTPROMPT_KEYTYPE;

typedef struct rtprompt_termio_t {
    struct termios  ios;
    uint8_t         raw_mode;

    uint8_t         insert_mode;
    uint8_t         cursorx;
} rtprompt_termio_t;


PROMPT_API RTPROMPT_STATUS  rtprompt_init(RTPROMPT_ERRNO *errcode);
PROMPT_API void             rtprompt_finalize();

/// Question Section ///

typedef struct rtprompt_question_t {
    const char * const   prompt;
    const char *         default_response;

    const char * const   validation;
    
    char * const         response;
    const size_t         response_size;
    size_t               response_length;
} rtprompt_question_t;

PROMPT_API RTPROMPT_STATUS rtprompt_question(rtprompt_question_t * const prompt, uint8_t indentation, RTPROMPT_ERRNO *errcode);


/// Selection Section ///

#define RTPROMPT_OPTION_INIT(option) \
    (option)->code   = NULL; \
    (option)->prompt = NULL

#define RTPROMPT_OPTION_FINALIZE(option) \
    if ((option)->code   != NULL) free((option)->code); \
    if ((option)->prompt != NULL) free((option)->prompt); \
    (option)->code   = NULL; \
    (option)->prompt = NULL

typedef struct rtprompt_option_t {    
    char *code;
    char *prompt;
} rtprompt_option_t;

typedef struct rtprompt_selection_t {
    const char * const  prompt;

    rtprompt_option_t  *options;
    uint8_t             options_size;
    uint8_t             options_item;

    uint8_t             option_selected;

    char * const         response;
    const size_t         response_size;
    size_t               response_length;
} rtprompt_selection_t;

RTPROMPT_STATUS rtprompt_selection_init(rtprompt_selection_t *selection, uint8_t options_size, RTPROMPT_ERRNO *errcode);
void            rtprompt_selection_finalize(rtprompt_selection_t *selection);

RTPROMPT_STATUS rtprompt_selection(rtprompt_selection_t * const prompt, uint8_t indentation, RTPROMPT_ERRNO *errcode);

/// Yes/No Question Section ///

typedef struct rtprompt_yesno_question_t {
    const char * const   prompt;

    char * const         response;
    const size_t         response_size;
    size_t               response_length;
} rtprompt_yesno_question_t;

RTPROMPT_STATUS rtprompt_yesno_question(rtprompt_yesno_question_t * const prompt, uint8_t indentation, RTPROMPT_ERRNO *errcode);


/// Generic Prompt ///

typedef struct rtprompt_t {
    union {
        rtprompt_question_t       question;
        rtprompt_selection_t      selection;
        rtprompt_yesno_question_t yesno;
    }          prompt;

    const void * const            parameters;
    uint8_t                       identation;

    RTPROMPT_STATUS (*prompt_fn)(void * const prompt, const void * const parameters, uint8_t identation, RTPROMPT_ERRNO *errcode);
} rtprompt_t;


#endif