#include "game.h"
#include "message.h"
#include <stdlib.h>
#include <memory.h>
#include <ncurses.h>
#include <assert.h>

// shortcut to push to a non-pointer
#define RL_PUSH(heap, item) \
    if (heap == NULL) { \
        *(&heap) = rl_heap_create(1, NULL); \
        assert(heap); \
    } \
    rl_heap_insert(heap, item);

static int resting = 0; // 1 if player resting
static int inMenu = 0; // one of MENU consts if in menu
static Direction runDir = { 0, 0 }; // direction player is running

int get_menu() { return inMenu; }
int is_running() { return runDir.xdir != 0 || runDir.ydir != 0; }

int increase_depth(Dungeon *dungeon);
int decrease_depth(Dungeon *dungeon);
void move_player(Mob *player, RL_Point coords, Level *level);
void run_player(Mob *player, Direction dir, Level *level);
void tick(Dungeon *dungeon);
void tick_mobs(Level *level);
void cleanup(Dungeon *dungeon);
void menu_management(int input, Level *level);
int gameloop(Dungeon *dungeon, int input)
{
    Level *level = dungeon->level;
    Mob *player = dungeon->player;

    if (inMenu)
    {
        menu_management(input, level);
        input = '.'; // do nothing
    }

    if (resting || is_running())
        input = '.'; // do nothing

    switch (input)
    {
        const Mob *m;

        case 'Q':
            return GAME_QUIT;

        case KEY_LEFT:
        case 'h':
            move_player(player, RL_XY(player->coords.x - 1, player->coords.y), level);
            break;
        case KEY_RIGHT:
        case 'l':
            move_player(player, RL_XY(player->coords.x + 1, player->coords.y), level);
            break;
        case KEY_DOWN:
        case 'j':
            move_player(player, RL_XY(player->coords.x, player->coords.y + 1), level);
            break;
        case KEY_UP:
        case 'k':
            move_player(player, RL_XY(player->coords.x, player->coords.y - 1), level);
            break;

        case 'H':
            run_player(player, DIRECTION(-1, 0), level);
            break;
        case 'L':
            run_player(player, DIRECTION(1, 0), level);
            break;
        case 'J':
            run_player(player, DIRECTION(0, 1), level);
            break;
        case 'K':
            run_player(player, DIRECTION(0, -1), level);
            break;

        case ',':
        case 'g':
            // get all items from floor
            Item *item;
            while (item = rl_heap_pop(level->items[(int)player->coords.y][(int)player->coords.x]))
            {
                if (!give_mob_item(player, item)) {
                    // abort if player inventory is full
                    RL_PUSH(level->items[(int)player->coords.y][(int)player->coords.x], item);
                    break;
                }
            }

            break;

        case 'i':
            // open inventory menu
            inMenu = MENU_INVENTORY;
            break;

        case 'w':
            // open wield menu
            inMenu = MENU_WIELD;
            break;

        case 'W':
            // open wear menu
            inMenu = MENU_WEAR;
            break;

        case 'f':
            if (player->equipment.readied) {
                // already readied projectile
                message("Choose a direction");
                inMenu = MENU_DIRECTION;
            }
            break;
        case 't':
            // open throw menu
            inMenu = MENU_THROW;
            break;

        case 'q':
            // open quaff menu
            inMenu = MENU_QUAFF;
            break;

        case 'r':
            // open read menu
            inMenu = MENU_READ;
            break;

        case 'd':
            // open throw menu
            inMenu = MENU_DROP;
            break;

        case 'R':
            resting = 1;
            break;

        case '>': // check for downstair
            if (level->downstair_loc.x == player->coords.x && level->downstair_loc.y == player->coords.y)
            {
                if (dungeon->level->depth == MAX_LEVEL)
                    return GAME_WON;
                if (!increase_depth(dungeon))
                    return GAME_OOM;

                return GAME_PLAYING;
            }

            break;

        case '<': // check for upstair
            if (level->upstair_loc.x == player->coords.x && level->upstair_loc.y == player->coords.y)
            {
                if (dungeon->level->depth == 1)
                    return GAME_QUIT;
                if (!decrease_depth(dungeon))
                    return GAME_OOM;

                return GAME_PLAYING;
            }

            break;
    }

    // skip turn if we're still in the inventory menu
    if (inMenu)
        return GAME_PLAYING;

    // check for player death (i.e. damaged self)
    if (player->hp <= 0)
        return GAME_DEATH;

    // cleanup dead mobs
    cleanup(dungeon);

    // heal player, increase turn count, and decrement smell
    tick(dungeon);

    // update seen tiles (need to update before AI)
    rl_fov_calculate_for_map(level->map, player->coords, FOV_RADIUS, rl_distance_manhattan);

    // display message for item(s) on current tile
    RL_Heap *is = level->items[(int)player->coords.y][(int)player->coords.x];
    if (is) {
        int length = rl_heap_length(is);
        Item *item = rl_heap_peek(is);
        if (item && length > 0) {
            char *buffer = malloc(sizeof(char) * MAX_WIDTH + 1);
            if (item->amount == 1)
                snprintf(buffer, MAX_WIDTH + 1, "You see %s", item->name);
            else // pluralize
                if (item->pluralName)
                    snprintf(buffer, MAX_WIDTH + 1, "You see %d %s",
                            item->amount,
                            item->pluralName);
                else
                    snprintf(buffer, MAX_WIDTH + 1, "You see %d %ss",
                            item->amount,
                            item->name);
            if (length > 1)
                snprintf(buffer + strlen(buffer), MAX_WIDTH + 1 - strlen(buffer), " (and %d more items)", length - 1);
            message(buffer);
        }
    }

    // be a bit kind & handle mob AI only when *not* changing depth
    if (level->depth == dungeon->level->depth)
        tick_mobs(level);
    else
        // changed depth - get the new level
        level = dungeon->level;

    // check for player death
    if (player->hp <= 0)
        return GAME_DEATH;

    return GAME_PLAYING;
}

