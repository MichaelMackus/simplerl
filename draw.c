#include "draw.h"
#include "message.h"
#include <ncurses.h>
#include <string.h>

int init()
{
    initscr();            /* Start curses mode         */
    raw();                /* Line buffering disabled    */
    keypad(stdscr, TRUE); /* We get F1, F2 etc..        */
    noecho();             /* Don't echo() while we do getch */
    curs_set(0);          /* hide cursor */

    return 0;
}

void deinit()
{
    endwin();             /* End curses mode          */
}

// temporary global to hold previous map state, so we only draw what was changed
// THANKS curses!
char map[MAX_HEIGHT + MAX_MESSAGES][MAX_WIDTH];

void render_mob(const Mob *mob);
void render_messages(const char **messages);
void render_level(Level *level);
void draw(const char map[][MAX_WIDTH], const char prevMap[][MAX_WIDTH]);
void update(const Dungeon *dungeon, const char **messages)
{
    // fill previous map
    char prevMap[MAX_HEIGHT + MAX_MESSAGES][MAX_WIDTH];
    for (int y = 0; y < MAX_HEIGHT + MAX_MESSAGES; ++y)
        for (int x = 0; x < MAX_WIDTH; ++x)
            prevMap[y][x] = map[y][x];

    // render onto our current map
    render_level(dungeon->level);
    render_mob(dungeon->player);
    render_messages(messages);

    // draw difference from old map & new map
    draw(map, prevMap);
}

/*************/
/**         **/
/** private **/
/**         **/
/*************/

void render_level(Level *level)
{
    // draw tiles array
    for (int y = 0; y < MAX_HEIGHT; ++y)
    {
        move(y+5, 0);
        for (int x = 0; x < MAX_WIDTH; ++x)
        {
            // draw character, offset y by 5 for message area
            map[y][x] = tile_symbol(level->tiles[y][x]);
        }
    }
}

void render_mob(const Mob *mob)
{
    map[mob->y][mob->x] = mob->symbol;
}

void render_messages(const char **messages)
{
    for (int y=0; y<MAX_MESSAGES; ++y)
    {
        for (int x = 0; x < MAX_MESSAGE_LENGTH; ++x)
        {
            if (messages[y] != NULL)
                map[y+MAX_HEIGHT][x] = messages[y][x];
        }
    }
}

// only draw difference in map to curses window
void draw(const char map[][MAX_WIDTH], const char prevMap[][MAX_WIDTH])
{
    for (int y = 0; y < MAX_HEIGHT + MAX_MESSAGES; ++y)
    {
        for (int x = 0; x < MAX_WIDTH; ++x)
        {
            if (map[y][x] != prevMap[y][x])
            {
                mvaddch(y, x, map[y][x]);
                refresh();
            }
        }
    }
}
