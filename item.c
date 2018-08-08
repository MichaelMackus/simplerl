#include "item.h"
#include "random.h"

char item_symbol(int itemType)
{
    switch (itemType)
    {
        case ITEM_WEAPON:
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

char inventory_symbol(const Item *item, Items items)
{
    int start = (int) 'a';
    int max = (int) 'z' + 1;
    for (int i = 0; i < max; ++i)
    {
        Item *cur = items.content[i];
        if (item->name == cur->name)
            return (char) (start + i);
    }

    // invalid input
    return '\0';
}

int total_gold(Items items)
{
    int total = 0;
    for (int i = 0; i < items.count; ++i)
        if (items.content[i]->type == ITEM_GOLD)
            total += items.content[i]->amount;

    return total;
}

Item *generate_gold(int depth);
Item *generate_weapon(int depth);
Item *generate_armor(int depth);
Item *create_item(int depth, int type)
{
    if (type == ITEM_GOLD)
        return generate_gold(depth);
    else if (type == ITEM_WEAPON)
        return generate_weapon(depth);
    else if (type == ITEM_ARMOR)
        return generate_armor(depth);
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

Item *move_item(Item *item, Items *items)
{
    if (item == NULL)
        return NULL;

    size_t count = items ? items->count : 0;

    // search for item in list & increase count of item to simplify inventory management
    for (int i = 0; i < count; ++i)
        if (items->content[i]->name == item->name)
        {
            items->content[i]->amount += item->amount;
            free(item);

            return items->content[i];
        }

    if (insert_item(item, items) == 0)
        return NULL;

    return item;
}

Item *take_item(Items *items)
{
    if (items == NULL || items->count <= 0)
        return NULL;

    Item *item = items->content[items->count - 1];
    items->content[items->count - 1] = NULL;
    --items->count;

    return item;
}

int move_items(Items *src, Items *target)
{
    for (int i = 0; i < src->count; ++i)
    {
        if (move_item(src->content[i], target) != NULL)
        {
            src->content[i] = NULL;
        }
        else
        {
            // OOM error
            return 0;
        }
    }

    free(src->content);
    src->content = NULL;
    src->count = src->size = 0;

    return 1;
}

void free_items(Items items)
{
    if (items.content != NULL)
        for (int i = 0; i < items.count; ++i)
            if (items.content[i] != NULL)
                free(items.content[i]);
    free(items.content);
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

    // weapon defaults
    item->type = ITEM_ARMOR;
    item->amount = 1;
    item->armor.material = MATERIAL_METAL;

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

    // weapon defaults
    item->type = ITEM_WEAPON;
    item->amount = 1;
    item->damage.type = DAMAGE_SLASH;
    item->damage.min = 1;
    item->damage.twoHanded = 0;

    return item;
}

Item *club()
{
    Item *weapon = init_weapon();
    weapon->damage.max = 4;
    weapon->damage.type = DAMAGE_BLUNT;
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
    weapon->damage.type = DAMAGE_BLUNT;
    weapon->name = weapon->unknownName = "club";

    return weapon;
}

Item *quarterstaff()
{
    Item *weapon = init_weapon();
    weapon->damage.max = 6;
    weapon->damage.type = DAMAGE_BLUNT;
    weapon->damage.twoHanded = 1;
    weapon->name = weapon->unknownName = "quarterstaff";

    return weapon;
}

Item *long_sword()
{
    Item *weapon = init_weapon();
    weapon->damage.max = 8;
    weapon->name = weapon->unknownName = "long sword";

    return weapon;
}

Item *bastard_sword()
{
    Item *weapon = init_weapon();
    weapon->damage.max = 10;
    weapon->damage.twoHanded = 1;
    weapon->name = weapon->unknownName = "bastard sword";

    return weapon;
}

Item *flail()
{
    Item *weapon = init_weapon();
    weapon->damage.max = 8;
    weapon->damage.type = DAMAGE_BLUNT;
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
    weapon->damage.type = DAMAGE_SILVER;
    weapon->name = weapon->unknownName = "silver sword";

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
        if (percent <= 25)
            return club();
        else if (percent <= 50)
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
        else if (percent <= 35)
            return quarterstaff();
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
            return long_sword();
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
