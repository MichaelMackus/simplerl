#ifndef MESSAGE_H
#define MESSAGE_H

#define MAX_MESSAGE_LENGTH 80
#define MAX_MESSAGES 5

#include <stdarg.h>
#include <stdbool.h>

// TODO debug (printf) macro
// #ifdef DEBUG printf ???

// get a message at specified index
const char *get_message(int index);

// initialize message buffer
bool init_messages();

// simple function to reserve memory for message and insert it into message list
int message(const char *fmt, ...);

#endif
