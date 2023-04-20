#include <cstdlib>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

typedef std::string Key;
typedef std::string Value;

constexpr double load_factor                 = 0.5;
constexpr std::size_t default_table_size     = 4;
constexpr std::size_t default_max_table_size = 8192;

std::size_t hash_function(Key input) { return std::hash<Key>{}(input); }

constexpr std::size_t (*default_hash_function)(Key input) = &hash_function;

struct Item
{
    Key key;
    Value value;
    bool is_tombstone;

    Item(Key key, Value value) : key(key), value(value), is_tombstone(false){};
};

struct KeyValue
{
    Key key;
    Value value;
    KeyValue(Key key, Value value) : key(key), value(value){};
    KeyValue(){};
};

static const Item tombstone = Item("", "");
// renamed because it's not a field

struct Ht
{
    std::size_t (*hash)(Key input) = nullptr;
    std::size_t size               = 0;
    std::size_t min_size           = 0;
    std::size_t max_size           = 0;
    unsigned item_count            = 0;
    Item **items                   = nullptr;

    Ht(){};
};

Ht *create(std::size_t size, std::size_t min_size, std::size_t max_size,
           std::size_t (*hash)(Key input));

Ht *create(std::size_t size, std::size_t (*hash)(Key input));

Ht *create();

void destroy(Ht *&ht);
void clear(Ht *&ht);

void set_hash_function(Ht *&ht, std::size_t (*hash)(std::string input));

void throw_nonexistent(Ht *&ht, std::size_t key_hash, Key key);

Value get(Ht *&ht, Key key);

Item *find(Ht *&ht, Key key);

void insert(Ht *&ht, Key key, Value value);

void remove(Ht *&ht, Key key);

void inserts(Ht *&ht, KeyValue *dict, size_t size);

void removes(Ht *&ht, KeyValue *dict, size_t size);

void resize_table(Ht *&ht, const double resize_factor);

void extend_table(Ht *&ht);

void shrink_table(Ht *&ht);

void clear(Ht *&ht);

void print_table(Ht *&ht);

int main() {
    Ht *ht = create(4, &hash_function);

    size_t dict_size = 11;
    KeyValue *dict   = new KeyValue[dict_size];
    dict[0]          = KeyValue("key0", "value0");
    dict[1]          = KeyValue("key1", "value1");
    dict[2]          = KeyValue("key2", "value2");
    dict[3]          = KeyValue("key3", "value3");
    dict[4]          = KeyValue("key4", "value4");
    dict[5]          = KeyValue("key5", "value5");
    dict[6]          = KeyValue("key6", "value6");
    dict[7]          = KeyValue("key7", "value7");
    dict[8]          = KeyValue("key8", "value8");
    dict[9]          = KeyValue("key9", "value9");
    dict[10]         = KeyValue("key10", "value10");

    print_table(ht);

    inserts(ht, dict, dict_size);

    print_table(ht);

    insert(ht, "key0", "value11231");

    print_table(ht);

    std::cout << get(ht, "key2") << std::endl;
    std::cout << get(ht, "key8") << std::endl;

    removes(ht, dict, dict_size);

    print_table(ht);

    // std::cout << get(ht, "key2") << std::endl;

    // remove(ht, "key2");
    // remove(ht, "key3");
    clear(ht);
    print_table(ht);

    //

#ifdef BAD_ACCESS_TEST
    std::cout << get(ht, "very bad (very nonexistent) key") << std::endl;
#endif

    destroy(ht);

    return 0;
}

Ht *create(std::size_t size, std::size_t min_size, std::size_t max_size,
           std::size_t (*hash)(Key input)) {
    Ht *ht       = new Ht;
    ht->size     = size;
    ht->max_size = max_size;
    ht->min_size = min_size;
    ht->hash     = hash;
    ht->items    = new Item *[size];

    for (std::size_t item = 0; item < size; item++)
        ht->items[item] = nullptr;

    return ht;
}

void set_hash_function(Ht *&ht, std::size_t (*hash)(std::string input)) {
    ht->hash = hash;
}

