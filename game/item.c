#include <string.h>

#include "item.h"
#include "random.h"

/* static const char **unknownItems; */
/* static const char **knownItems; */

char item_symbol(int itemType)
{
    switch (itemType)
    {
        case ITEM_WEAPON:
        case ITEM_PROJECTILE:
            return ')';
        case ITEM_ARMOR:
            return '[';
        case ITEM_GOLD:
            return '$';
        case ITEM_SCROLL:
            return '?';
        case ITEM_POTION:
            return '!';
        default:
            return '{'; // something is obviously wrong
    }
}

char item_menu_symbol(int num)
{
    int start = (int) 'a';
    int max = (int) 'z' + 1;
    for (int i = 0; i < max; ++i)
    {
        if (num == i)
            return (char) (start + i);
    }

    // invalid input
    return '\0';
}

int total_gold(Item **items, int itemCount)
{
    int total = 0;
    for (int i = 0; i < itemCount; ++i)
        if (items[i]->type == ITEM_GOLD)
            total += items[i]->amount;

    return total;
}

int is_stackable(Item item)
{
    switch (item.type) {
        case ITEM_GOLD:
        case ITEM_PROJECTILE:
        case ITEM_POTION:
        case ITEM_SCROLL:
            return 1;

        case ITEM_WEAPON:
            if (strcmp(item.name, "dagger") == 0) return 1;

            break;
    }

    return 0;
}

int latestItemId = 0;

Item *generate_gold(int depth);
Item *generate_weapon(int depth);
Item *generate_armor(int depth);
Item *generate_potion(int depth);
Item *generate_scroll(int depth);
Item *create_item(int depth, int type)
{
    Item *i = NULL;

    if (type == ITEM_GOLD)
        i = generate_gold(depth);
    else if (type == ITEM_WEAPON)
        i = generate_weapon(depth);
    else if (type == ITEM_ARMOR)
        i = generate_armor(depth);
    else if (type == ITEM_POTION)
        i = generate_potion(depth);
    else if (type == ITEM_SCROLL)
        i = generate_scroll(depth);

    return i;
}

// type-specific item generators

Item *generate_gold(int depth)
{
    Item *item = malloc(sizeof(Item));

    if (item == NULL)
        return NULL;

    *item = (Item) {0};

    // generate depth*50/2 - depth*50 gold
    item->amount = generate(depth*50/2, depth*50);
    item->type = ITEM_GOLD;
    item->name = item->unknownName = "gold";
    item->pluralName = "gold";
    item->id = latestItemId++; // give item random ID

    return item;
}

Item *generate_potion(int depth)
{
    (void) depth; // currently unused
    Item *item = malloc(sizeof(Item));

    if (item == NULL)
        return NULL;

    *item = (Item) {0};
    item->amount = 1;
    item->type = ITEM_POTION;
    item->id = latestItemId++; // give item random ID

    if (generate(1, 100) < 50) {
        item->name = item->unknownName = "healing potion";
        item->pluralName = "healing potions";
        item->potion = POTION_HEAL;
    } else {
        item->name = item->unknownName = "acidic potion";
        item->pluralName = "acidic potions";
        item->potion = POTION_ACID;
    }

    return item;
}

Item *generate_scroll(int depth)
{
    (void) depth; // currently unused
    Item *item = malloc(sizeof(Item));

    if (item == NULL)
        return NULL;

    *item = (Item) {0};
    item->amount = 1;
    item->type = ITEM_SCROLL;
    item->id = latestItemId++; // give item random ID

    int r = generate(1, 100);
    if (r < 33) {
        item->name = item->unknownName = "scroll of fire";
        item->pluralName = "scrolls of fire";
        item->potion = SCROLL_FIRE;
    } else {
        item->name = item->unknownName = "scroll of teleportation";
        item->pluralName = "scrolls of teleportation";
        item->potion = SCROLL_TELEPORT;
    /* Not really useful until we generate random item names... */
    /* } else { */
    /*     item->name = item->unknownName = "scroll of identify"; */
    /*     item->pluralName = "scrolls of identify"; */
    /*     item->potion = SCROLL_IDENTIFY; */
    }

    return item;
}


/*************/
/**         **/
/**  armor  **/
/**         **/
/*************/

Item *init_armor()
{
    Item *item = malloc(sizeof(Item));

    if (item == NULL)
        return NULL;

    *item = (Item) {0};

    // weapon defaults
    item->type = ITEM_ARMOR;
    item->amount = 1;
    item->armor.material = MATERIAL_METAL;
    item->id = latestItemId++; // give item random ID

    return item;
}

Item *leather()
{
    Item *item = init_armor();
    item->name = item->unknownName = "leather armor";
    item->armor.damageReduction = 1;
    item->armor.material = MATERIAL_LEATHER;

    return item;
}

Item *ring_mail()
{
    Item *item = init_armor();
    item->name = item->unknownName = "ring mail";
    item->armor.damageReduction = 2;

    return item;
}

Item *splint_mail()
{
    Item *item = init_armor();
    item->name = item->unknownName = "splint mail";
    item->armor.damageReduction = 3;

    return item;
}

Item *plate_mail()
{
    Item *item = init_armor();
    item->name = item->unknownName = "plate mail";
    item->armor.damageReduction = 4;

    return item;
}

Item *full_plate()
{
    Item *item = init_armor();
    item->name = item->unknownName = "full plate";
    item->armor.damageReduction = 5;

    return item;
}

Item *dragon_hide()
{
    Item *item = init_armor();
    item->name = item->unknownName = "dragon hide";
    item->armor.damageReduction = 6;
    item->armor.material = MATERIAL_DRAGON;

    return item;
}

