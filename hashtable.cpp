#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>

struct Item
{
    std::string value;
    Item(std::string value) : value(value){};
};

struct Ht
{
    std::size_t (*hash)(std::string input) = nullptr;
    std::size_t size;
    Item **items = new Item *;

    Ht(){};
};

Ht *create(const std::size_t size, std::size_t (*hash)(std::string input));

void destroy(Ht *&ht);
// void clear(Ht *&ht);
// void set_hash_function(Ht *&ht, std::size_t (*hash)(std::string input));

Item *get_item(Ht *&ht, std::string key);
void insert_item(Ht *&ht, std::string key, Item *&item);
// void remove_item(Ht *&ht, std::string key);

// void insert_items(Ht *&ht, std::string *keys, Item **&items);
// void remove_items(Ht *&ht, std::string *keys);

std::size_t hash_function(std::string input);

int main() {
    Ht *ht = create(8, &hash_function);

    Item *item = new Item("valera");
    insert_item(ht, "sasha", item);

    std::cout << hash_function("sasha") % ht->size << std::endl;
    std::cout << get_item(ht, "sasha")->value << std::endl;

    destroy(ht);

    return 0;
}

std::size_t hash_function(std::string input) {
    return std::hash<std::string>{}(input);
}

Ht *create(const std::size_t size, std::size_t (*hash)(std::string input)) {
    Ht *ht   = new Ht;
    ht->size = size;
    ht->hash = hash;

    return ht;
}

void destroy(Ht *&ht) {
    delete ht->items;
    delete ht;
}

void insert_item(Ht *&ht, std::string key, Item *&item) {
    std::size_t key_hash = ht->hash(key) % ht->size;

    ht->items[key_hash] = item;
}

Item *get_item(Ht *&ht, std::string key) {
    std::size_t key_hash = ht->hash(key) % ht->size;

    return ht->items[key_hash];
}