Ht *create(std::size_t size, std::size_t (*hash)(Key input)) {
    return create(size, size, default_max_table_size, hash);
}

Ht *create() { return create(default_table_size, default_hash_function); }

void removes(Ht *&ht, KeyValue *dict, size_t size) {
    for (int i = 0; i < size; ++i) {
        remove(ht, dict[i].key);
    }
}

void destroy(Ht *&ht) {
    delete[] ht->items;
    delete ht;
}

void insert(Ht *&ht, Key key, Value value) {
    Item *item = new Item(key, value);

    if (ht->item_count >= ht->size * load_factor)
        extend_table(ht);

    std::size_t key_hash = ht->hash(item->key) % ht->size;

    std::size_t found_position = key_hash;
    bool is_found_position     = false;

    while (ht->items[key_hash] != nullptr) {
        if (ht->items[key_hash]->key == key)
            return;

        if (ht->items[key_hash]->is_tombstone)
            is_found_position = true;

        key_hash++;
        if (key_hash == ht->size)
            key_hash -= ht->size;

        if (!is_found_position)
            found_position = key_hash;
    }

    ht->items[found_position] = item;
    ht->item_count++;
}

void inserts(Ht *&ht, KeyValue *dict, size_t size) {
    for (int i = 0; i < size; ++i) {
        insert(ht, dict[i].key, dict[i].value);
    }
}

void throw_nonexistent(Ht *&ht, std::size_t key_hash, Key key) {
    if (ht->items[key_hash] == nullptr)
        throw std::out_of_range("There is no element associated with key `" +
                                std::string(key) + "'.");
}

Value get(Ht *&ht, Key key) {
    Item *item           = find(ht, key);
    std::size_t key_hash = ht->hash(key) % ht->size;
    if (item == nullptr)
        throw_nonexistent(ht, key_hash, key);

    return item->value;
}

Item *find(Ht *&ht, Key key) {
    std::size_t key_hash = ht->hash(key) % ht->size;

    //        throw_nonexistent(ht, key_hash, key);

    while (ht->items[key_hash] != nullptr) {
        if (ht->items[key_hash]->key == key)
            return ht->items[key_hash];

        key_hash++;
        if (key_hash == ht->size)
            key_hash -= ht->size;
    }
    return nullptr;
}

void clear(Ht *&ht) {
    delete[] ht->items;
    ht->items = new Item *[ht->size];
    for (std::size_t item = 0; item < ht->size; item++)
        ht->items[item] = nullptr;
    return;
}

void remove(Ht *&ht, Key key) {
    if (ht->item_count <= ht->size * (load_factor * 0.5))
        shrink_table(ht);

    std::size_t key_hash = ht->hash(key) % ht->size;

    if (ht->items[key_hash] == nullptr)
        return;

    while (ht->items[key_hash]->key != key or
           ht->items[key_hash]->is_tombstone) {
        key_hash++;
        if (key_hash == ht->size)
            key_hash -= ht->size;
        if (ht->items[key_hash] == nullptr)
            return;
    }

    ht->items[key_hash]->is_tombstone = true;
    ht->item_count--;
}

void resize_table(Ht *&ht, const double resize_factor) {
    Ht *new_ht =
        create(ht->size * resize_factor, ht->min_size, ht->max_size, ht->hash);

    for (std::size_t item_i = 0; item_i < ht->size; item_i++) {
        Item *item = ht->items[item_i];

        if (item != nullptr)
            if (!item->is_tombstone)
                insert(new_ht, item->key, item->value);
    }

    destroy(ht);
    ht = new_ht;
}

void shrink_table(Ht *&ht) {
    if (ht->size * 0.5 >= ht->min_size)
        resize_table(ht, 0.5);
}

void extend_table(Ht *&ht) {
    if (ht->size * 2 <= ht->max_size)
        resize_table(ht, 2);
}

void print_table(Ht *&ht) {
    //    bool is_empty = true; //detected as not used

    std::cout << "Table size: " << ht->size
              << ", item count: " << ht->item_count << std::endl;

    if (ht->item_count == 0) {
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
