#include "opt_cuckoo_nicelocks_bfs.cpp"

int main()
{
    for (int t_num = 1; t_num < 200; t_num += 1)
    {
        OptCuckoo<int> Cuckoo(8000 * 1000);
        // OptCuckoo<int> Cuckoo(10);
        vector<thread> threads;
        const int ops = 27000 * 1000;
        // const int ops = 38;
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
                cout<<i<<endl;
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
        cout << "threads : " << t_num << " time(ms) : " << get_duration_ms(s, e) / 1000000 << endl;
        for (int i = 0; i < ops; i++)
        {
            if(Cuckoo.get(args[i].first) != args[i].second){
                cout<<i<<endl;
                cout<<Cuckoo.get(args[i].first)<<endl;
                cout<<args[i].first<<endl;
                cout<<args[i].second<<endl;
                Cuckoo.debug();
                exit(0);
            }
        }
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