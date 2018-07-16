#include "draw.h"
#include "message.h"
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

int init()
{
    initscr();            /* Start curses mode         */
    raw();                /* Line buffering disabled    */
    keypad(stdscr, TRUE); /* We get F1, F2 etc..        */
    noecho();             /* Don't echo() while we do getch */
    curs_set(0);          /* hide cursor */

    int mx, my;
    getmaxyx(stdscr, my, mx);
    if (mx < MAX_WIDTH || my < MAX_HEIGHT)
        return 0;

    return 1;
}

void deinit()
{
    endwin();             /* End curses mode          */
}

// temporary global to hold previous map state, so we only draw what was changed
// THANKS curses!
char drawBuffer[MAX_HEIGHT][MAX_WIDTH];

void render_mob(const Mob *mob);
void render_messages();
void render_level(Level *level, const Mob *player);
void draw(const char drawBuffer[][MAX_WIDTH], const char prevDrawBuffer[][MAX_WIDTH]);
void drawStatus(const Dungeon *dungeon);
void render(const Dungeon *dungeon)
{
    const Mob *player = dungeon->player;

    // fill previous map
    char prevDrawBuffer[MAX_HEIGHT][MAX_WIDTH];
    for (int y = 0; y < MAX_HEIGHT; ++y)
        for (int x = 0; x < MAX_WIDTH; ++x)
            prevDrawBuffer[y][x] = drawBuffer[y][x];

    // render onto our current map
    render_level(dungeon->level, player);

    // render mobs
    for (int i = 0; i < MAX_MOBS; ++i)
    {
        const Mob *mob = dungeon->level->mobs[i];
        if (mob != NULL && can_see(player, mob->coords, dungeon->level->tiles))
            render_mob(mob);
    }

    // render player last (should always be seen)
    render_mob(dungeon->player);

    // draw difference from old map & new map, and messages
    draw(drawBuffer, prevDrawBuffer);

    // draw status area & messages
    drawStatus(dungeon);
}

/*************/
/**         **/
/** private **/
/**         **/
/*************/

void render_level(Level *level, const Mob *player)
{
    // draw tiles array
    for (int y = 0; y < MAX_HEIGHT; ++y)
    {
        move(y+5, 0);
        for (int x = 0; x < MAX_WIDTH; ++x)
        {
            // draw tile symbol if the player can see it
            if (can_see(player, xy(x, y), level->tiles) ||
                    level->tiles[y][x].seen)
                drawBuffer[y][x] = tile_symbol(level->tiles[y][x]);
            else
                drawBuffer[y][x] = tile_symbol(create_tile(TILE_NONE));
        }
    }
}

void render_mob(const Mob *mob)
{
    drawBuffer[mob->coords.y][mob->coords.x] = mob->symbol;
}

// only draw difference in map to curses window
void draw(const char drawBuffer[][MAX_WIDTH], const char prevDrawBuffer[][MAX_WIDTH])
{
    for (int y = 0; y < MAX_HEIGHT; ++y)
    {
        for (int x = 0; x < MAX_WIDTH; ++x)
        {
            if (drawBuffer[y][x] != prevDrawBuffer[y][x])
            {
                mvaddch(y, x, drawBuffer[y][x]);
                refresh();
            }
        }
    }
}

// draw status
void drawStatus(const Dungeon *dungeon)
{
    // clear status area
    for (int y = 0; y < MAX_MESSAGES; ++y)
    {
        move(MAX_HEIGHT + y, 0);
        clrtoeol();
    }

    move(MAX_HEIGHT, 0);

    // re-render status area
    char *status;
    status = malloc(sizeof(char) * (MAX_WIDTH/2));
    sprintf(status, "HP: %d / %d\n", dungeon->player->hp, dungeon->player->maxHP);
    addstr(status);
    sprintf(status, "LVL: %d, EXP: %d\n", dungeon->player->attrs.level, dungeon->player->attrs.exp);
    addstr(status);
    sprintf(status, "Depth: %d\n", dungeon->level->depth);
    addstr(status);

    // re-render messages
    for (int y = 0; y < MAX_MESSAGES; ++y)
    {
        if (messages[y] != NULL)
        {
            mvaddstr(y + MAX_HEIGHT, MAX_WIDTH / 2, messages[y]);
        }
    }

    refresh();
}
