#include "message.h"
#include <stdlib.h>
#include <memory.h>

int insert_message(const char *message, const char **messages)
{
    if (message == NULL)
        return 1;

    // free the last message
    if (messages[MAX_MESSAGES-1])
    {
        free((char*) messages[MAX_MESSAGES-1]);
    }

    // rotate all messages up one index
    for (int i = MAX_MESSAGES-1; i > 0; --i)
    {
        messages[i] = messages[i - 1];
    }

    // insert message at index 0
    messages[0] = message;

    return 0;
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap); // FIXME this shouldn't be necessary...
const char *create_message(const char *fmt, va_list args)
{
    char *message;
    message = malloc(MAX_MESSAGE_LENGTH + 1);
    memset((char*) message, 0, MAX_MESSAGE_LENGTH + 1);

    int bytes = vsnprintf(message, MAX_MESSAGE_LENGTH + 1, fmt, args);

    if (bytes > MAX_MESSAGE_LENGTH + 1)
    {
        // error! can't reserve more than max message length
        free(message);

        return NULL;
    }

    return message;
}

int message(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    const char *msg = create_message(fmt, args);

    va_end(args);

    return insert_message(msg, messages);
}
