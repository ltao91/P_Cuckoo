#pragma once
#include <functional>
#include <atomic>
#include <vector>
#include <mutex>
#include <string>

using namespace std;

template <class T, class U>
class CoarseHashMap
{
public:
    CoarseHashMap(int t_table_size = 64, float t_max_load_factor = 1.5f)
    {
        table_size = t_table_size;
        max_load_factor = t_max_load_factor;
        table.resize(table_size);
        table_locks=vector<mutex>(table_size);
    }
    ~CoarseHashMap()
    {
        for (auto i : table)
        {
            for (auto j : i)
            {
                delete j;
            }
        }
    }
    float get_load_factor()
    {
        return 1.0 * key_size / table_size;
    }

    int get_key_size()
    {
        return key_size;
    }
    U get(T key)
    {
        std::hash<T> haser;
        int hashed_address = hasher(key) & table_size;

        for (auto i : table.at(hashed_address))
        {
            if (i->key == key)
            {
                return i->val;
            }
        }
        // throw KeyNotFoundError(to_string(key).c_str());
    }
    void put(T key, U val)
    {
        hash<T> hasher;
        int hashed_address = hasher(key) % table_size;

        table_locks[hashed_address].lock();
        for (auto i : table.at(hashed_address))
        {
            if (i->key == key)
            {
                i->val = val;
                return;
            }
        }
        key_size++;

        HashEntry *temp = new HashEntry();
        temp->key = key;
        temp->val = val;

        table.at(hashed_address).push_back(temp);
        table_locks[hashed_address].unlock();

        // if (get_load_factor() > max_load_factor)
        // {
        //     resize();
        // }
    }

    bool remove(T key)
    {
        std::hash<T> hasher;
        int hashed_address = hasher(key) % table_size;
        for (int i = 0; i < table.at(hashed_address).size(); i++)
        {
            if (table.at(hashed_address)[i]->key == key)
            {
                table.at(hashed_address).erase(table.at(hashed_address).begin() + i);
                key_size--;
                return true;
            }
        }
        return false;
    }

private:
    class HashEntry
    {
    public:
        T key;
        U val;
    };
    int table_size;

    atomic_int key_size;

    float max_load_factor;

    vector<vector<HashEntry *>> table;
    vector<mutex> table_locks;

    void resize()
    {
        auto old_table = table;
        table_size *= 2;
        key_size = 0;
        table = vector<vector<HashEntry *>>(table_size);
        for (auto i : old_table)
        {
            for (auto j : i)
            {
                put(j->key, j->val);
                delete j;
            }
        }
    }
};