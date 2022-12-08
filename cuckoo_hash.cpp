#include <iostream>
#include <string>
#include <vector>
#include "common/hash.h"
using namespace std;

template <class T>
class CuckooHashMap
{
public:
    const int SLOTS_PER_BUCKET = 4;
    const int MAX_ITERS = 25;
    CuckooHashMap(int t_num_buckets = 64)
    {
        num_buckets = t_num_buckets;
        table = vector<vector<HashPointer *>>(num_buckets, vector<HashPointer *>(SLOTS_PER_BUCKET, NULL));
    }
    int unfound = 0;
    T get(string key)
    {
        uint32_t h1 = 0, h2 = 0;
        hashlittle2(key.c_str(), key.length(), &h1, &h2);
        h1 = (h1 + num_buckets) % num_buckets;
        h2 = (h2 + num_buckets) % num_buckets;
        unsigned char tag = 'a'; // todo

        // first bucket;
        for (int i = 0; i < SLOTS_PER_BUCKET; i++)
        {
            auto hash_pointer = table[h1][i];
            if (hash_pointer != NULL && tag == hash_pointer->tag)
            {
                HashEntry *ptr = hash_pointer->ptr;
                if (key == ptr->key)
                {
                    return ptr->val;
                }
            }
        }

        // second bucket;
        for (int i = 0; i < SLOTS_PER_BUCKET; i++)
        {
            auto hash_pointer = table[h2][i];
            if (hash_pointer != NULL && tag == hash_pointer->tag)
            {
                HashEntry *ptr = hash_pointer->ptr;
                if (key == ptr->key)
                {
                    return ptr->val;
                }
            }
        }
        T VOID;
        unfound++;
        return VOID;
    }

    void put(string key, T val)
    {
        int counter = 0;
        for (; counter < MAX_ITERS; counter++)
        {
            uint32_t h1 = 0, h2 = 0;
            hashlittle2(key.c_str(), key.length(), &h1, &h2);
            h1 = (h1 + num_buckets) % num_buckets;
            h2 = (h2 + num_buckets) % num_buckets;

            unsigned tag = 'a'; // todo

            // first bucket;
            for (int i = 0; i < SLOTS_PER_BUCKET; i++)
            {
                auto hash_pointer = table[h1][i];
                if (hash_pointer == NULL)
                {
                    HashEntry *hash_entry = new HashEntry();
                    hash_entry->key = key;
                    hash_entry->val = val;

                    hash_pointer = new HashPointer();
                    hash_pointer->ptr = hash_entry;
                    hash_pointer->tag = tag;
                    table[h1][i] = hash_pointer;
                    return;
                }
            }

            // second bucket;
            for (int i = 0; i < SLOTS_PER_BUCKET; i++)
            {
                auto hash_pointer = table[h2][i];
                if (hash_pointer == NULL)
                {
                    HashEntry *hash_entry = new HashEntry();
                    hash_entry->key = key;
                    hash_entry->val = val;

                    hash_pointer = new HashPointer();
                    hash_pointer->ptr = hash_entry;
                    hash_pointer->tag = tag;
                    table[h2][i] = hash_pointer;
                    return;
                }
            }

            // IF both of bucket are full
            int index = rand() % (2 * SLOTS_PER_BUCKET);
            HashPointer *sub;
            if (index < SLOTS_PER_BUCKET)
            {
                sub = table[h1][index];
            }
            else
            {
                sub = table[h2][index - SLOTS_PER_BUCKET];
            }
            string t_key = sub->ptr->key;
            T t_val = sub->ptr->val;

            sub->tag = tag;
            sub->ptr->key = key;
            sub->ptr->val = val;

            key = t_key;
            val = t_val;
        }
        cout << "ABORTED" << endl;
        ;
    }

private:
    class HashEntry
    {
    public:
        string key;
        T val;
    };
    class HashPointer
    {
    public:
        unsigned char tag;
        HashEntry *ptr;
    };
    vector<vector<HashPointer *>> table;
    int num_buckets;
    unsigned char tag_hash(std::string to_hash)
    {
        int hash = 0;
        for (int i = 0; i < to_hash.length(); i++)
        {
            hash = ((37 * hash) + to_hash.at(i)) & 0xff;
        }
        return hash;
    }
};