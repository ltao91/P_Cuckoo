#include "opt_cuckoo_nicelocks_bfs.cpp"
#include <map>
int main()
{
    for (int t_num = 1; t_num < 200; t_num += 1)
    {
        cout << "start" << endl;
        // OptCuckoo<int> Cuckoo(8000 * 1000);
        OptCuckoo<int> Cuckoo(100);
        vector<thread> threads;
        // const int ops = 27000 * 1000;
        const int ops = 380;
        vector<pair<string, int>> args(ops);
        for (int i = 0; i < ops; i++)
        {
            args[i].first = "random:temp" + to_string(i);
            args[i].second = i;
        }
        auto put = [](int tid, vector<pair<string, int>> &args, OptCuckoo<int> &Cuckoo, int t_num)
        {
            for (int i = tid; i < ops; i += t_num)
            {
                Cuckoo.put(args[i].first, args[i].second, tid + 1);
            }
        };

        auto s = get_now();
        for (int i = 0; i < t_num; i++)
        {
            threads.emplace_back(put, i, ref(args), ref(Cuckoo), t_num);
        }
        for (int i = 0; i < t_num; i++)
        {
            threads[i].join();
        }
        auto e = get_now();
        cout << "    threads : " << t_num << " time(ms) : " << get_duration_ms(s, e) / 1000000 << endl;
        for (int i = 0; i < ops; i++)
        {
            if (Cuckoo.get(args[i].first) != args[i].second)
            {

                uint32_t h1 = 0, h2 = 0;
                cout << i << endl;
                cout << Cuckoo.get(args[i].first) << endl;
                cout << args[i].first << endl;
                cout << args[i].second << endl;
                Cuckoo.debug();
                hashlittle2(args[i].first.c_str(), args[i].first.length(), &h1, &h2);
                cout << h1 % Cuckoo.table_size << " " << h2 % Cuckoo.table_size << endl;
                cout << endl;
                cout << endl;
                map<OptCuckoo<int>::Node *, int> m;
                for (auto &i : Cuckoo.table)
                {
                    for (auto j : i)
                    {
                        if (j != nullptr)
                        {
                            m[j]++;
                            if (m[j] > 2)
                            {
                                cout << j->data->key << "  " << j->data->val << endl;
                            }
                        }
                    }
                }
                exit(0);
            }
        }
        cout << "        works correctly" << endl;
    }
    // for (int t_num = 1; t_num < 60; t_num += 1)
    // {
    //     OptCuckoo<int> Cuckoo(800 * 1000);
    //     vector<thread> threads;
    //     const int ops = 2400 * 1000;
    //     vector<pair<string, int>> args(ops);
    //     for (int i = 0; i < ops; i++)
    //     {
    //         args[i].first = "random:" + to_string(i);
    //         args[i].second = i;
    //     }
    //     auto put = [](int tid, vector<pair<string, int>> &args, OptCuckoo<int> &Cuckoo, int t_num)
    //     {
    //         for (int i = tid; i < ops; i += t_num)
    //         {
    //             Cuckoo.put(args[i].first, args[i].second, tid + 1);
    //         }
    //     };

    //     auto s = get_now();
    //     for (int i = 0; i < t_num; i++)
    //     {
    //         threads.emplace_back(put, i, ref(args), ref(Cuckoo), t_num);
    //     }
    //     for (int i = 0; i < t_num; i++)
    //     {
    //         threads[i].join();
    //     }
    //     auto e = get_now();
    //     cout << "threads : " << t_num << " time(ms) : " << get_duration_ms(s, e) / 1000000 << endl;
    // }
}