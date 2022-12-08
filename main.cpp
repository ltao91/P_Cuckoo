#include <iostream>
#include <unordered_map>
#include <thread>
#include <functional>
#include "coarse_hash_map.cpp"
#include "hash_map.cpp"
#include "cuckoo_hash.cpp"
#include "common/time.cpp"
#include "common/hash.h"
using namespace std;

const int NUM_THREADS = 8;
const int NUM_BUCKET = 35 * 100 * 1000;
// const int NUM_OPS=10;
const int NUM_OPS = 50 * 100 * 1000;

class Args
{
public:
    CoarseHashMap<int, string> *my_map;
    int thread_id;
    int map_size;
};

void benchmark_hashmap()
{
    cout << "Bench marking normal hashmap" << endl;
    double best_ms;
    best_ms = 1e30;
    for (int i = 0; i < 1; i++)
    {
        auto start = get_now();
        HashMap<int, string> my_map(NUM_BUCKET);
        for (int j = 0; j < NUM_OPS; j++)
        {
            my_map.put(i, "random_value" + to_string(i));
        }
        auto end = get_now();
        best_ms = min(best_ms, get_duration_ms(start, end));
    }
    cout << "time(ms) : " << best_ms << endl;
}

void benchmark_cuckoo_hashmap()
{
    cout << "Bench marking cuckoo hashmap" << endl;
    double best_ms;
    best_ms = 1e30;
    for (int i = 0; i < 1; i++)
    {
        auto start = get_now();
        CuckooHashMap<int> my_map(NUM_BUCKET);
        for (int j = 0; j < NUM_OPS; j++)
        {
            my_map.put("random_value" + to_string(j), i);
        }
        auto end = get_now();
        best_ms = min(best_ms, get_duration_ms(start, end));
    }
    cout << "time(ms) : " << best_ms << endl;
}

void thread_send_requests(Args &args)
{
    CoarseHashMap<int, string> *my_map = args.my_map;
    int thread_id = args.thread_id;
    int map_size = args.map_size;
    for (int i = thread_id; i < map_size; i += NUM_THREADS)
    {
        my_map->put(i, "random_value" + to_string(i));
    }
}

void benchmark_coarse_hashmap()
{
    cout << "Benchmarking coarse hashmap" << endl;
    double best_ms;
    best_ms = 1e30;
    for (int i = 0; i < 1; i++)
    {
        auto start = get_now();
        CoarseHashMap<int, string> my_map(NUM_BUCKET);
        vector<thread> threads;
        vector<Args> args(NUM_THREADS);

        for (int j = 0; j < NUM_THREADS; j++)
        {
            args[j].my_map = &my_map;
            args[i].thread_id = j;
            args[i].map_size = NUM_OPS;
        }
        for (int j = 0; j < NUM_THREADS; j++)
        {
            threads.emplace_back(thread_send_requests, ref(args[j]));
        }
        for (int j = 0; j < NUM_THREADS; j++)
        {
            threads[j].join();
        }
        auto end = get_now();
        best_ms = min(best_ms, get_duration_ms(start, end));
    }
    cout << "time(ms) : " << best_ms << endl;
}

int main()
{
    benchmark_cuckoo_hashmap();
    benchmark_hashmap();
    benchmark_coarse_hashmap();
}