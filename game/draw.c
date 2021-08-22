#include "game.h"
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

void render_messages();
void render_message(const char *message, int y, int x); // TODO use this for other messages
void draw(const char drawBuffer[][MAX_WIDTH], const char prevDrawBuffer[][MAX_WIDTH]);
void draw_status(const Dungeon *dungeon);
char get_symbol(Level *level, rl_coords coords);
void render(const Dungeon *dungeon)
{
    const Mob *player = dungeon->player;

    // fill previous map
    char prevDrawBuffer[MAX_HEIGHT][MAX_WIDTH];
    for (int y = 0; y < MAX_HEIGHT; ++y)
        for (int x = 0; x < MAX_WIDTH; ++x)
            prevDrawBuffer[y][x] = drawBuffer[y][x];

    // render onto our current map
    for (int y = 0; y < MAX_HEIGHT; ++y)
    {
        for (int x = 0; x < MAX_WIDTH; ++x)
        {
            drawBuffer[y][x] = get_symbol(dungeon->level, RL_XY(x, y));
        }
    }

    if (get_menu())
    {
        if (player->itemCount)
        {
            Equipment equipment = player->equipment;

            // render inventory
            char *buffer = malloc(sizeof(char) * (MAX_WIDTH + 1)); // width + 1 for null byte
            char equippedStr[MAX_WIDTH + 1];
            int y = 0;
            if (buffer != NULL)
                for (int i = 0; i < player->itemCount; ++i)
                {
                    Item *item = player->items[i];

                    if (item->type == ITEM_WEAPON &&
                            equipment.weapon != NULL &&
                            equipment.weapon->name == item->name)
                        strcpy(equippedStr, " (equipped)");
                    else if (item->type == ITEM_ARMOR &&
                            equipment.armor != NULL &&
                            equipment.armor->name == item->name)
                        strcpy(equippedStr, " (equipped)");
                    else
                        strcpy(equippedStr, "");

                    char sym;
                    if (i == 0) sym = '$'; // gold inventory symbol
                    else sym = item_menu_symbol(i - 1);

                    if (item->amount == 1)
                        snprintf(buffer, MAX_WIDTH + 1, "%c - %d %s%s",
                                sym,
                                item->amount,
                                item->name,
                                equippedStr);
                    else // pluralize
                        if (item->pluralName)
                            snprintf(buffer, MAX_WIDTH + 1, "%c - %d %s%s",
                                    sym,
                                    item->amount,
                                    item->pluralName,
                                    equippedStr);
                        else
                            snprintf(buffer, MAX_WIDTH + 1, "%c - %d %ss%s",
                                    sym,
                                    item->amount,
                                    item->name,
                                    equippedStr);

                    render_message((const char*) buffer, y++, 0);
                }
            free(buffer);
        }
        else
            render_message("No items in inventory.", 0, 0);
    }

    // draw difference from old map & new map
    draw(drawBuffer, prevDrawBuffer);

    // draw status area & messages
    draw_status(dungeon);
}

/*************/
/**         **/
/** private **/
/**         **/
/*************/

void render_message(const char *message, int y, int x)
{
    for (int i = 0; i < strlen(message); ++i)
        drawBuffer[y][x + i] = message[i];
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
void draw_status(const Dungeon *dungeon)
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
    sprintf(status, "Gold: %d\n", total_gold(dungeon->level->player->items, dungeon->level->player->itemCount));
    addstr(status);

    // re-render messages
    for (int y = 0; y < MAX_MESSAGES; ++y)
    {
        if (get_message(y) != NULL)
        {
            mvaddstr(y + MAX_HEIGHT, MAX_WIDTH / 2, get_message(y));
        }
    }

    refresh();
}

/* void print_mob_list(Mobs mobs) */
/* { */
/*     // mob counts, indexed by the ASCII code of the mob symbol for now */
/*     int maxCount = (int) 'z' + 1; */
/*     int count[maxCount]; */

/*     // reset the array to all zeros */
/*     memset(count, 0, sizeof(int)*maxCount); */

/*     for (int i = 0; i < mobs.count; ++i) */
/*     { */
/*         Mob *mob = mobs.content[i]; */
/*         if (mob == NULL) */
/*             continue; */

/*         // increment count based on ascii code of mob */
/*         int code = (int) mob->symbol; */
/*         ++count[code]; */
/*     } */

/*     for (int i = 0; i < maxCount; ++i) */
/*         if (count[i] > 0) */
/*         { */
/*             const char *name = mob_name((char) i); */
/*             if (count[i] == 1) */
/*                 printf("%d %s\n", count[i], name); */
/*             else */
/*                 // simple plural check */
/*                 printf("%d %ss\n", count[i], name); */
/*         } */
/* } */

char get_symbol(Level *level, rl_coords coords)
{
    rl_map *map = level->map;
    Mob *player = level->player;
    int x = coords.x, y = coords.y;

    if (!rl_in_map_bounds(map, coords) ||
            (!can_see(player->coords, coords, level->tiles) &&
             !level->tiles[y][x].seen))
    {
        return ' ';
    }

    /**
     * Monster symbol
     */
    for (int i = 0; i < MAX_MOBS; ++i)
    {
        const Mob *mob = get_mob(level, coords);
        if (mob != NULL && can_see(player->coords, mob->coords, level->tiles))
            return mob->symbol;
    }

    /**
     * Item symbol
     */
    Tile t = level->tiles[y][x];
    if (t.items) {
        Item *i = rl_peek(t.items);

        return item_symbol(i->type);
    }

    /**
     * Stairs symbol
     */
    if (level->upstair_loc.x == x && level->upstair_loc.y == y)
        return '<';
    if (level->downstair_loc.x == x && level->downstair_loc.y == y)
        return '>';

    /**
     * type symbol
     */
    rl_tile type = rl_get_tile(map, coords);
    if (type == RL_TILE_ROOM)    return '.';
    if (type == RL_TILE_PASSAGE) return '#';
    if (type == RL_TILE_DOORWAY) return '+';
    if (type == RL_TILE_WALL)
    {
        // show different char depending on side of wall
        char connections = rl_wall_connections(map, coords) | rl_door_connections(map, coords);

        if (connections & RL_CONNECTION_R || connections & RL_CONNECTION_L)
            return '-';
        else if (connections & RL_CONNECTION_U || connections & RL_CONNECTION_D)
            return '|';
        else
            return '0';
    }

    return ' ';
}
