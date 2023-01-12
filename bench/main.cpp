#include <iostream>
#include <string>

#include "opt_cuckoo.cpp"

using namespace std;

int main()
{
  const int ops = 160*1000*1000;
  vector<pair<string, int>> args(ops);
  OptCuckoo<int> Cuckoo(80 * 1000 * 1000);
  for (int i = 0; i < ops; i++)
  {
    args[i].first = "random:temp" + to_string(i);
    args[i].second = i;
  }
  for (int i = 0; i < ops; i++)
  {
    Cuckoo.put(args[i].first,args[i].second,1);
  }
  for (int t_num = 1; t_num < 200; t_num *= 2)
  {
    double ms_sum = 0;
    double ms_best = 1000000000;
    for (int loop = 0; loop < 1; loop++)
    {
      cout << "start : " << loop << endl;
      vector<thread> threads;
      auto read = [](int tid, vector<pair<string, int>> &args, OptCuckoo<int> &Cuckoo, int t_num)
      {
        for (int i = tid; i < ops; i += t_num)
        {
          auto res=Cuckoo.get(args[i].first);
        }
      };

      auto s = get_now();
      for (int i = 0; i < t_num; i++)
      {
        threads.emplace_back(read, i, ref(args), ref(Cuckoo), t_num);
      }
      for (int i = 0; i < t_num; i++)
      {
        threads[i].join();
      }
      auto e = get_now();
      ms_sum += get_duration_ms(s, e) / 1000000;
      ms_best = min(ms_best, get_duration_ms(s, e) / 1000000);
    }
    cout << ms_sum / 10 << endl;
    cout << ms_best << endl;
  }
}