/*************/
/**         **/
/** private **/
/**         **/
/*************/

void move_player(Mob *player, RL_Point coords, Level *level)
{
    // first, check for mob
    Mob *target = get_mob(level, coords);

    if (target != NULL)
    {
        // there was an enemy there!
        int dmg = attack(player, target, player->equipment.weapon);

        if (dmg > 0)
            message("You hit the %s for %d damage!",
                    mob_name(target->symbol),
                    dmg);
        else if (dmg == 0)
            message("You missed the %s!",
                    mob_name(target->symbol));
    }
    else
    {
        // move & set the smell of the player again
        move_mob(player, coords, level);
        taint(player->coords, level);
    }
}

void run_player(Mob *player, Direction dir, Level *level)
{
    runDir = dir;
    move_player(player, RL_XY(player->coords.x + dir.xdir, player->coords.y + dir.ydir), level);
}

// mob AI & spawning
// TODO add simple mob movement (instead of just sitting there)
// TODO add simple sound AI (i.e. mob should be able to hear combat further than they can smell, and also remember that)
void tick_mob(Mob *mob, Level *level);
RL_Point smelliest(RL_Point coords, Level *level);
int can_smell(RL_Point coords, Level *level);
void tick_mobs(Level *level)
{
    for (int i = 0; i < MAX_MOBS; ++i)
        if (level->mobs[i] != NULL)
            tick_mob(level->mobs[i], level);

    // 1/10 chance of new mob every turn
    if (generate(1, 10) == 10)
    {
        // get random coordinates for new mob, must not be near player
        RL_Point coords = random_passable_coords(level);
        while (rl_map_is_visible(level->map, coords) ||
                can_smell(coords, level) ||
                (level->upstair_loc.x == coords.x && level->upstair_loc.y == coords.y) ||
                (level->downstair_loc.x == coords.x && level->downstair_loc.y == coords.y))
        {
            coords = random_passable_coords(level);
        }

        Mob *mob = create_mob(level->depth, coords);

        if (mob == NULL)
            return;

        if (!insert_mob(mob, level->mobs))
            free(mob);
    }
}

