#include <iostream>
#include <string>
#include <random>
#include <sys/types.h>
#include <unistd.h>
#include "opt_cuckoo.cpp"

using namespace std;

class HOGE{
	int n;
  public:
	  HOGE():n(10){
	  }

};

int main()
{
  const int ops = 16000 * 10000;
  vector<pair<string, int>> args(ops/100000);
  //for (int i = 0; i < ops; i++)
  //{
  //  args[i].first = "random:temp" + to_string(i);
  //  args[i].second = i;
  //}
  //auto rargs=args;
  //std::random_device seed_gen;
  //std::mt19937 engine(seed_gen());
  //std::shuffle(rargs.begin(),rargs.end(),engine);
  for (int t_num = 1; t_num < 300; t_num *= 2)
  {
    double ms_sum = 0;
    double ms_best = 1000000000;
    for (int loop = 0; loop < 10; loop++)
    {
	    //std::cout<<"start:"<<loop<<std::endl;
      //OptCuckoo<int> Cuckoo(5000 * 1000);
      OptCuckoo<int> Cuckoo(10);
	    vector<thread> threads;
      auto write = [](bool &flag,int tid, vector<pair<string, int>> &args, OptCuckoo<int> &Cuckoo, int t_num)
      {
	      while(!flag){
	      }
        for (int i = tid; i < ops; i += t_num)
        {
          //Cuckoo.put(args[i].first, args[i].second, i + 1);
          auto p=new HOGE();
	}
      };
      auto read = [](bool &flag,int tid,vector<pair<string,int>> &args,OptCuckoo<int> &Cuckoo, int t_num)
      {
	      while(!flag){
	      }
        for(int i=tid;i<ops;i+=t_num){
	  Cuckoo.get(args[i].first);
	}
      };

      bool f=false;
      for (int i = 0; i < t_num; i++)
      {
        threads.emplace_back(write, ref(f),i, ref(args), ref(Cuckoo), t_num);
      }
      //for (int i = 0; i < t_num; i++)
      //{
      //  threads.emplace_back(read, ref(f),i, ref(args), ref(Cuckoo), t_num);
      //}
      auto s=get_now();
      f=true;
      for (int i = 0; i < t_num; i++)
      {
        threads[i].join();
      }
      auto e = get_now();
      ms_sum += get_duration_ms(s, e) / 1000000;
      ms_best = min(ms_best, get_duration_ms(s, e) / 1000000);
    }
    cout << ms_best << endl;
  }
}
