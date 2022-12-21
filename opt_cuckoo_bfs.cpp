#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <cassert>
#include <unordered_map>
#include "common/hash.h"
#include "common/time.cpp"
#include <algorithm>
using namespace std;

// T val   string key
template <class T>
class OptCuckoo
{
public:
    class Data
    {
    public:
        string key;
        T val;
        Data()
        {
        }
        Data(string t_key, T t_val)
        {
            key = t_key;
            val = t_val;
        }
    };
    class Node
    {
    public:
        unsigned char tag; // why? how many bits?
        Data *data;
        Node()
        {
        }
        Node(unsigned char t_tag, string t_key, T t_val)
        {
            tag = t_tag;
            data = new Data(t_key, t_val);
        }
    };

    const int SLOTS_NUM = 4;
    const int MAX_LOOP_FOR_PUT = 80 * 1000;

    int table_size;
    int key_size;
    int key_versions_size;

    vector<vector<Node *>> table;
    vector<vector<mutex>> table_locks;
    vector<vector<int>> visited;
    vector<vector<int>> key_versions;
    vector<vector<mutex>> key_versions_locks;

    // mutex giant_write_lock;

    unsigned char get_tag(const uint32_t input)
    {
        uint32_t t = input & 0xff;
        return (unsigned char)t + (t == 0);
    }

    void ABORT()
    {
        // this is for debug and for annotation
    }

    OptCuckoo(int t_table_size)
    {
        table_size = t_table_size;
        table = vector<vector<Node *>>(table_size, vector<Node *>(SLOTS_NUM));
        visited = vector<vector<int>>(table_size, vector<int>(SLOTS_NUM));
        key_versions = vector<vector<int>>(table_size, vector<int>(SLOTS_NUM));
        for (int i = 0; i < table_size; i++)
        {
            key_versions_locks.emplace_back(vector<mutex>(SLOTS_NUM));
            table_locks.emplace_back(vector<mutex>(SLOTS_NUM));
        }

        key_versions_size = table_size;
        longest = vector<pair<pair<int, int>, T>>();
    }
    double get_version_t = 0;

    int get_version(int l, int r)
    {
        int res;
        auto s = get_now();
        key_versions_locks[l][r].lock();
        res = key_versions[l][r];
        key_versions_locks[l][r].unlock();
        auto e = get_now();
        get_version_t += get_duration_ms(s, e);
        return res;
    }
    double increase_version_t = 0;
    void increase_version(int l, int r)
    {
        auto s = get_now();
        key_versions_locks[l][r].lock();
        key_versions[l][r]++;
        key_versions_locks[l][r].unlock();
        auto e = get_now();
        increase_version_t += get_duration_ms(s, e);
    }

    T get(std::string key)
    {
        uint32_t h1 = 0, h2 = 0;
        hashlittle2(key.c_str(), key.length(), &h1, &h2);
        unsigned char tag = get_tag(h1);
        if (h1 < 0)
            h1 += table_size;
        if (h2 < 0)
            h2 += table_size;
        h1 = (h1) % table_size;
        h2 = (h2) % table_size;

        // let's try to get
        while (true)
        {
            for (int i = 0; i < SLOTS_NUM; i++)
            {
                Node *node = table[h1][i];
                if (node != NULL && node->tag == tag && node->data != NULL)
                {
                    Data *data = node->data;
                    uint32_t start_version = get_version(h1, i);
                    if (key == data->key)
                    {
                        T val = data->val;
                        uint32_t end_version = get_version(h1, i);
                        if (start_version != end_version || start_version & 0x1)
                        {
                            ABORT();
                            continue;
                        }
                        return val;
                    }
                }
            }

            for (int i = 0; i < SLOTS_NUM; i++)
            {
                Node *node = table[h2][i];
                if (node != NULL && node->tag == tag && node->data != NULL)
                {
                    Data *data = node->data;
                    uint32_t start_version = get_version(h2, i);
                    if (key == data->key)
                    {
                        T val = data->val;
                        uint32_t end_version = get_version(h2, i);
                        if (start_version != end_version || start_version & 0x1)
                        {
                            ABORT();
                            continue;
                        }
                        return val;
                    }
                }
            }

            // not found
            T temp_val;
            return temp_val;
        }
    }
    double release_time = 0;
    void release_visited(vector<pair<int, int>> &path, int TID)
    {
        for (auto i : path)
        {
            if (visited[i.first][i.second] == TID)
                visited[i.first][i.second] = 0;
        }
    }

    int aborted_num = 0;
    int validation_fail = 0;
    double sum_time = 0;
    int evict_null = 0;
    vector<pair<pair<int, int>, T>> longest;

    void put(std::string key, T val, int TID)
    {
        auto s = get_now();
        int i = 0;
        while (!put_impl(key, val, TID))
        {
            ABORT();
            i++;
            aborted_num++;
            if (i >= 10)
            {
                cout << "Too many abort" << endl;
                exit(0);
                return;
            }
        }
        auto e = get_now();
        sum_time += get_duration_ms(s, e);
    }
    double write_lock_time = 0;
    double hash_time = 0;
    double w_1 = 0, w_2 = 0, w_3 = 0;
    int c1 = 0, c2 = 0, c3 = 0;

