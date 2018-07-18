#include "draw.h"
#include "game.h"
#include "message.h"

#include <ncurses.h>
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
    messages = malloc(sizeof(char*) * MAX_MESSAGE_LENGTH * MAX_MESSAGES);
    if (messages == NULL)
        return ERROR_OOM;

    // randomize initial level
    if (!init_level(dungeon->level, dungeon->player))
        return ERROR_OOM;

    int result = GAME_PLAYING;
    char input;
    while (result == GAME_PLAYING)
    {
        // render & update curses
        render(dungeon);

        if (handle_input(dungeon))
            input = getch();
        else
            input = '.';

        // trigger gameloop
        result = gameloop(dungeon, input);
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

    printf("Do you want to see the mobs you killed? [Y/n] ");
    input = getchar();

    if (input == 'y' || input == 'Y' || input == '\n')
        print_mob_list(dungeon->killed);

    printf("You reached dungeon level %d. Your player was level %d and collected %d gold.\n\n",
            max_depth(dungeon),
            dungeon->player->attrs.level,
            total_gold(dungeon->player->items));

    return 0;
}