Item *dragon_plate()
{
    Item *item = init_armor();
    item->name = item->unknownName = "dragon plate";
    item->armor.damageReduction = 10;
    item->armor.material = MATERIAL_DRAGON;

    return item;
}

Item *generate_armor(int depth)
{
    // percentile roll
    int percent = generate(1, 100);

    // TODO probably want to actually generate dragonhide on dragons

    // generate different armor based on depth
    if (depth <= 2)
    {
        if (percent <= 75)
            return leather();
        else
            return ring_mail();
    }
    else if (depth <= 5)
    {
        if (percent <= 25)
            return leather();
        else if (percent <= 75)
            return ring_mail();
        else
            return splint_mail();
    }
    else
    {
        if (percent <= 20)
            return ring_mail();
        else if (percent <= 35)
            return splint_mail();
        else if (percent <= 75)
            return plate_mail();
        else if (percent <= 85)
            return full_plate();
        else if (percent <= 95)
            return dragon_hide();
        else
            return dragon_plate();
    }

    return NULL;
}


/*************/
/**         **/
/** weapons **/
/**         **/
/*************/

Item *init_weapon()
{
    Item *item = malloc(sizeof(Item));

    if (item == NULL)
        return NULL;

    *item = (Item) {0};

    // weapon defaults
    item->type = ITEM_WEAPON;
    item->amount = 1;
    item->damage.type = WEAPON_SLASH;
    item->damage.min = 1;
    item->damage.range = 5;
    item->id = latestItemId++; // give item random ID

    return item;
}

Item *club()
{
    Item *weapon = init_weapon();
    weapon->damage.max = 4;
    weapon->damage.type = WEAPON_BLUNT;
    weapon->name = weapon->unknownName = "club";

    return weapon;
}

Item *dagger()
{
    Item *weapon = init_weapon();
    weapon->damage.max = 4;
    weapon->name = weapon->unknownName = "dagger";

    return weapon;
}

Item *short_sword()
{
    Item *weapon = init_weapon();
    weapon->damage.max = 6;
    weapon->name = weapon->unknownName = "short sword";

    return weapon;
}

Item *mace()
{
    Item *weapon = init_weapon();
    weapon->damage.max = 6;
    weapon->damage.type = WEAPON_BLUNT;
    weapon->name = weapon->unknownName = "club";

    return weapon;
}

Item *quarterstaff()
{
    Item *weapon = init_weapon();
    weapon->damage.max = 6;
    weapon->damage.type = WEAPON_BLUNT & WEAPON_TWOHANDED;
    weapon->name = weapon->unknownName = "quarterstaff";
    weapon->pluralName = "quarterstaves";

    return weapon;
}

Item *long_sword()
{
    Item *weapon = init_weapon();
    weapon->damage.max = 8;
    weapon->damage.type &= WEAPON_TWOHANDED;
    weapon->name = weapon->unknownName = "long sword";

    return weapon;
}

Item *bastard_sword()
{
    Item *weapon = init_weapon();
    weapon->damage.max = 10;
    weapon->damage.type &= WEAPON_TWOHANDED;
    weapon->name = weapon->unknownName = "bastard sword";

    return weapon;
}

Item *flail()
{
    Item *weapon = init_weapon();
    weapon->damage.max = 8;
    weapon->damage.type = WEAPON_BLUNT;
    weapon->name = weapon->unknownName = "flail";

    return weapon;
}

Item *masterwork_sword()
{
    Item *weapon = init_weapon();
    weapon->damage.min = 2;
    weapon->damage.max = 10;
    weapon->name = weapon->unknownName = "masterwork sword";

    return weapon;
}

Item *masterwork_bastard_sword()
{
    Item *weapon = init_weapon();
    weapon->damage.min = 2;
    weapon->damage.max = 12;
    weapon->name = weapon->unknownName = "masterwork bastard sword";

    return weapon;
}

Item *silver_sword()
{
    Item *weapon = init_weapon();
    weapon->damage.min = 2;
    weapon->damage.max = 10;
    weapon->damage.type = WEAPON_SILVER;
    weapon->name = weapon->unknownName = "silver sword";

    return weapon;
}

Item *arrow()
{
    Item *weapon = init_weapon();
    weapon->type = ITEM_PROJECTILE;
    weapon->damage.min = 1;
    weapon->damage.max = 4;
    weapon->damage.type = WEAPON_PIERCE;
    weapon->damage.projectile = PROJECTILE_ARROW;
    weapon->name = weapon->unknownName = "arrow";

    return weapon;
}

Item *generate_weapon(int depth)
{
    // percentile roll
    int percent = generate(1, 100);

    // TODO artifact/unique weapons

    // generate different weapons based on depth
    if (depth <= 2)
    {
        if (percent <= 50)
            return dagger();
        else if (percent <= 75)
            return short_sword();
        else
            return quarterstaff();
    }
    else if (depth <= 5)
    {
        if (percent <= 20)
            return dagger();
        else if (percent <= 30)
            return arrow();
        else if (percent <= 50)
            return mace();
        else if (percent <= 75)
            return long_sword();
        else
            return bastard_sword();
    }
    else
    {
        if (percent <= 20)
            return arrow();
        else if (percent <= 35)
            return bastard_sword();
        else if (percent <= 75)
            return flail();
        else if (percent <= 85)
            return masterwork_sword();
        else if (percent <= 95)
            return masterwork_bastard_sword();
        else
            return silver_sword();
    }

    return NULL;
}
