#include <cstdlib>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

typedef std::string Key;
typedef std::string Value;

constexpr double load_factor = 0.5;
constexpr std::size_t default_table_size = 4;

std::size_t hash_function(Key input)
{return std::hash<Key>{}(input);}

constexpr std::size_t (*default_hash_function)(Key input) = &hash_function;

struct Item
{
    Key key;
    Value value;
    bool is_tombstone;

    Item(Key key, Value value) : key(key), value(value), is_tombstone(false)
    {};
};

static const Item is_tombstone = Item("", "");

struct Ht
{
    std::size_t (*hash)(Key input) = nullptr;

    std::size_t size = 0;
    unsigned item_count = 0;
    Item **items = nullptr;

    Ht()
    {};
};

Ht *create(std::size_t size, std::size_t (*hash)(Key input));

Ht *create();

void destroy(Ht *&ht);
// void clear(Ht *&ht);
// void set_hash_function(Ht *&ht, std::size_t (*hash)(std::string input));

void throw_nonexistent(Ht *&ht, std::size_t key_hash, Key key);

Value get(Ht *&ht, Key key);

Item* find(Ht *&ht, Key key);

void insert(Ht *&ht, Key key, Value value);

void remove(Ht *&ht, Key key);

void inserts(Ht *&ht, std::string *keys, std::string *values, int number);
// void removes(Ht *&ht, std::string *keys);

void resize_table(Ht *&ht, const double resize_factor);

void extend_table(Ht *&ht);

void shrink_table(Ht *&ht);

void print_table(Ht *&ht);

int main()
{
    Ht *ht = create(4, &hash_function);

    std::string keys[] = {"key1", "key2", "key3"};
    std::string values[] = {"value1", "value2", "value3"};
    inserts(ht, keys, values, 3);
    print_table(ht);

    remove(ht, "key1");
    remove(ht, "key2");
    remove(ht, "key3");

    print_table(ht);

    std::cout << get(ht, "key1") << std::endl;
    std::cout << get(ht, "key2") << std::endl;
//

#ifdef BAD_ACCESS_TEST
    std::cout << get(ht, "very bad (very nonexistent) key") << std::endl;
#endif

    destroy(ht);

    return 0;
}

Ht *create(std::size_t size, std::size_t (*hash)(Key input))
{
    Ht *ht = new Ht;
    ht->size = size;
    ht->hash = hash;
    ht->items = new Item *[size];

    for (std::size_t item = 0; item < size; item++)
        ht->items[item] = nullptr;

    return ht;
}

Ht *create()
{return create(default_table_size, default_hash_function);}

void destroy(Ht *&ht)
{
    delete[] ht->items;
    delete ht;
}

void insert(Ht *&ht, Key key, Value value)
{
    Item *item = new Item(key, value);

    if (ht->item_count >= ht->size * load_factor)
        extend_table(ht);

    std::size_t key_hash = ht->hash(item->key) % ht->size;

    while (ht->items[key_hash] != nullptr)
    {
        if (ht->items[key_hash]->is_tombstone == true)
            break;

        key_hash++;
        if (key_hash == ht->size)
            key_hash -= ht->size;
    }

    ht->items[key_hash] = item;
    ht->item_count++;
}

void inserts(Ht *&ht, std::string *keys, std::string *values, int number)
{
    for (int i = 0; i < number; ++i)
    {
        insert(ht, keys[i], values[i]);
    }
}

void throw_nonexistent(Ht *&ht, std::size_t key_hash, Key key)
{
    if (ht->items[key_hash] == nullptr)
        throw std::out_of_range("There is no element associated with key `" +
                                std::string(key) + "'.");
}

Value get(Ht *&ht, Key key)
{
    Item *item = find(ht, key);
    if (item != nullptr)
        return item->value;
    else return "";
}

Item* find(Ht *&ht, Key key)
{
    std::size_t key_hash = ht->hash(key) % ht->size;

//    throw_nonexistent(ht, key_hash, key);

    while (ht->items[key_hash] != nullptr)
    {
        if (ht->items[key_hash]->key == key)
            return ht->items[key_hash];

        key_hash++;
        if (key_hash == ht->size)
            key_hash -= ht->size;

//        throw_nonexistent(ht, key_hash, key);
    }

    return nullptr;
}

void remove(Ht *&ht, Key key)
{
    if (ht->item_count <= ht->size * (load_factor * 0.5))
        shrink_table(ht);

    std::size_t key_hash = ht->hash(key) % ht->size;

    if (ht->items[key_hash] == nullptr)
        return;

    while (ht->items[key_hash]->key != key or
           ht->items[key_hash]->is_tombstone)
    {
        key_hash++;
        if (key_hash == ht->size)
            key_hash -= ht->size;
        if (ht->items[key_hash] == nullptr)
            return;
    }

    ht->items[key_hash]->is_tombstone = true;
    ht->item_count--;
}

void resize_table(Ht *&ht, const double resize_factor)
{
    Ht *new_ht = create(ht->size * resize_factor, ht->hash);

    for (std::size_t item_i = 0; item_i < ht->size; item_i++)
    {
        Item *item = ht->items[item_i];

        if (item != nullptr)
            if (!item->is_tombstone)
                insert(new_ht, item->key, item->value);
    }

    destroy(ht);
    ht = new_ht;
}

void shrink_table(Ht *&ht)
{resize_table(ht, 0.5);}

void extend_table(Ht *&ht)
{resize_table(ht, 2);}

void print_table(Ht *&ht)
{
    bool is_empty = true;

    std::cout << "Table size: " << ht->size
              << ", item count: " << ht->item_count << std::endl;

    if (ht->item_count == 0)
    {
        std::cout << "Empty!" << std::endl << std::endl;
        return;
    }

    for (std::size_t item = 0; item < ht->size; item++)
        if (ht->items[item] != nullptr)
            if (!ht->items[item]->is_tombstone)
                std::cout << "Key: `" << ht->items[item]->key << "', value: `"
                          << ht->items[item]->value
                          << "', calculated position: "
                          << ht->hash(ht->items[item]->key) % ht->size
                          << ", real position: " << item << std::endl;

    std::cout << std::endl;
}
