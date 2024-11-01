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
#include <regex.h> 

#include "rtcmake_prompt.h"

PROMPT_API rtprompt_termio_t term;
PROMPT_API RTPROMPT_STATUS   rtprompt_term_mode(uint8_t raw_mode, RTPROMPT_ERRNO *errcode);
PROMPT_API RTPROMPT_KEYTYPE  rtprompt_term_readkey(char *key_value, RTPROMPT_ERRNO *errcode);
PROMPT_API RTPROMPT_STATUS   rtprompt_term_erase_lines(uint8_t number_lines, RTPROMPT_ERRNO *errcode);

typedef struct line_buffer_t {
    char * const  buffer;
    const size_t  size;
    size_t        length;
} line_buffer_t;

static RTPROMPT_STATUS insert_char_to_buffer__(line_buffer_t *line, char value, RTPROMPT_ERRNO *errcode)
{
    if (line->length >= line->size - 1) {
        RETURN_STATUS(errcode, ERRNO_NO_ERROR, STATUS_OK);
    }

    if (line->length == term.cursorx) {
        if (write(STDOUT_FILENO, &value, 1) != 1) {
            RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
        }

        line->buffer[line->length] = value;
        line->length++;
        term.cursorx++;
    }
    else if (term.cursorx < line->length) {
        if (term.insert_mode == 1) {
            if (write(STDOUT_FILENO, &value, 1) != 1) {
                RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
            }
            line->buffer[term.cursorx] = value;
            term.cursorx++;
        }
        else {
            memmove(&line->buffer[term.cursorx + 1], &line->buffer[term.cursorx], line->length - term.cursorx);
            line->buffer[term.cursorx] = value;
            line->length++;

            size_t write_length = line->length - term.cursorx;
            if (write(STDOUT_FILENO, &line->buffer[term.cursorx], write_length) != write_length) {
                RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
            }

            if (write_length > 1) {
                char move_cursor[16];

                write_length = snprintf(move_cursor, sizeof(move_cursor), "\x1b[%zuD", write_length - 1);
                if (write(STDOUT_FILENO, move_cursor, write_length) != write_length) {
                    RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
                }
            }
            term.cursorx++;
        }
    }

    RETURN_STATUS(errcode, ERRNO_NO_ERROR, STATUS_OK);
}

static RTPROMPT_STATUS delete_char_from_buffer_using_backspace__(line_buffer_t *line, RTPROMPT_ERRNO *errcode) 
{
    if (line->length == 0) {
        RETURN_STATUS(errcode, ERRNO_NO_ERROR, STATUS_OK);
    }

    if (term.cursorx > 0) {
        size_t length = line->length - term.cursorx;

        memmove(&line->buffer[term.cursorx - 1], &line->buffer[term.cursorx], length);
        line->length--;
        line->buffer[line->length] = 0x00;
        term.cursorx--;
        

        if (write(STDOUT_FILENO, "\x1b[1D\x1b[K", 7) != 7) {
            RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
        }

        size_t write_length = length;
        if (write(STDOUT_FILENO, &line->buffer[term.cursorx], write_length) != write_length) {
            RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
        }

        if (write_length > 0) {
            char move_cursor[16];
            
            write_length = snprintf(move_cursor, sizeof(move_cursor), "\x1b[%zuD", length);
            if (write(STDOUT_FILENO, move_cursor, write_length) != write_length) {
                RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
            }
        }
    }

    RETURN_STATUS(errcode, ERRNO_NO_ERROR, STATUS_OK);
}

static RTPROMPT_STATUS delete_char_from_buffer_using_delete__(line_buffer_t *line, RTPROMPT_ERRNO *errcode) {
    if (line->length == 0) {
        RETURN_STATUS(errcode, ERRNO_NO_ERROR, STATUS_OK);
    }

    if (term.cursorx == line->length - 1) {
        line->buffer[line->length - 1] = 0x00;
        line->length--;

        if (write(STDOUT_FILENO, "\x1b[K", 3) != 3) {
            RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
        }
    }
    else if (term.cursorx < line->length - 1) {
        size_t length = line->length - term.cursorx;
        
        memmove(&line->buffer[term.cursorx], &line->buffer[term.cursorx + 1], length);
        line->buffer[line->length] = 0x00;
        line->length--;

        if (write(STDOUT_FILENO, "\x1b[K", 3) != 3) {
            RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
        }

        size_t write_length = line->length - term.cursorx;
        if(write(STDOUT_FILENO, &line->buffer[term.cursorx], write_length) != write_length) {
            RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
        }

        if (length > 1) {
            char move_cursor[16];
            
            write_length = snprintf(move_cursor, sizeof(move_cursor), "\x1b[%zuD", length - 1);
            if (write(STDOUT_FILENO, move_cursor, write_length) != write_length) {
                RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
            }
        }
    }

    RETURN_STATUS(errcode, ERRNO_NO_ERROR, STATUS_OK);
}

