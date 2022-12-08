#include "../cuckoo_hash.cpp"
#pragma once

const int NUM_THREADS = 8;
const int NUM_BUCKET = 35 * 100 * 1000;
const int NUM_OPS = 50 * 100 * 1000;


void test()
{
    CuckooHashMap<int> my_map(NUM_BUCKET);
    for (int i = 0; i < NUM_OPS; i++)
    {
        my_map.put("random_value" + to_string(i), i);
    }
    for (int i = 0; i < NUM_OPS; i++)
    {
        auto n=my_map.get("random_value" + to_string(i));
        assert(n==i);
    }
    for(int i=NUM_OPS;i<NUM_OPS+1*1000;i++){
        auto n=my_map.get("random_value" + to_string(i));
    }
    assert(my_map.unfound==1*1000);
}

int main()
{
    test();
}