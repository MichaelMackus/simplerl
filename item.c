#include "item.h"
#include "random.h"

Item *create_item(int depth, int type, const char **unknownItems, const char **knownItems)
{
    Item *item = malloc(sizeof(Item));

    if (item == NULL)
        return NULL;

    item->type = type;

    if (type == ITEM_GOLD)
    {
        // generate depth*50/2 - depth*50 gold
        int amount = generate(depth*50/2, depth*50);
        item->strength = amount;
        item->unknownName = item->name = "Gold";
    }
    // TODO rest of item types
    else
    {
        // error case
        free(item);

        return NULL;
    }

    return item;
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
