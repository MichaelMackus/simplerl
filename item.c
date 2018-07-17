#include "item.h"
#include "random.h"

Item *generate_gold(int depth);
Item *generate_weapon(int depth);
Item *create_item(int depth, int type)
{
    if (type == ITEM_GOLD)
        return generate_gold(depth);
    else if (type == ITEM_WEAPON)
        return generate_weapon(depth);
    // TODO rest of item types

    return NULL;
}

#define ITEM_SIZE_INCREMENT 10

Items initialize_items()
{
    Items items;
    items.content = NULL;
    items.size = items.count = 0;

    return items;
}

int insert_item(Item *item, Items *items)
{
    if (item == NULL)
        return 0;

    size_t count = items ? items->count : 0;
    size_t size = items ? items->size : 0;

    if (size <= count)
    {
        Item **tmp = realloc(items->content, sizeof(Item*) * (size + ITEM_SIZE_INCREMENT));

        if (tmp == NULL)
            return 0;

        items->content = tmp;
        items->size += ITEM_SIZE_INCREMENT;
    }

    items->content[count] = item;
    ++items->count;

    return 1;
}


// type-specific item generators

Item *generate_gold(int depth)
{
    Item *item = malloc(sizeof(Item));

    if (item == NULL)
        return NULL;

    // generate depth*50/2 - depth*50 gold
    item->amount = generate(depth*50/2, depth*50);
    item->type = ITEM_GOLD;
    item->name = item->unknownName = "Gold";

    return item;
}


// weapons

Item *club(Item *weapon)
{
    weapon->damage.max = 4;
    weapon->damage.type = DAMAGE_BLUNT;
    weapon->name = weapon->unknownName = "club";
    return weapon;
}

Item *dagger(Item *weapon)
{
    weapon->damage.max = 4;
    weapon->name = weapon->unknownName = "dagger";
    return weapon;
}

Item *short_sword(Item *weapon)
{
    weapon->damage.max = 6;
    weapon->name = weapon->unknownName = "short sword";
    return weapon;
}

Item *mace(Item *weapon)
{
    weapon->damage.max = 6;
    weapon->damage.type = DAMAGE_BLUNT;
    weapon->name = weapon->unknownName = "club";
    return weapon;
}

Item *quarterstaff(Item *weapon)
{
    weapon->damage.max = 8;
    weapon->damage.type = DAMAGE_BLUNT;
    weapon->damage.twoHanded = 1;
    weapon->name = weapon->unknownName = "quarterstaff";
    return weapon;
}

Item *long_sword(Item *weapon)
{
    weapon->damage.max = 8;
    weapon->name = weapon->unknownName = "long sword";
    return weapon;
}

Item *bastard_sword(Item *weapon)
{
    weapon->damage.max = 10;
    weapon->damage.twoHanded = 1;
    weapon->name = weapon->unknownName = "bastard sword";
    return weapon;
}

Item *flail(Item *weapon)
{
    weapon->damage.max = 8;
    weapon->damage.type = DAMAGE_BLUNT;
    weapon->name = weapon->unknownName = "flail";
    return weapon;
}

Item *masterwork_sword(Item *weapon)
{
    weapon->damage.min = 2;
    weapon->damage.max = 10;
    weapon->name = weapon->unknownName = "masterwork sword";
    return weapon;
}

Item *masterwork_bastard_sword(Item *weapon)
{
    weapon->damage.min = 2;
    weapon->damage.max = 12;
    weapon->name = weapon->unknownName = "masterwork bastard sword";
    return weapon;
}

Item *silver_sword(Item *weapon)
{
    weapon->damage.min = 2;
    weapon->damage.max = 10;
    weapon->damage.type = DAMAGE_SILVER;
    weapon->name = weapon->unknownName = "silver sword";
    return weapon;
}


Item *generate_weapon(int depth)
{
    Item *item = malloc(sizeof(Item));

    if (item == NULL)
        return NULL;

    // weapon defaults
    item->type = ITEM_WEAPON;
    item->damage.type = DAMAGE_SLASH;
    item->damage.min = 1;
    item->damage.twoHanded = 0;

    // percentile roll
    int percent = generate(1, 100);

    // TODO artifact/unique weapons

    // generate different weapons based on depth
    if (depth <= 2)
    {
        if (percent <= 25)
            return club(item);
        else if (percent <= 50)
            return dagger(item);
        else if (percent <= 75)
            return short_sword(item);
        else
            return quarterstaff(item);
    }
    else if (depth <= 5)
    {
        if (percent <= 20)
            return dagger(item);
        else if (percent <= 35)
            return quarterstaff(item);
        else if (percent <= 50)
            return mace(item);
        else if (percent <= 75)
            return long_sword(item);
        else
            return bastard_sword(item);
    }
    else
    {
        if (percent <= 20)
            return long_sword(item);
        else if (percent <= 35)
            return bastard_sword(item);
        else if (percent <= 75)
            return flail(item);
        else if (percent <= 85)
            return masterwork_sword(item);
        else if (percent <= 95)
            return masterwork_bastard_sword(item);
        else
            return silver_sword(item);
    }

    free(item);

    return NULL;
}