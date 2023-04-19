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
    Item *next = nullptr;
    Item(Key key, Value value) : key(key), value(value), next(nullptr){};
};

static const Item tombstone = Item("", "");

struct Ht
{
    std::size_t (*hash)(Key input) = nullptr;
    std::size_t size               = 0;
    unsigned item_count            = 0;
    Item ***items                  = nullptr;

    Ht(){};
};

Ht *create(std::size_t size, std::size_t (*hash)(Key input));
Ht *create();

void destroy(Ht *&ht);
// void clear(Ht *&ht);
// void set_hash_function(Ht *&ht, std::size_t (*hash)(std::string input));

void throw_nonexistent(Ht *&ht, std::size_t key_hash, Key key);

Value get(Ht *&ht, Key key);

Item *find(Ht *&ht, Key key);

void insert(Ht *&ht, Key key, Value value);

void remove(Ht *&ht, Key key);

void inserts(Ht *&ht, std::string *keys, Item **&items);
// void removes(Ht *&ht, std::string *keys)

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
    insert(ht, "asldkfhskdhl", "poxyu");

    print_table(ht);

    remove(ht, "sasha0");
    remove(ht, "sasha1");
    remove(ht, "sasha2");
    remove(ht, "sasha3");
    remove(ht, "sasha4");
    remove(ht, "sasha5");
    remove(ht, "sasha6");

    print_table(ht);

    std::cout << get(ht, "sergey") << std::endl;
    std::cout << get(ht, "asldkfhskdhl") << std::endl;
    std::cout << std::endl;

    remove(ht, "sasha7");
    remove(ht, "sergey");
    remove(ht, "asldkfhskdhl");

    print_table(ht);

    destroy(ht);

    return 0;
}

Ht *create(std::size_t size, std::size_t (*hash)(Key input)) {
    Ht *ht    = new Ht;
    ht->size  = size;
    ht->hash  = hash;
    ht->items = new Item **[size];

    for (std::size_t item = 0; item < size; item++) {
        ht->items[item] = new Item *[size];
        for (std::size_t subitem = 0; subitem < size; subitem++) {
            ht->items[item][subitem] = nullptr;
        }
    }

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

    if (ht->items[key_hash][0]->next == nullptr) {   // FIXME а тут есть
                                                     // гарантия, что
                                                     // items[key_hash][0] !=
                                                     // nullptr?
        ht->items[key_hash][0] = item;
        ht->item_count++;
    } else {
        for (int i = 0; ht->items[key_hash][i]->next != nullptr;) {
            i++;
            if (ht->items[key_hash][i] == nullptr) {
                ht->items[key_hash][i]           = item;
                ht->items[key_hash][i - 1]->next = ht->items[key_hash][i];
                ht->item_count++;
                return;
            }
        }
    }
}

void inserts(Ht *&ht, std::string *keys, std::string *values, int number) {
    for (int i = 0; i < number; ++i) {
        insert(ht, keys[i], values[i]);
    }
}

void throw_nonexistent(Ht *&ht, std::size_t key_hash, Key key) {
    if (ht->items[key_hash][0] == nullptr)
        throw std::out_of_range("There is no element associated with key `" +
                                std::string(key) + "'.");
}

Value get(Ht *&ht, Key key) {
    Item *item           = find(ht, key);
    std::size_t key_hash = ht->hash(key) % ht->size;

    if (item != nullptr)
        return item->value;

    else {
        throw_nonexistent(ht, key_hash, key);
        return tombstone.value;
    }
}

Item *find(Ht *&ht, Key key) {
    std::size_t key_hash = ht->hash(key) % ht->size;

    int temp = 0;
    if (ht->items[key_hash] == nullptr) {
        //        throw_nonexistent(ht, key_hash, key); // this will be called
        //        in get function
        return nullptr;
    }

    else if (ht->items[key_hash][temp] == nullptr) {
        //        throw_nonexistent(ht, key_hash, key); // this will be called
        //        in get function
        return nullptr;
    }

    ++temp;   // because the ht->items[key_hash][0] passed all our tests in
              // if-s above
    while (ht->items[key_hash][temp]->key != key) {
        if (ht->items[key_hash][temp]->next != nullptr)
            temp++;

        else {
            //        throw_nonexistent(ht, key_hash, key); // this will be
            //        called in get function
            return nullptr;
        }
    }

    return ht->items[key_hash][temp];
}

void remove(Ht *&ht, Key key) {
    if (ht->item_count <= ht->size * (load_factor * 0.5))
        shrink_table(ht);

    std::size_t key_hash = ht->hash(key) % ht->size;
    int temp             = 0;

    if (ht->items[key_hash][0] == nullptr)
        return;

    while (ht->items[key_hash][temp]->key != key) {
        temp++;
        if (ht->items[key_hash][temp] == nullptr)
            return;
    }
    if (temp > 0) {
        ht->items[key_hash][temp - 1]->next = nullptr;
    }
    ht->items[key_hash][temp]        = nullptr;
    ht->items[key_hash][temp]->key   = "";
    ht->items[key_hash][temp]->value = "";
    ht->item_count--;
}

void resize_table(Ht *&ht, const double resize_factor) {
    Ht *new_ht = create(ht->size * resize_factor, ht->hash);

    for (std::size_t item_i = 0; item_i < ht->size; item_i++) {
        // Item *item = ht->items[item_i];

        // if (item != nullptr)
        // if (!item->tombstone)
        // insert(new_ht, item->key, item->value);
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

    // for (std::size_t item = 0; item < ht->size; item++)
    //  if (ht->items[item][0] != nullptr)
    //    if (!ht->items[item][0].next == nullptr)
    //      std::cout << "Key: `" << ht->items[item]->key << "', value: `"
    //                << ht->items[item]->value
    //               << "', calculated position: "
    //               << ht->hash(ht->items[item]->key) % ht->size
    //               << ", real position: " << item << std::endl;

    std::cout << std::endl;
}
