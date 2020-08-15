#include "../ConfiguratorJson/ConfiguratorJson.h"
#include <iostream>

using namespace std;
using namespace codepi;

struct Struct{
  int a,b,c;
};

struct StructConfig :public Struct, public ConfiguratorJson {
  StructConfig& operator= (const Struct& o){
    a=o.a;
    b=o.b;
    c=o.c;
    return *this;
  }

  CFGJS_HEADER(StructConfig)
  CFGJS_ENTRY1(a)
  CFGJS_ENTRY1(b)
  CFGJS_ENTRY1(c)
  CFGJS_TAIL
};

int main(){
  Struct s1;
  s1.a = 1;
  s1.b = 2;
  s1.c = 3;

  StructConfig sc1;
  sc1 = s1;
  
  sc1.to_stream(cout,2);
  cout << "\n";

  return 0;
}