// returns -1 on move, 0 or more damage on attack
int move_or_attack(Mob *attacker, RL_Point coords, Level *level)
{
    // first, check for mob
    Mob *target = get_mob(level, coords);

    if (target != NULL)
        return attack(attacker, target, attacker->equipment.weapon);
    else
        move_mob(attacker, coords, level);

    return -1;
}

void tick_mob(Mob *mob, Level *level)
{
    Mob *player = level->player;

    if (rl_map_is_visible(level->map, mob->coords))
    {
        int dmg = -1;

        // advance mob towards player, if there is no enemy at target spot
        if (player->coords.x > mob->coords.x &&
                rl_map_is_passable(level->map, RL_XY(mob->coords.x + 1, mob->coords.y)) &&
                get_enemy(level, RL_XY(mob->coords.x + 1, mob->coords.y)) == NULL)
            dmg = move_or_attack(mob, RL_XY(mob->coords.x + 1, mob->coords.y), level);
        else if (player->coords.x < mob->coords.x &&
                rl_map_is_passable(level->map, RL_XY(mob->coords.x - 1, mob->coords.y)) &&
                get_enemy(level, RL_XY(mob->coords.x - 1, mob->coords.y)) == NULL)
            dmg = move_or_attack(mob, RL_XY(mob->coords.x - 1, mob->coords.y), level);
        else if (player->coords.y > mob->coords.y &&
                rl_map_is_passable(level->map, RL_XY(mob->coords.x, mob->coords.y + 1)) &&
                get_enemy(level, RL_XY(mob->coords.x, mob->coords.y + 1)) == NULL)
            dmg = move_or_attack(mob, RL_XY(mob->coords.x, mob->coords.y + 1), level);
        else if (player->coords.y < mob->coords.y &&
                rl_map_is_passable(level->map, RL_XY(mob->coords.x, mob->coords.y - 1)) &&
                get_enemy(level, RL_XY(mob->coords.x, mob->coords.y - 1)) == NULL)
            dmg = move_or_attack(mob, RL_XY(mob->coords.x, mob->coords.y - 1), level);
        else
            return;

        if (dmg > 0)
            message("You got hit by the %s for %d damage!",
                    mob_name(mob->symbol),
                    dmg);
        else if (dmg == 0)
            message("The %s missed!", mob_name(mob->symbol));
    }
    else
    {
        // if mob can't see, they can still smell the player (thanks NetHack!)
        if (can_smell(mob->coords, level))
        {
            RL_Point coords = smelliest(mob->coords, level);
            move_or_attack(mob, coords, level);
        }
    }
}

void reward_exp(Mob *player, Mob *mob)
{
    // calculate exp based on difficulty of mob
    player->attrs.exp += 100*mob->difficulty;

    // max level
    if (player->attrs.level == MAX_PLAYER_LEVEL)
        return;

    // level up condition
    if (player->attrs.exp >= player->attrs.expNext)
    {
        ++player->attrs.level;
        player->attrs.expNext = player->attrs.expNext + player->attrs.level*1000;
        // TODO figure out better stat system
        player->maxHP += 5;
        player->hp = player->maxHP;
    }
}

void tick(Dungeon *dungeon)
{
    Level *level = dungeon->level;
    Mob *player = level->player;

    // reduce smell on all tiles not on current player x, y
    for (int y = 0; y < MAX_HEIGHT; ++y)
        for (int x = 0; x < MAX_WIDTH; ++x)
            if (!(player->coords.x == x && player->coords.y == y) &&
                    level->smell[y][x] > 0)
                --level->smell[y][x];

    if (dungeon->turn % 10 == 0)
    {
        // heal player every 10 turns
        if (player->hp < player->maxHP)
            player->hp += 1;
    }

    ++dungeon->turn;
}