    bool put_impl(std::string key, T val, int TID)
    {
        string original_key = key;
        T original_val = val;
        unsigned char original_tag;
        int counter = 0;
        uint32_t h1 = 0, h2 = 0;
        vector<pair<int, int>> path;
        vector<int> path_versions_history;
        bool is_success = false;

        while (counter < MAX_LOOP_FOR_PUT)
        {
            h1 = 0;
            h2 = 0;
            hashlittle2(key.c_str(), key.length(), &h1, &h2);
            unsigned char tag = get_tag(h1);
            if (counter == 0)
                original_tag = tag;
            if (h1 < 0)
                h1 += table_size;
            if (h2 < 0)
                h2 += table_size;
            h1 = (h1) % table_size;
            h2 = (h2) % table_size;

            // search in h1
            for (int i = 0; i < SLOTS_NUM; i++)
            {
                Node *node = table[h1][i];
                if (node == NULL)
                {
                    path_versions_history.push_back(get_version(h1, i));
                    path.push_back(make_pair(h1, i));
                    is_success = true;
                    break;
                }
                else if (counter == 0 && node->data->key == key)
                {
                    table_locks[h1][i].lock();
                    if (node->data->key != key)
                    {
                        table_locks[h1][i].unlock();
                        continue;
                    }
                    increase_version(h1, i);
                    node->data->val = val;
                    increase_version(h1, i);
                    table_locks[h1][i].unlock();
                    cout<<"HI!"<<endl;
                    return true;
                }
            }
            if (is_success)
            {
                break;
            }

            // search in h2
            for (int i = 0; i < SLOTS_NUM; i++)
            {
                Node *node = table[h2][i];
                if (node == NULL)
                {
                    path_versions_history.push_back(get_version(h2, i));
                    path.push_back(make_pair(h2, i));
                    is_success = true;
                    break;
                }
                else if (counter == 0 && node->data->key == key)
                {
                    table_locks[h2][i].lock();
                    if (node->data->key != key)
                    {
                        table_locks[h2][i].unlock();
                        continue;
                    }
                    increase_version(h2, i);
                    node->data->val = val;
                    increase_version(h2, i);
                    table_locks[h2][i].unlock();
                    return true;
                }
            }
            if (is_success)
            {
                break;
            }

            // if both of them are full, choose node to evict
            Node *evict_node = NULL;
            for (int i = 0; i < SLOTS_NUM; i++)
            {
                if (visited[h1][i] == 0)
                {
                    visited[h1][i] = TID;
                    path_versions_history.push_back(get_version(h1, i));
                    path.push_back(make_pair(h1, i));
                    evict_node = table[h1][i];
                    break;
                }
                if (visited[h2][i] == 0)
                {
                    visited[h2][i] = TID;
                    path_versions_history.push_back(get_version(h2, i));
                    path.push_back(make_pair(h2, i));
                    evict_node = table[h2][i];
                    break;
                }
            }
            if (evict_node == NULL)
            {
                release_visited(path, TID);
                evict_null++;
                return false;
            }
            key = evict_node->data->key;
            val = evict_node->data->val;
            counter++;
        }
        if (!is_success)
        {
            ABORT();
            release_visited(path, TID);
            return false;
        }
        auto original_node = new Node(original_tag, original_key, original_val);
        if (path.size() == 1)
        {
            pair<int, int> index = path.front();
            table_locks[index.first][index.second].lock();
            if (table[index.first][index.second] != NULL)
            {
                ABORT();
                table_locks[index.first][index.second].unlock();
                release_visited(path, TID);
                return false;
            }
            // Node *node = new Node(original_tag, key, val);
            increase_version(index.first, index.second);
            table[index.first][index.second] = original_node;
            increase_version(index.first, index.second);
            table_locks[index.first][index.second].unlock();
            release_visited(path, TID);
            return true;
        }

        auto path_for_lock = path;
        sort(path_for_lock.begin(), path_for_lock.end());
        for (auto i : path_for_lock)
        {
            table_locks[i.first][i.second].lock();
        }
        auto unlock_all_pathlocks = [&]()
        {
            for (auto i : path_for_lock)
            {
                table_locks[i.first][i.second].unlock();
            }
        };

        for (int i = 0; i < path.size(); i++)
        {
            if (get_version(path[i].first, path[i].second) != path_versions_history[i] || path_versions_history[i] & 0x1)
            {
                ABORT();
                unlock_all_pathlocks();
                release_visited(path, TID);
                return false;
            }
        }
        for (int i = path.size() - 1; i > 0; i--)
        {
            increase_version(path[i].first, path[i].second);
            table[path[i].first][path[i].second] = table[path[i - 1].first][path[i - 1].second];
            increase_version(path[i].first, path[i].second);
        }
        increase_version(path[0].first, path[0].second);
        table[path[0].first][path[0].second] = original_node;
        increase_version(path[0].first, path[0].second);
        unlock_all_pathlocks();
        release_visited(path, TID);
        return true;
    }
    void debug()
    {
        for (int i = 0; i < table_size; i++)
        {
            for (int j = 0; j < SLOTS_NUM; j++)
            {
                if (table[i][j] == NULL)
                {
                    cout << "VOID  NON   ";
                    continue;
                }
                cout << (int)table[i][j]->tag << " " << table[i][j]->data->key << "  " << table[i][j]->data->val << "   ";
            }
            cout << endl;
        }
    }
};