RTPROMPT_STATUS readline__(line_buffer_t * const line, const char *default_value, RTPROMPT_ERRNO *errcode) 
{
    char          key_value = (char)0x00;
    term.cursorx            = 0;
    
    while (1) {
        RTPROMPT_KEYTYPE key_type = rtprompt_term_readkey(&key_value, errcode);
        switch (key_type) {
            case KEY_ERROR:
                return STATUS_ERR;

            case KEY_CTRL:
                if (key_value == 'Q') {
                    RETURN_STATUS(errcode, ERRNO_NO_ERROR, STATUS_EXIT);
                }
                break;

            case KEY_ENTER:
                if (line->length == 0 && default_value != NULL) {
                    size_t default_len = strlen(default_value);
                    if (default_len < line->size - 1) {
                        memcpy(line->buffer, default_value, default_len);
                        line->length = default_len;
                        if (write(STDOUT_FILENO, line->buffer, line->length) != line->length) {
                            RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
                        }
                    }
                }
                RETURN_STATUS(errcode, ERRNO_NO_ERROR, STATUS_OK);

            case KEY_CHAR:
                if (insert_char_to_buffer__(line, key_value, errcode) != STATUS_OK) {
                    return STATUS_ERR;  // Insertion error
                }

                if (default_value != NULL && line->length == term.cursorx) {
                    if (write(STDOUT_FILENO, "\x1b[K", 3) != 3) {
                        RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
                    }
                }
                break;
            
            case KEY_BACKSPACE:
                if (delete_char_from_buffer_using_backspace__(line, errcode) != STATUS_OK) {
                    return STATUS_ERR;
                }

                if (default_value != NULL && line->length == 0 && line->length == term.cursorx) {
                    char out[32];
                    int out_length = snprintf(out, sizeof(out), "\x1b[90m%s\x1b[0m\x1b[%zuD", default_value, strlen(default_value));
                    if (write(STDOUT_FILENO, out, out_length) != out_length) {
                        RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
                    }
                }
                break;
            
            case KEY_DEL:
                if (delete_char_from_buffer_using_delete__(line, errcode) != STATUS_OK) {
                    return STATUS_ERR;;
                }

                if (default_value != NULL && line->length == 0 && line->length == term.cursorx) {
                    char out[32];
                    int out_length = snprintf(out, sizeof(out), "\x1b[90m%s\x1b[0m\x1b[%zuD", default_value, strlen(default_value));
                    if (write(STDOUT_FILENO, out, out_length) != out_length) {
                        RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
                    }
                }
                break;

            case KEY_INS:
                term.insert_mode = !term.insert_mode;
                if (write(STDOUT_FILENO, term.insert_mode ? "\x1b[3 q" : "\x1b[1 q", 5) != 5) {
                    RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
                }
                break;

            case KEY_RIGHT:
                if (term.cursorx < line->length) {
                    // Move right
                    if (write(STDOUT_FILENO, "\x1b[1C", 4) != 4) {
                        RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
                    }
                    term.cursorx++;
                }
                break;

            case KEY_LEFT:
                if (term.cursorx > 0) {
                    // Move left
                    if (write(STDOUT_FILENO, "\x1b[1D", 4) != 4) {
                        RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
                    }
                    term.cursorx--;
                }
                break;

            default:
                break;
        }
    }

    RETURN_STATUS(errcode, ERRNO_NO_ERROR, STATUS_OK);
}

static RTPROMPT_STATUS write_message__(const char *msg, uint8_t indentation, RTPROMPT_ERRNO *errcode) 
{
    char          out[256] = {0};
    size_t        out_length      = 0;

    uint8_t       is_done   = 0;
    char          key_value = 0x00;

    out_length = snprintf(out, sizeof(out), "\n\r%*s\x1b[1;5m%s\x1b[0m", indentation, "", msg);
    if (write(STDOUT_FILENO, out, out_length) != out_length) {
        RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
    }

    while (is_done == 0) {
        RTPROMPT_KEYTYPE key_type = rtprompt_term_readkey(&key_value, errcode);
        switch (key_type) {
            case KEY_ERROR:
                return STATUS_ERR; 
            
            case KEY_CTRL:
                if (key_value == 'Q') {
                    RETURN_STATUS(errcode, ERRNO_NO_ERROR, STATUS_EXIT);
                }
                break;

            default:
                is_done = 1;
                break;
        }
    }

    if (write(STDOUT_FILENO, "\n\r", 2) != 2) {
        RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
    }

    RETURN_STATUS(errcode, ERRNO_NO_ERROR, STATUS_OK);
}