// cleanup dead mobs
void cleanup(Dungeon *dungeon)
{
    Level *level = dungeon->level;
    Mob *player = level->player;

    for (int i = 0; i < MAX_MOBS; ++i)
    {
        if (level->mobs[i] != NULL)
        {
            // kill mob if HP 0
            Mob *mob = level->mobs[i];
            if (mob->hp <= 0)
            {
                // transfer items & equipment to floor
                Item *item;
                for (int i=0; i<mob->itemCount; i++) {
                    item = mob->items[i];
                    RL_PUSH(level->items[(int)mob->coords.y][(int)mob->coords.x], item);
                }

                // add to killed mobs queue
                RL_PUSH(dungeon->killed, mob);

                // clear mob in level & reward exp
                reward_exp(player, mob);
                level->mobs[i] = NULL;

                message("The %s has died.", mob_name(mob->symbol));
            }
        }
    }
}

// change current depth to next level deep
// if there is no next level, create one
// return 0 on error
// TODO should probably remove smells when changing depth
int increase_depth(Dungeon *dungeon)
{
    if (dungeon->level->depth == MAX_LEVEL)
        return 0;

    if (dungeon->level->next == NULL)
    {
        // initialize next level
        Level *level;
        level = create_level(dungeon->level->depth + 1);

        // set our link relationship to next level
        dungeon->level->next = level;
        level->prev = dungeon->level;

        // randomly fill dungeon
        if (!init_level(level, dungeon->player))
            return 0;
    }

    // now we can update the current level
    dungeon->level = dungeon->level->next;

    // place player on upstair
    dungeon->player->coords = dungeon->level->upstair_loc;

    // update FOV
    rl_fov_calculate_for_map(dungeon->level->map, dungeon->player->coords, FOV_RADIUS, rl_distance_manhattan);

    return 1;
}

// change current depth to previous level
// return 0 on error
// TODO should probably remove smells when changing depth
int decrease_depth(Dungeon *dungeon)
{
    if (dungeon->level->prev == NULL)
        return 0;

    // set level to previous level
    dungeon->level = dungeon->level->prev;

    // place player on downstair
    dungeon->player->coords = dungeon->level->downstair_loc;

    return 1;
}

int handle_input(Dungeon *dungeon)
{
    Mob *player = dungeon->player;
    Level *level = dungeon->level;

    if (resting)
    {
        if (player->hp >= player->maxHP)
            resting = 0;

        // if player can see any mobs, reset resting flag
        for (int i = 0; i < MAX_MOBS; ++i)
            if (level->mobs[i] != NULL && rl_map_is_visible(level->map, level->mobs[i]->coords))
                resting = 0;

        // if we're still resting, don't handle input
        if (resting)
            return 0;
    }

    if (is_running())
    {
        Direction dir = runDir;
        RL_Point target = RL_XY(player->coords.x + dir.xdir, player->coords.y + dir.ydir);

        // if player can see any mobs, reset running flag
        for (int i = 0; i < MAX_MOBS; ++i)
            if (level->mobs[i] != NULL && rl_map_is_visible(level->map, level->mobs[i]->coords))
                runDir = DIRECTION(0, 0);

        if (!rl_map_is_passable(level->map, target) ||
                get_enemy(level, target) != NULL)
        {
            runDir = DIRECTION(0, 0);
        }

        // if we're still running, move the player and don't handle input
        if (is_running())
        {
            move_player(player, target, level);

            return 0;
        }
    }

    return 1;
}

// TODO allow mobs to smell diagonally
RL_Point smelliest(RL_Point coords, Level *level)
{
    RL_Point smelliest = coords;
    int smell = 0;

    RL_Point nearby_coords[4] = {
        RL_XY(coords.x,     coords.y + 1),
        RL_XY(coords.x,     coords.y - 1),
        RL_XY(coords.x + 1, coords.y),
        RL_XY(coords.x - 1, coords.y),
    };

    for (int i = 0; i < 4; ++i) {
        if (!rl_map_is_passable(level->map, nearby_coords[i])) continue;
        Mob *mob = get_enemy(level, nearby_coords[i]);
        if (mob == NULL && level->smell[(int)nearby_coords[i].y][(int)nearby_coords[i].x] > smell) {
            smelliest = nearby_coords[i];
            smell = level->smell[(int)nearby_coords[i].y][(int)nearby_coords[i].x];
        }
    }

    return smelliest;
}

