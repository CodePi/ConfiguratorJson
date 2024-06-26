#include "../ConfiguratorJson/Optional.h"
#include <vector>
#include <stdio.h>

using namespace std;
using namespace codepi;

bool g_all_pass = true;

vector<int> rvalue(int**p){
  vector<int> v = {1,2,3};
  *p = v.data();
  return v;
}

const char* pf(bool test){
  if(test==true) return "pass";
  else {
      g_all_pass = false;
      return "fail";
  }
}

int main(){

  // test1
  vector<int> v={1,2,3};
  int* orig = v.data();
  Optional<vector<int>> ov(std::move(v));
  int* curr = ov->data();
  printf("construct by move contents:\t%p\t%p\t%s\n", orig, curr, pf(orig==curr));

  // test2
  Optional<vector<int>> ov2(rvalue(&orig));
  curr = ov2->data();
  printf("construct by rvalue:\t\t%p\t%p\t%s\n", orig, curr, pf(orig==curr));

  // test3
  vector<int> v3={1,2,3};
  orig = v3.data();
  Optional<vector<int>> ov3(v3);
  curr = ov3->data();
  printf("construct by copy:\t\t%p\t%p\t%s\n", orig, curr, pf(orig!=curr));
  
  // test4
  orig = ov3->data();
  Optional<vector<int>> ov4(ov3);
  curr = ov4->data();
  printf("construct by copy wrapper:\t%p\t%p\t%s\n", orig, curr, pf(orig!=curr));

  // test5
  Optional<vector<int>> ov5(Optional<vector<int>>(rvalue(&orig)));
  curr = ov5->data();
  printf("construct by double rvalue:\t%p\t%p\t%s\n", orig, curr, pf(orig==curr));
  
  // test6
  Optional<vector<int>> ov6;
  ov6 = rvalue(&orig);
  curr = ov6->data();
  printf("assignment rvalue:\t\t%p\t%p\t%s\n", orig, curr, pf(orig==curr));

  // test7 
  Optional<vector<int>> ov7;
  ov7 = Optional<vector<int>>(rvalue(&orig));
  curr = ov7->data();
  printf("assignment double rvalue:\t%p\t%p\t%s\n", orig, curr, pf(orig==curr));
  
  // test8
  const vector<int> v8 = {1,2,3};
  orig = (int*)v8.data();
  Optional<vector<int>> ov8(v8);
  curr = ov8->data();
  printf("construct by const:\t\t%p\t%p\t%s\n", orig, curr, pf(orig!=curr));

  // test9 
  vector<int> v9 = {1,2,3};
  Optional<vector<int>> ov9;
  orig = v9.data();
  ov9 = v9;
  curr = ov9->data();
  printf("assignment by copy:\t\t%p\t%p\t%s\n", orig, curr, pf(orig!=curr));
  
  // test10 
  Optional<vector<int>> v10(vector<int>({1,2,3}));
  Optional<vector<int>> ov10;
  orig = v10->data();
  ov10 = v10;
  curr = ov10->data();
  printf("assignment by copy wrapper:\t%p\t%p\t%s\n", orig, curr, pf(orig!=curr));

  // test11
  const Optional<int> v11 = 5;
  const int& v11i = v11;
  printf("const ref test: %s\n", pf(v11==v11i));

  if(!g_all_pass) {
      printf("not all pass\n");
      return -1;
  }
  printf("all pass\n");
  return 0;
}