static RTPROMPT_STATUS write_prompt__(const rtprompt_question_t *prompt, uint8_t indentation, const char *default_response, RTPROMPT_ERRNO *errcode) 
{
    char out[256]  = {0};
    int out_length = 0;

    if (default_response == NULL) {
        out_length = snprintf(out, sizeof(out), "%*s\x1b[37m%s? \x1b[0m", indentation, "", prompt->prompt);
    } else {
        out_length = snprintf(out, sizeof(out), "%*s\x1b[37m%s? \x1b[90m%s\x1b[0m\x1b[%zuD", indentation, "", prompt->prompt, default_response, strlen(default_response));
    }

    if (write(STDOUT_FILENO, out, out_length) != out_length) {
        RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
    }

    RETURN_STATUS(errcode, ERRNO_NO_ERROR, STATUS_OK);
}

static RTPROMPT_STATUS handle_message__(uint8_t indentation, RTPROMPT_ERRNO *errcode) 
{
    RTPROMPT_STATUS status = write_message__("invalid value, not match with validation string", indentation, errcode);
    if (status == STATUS_OK) {
        char out[256] = {0};
        int out_length = snprintf(out, sizeof(out), "\x1b[2K\x1b[1A\x1b[2K\x1b[1A\x1b[2K");

        if (write(STDOUT_FILENO, out, out_length) != out_length) {
            SET_ERROR(errcode, ERRNO_IO_WRITE);
            status = STATUS_ERR;
        }
    }

    return status;
}

static int handle_validation__(const char *value, const char *validation, RTPROMPT_ERRNO *errcode) 
{
    regex_t regex;
    int     regcode = 0;

    int     status  = 1;

    regcode = regcomp(&regex, validation, 0);
    if (regcode != 0) {
        RETURN_STATUS(errcode,  ERRNO_REGEX_COMPILE, -1);
    }   

    regcode = regexec(&regex, value, 0, NULL, 0);
    if (regcode == REG_NOMATCH) {
        status = 0;
    }
    else if (regcode == 0) {
        status = 1;
    }
    else {
        status = -1;
    }

    regfree(&regex);

    RETURN_STATUS(errcode,  ERRNO_REGEX_COMPILE, status);
}

RTPROMPT_STATUS rtprompt_question(rtprompt_question_t * const prompt, uint8_t indentation, RTPROMPT_ERRNO *errcode)
{
    if (prompt == NULL || prompt->prompt == NULL || prompt->response == NULL || prompt->response_size == 0) {
        RETURN_STATUS(errcode, ERRNO_INVALID_ARGS, STATUS_ERR);
    }

    line_buffer_t line = {
        .buffer = prompt->response,
        .size   = prompt->response_size,
        .length = 0
    };

    RTPROMPT_STATUS status = rtprompt_term_mode(1, errcode);
    if (status != STATUS_OK) {
        return status;
    }

    uint8_t is_done = 0;
    while (!is_done) { 
        memset(line.buffer, 0x00, line.size);
        line.length  = 0;
        term.cursorx = 0;

        status = write_prompt__(prompt, indentation, prompt->default_response, errcode);
        if (status != STATUS_OK) {
            rtprompt_term_mode(0, NULL);
            return status;
        }

        status = readline__(&line, prompt->default_response, errcode);
        if (status != STATUS_OK) {
            is_done = 1;
            continue;
        }

        if (prompt->validation != NULL) {
            int validation_result = handle_validation__(prompt->response, prompt->validation, errcode);
            if (validation_result == -1) {
                status  = STATUS_ERR;
                is_done = 1;
                continue;
            }
            else if (validation_result == 0) {
                status = handle_message__(indentation + strlen(prompt->prompt), errcode);
                if (status != STATUS_OK) {
                    is_done = 1;
                }
            }
            else {
                is_done = 1;
            }
        }
        else {
            is_done = 1;
        }
    }

    if (write(STDOUT_FILENO, "\r\n", 2) != 2) {
        rtprompt_term_mode(0, NULL);
        RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
    }
    
    rtprompt_term_mode(0, NULL);

    if (status == STATUS_OK) {
        prompt->response_length = line.length;
    }

    return status;
}