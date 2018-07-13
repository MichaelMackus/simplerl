#include "draw.h"
#include "message.h"
#include <ncurses.h>

/*************/
/**         **/
/** public  **/
/**         **/
/*************/

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

void render_mob(const Mob *mob);
void render_messages(const char **messages);
void update(const Dungeon *dungeon, const char **messages)
{
    erase();
    render_mob(dungeon->player);
    render_messages(messages);
    refresh();
}

/*************/
/**         **/
/** private **/
/**         **/
/*************/

void render_mob(const Mob *mob)
{
    mvaddch(mob->y, mob->x, mob->symbol);
}

void render_messages(const char **messages)
{
    for (int y=0; y<MAX_MESSAGES; ++y)
    {
        mvaddstr(y, 0, messages[y]);
    }
}
