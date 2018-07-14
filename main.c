#include "draw.h"
#include "game.h"
#include "map.h"
#include "message.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define ERROR_OOM 1  // out of memory error
#define ERROR_INIT 2 // curses initialization error
#define ERROR_GAME 3 // internal game error

int main()
{
    // initialize curses
    if (!init())
        return ERROR_INIT;

    // initialize dungeon
    Dungeon *dungeon = create_dungeon();
    if (dungeon == NULL)
        return ERROR_OOM;

    // allocate messages array
    const char **messages;
    messages = malloc(sizeof(char) * MAX_MESSAGE_LENGTH * MAX_MESSAGES);
    if (messages == NULL)
        return ERROR_OOM;

    // initialize curses
    init_game(dungeon, messages);

    int result = GAME_PLAYING;
    while (result == GAME_PLAYING)
    {
        // render & update curses
        update(dungeon, messages);

        // trigger gameloop
        result = gameloop(dungeon, messages);
    }

    // de-initialize curses
    deinit();

    if (result == GAME_WON)
        printf("You won!\n");
    else if (result == GAME_QUIT)
        printf("You quit.\n");
    else if (result == GAME_DEATH)
        printf("Oh no, you died :(\n");
    else if (result == GAME_OOM)
        return ERROR_OOM;
    else if (result == GAME_ERROR)
        return ERROR_GAME;
    else
        printf("Undefined game loop result...\n");

    return 0;
}
