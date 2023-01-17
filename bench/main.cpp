#include <iostream>
#include <string>
#include <random>

#include "opt_cuckoo.cpp"

using namespace std;

int main()
{
  const int ops = 16000 * 1000;
  vector<pair<string, int>> args(ops);
  OptCuckoo<int> Cuckoo(8000*1000);
  for (int i = 0; i < ops; i++)
  {
    args[i].first = "random:temp" + to_string(i);
    args[i].second = i;
  }

  auto args_r=args;
  std::random_device seed_gen;
  std::mt19937 engine(seed_gen());
  std::shuffle(args_r.begin(), args_r.end(), engine);
  assert(args_r.size()==args.size());
  for (int t_num = 1; t_num < 200; t_num *= 2)
  {
    double ms_sum = 0;
    double ms_best = 1000000000;
    for (int loop = 0; loop < 10; loop++)
    {
      cout << "start : " << loop << endl;
      vector<thread> threads;
      auto read = [](int tid, vector<pair<string, int>> &args, OptCuckoo<int> &Cuckoo, int t_num)
      {
        for (int i = tid; i < ops; i += t_num)
        {
          auto x=Cuckoo.get(args[i].first);
        }
      };
      auto write = [](int tid, vector<pair<string, int>> &args, OptCuckoo<int> &Cuckoo, int t_num)
      {
        for (int i = tid; i<ops;i +=t_num){
          Cuckoo.put(args[i].first,args[i].second,i+1);
        }
      };

      auto s = get_now();
      for (int i = 0; i < t_num; i++)
      {
        threads.emplace_back(write, i, ref(args), ref(Cuckoo), t_num);
      }
      for(int i=0;i<t_num;i++)
      {
        threads.emplace_back(read,i,ref(args_r),ref(Cuckoo),t_num);
      }
      for (int i = 0; i < t_num*2; i++)
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
