/*
 *
 * 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>

#include "rtcmake_prompt.h"

PROMPT_API rtprompt_termio_t term;
PROMPT_API RTPROMPT_STATUS   rtprompt_term_mode(uint8_t raw_mode, RTPROMPT_ERRNO *errcode);
PROMPT_API RTPROMPT_KEYTYPE  rtprompt_term_readkey(char *key_value, RTPROMPT_ERRNO *errcode);
PROMPT_API RTPROMPT_STATUS   rtprompt_term_erase_lines(uint8_t number_lines, RTPROMPT_ERRNO *errcode);

RTPROMPT_STATUS rtprompt_selection_init(rtprompt_selection_t *selection, uint8_t options_size, RTPROMPT_ERRNO *errcode)
{
    if (selection == NULL) {
        RETURN_STATUS(errcode, ERRNO_INVALID_ARGS, STATUS_ERR);
    }

    memset(selection, 0x00, sizeof(rtprompt_selection_t));
    selection->options = (rtprompt_option_t *)malloc(sizeof(rtprompt_option_t) * options_size);
    if (selection->options == NULL) {
        RETURN_STATUS(errcode, ERRNO_INVALID_ALLOCATE, STATUS_ERR);
    }

    selection->options_size = options_size;
    for (uint8_t idx = 0; idx < options_size; ++idx) {
        RTPROMPT_OPTION_INIT(&selection->options[idx]);
    }

    RETURN_STATUS(errcode, ERRNO_NO_ERROR, STATUS_OK);
}

void rtprompt_selection_finalize(rtprompt_selection_t *selection) 
{
    if (selection != NULL) {
        for (uint16_t idx = 0; idx < selection->options_size; ++idx) {
            RTPROMPT_OPTION_FINALIZE(&selection->options[idx]);
        }
    }
}

static RTPROMPT_STATUS write_prompt__(const rtprompt_selection_t * const prompt, uint8_t indentation, RTPROMPT_ERRNO *errno) 
{
    char out[256]   = {0};
    int  out_length = 0;

    out_length = snprintf(out, sizeof(out), "\r%*s\x1b[37m%s? \x1b[90m%s\x1b[0m", indentation, "",  prompt->prompt, prompt->options[prompt->option_selected].prompt);
    if (write(STDOUT_FILENO, out, out_length) != out_length) {
        RETURN_STATUS(errno, ERRNO_IO_WRITE, STATUS_ERR);
    }

    for (size_t index = 0; index < prompt->options_item; ++index) {
        if (index == prompt->option_selected) {
            out_length = snprintf(out, sizeof(out), "\n\r%*s\x1b[34m\U000025CF %s\x1b[0m", indentation + 3, "", prompt->options[index].prompt);
        }
        else {
            out_length = snprintf(out, sizeof(out), "\n\r%*s\x1b[37m\U000025CF %s\x1b[0m", indentation + 3, "", prompt->options[index].prompt);
        }

        if (write(STDOUT_FILENO, out, out_length) != out_length) {
            RETURN_STATUS(errno, ERRNO_IO_WRITE, STATUS_ERR);
        }
    }

    RETURN_STATUS(errno, ERRNO_NO_ERROR, STATUS_OK);
}

RTPROMPT_STATUS rtprompt_selection(rtprompt_selection_t * const prompt, uint8_t indentation, RTPROMPT_ERRNO *errcode)
{
    if (prompt == NULL  || prompt->prompt == NULL || prompt->options == NULL || prompt->options_size == 0 || prompt->options_item == 0) {
        RETURN_STATUS(errcode, ERRNO_INVALID_ARGS, STATUS_ERR); 
    }

    RTPROMPT_STATUS status = rtprompt_term_mode(1, errcode);
    if (status != STATUS_OK) {
        return status;
    }

    if (write(STDOUT_FILENO, "\x1b[?25l", 6) != 6) {
        RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
    }

    prompt->option_selected = 0;
    char    key_value       = 0x00;
    uint8_t is_done         = 0;
    uint8_t rewrite         = 1;

    while (!is_done) {
        if (rewrite) {
            status = write_prompt__(prompt, indentation, errcode);
            if (status != STATUS_OK) {
                is_done = 1;
                continue;
            }
            rewrite = 0;
        }

        RTPROMPT_KEYTYPE key_type = rtprompt_term_readkey(&key_value, errcode);
        switch (key_type) {
            case KEY_ERROR:
                is_done = 1;
                status  = STATUS_ERR;
                break;

            case KEY_CTRL:
                if (key_value == 'Q') {
                    is_done = 1;
                    status  = STATUS_EXIT;
                }
                break;

            case KEY_ENTER:
                is_done = 1;
                status = rtprompt_term_erase_lines(prompt->options_item + 1, errcode);
                if (status == STATUS_OK) {
                    char out[256] = {0};
                    int out_length = 0;

                    out_length = snprintf(out, sizeof(out), "\r%*s\x1b[37m%s? \x1b[0m%s\x1b[0m", indentation, "",  prompt->prompt, prompt->options[prompt->option_selected].prompt);
                    if (write(STDOUT_FILENO, out, out_length) != out_length) {
                        SET_ERROR(errcode, ERRNO_IO_WRITE);
                        status = STATUS_ERR;
                    }
                    else {
                        SET_ERROR(errcode, ERRNO_NO_ERROR);
                        status = STATUS_OK;
                    }
                }
                break;

                

            case KEY_UP:
                if (prompt->option_selected > 0) {
                    status = rtprompt_term_erase_lines(prompt->options_item + 1, errcode);
                    if (status != STATUS_OK) {
                        is_done = 1;
                        status  = STATUS_ERR;
                    }
                    else {
                        prompt->option_selected--;
                        rewrite = 1;
                    }
                }
                break;

            case KEY_DOWN:
                if (prompt->option_selected < prompt->options_item - 1) {
                    status = rtprompt_term_erase_lines(prompt->options_item + 1, errcode);
                    if (status != STATUS_OK) {
                        is_done = 1;
                        status  = STATUS_ERR;
                    }
                    else {
                        prompt->option_selected++;
                        rewrite = 1;
                    }
                }
                break;

            default:
                break;
        }
    }

    if (write(STDOUT_FILENO, "\x1b[?25h\n\r", 8) != 8) {
        rtprompt_term_mode(0, NULL);
        RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
    }

    rtprompt_term_mode(0, NULL);

    if (status == STATUS_OK) {
        snprintf(prompt->response, prompt->response_size, "%s", prompt->options[prompt->option_selected].code);
        prompt->response_length = strlen(prompt->response);
    }

    return status;
}