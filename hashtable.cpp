#include <cstdlib>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

typedef std::string Key;
typedef std::string Value;

constexpr double load_factor             = 0.5;
constexpr std::size_t default_table_size = 4;

std::size_t hash_function(Key input) { return std::hash<Key>{}(input); }

constexpr std::size_t (*default_hash_function)(Key input) = &hash_function;

struct Item
{
    Key key;
    Value value;
    bool is_tombstone;
    Item(Key key, Value value) : key(key), value(value), is_tombstone(false){};
};

static const Item is_tombstone = Item("", "");

struct Ht
{
    std::size_t (*hash)(Key input) = nullptr;
    std::size_t size               = 0;
    unsigned item_count            = 0;
    Item **items                   = nullptr;

    Ht(){};
};

Ht *create(std::size_t size, std::size_t (*hash)(Key input));
Ht *create();

void destroy(Ht *&ht);
// void clear(Ht *&ht);
// void set_hash_function(Ht *&ht, std::size_t (*hash)(std::string input));

void is_nonexistent(Ht *&ht, std::size_t key_hash);
Value get(Ht *&ht, Key key);
void insert(Ht *&ht, Key key, Value value);
void remove(Ht *&ht, Key key);

// void inserts(Ht *&ht, std::string *keys, Item **&items);
// void removes(Ht *&ht, std::string *keys);

void resize_table(Ht *&ht, const double resize_factor);
void extend_table(Ht *&ht);
void shrink_table(Ht *&ht);

void print_table(Ht *&ht);

int main() {
    Ht *ht = create(4, &hash_function);

    insert(ht, "sasha0", "valera0");
    insert(ht, "sasha1", "valera1");
    insert(ht, "sasha2", "valera2");
    insert(ht, "sasha3", "valera3");
    insert(ht, "sasha4", "valera4");
    insert(ht, "sasha5", "valera5");
    insert(ht, "sasha6", "valera6");
    insert(ht, "sasha7", "valera7");
    insert(ht, "sergey", "victor");
    insert(ht, "sergeysalfjsa dlfalfjasdlfklaflkdflafd jsdlfksa lfjs ldf",
           "victor");

    print_table(ht);
    extend_table(ht);
    print_table(ht);
    shrink_table(ht);
    print_table(ht);

    remove(ht, "sasha0");
    remove(ht, "sasha1");
    remove(ht, "sasha2");
    remove(ht, "sasha3");
    remove(ht, "sasha4");
    remove(ht, "sasha5");
    remove(ht, "sasha6");
    remove(ht, "sasha7");
    remove(ht, "sergey");
    remove(ht, "sergeysalfjsa dlfalfjasdlfklaflkdflafd jsdlfksa lfjs ldf");

    print_table(ht);

    destroy(ht);

    return 0;
}

Ht *create(std::size_t size, std::size_t (*hash)(Key input)) {
    Ht *ht    = new Ht;
    ht->size  = size;
    ht->hash  = hash;
    ht->items = new Item *[size];

    for (std::size_t item = 0; item < size; item++)
        ht->items[item] = nullptr;

    return ht;
}
Ht *create() { return create(default_table_size, default_hash_function); }

void destroy(Ht *&ht) {
    delete[] ht->items;
    delete ht;
}

void insert(Ht *&ht, Key key, Value value) {
    Item *item = new Item(key, value);

    if (ht->item_count >= ht->size * load_factor)
        extend_table(ht);

    std::size_t key_hash = ht->hash(item->key) % ht->size;

    while (ht->items[key_hash] != nullptr) {
        if (ht->items[key_hash]->is_tombstone == true)
            break;

        key_hash++;
        if (key_hash == ht->size)
            key_hash -= ht->size;
    }

    ht->items[key_hash] = item;
    ht->item_count++;
}

void is_nonexistent(Ht *&ht, std::size_t key_hash) {
    if (ht->items[key_hash] == nullptr)
        throw std::out_of_range(
            "There is no element associated with the provided key.");
}

Value get(Ht *&ht, Key key) {
    std::size_t key_hash = ht->hash(key) % ht->size;

    is_nonexistent(ht, key_hash);

    while (ht->items[key_hash]->key != key &&
           ht->items[key_hash]->is_tombstone) {
        key_hash++;
        if (key_hash == ht->size)
            key_hash -= ht->size;

        is_nonexistent(ht, key_hash);
    }

    return ht->items[key_hash]->value;
}

void remove(Ht *&ht, Key key) {
    if (ht->item_count <= ht->size * (load_factor * 0.5))
        shrink_table(ht);

    std::size_t key_hash = ht->hash(key) % ht->size;

    if (ht->items[key_hash] == nullptr)
        return;

    while (ht->items[key_hash]->key != key &&
           ht->items[key_hash]->is_tombstone) {
        key_hash++;
        if (key_hash == ht->size)
            key_hash -= ht->size;
    }

    ht->items[key_hash]->is_tombstone = true;
    ht->item_count--;
}

void resize_table(Ht *&ht, const double resize_factor) {
    Ht *new_ht = create(ht->size * resize_factor, ht->hash);

    for (std::size_t item_i = 0; item_i < ht->size; item_i++) {
        Item *item = ht->items[item_i];

        if (item != nullptr)
            if (!item->is_tombstone)
                insert(new_ht, item->key, item->value);
    }

    destroy(ht);
    ht = new_ht;
}

void shrink_table(Ht *&ht) { resize_table(ht, 0.5); }
void extend_table(Ht *&ht) { resize_table(ht, 2); }

void print_table(Ht *&ht) {
    bool is_empty = true;

    std::cout << "Table size: " << ht->size
              << ", item count: " << ht->item_count << std::endl;

    if (ht->item_count == 0) {
        std::cout << "Empty!" << std::endl << std::endl;
        return;
    }

    for (std::size_t item = 0; item < ht->size; item++)
        if (ht->items[item] != nullptr)
            std::cout << "Key: `" << ht->items[item]->key << "', value: `"
                      << ht->items[item]->value << "', calculated position: "
                      << ht->hash(ht->items[item]->key) % ht->size
                      << ", real position: " << item << std::endl;

    std::cout << std::endl;
}