int can_smell(RL_Point coords, Level *level)
{
    RL_Point nearby_coords[4] = {
        RL_XY(coords.x,     coords.y + 1),
        RL_XY(coords.x,     coords.y - 1),
        RL_XY(coords.x + 1, coords.y),
        RL_XY(coords.x - 1, coords.y),
    };

    for (int i = 0; i < 4; ++i) {
        if (!rl_map_is_passable(level->map, nearby_coords[i])) continue;
        Mob *mob = get_enemy(level, nearby_coords[i]);
        int smell = level->smell[(int)nearby_coords[i].y][(int)nearby_coords[i].x];
        if (mob == NULL && smell)
            return 1;
    }

    return 0;
}

Mob *mob_in_dir(Level *level, Direction dir);
void apply_item_effects(Level *level, Mob *mob, Item *item);
void menu_management(int input, Level *level)
{
    Mob *player = level->player;

    if (inMenu == MENU_WIELD)
    {
        // wield chosen weapon
        for (int i = 0; i < player->itemCount; ++i)
        {
            Item *item = player->items[i];
            if (item_menu_symbol(i - 1) == input)
            {
                if (item->type == ITEM_WEAPON)
                    player->equipment.weapon = item;
                else
                    message("That is not a weapon!");

                break;
            }
        }
    }

    if (inMenu == MENU_WEAR)
    {
        // wear chosen armor
        for (int i = 0; i < player->itemCount; ++i)
        {
            Item *item = player->items[i];
            if (item_menu_symbol(i - 1) == input)
            {
                if (item->type == ITEM_ARMOR)
                    player->equipment.armor = item;
                else
                    message("That is not wearable!");

                break;
            }
        }
    }

    if (inMenu == MENU_DROP)
    {
        // drop chosen item
        for (int i = 0; i < player->itemCount; ++i)
        {
            Item *item = player->items[i];
            if (item_menu_symbol(i - 1) == input)
            {
                // transfer to ground tile
                if (remove_mob_item(player, item)) {
                    RL_PUSH(level->items[(int)player->coords.y][(int)player->coords.x], item);
                }

                break;
            }
        }
    }

    if (inMenu == MENU_QUAFF)
    {
        // quaff potion
        for (int i = 0; i < player->itemCount; ++i)
        {
            Item *item = player->items[i];
            if (item_menu_symbol(i - 1) == input)
            {
                if (item->type == ITEM_POTION)
                {
                    apply_item_effects(level, player, item);

                    decrement_mob_item(player, item);
                }
                else
                {
                    message("That is not drinkable!");
                }

                break;
            }
        }
    }

    if (inMenu == MENU_READ)
    {
        // read scroll
        for (int i = 0; i < player->itemCount; ++i)
        {
            Item *item = player->items[i];
            if (item_menu_symbol(i - 1) == input)
            {
                if (item->type == ITEM_SCROLL)
                {
                    apply_item_effects(level, player, item);
                    decrement_mob_item(player, item);
                }
                else
                {
                    message("That is not readable!");
                }

                break;
            }
        }
    }

    if (inMenu == MENU_THROW)
    {
        // throw chosen projectile
        for (int i = 0; i < player->itemCount; ++i)
        {
            Item *item = player->items[i];
            if (item_menu_symbol(i - 1) == input)
            {
                if (item->type == ITEM_WEAPON || item->type == ITEM_PROJECTILE || item->type == ITEM_POTION)
                {
                    player->equipment.readied = item; // ready item for throwing
                    message("Choose a direction");
                    inMenu = MENU_DIRECTION;

                    return;
                }
                else
                {
                    // TODO let them throw it anyway?
                    message("That is not throwable!");
                }

                break;
            }
        }
    }

    if (inMenu == MENU_DIRECTION)
    {
        Item *item = player->equipment.readied;
        Direction dir = {0};

        // throw chosen projectile in directions
        // TODO actually throw the projectile
        switch (input)
        {
            case KEY_LEFT:
            case 'h':
                decrement_mob_item(player, item);
                dir = DIRECTION(-1, 0);
                break;
            case KEY_RIGHT:
            case 'l':
                decrement_mob_item(player, item);
                dir = DIRECTION(1,  0);
                break;
            case KEY_DOWN:
            case 'j':
                decrement_mob_item(player, item);
                dir = DIRECTION(0,  1);
                break;
            case KEY_UP:
            case 'k':
                decrement_mob_item(player, item);
                dir = DIRECTION(0, -1);
                break;

            default:
                message("That is an invalid direction");
                break;
        }

        Mob *target = mob_in_dir(level, dir);
        if (target) {
            if (item->type == ITEM_POTION) {
                apply_item_effects(level, target, item);
            } else {
                int dmg = attack(level->player, target, item);

                if (dmg > 0)
                    message("You hit the %s for %d damage!",
                            mob_name(target->symbol),
                            dmg);
                else if (dmg == 0)
                    message("You missed the %s!",
                            mob_name(target->symbol));
                if (dmg < 0)
                    message("You healed the %s for %d damage!",
                            mob_name(target->symbol),
                            -1 * dmg);
            }
        }
    }

    // turn off inventory management
    inMenu = 0;
}

