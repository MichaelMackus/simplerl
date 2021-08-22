#include "game.h"
#include "message.h"
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

int hasColor;

typedef struct DrawTile {
    char symbol;
    int colorPair; // for curses color
    int attr;              // bold, etc.
} DrawTile;

#define COLOR_PAIR_DEFAULT 1
#define COLOR_PAIR_GREEN   2
#define COLOR_PAIR_BROWN   3
#define COLOR_PAIR_YELLOW  4
#define COLOR_PAIR_BLACK   5
#define COLOR_PAIR_PURPLE  6

// temporary global to hold previous map state, so we only draw what was changed
DrawTile drawBuffer[MAX_HEIGHT][MAX_WIDTH];

int init(int enableColor)
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

    hasColor = enableColor ? has_colors() : 0;
    if (hasColor) {
        start_color();
        use_default_colors();
        init_pair(COLOR_PAIR_DEFAULT, -1,            -1);
        init_pair(COLOR_PAIR_GREEN,   COLOR_GREEN,   -1);
        init_pair(COLOR_PAIR_BROWN,   COLOR_RED,     -1);
        init_pair(COLOR_PAIR_YELLOW,  COLOR_YELLOW,  -1);
        init_pair(COLOR_PAIR_BLACK,   COLOR_BLACK,   -1);
        init_pair(COLOR_PAIR_PURPLE,  COLOR_MAGENTA, -1);
    }

    return 1;
}

void deinit()
{
    endwin();             /* End curses mode          */
}

void render_messages();
void render_message(const char *message, int y, int x); // TODO use this for other messages
void draw(const DrawTile drawBuffer[][MAX_WIDTH], const DrawTile prevDrawBuffer[][MAX_WIDTH]);
void draw_status(const Dungeon *dungeon);
DrawTile get_tile(Level *level, rl_coords coords);
void render(const Dungeon *dungeon)
{
    const Mob *player = dungeon->player;

    // store previous buffer for comparison
    DrawTile prevDrawBuffer[MAX_HEIGHT][MAX_WIDTH];
    memcpy(prevDrawBuffer, drawBuffer, sizeof(DrawTile) * MAX_HEIGHT * MAX_WIDTH);

    // render onto our current map
    for (int y = 0; y < MAX_HEIGHT; ++y)
    {
        for (int x = 0; x < MAX_WIDTH; ++x)
        {
            drawBuffer[y][x] = get_tile(dungeon->level, RL_XY(x, y));
        }
    }

    if (get_menu() && get_menu() != MENU_DIRECTION)
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
                            equipment.weapon->id == item->id)
                        strcpy(equippedStr, " (equipped)");
                    else if (item->type == ITEM_ARMOR &&
                            equipment.armor != NULL &&
                            equipment.armor->id == item->id)
                        strcpy(equippedStr, " (equipped)");
                    else
                        strcpy(equippedStr, "");

                    char sym;
                    if (i == 0) sym = '$'; // gold inventory symbol
                    else sym = item_menu_symbol(i - 1);

                    if (item->amount == 1)
                        snprintf(buffer, MAX_WIDTH + 1, "%c - %s%s",
                                sym,
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
    for (int i = 0; i < strlen(message); ++i) {
        DrawTile t = {0};
        t.symbol = message[i];
        t.attr = A_BOLD;
        t.colorPair = COLOR_PAIR_DEFAULT;
        drawBuffer[y][x + i] = t;
    }
}

