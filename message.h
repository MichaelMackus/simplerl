#ifndef MESSAGE_H
#define MESSAGE_H

#define MAX_MESSAGE_LENGTH 80
#define MAX_MESSAGES 5

#include <stdarg.h>

const char **messages;

// TODO debug (printf) macro
// #ifdef DEBUG printf ???

// simple function to reserve memory for message and insert it into message list
int message(const char *fmt, ...);

#endif
