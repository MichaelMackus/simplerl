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

void render_mob(const Mob *mob);
void render_messages();
void render_message(const char *message, int y, int x); // TODO use this for other messages
void render_level(Level *level, const Mob *player);
void draw(const char drawBuffer[][MAX_WIDTH], const char prevDrawBuffer[][MAX_WIDTH]);
void draw_status(const Dungeon *dungeon);
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
        if (mob != NULL && can_see(player->coords, mob->coords, dungeon->level->tiles))
            render_mob(mob);
    }

    // render player last (should always be seen)
    render_mob(dungeon->player);

    if (get_menu())
    {
        if (player->items.count)
        {
            Equipment equipment = player->equipment;

            // render inventory, TODO paginate
            char *buffer = malloc(sizeof(char) * (MAX_WIDTH + 1)); // width + 1 for null byte
            char equippedStr[MAX_WIDTH + 1];
            int y = 0;
            if (buffer != NULL)
                for (int i = 0; i < player->items.count; ++i)
                {
                    Item *item = player->items.content[i];

                    if (item->type == ITEM_GOLD)
                        continue;

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

                    if (item->amount == 1)
                        snprintf(buffer, MAX_WIDTH + 1, "%c - %d %s%s",
                                inventory_symbol(item, player->items),
                                item->amount,
                                item->name,
                                equippedStr);
                    else // pluralize
                        snprintf(buffer, MAX_WIDTH + 1, "%c - %d %ss%s",
                                inventory_symbol(item, player->items),
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

void render_level(Level *level, const Mob *player)
{
    // draw tiles array
    for (int y = 0; y < MAX_HEIGHT; ++y)
    {
        for (int x = 0; x < MAX_WIDTH; ++x)
        {
            // draw tile symbol if the player can see it
            if (can_see(player->coords, xy(x, y), level->tiles) ||
                    level->tiles[y][x].seen)
            {
                Tile t = level->tiles[y][x];
                if (t.items.count > 0)
                    // draw top item
                    drawBuffer[y][x] = item_symbol(t.items.content[t.items.count - 1]->type);
                else
                    drawBuffer[y][x] = tile_symbol(t);
            }
            else
                drawBuffer[y][x] = tile_symbol(create_tile(TILE_NONE));
        }
    }
}

void render_mob(const Mob *mob)
{
    drawBuffer[mob->coords.y][mob->coords.x] = mob->symbol;
}

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
    sprintf(status, "Gold: %d\n", total_gold(dungeon->level->player->items));
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

void print_mob_list(Mobs mobs)
{
    // mob counts, indexed by the ASCII code of the mob symbol for now
    int maxCount = (int) 'z' + 1;
    int count[maxCount];

    // reset the array to all zeros
    memset(count, 0, sizeof(int)*maxCount);

    for (int i = 0; i < mobs.count; ++i)
    {
        Mob *mob = mobs.content[i];
        if (mob == NULL)
            continue;

        // increment count based on ascii code of mob
        int code = (int) mob->symbol;
        ++count[code];
    }

    for (int i = 0; i < maxCount; ++i)
        if (count[i] > 0)
        {
            const char *name = mob_name((char) i);
            if (count[i] == 1)
                printf("%d %s\n", count[i], name);
            else
                // simple plural check
                printf("%d %ss\n", count[i], name);
        }
}
