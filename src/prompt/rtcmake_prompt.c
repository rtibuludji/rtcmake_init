/*
 *
 * 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include <termios.h>

#include "rtcmake_prompt.h"

static uint8_t     is_initialize = 0;
rtprompt_termio_t  term          = {};

RTPROMPT_STATUS rtprompt_init(RTPROMPT_ERRNO *errcode)
{
    if (is_initialize == 0) {
        is_initialize = 1;

        memset(&term, 0x00, sizeof(rtprompt_termio_t));

        if (tcgetattr(STDERR_FILENO, &term.ios) == -1) {
            errno = ENOTTY;
            RETURN_STATUS(errcode, ERRNO_TERM_NOTTY, STATUS_ERR);
        }

        if (!isatty(STDIN_FILENO)) {
            errno = ENOTTY;
            RETURN_STATUS(errcode, ERRNO_TERM_NOTTY, STATUS_ERR);
        }
    }

    RETURN_STATUS(errcode, ERRNO_NO_ERROR, STATUS_OK);
}

void rtprompt_finalize() {
    if (is_initialize == 1) {
        if (term.raw_mode == 1) {
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &term.ios);
        }

        if (write(STDOUT_FILENO, "\x1b[1 q\x1b[0m", 10) != 0) {
            // noop, nothing todo when failed
        }
    }

    memset(&term, 0x00, sizeof(rtprompt_termio_t));
    is_initialize = 0;
}

PROMPT_API RTPROMPT_STATUS rtprompt_term_mode(uint8_t raw_mode, RTPROMPT_ERRNO *errcode)
{
    if (is_initialize == 0) {
        RETURN_STATUS(errcode, ERRNO_TERM_NOT_INIT, STATUS_ERR);
    }

    if (raw_mode == 1) {
        struct termios raw; 

        raw = term.ios;

        raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); /* input modes: no break, no CR to NL, no parity check, no strip char, no start/stop output control. */
        raw.c_oflag &= ~(OPOST);                                  /* output modes - disable post processing */
        raw.c_cflag |= (CS8);                                     /* control modes - set 8 bit chars */
        raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);          /* local modes - choing off, canonical off, no extended functions, no signal chars (^Z,^C) */
        raw.c_cc[VMIN] = 1;                                       /* control chars - 1 byte  */
        raw.c_cc[VTIME] = 0;                                      /* control chars - no timer */

        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) < 0) {
            errno = ENOTTY;
            RETURN_STATUS(errcode, ERRNO_TERM_NOTTY, STATUS_ERR);
        }

        term.raw_mode = 1;
    }
    else {
        if (term.raw_mode == 1) {
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &term.ios);
            term.raw_mode = 0;
        }
        if (write(STDOUT_FILENO, "\x1b[1 q\x1b[0m", 10) == 10) {
            RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
        }
    }

    RETURN_STATUS(errcode, ERRNO_NO_ERROR, STATUS_OK);
}

enum {
    K_ENTER = 13,
    K_ESC = 27,
    K_BACKSPACE = 127
};

#define CTRL_KEY(k) ((uint8_t)(k) & (uint8_t)0x1f)

static RTPROMPT_KEYTYPE read_esc_sequence__(RTPROMPT_ERRNO *errcode)
{
    char esc[3];

    if (read(STDIN_FILENO, &esc[0], 1) == -1 || read(STDIN_FILENO, &esc[1], 1) == -1) {
        RETURN_STATUS(errcode, ERRNO_IO_READ, KEY_ERROR)
    }

    if (esc[0] == '[') {
        if (esc[1] >= '0' && esc[1] <= '9') {
            if (read(STDIN_FILENO, &esc[2], 1) == -1) {
                RETURN_STATUS(errcode, ERRNO_IO_READ, KEY_ERROR)
            }

            if (esc[2] == '~') {
                if (esc[1] == '2') {  // Insert mode toggle
                    return KEY_INS;
                } 
                else if (esc[1] == '3') {  // Delete key
                    return KEY_DEL;
                }
            }
        } 
        else {
            switch (esc[1]) {
                case 'A':
                    return KEY_UP; 
                    break;  
                case 'B': 
                    return KEY_DOWN; 
                    break;  
                case 'C':
                    return KEY_RIGHT;
                    break;
                case 'D':
                    return KEY_LEFT;
                    break;
                default:
                    return KEY_NONE;
                    break;
            }
        }
    } 
    else if (esc[0] == 'O') {
        switch (esc[1]) {
            case 'H':
                return KEY_HOME;
                break;
            case 'F':            
                return KEY_END;
                break;
            default:
                return KEY_NONE;
                break;
        }
    }

    return KEY_ESC;     
}

PROMPT_API RTPROMPT_KEYTYPE rtprompt_term_readkey(char *key_value, RTPROMPT_ERRNO *errcode)
{
    if (key_value == NULL) {
        RETURN_STATUS(errcode, ERRNO_INVALID_ARGS, KEY_ERROR);
    }

    ssize_t nread        = 0;
    char    key_pressed  = (char)0;

    while ((nread = read(STDIN_FILENO, &key_pressed, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) {
            RETURN_STATUS(errcode, ERRNO_IO_READ, KEY_ERROR)
        }
    }

    RTPROMPT_KEYTYPE key = KEY_CHAR;
    switch (key_pressed) {
        case CTRL_KEY('q'):
            key = KEY_CTRL;
            *key_value = 'Q';
            break;
        case K_ENTER:
            key = KEY_ENTER;
            break;
        case K_ESC:
            key = read_esc_sequence__(errcode);
            break;
        case K_BACKSPACE:
            key = KEY_BACKSPACE;
            break;
        default:
            *key_value = key_pressed;
            break;
    }

    return key;
}

PROMPT_API RTPROMPT_STATUS  rtprompt_term_erase_lines(uint8_t number_lines, RTPROMPT_ERRNO *errcode)
{
    char out[256]    = {0};
    int  out_length  = 0;

    if (number_lines == 0) {
        RETURN_STATUS(errcode, ERRNO_NO_ERROR, STATUS_OK);
    }

    out_length = snprintf(out, sizeof(out), "\x1b[2K");
    for (uint8_t line = 1; line < number_lines; ++line) {
        out_length += snprintf(out + out_length, sizeof(out) - out_length, "\x1b[1A\x1b[2K");
    }

    out_length += snprintf(out + out_length, sizeof(out) - out_length, "\r");
    if (write(STDOUT_FILENO, out, out_length) != out_length) {
        RETURN_STATUS(errcode, ERRNO_IO_WRITE, STATUS_ERR);
    }

    RETURN_STATUS(errcode, ERRNO_NO_ERROR, STATUS_OK);
}