// only draw difference in map to curses window
void draw(const DrawTile drawBuffer[][MAX_WIDTH], const DrawTile prevDrawBuffer[][MAX_WIDTH])
{
    for (int y = 0; y < MAX_HEIGHT; ++y)
    {
        for (int x = 0; x < MAX_WIDTH; ++x)
        {
            if (drawBuffer[y][x].symbol != prevDrawBuffer[y][x].symbol ||
                drawBuffer[y][x].attr != prevDrawBuffer[y][x].attr ||
                drawBuffer[y][x].colorPair != prevDrawBuffer[y][x].colorPair)
            {
                if (hasColor) {
                    attron(COLOR_PAIR(drawBuffer[y][x].colorPair));
                    if (drawBuffer[y][x].attr)
                        attron(drawBuffer[y][x].attr);
                }
                mvaddch(y, x, drawBuffer[y][x].symbol);
                if (hasColor) {
                    attroff(COLOR_PAIR(drawBuffer[y][x].colorPair));
                    if (drawBuffer[y][x].attr)
                        attroff(drawBuffer[y][x].attr);
                }
            }
        }
    }

    refresh();
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

void print_mob_list(Mob **mobs, int mobCount)
{
    char mobsIndexed[mobCount]; // array of symbols (currently symbol is mob ID)
    int amountKilled[mobCount];
    int amountIndexed = 0;
    for (int i = 0; i < mobCount; ++i) {
        Mob *m = mobs[i];
        if (m == NULL) return;

        // ensure mob hasn't already been indexed
        int j;
        for (j = 0; j < mobCount; ++j) {
            if (mobsIndexed[j] == m->symbol) break;
        }
        if (mobsIndexed[j] == m->symbol) break;

        // count mobs
        amountKilled[i] = 0;
        for (j = 0; j < mobCount; ++j) {
            if (mobs[j] == NULL) return;
            if (mobs[j]->symbol == m->symbol) ++amountKilled[i];
        }
        mobsIndexed[i] = m->symbol;
        ++amountIndexed;
    }

    for (int i = 0; i < amountIndexed; ++i)
    {
        Mob *mob = mobs[i];

        const char *name = mob_name(mob->symbol);
        if (amountKilled[i] == 1)
            printf("%d %s\n", amountKilled[i], name);
        else
            // simple plural check
            printf("%d %ss\n", amountKilled[i], name);
    }
}

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

int get_color(Level *level, rl_coords coords)
{
    if (!rl_in_map_bounds(level, coords)) return COLOR_PAIR_DEFAULT;

    /**
     * Item colors
     */
    Tile t = level->tiles[coords.y][coords.x];
    const Mob *mob = get_mob(level, coords);
    if (mob == NULL && t.items) {
        Item *i = rl_peek(t.items);

        if (i->type == ITEM_ARMOR) {
            switch (i->armor.material) {
                case MATERIAL_METAL:
                    return COLOR_PAIR_DEFAULT;
                case MATERIAL_LEATHER:
                    return COLOR_PAIR_BROWN;
                case MATERIAL_DRAGON:
                    return COLOR_PAIR_PURPLE;
            }
        }

        if (i->type == ITEM_WEAPON) {
            if (i->damage.type == WEAPON_BLUNT) {
                return COLOR_PAIR_BROWN;
            } else {
                return COLOR_PAIR_DEFAULT;
            }
        }
    }

    char symbol = get_symbol(level, coords);
    switch (symbol) {
        case 'g':
            return COLOR_PAIR_GREEN;
        case 'o':
        case '$':
            return COLOR_PAIR_YELLOW;
        case 'r':
        case '+':
            if (can_see(level->player->coords, coords, level->tiles))
                return COLOR_PAIR_BROWN;
            else
                return COLOR_PAIR_DEFAULT;
        case '*':
            return COLOR_PAIR_BLACK;
        case 'k':
            return COLOR_PAIR_PURPLE;

        default:
            return COLOR_PAIR_DEFAULT;
    }
}

DrawTile get_tile(Level *level, rl_coords coords)
{
    Mob *player = level->player;

    DrawTile t = {0};
    t.symbol = get_symbol(level, coords);
    t.colorPair = get_color(level, coords);

    if (t.colorPair != COLOR_PAIR_BROWN &&
        can_see(player->coords, coords, level->tiles))
    {
        t.attr = A_BOLD;
    }

    return t;
}