// get mob in dir from player
Mob *mob_in_dir(Level *level, Direction dir)
{
    if (dir.xdir == 0 && dir.ydir == 0) return NULL;

    Mob *player = level->player;
    for (int x = player->coords.x; x < MAX_WIDTH && x >= 0;) {
        for (int y = player->coords.y; y < MAX_HEIGHT && y >= 0;) {
            x += dir.xdir;
            y += dir.ydir;

            if (!rl_map_is_passable(level->map, RL_XY(x, y))) {
                return NULL;
            }

            Mob *m = get_enemy(level, RL_XY(x, y));
            if (m) {
                return m;
            }
        }
    }

    return NULL;
}

// apply effects of item invoked by mob
void apply_item_effects(Level *level, Mob *mob, Item *item)
{
    Mob *player = level->player;
    int dmg;
    if (item->type == ITEM_POTION) {
        switch (item->potion) {
            case POTION_ACID:
                dmg = generate(1, 8);
                mob->hp -= dmg;

                break;
            case POTION_HEAL:
                dmg = -1 * generate(1, 8);
                mob->hp -= dmg;
                if (mob->hp > mob->maxHP)
                    mob->hp = mob->maxHP;

                break;

            default:
                return;
        }

        if (mob->coords.x == player->coords.x && mob->coords.y == player->coords.y) {
            if (dmg > 0) {
                message("Ouch, that burns! You were hurt for %d damage.", dmg);
            } else if (dmg < 0) {
                message("That feels better! You were healed for %d damage.", -1 * dmg);
            }
        } else {
            if (dmg > 0) {
                message("You burned the %s for %d damage.", mob_name(mob->symbol), dmg);
            } else if (dmg < 0) {
                message("You healed the %s for %d damage.", mob_name(mob->symbol), -1 * dmg);
            }
        }
    }

    if (item->type == ITEM_SCROLL) {
        switch (item->scroll) {
            case SCROLL_FIRE:
                for (int i = 0; i < MAX_MOBS; ++i) {
                    Mob *m = level->mobs[i];
                    if (m && rl_map_is_visible(level->map, m->coords)) {
                        // damage mob
                        int dmg = generate(1, 8);
                        message("You scorched the %s for %d damage.", mob_name(m->symbol), dmg);
                        m->hp -= dmg;
                    }
                }

                break;
            case SCROLL_TELEPORT:
                message("You feel disoriented.");
                RL_Point coords = random_passable_coords(level);
                player->coords = coords;

            default:
                break;
        }
    }
}
