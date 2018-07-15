#ifndef MESSAGE_H
#define MESSAGE_H

#define MAX_MESSAGE_LENGTH 80
#define MAX_MESSAGES 5

// TODO debug (printf) macro
// #ifdef DEBUG printf ???

// insert message to beginning, dropping older messages
int insert_message(const char *message, const char **messages);

// simple function to reserve memory for message, TODO expand with our object types
//
// FORMAT VARS (like printf):
//
//  %d - digit
//  %s - string
//
const char *create_message(const char *fmt, ...);

#endif
