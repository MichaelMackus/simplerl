#include "draw.h"
#include "game.h"
#include "message.h"

#define RL_IMPLEMENTATION
#include "lib/roguelike.h"

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>

#define ERROR_OOM 1  // out of memory error
#define ERROR_INIT 2 // curses initialization error
#define ERROR_GAME 3 // internal game error

int usage()
{
    printf("Usage: simplerl [--no-color]\n");

    return 99;
}

int main(int argc, const char **argv)
{
    int enableColor = 1;
    if (argc > 1 && strcmp(argv[1], "--no-color") == 0)
        enableColor = 0;
    if (argc > 1 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")))
        return usage();

    // initialize curses
    if (!init(enableColor)) {
        fprintf(stderr, "ERROR: Terminal size too small. The game requires a terminal of at least %d characters wide by %d characters tall.\n", MAX_WIDTH, MAX_HEIGHT);
        return ERROR_INIT;
    }

    // initialize dungeon
    unsigned long seed = time(0);
    Dungeon *dungeon = create_dungeon(seed);
    if (dungeon == NULL)
        return ERROR_OOM;

    // allocate messages array
    if (!init_messages())
        return ERROR_OOM;

    // randomize initial level
    if (!init_level(dungeon->level, dungeon->player))
        return ERROR_OOM;
    render(dungeon);

    // update initial FOV
    rl_fov_calculate(dungeon->level->fov, dungeon->level->map, dungeon->player->coords.x, dungeon->player->coords.y, FOV_RADIUS);

    int result = GAME_PLAYING;
    int input;
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

    // print some things that might be interesting to the user
    printf("\n");
    print_mob_list(dungeon->killed);
    printf("\n");
    printf("You reached dungeon level %d. Your player was level %d and collected %d gold.\n\n",
            max_depth(dungeon),
            dungeon->player->attrs.level,
            total_gold(dungeon->player->items, dungeon->player->itemCount));

    printf("Seed: %zu\n", seed);

    return 0;
}
