#ifndef MESSAGE_H
#define MESSAGE_H

#define MAX_MESSAGE_LENGTH 80
#define MAX_MESSAGES 5

// insert message to beginning, dropping older messages
void insert_message(const char *message, const char **messages);

// simple function to reserve memory for message, TODO expand with our object types
//
// FORMAT VARS (like printf):
//
//  %d - digit
//  %s - string
//
const char *create_message(const char *fmt, ...);

#endif
