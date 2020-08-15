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

  CFGJS_HEADER(Config1)
  CFGJS_ENTRY2(exampleIntValue, 12)
  CFGJS_ENTRY1(exampleFloat)
  CFGJS_ENTRY2(exampleString, "initial value")
  CFGJS_ENTRY1(exampleVector)
  CFGJS_ENTRY1(exampleMap)
  CFGJS_TAIL
};

struct Config2 : public ConfiguratorJson {
  Config1 exampleSub;
  int anotherInt;

  CFGJS_HEADER(Config2)
  CFGJS_ENTRY1(exampleSub)
  CFGJS_ENTRY1(anotherInt)
  CFGJS_TAIL
};

int main(){
  Config2 config2;

  config2.exampleSub.exampleIntValue = 1;
  config2.exampleSub.exampleString = "a string";
  config2.exampleSub.exampleVector = { 1, 2, 3 };
  config2.anotherInt = 2;
  cout << config2.to_string() << endl;
  config2.to_file("file3.txt");

  Config2 config2b;
  config2b.from_file("file3.txt");
  cout << config2b.to_string() << endl;

  assert(config2==config2b);

  return 0;
}
