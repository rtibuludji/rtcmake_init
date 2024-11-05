/*
 *
 * 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>

#include "rtcmake_prompt.h"

PROMPT_API rtprompt_termio_t term;
PROMPT_API RTPROMPT_STATUS   rtprompt_term_mode(uint8_t raw_mode, RTPROMPT_ERRNO *errcode);
PROMPT_API RTPROMPT_KEYTYPE  rtprompt_term_readkey(char *key_value, RTPROMPT_ERRNO *errcode);
PROMPT_API RTPROMPT_STATUS   rtprompt_term_erase_lines(uint8_t number_lines, RTPROMPT_ERRNO *errcode);

RTPROMPT_STATUS rtprompt_yesno_question(rtprompt_yesno_question_t * const prompt, uint8_t indentation, RTPROMPT_ERRNO *errcode)
{
    char     out[256]   = {0};
    ssize_t  out_length = 0;

    if (prompt == NULL || prompt->response == NULL) {
        RETURN_STATUS(errcode, ERRNO_INVALID_ARGS, STATUS_ERR)
    }

    RTPROMPT_STATUS status = rtprompt_term_mode(1, errcode);
    if (status != STATUS_OK) {
        return status;
    }

    
    out_length = snprintf(out, sizeof(out), "%*s\x1b[37m%s? \x1b[90m(Y)es/(N)o\x1b[0m", indentation, "", prompt->prompt);
    if (write(STDOUT_FILENO, out, out_length) != out_length) {
        RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
    } 

    uint8_t       is_done   = 0;
    char          key_value = 0x00;
    while (!is_done) {
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

            case KEY_CHAR:                
                key_value = (char)toupper(key_value);
                if (key_value == 'Y' || key_value == 'N') {
                    status  = STATUS_OK;
                    is_done = 1;

                    out_length = snprintf(out, sizeof(out), "\x1b[10D\x1b[K%s", key_value == 'Y' ? "Yes":"No");
                    if (write(STDOUT_FILENO, out, out_length) != out_length) {
                        SET_ERROR(errcode, ERRNO_IO_WRITE);
                        status = STATUS_ERR; 
                    }
                }
                break;

            default:
                break;
        }
    }
    
    if (write(STDOUT_FILENO, "\r\n", 2) != 2) {
        rtprompt_term_mode(0, NULL);
        RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
    }

    rtprompt_term_mode(0, NULL);

    if (status == STATUS_OK) {
        prompt->response[0] = key_value;
    }
    
    return status;
}
