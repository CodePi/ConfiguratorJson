#include "../ConfiguratorJson/ConfiguratorJson.h"

#include <map>
#include <vector>
#include <string>

using namespace std;
using namespace codepi;

struct Config1 : public ConfiguratorJson {
  int exampleIntValue;
  float exampleFloat;
  string exampleString;
  vector<int> exampleVector;
  map<string, int> exampleMap;
  int i,j,k;
  float x,y,z;

  CFGJS_HEADER(Config1)
  CFGJS_ENTRY_DEF(exampleIntValue, 12)
  CFGJS_ENTRY(exampleFloat)
  CFGJS_ENTRY_DEF(exampleString, "initial value")
  CFGJS_ENTRY(exampleVector)
  CFGJS_ENTRY(exampleMap)
  CFGJS_MULTIENTRY6(i,j,k,x,y,z) // multiple entries with auto default value
  CFGJS_TAIL
};

struct Config2 : public ConfiguratorJson {
  Config1 exampleSub;
  int anotherInt;

  CFGJS_HEADER(Config2)
  CFGJS_ENTRY(exampleSub)
  CFGJS_ENTRY(anotherInt)
  CFGJS_TAIL
};

int main(){
  Config2 config2;

  config2.exampleSub.exampleIntValue = 1;
  config2.exampleSub.exampleString = "a string";
  config2.exampleSub.exampleVector = { 1, 2, 3 };
  config2.exampleSub.exampleMap = {{"a",1},{"b",2}};
  config2.anotherInt = 2;
  cout << config2.to_string() << endl;
  config2.to_file("file3.txt");

  Config2 config2b;
  config2b.from_file("file3.txt");
  cout << config2b.to_string() << endl;

  assert(config2==config2b);

  return 0;
}
