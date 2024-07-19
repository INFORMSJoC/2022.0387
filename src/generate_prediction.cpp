// This file generate random prediction for release time and deadline of jobs
// using a normal distribution with mean=0 and get stddev from system arguments
#include<bits/stdc++.h>
using namespace std;
typedef long double ld;

int main(int argc, char* argv[])
{
  ld stddev = atof(argv[1]);
  
  srand(stddev * 10);
  default_random_engine generator;
  normal_distribution<double> distribution(0, stddev);

  int n;
  cin>>n;
  cout << n << endl;
  for(int i=0;i<n;i++)
    {
      ld r, d, w;
      cin >> r >> d >> w;
      // Generate prediction for release time
      ld p = distribution(generator) * (d - r) + r;
      // Generate prediction for deadline
      ld q = distribution(generator) * (d - r) + d;
      cout << p << " " << max(p, q) << " " << r << " " << d << " " << w << endl;
    }
}
