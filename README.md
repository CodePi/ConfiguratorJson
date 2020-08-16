# ConfiguratorJson
Header only hierarchical C++ struct human readable serializer/deserializer.  Originally made for configuration file reading/writing.

Tested with g++ 4.8 through 10 and clang++ 

### Example usage
``` cpp
struct Config1 : public ConfiguratorJson {
  int exampleIntValue;
  float exampleFloat;
  string exampleString;
  vector<int> exampleVector;
  map<string, int> exampleMap;
  int i,j,k;
  float x,y,z;

  CFGJS_HEADER(Config1)
  CFGJS_ENTRY_DEF(exampleIntValue, 12)  // entry with explicit default value
  CFGJS_ENTRY(exampleFloat)             // entry with auto default value for type
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
  config2.to_file("file.txt");

  Config2 config2b;
  config2b.from_file("file.txt");
  cout << config2b.to_string() << endl;
}
```
See tests directory for more examples.

#### Corresponding serialization
```
{
  "exampleSub":{
    "exampleIntValue":1,
    "exampleFloat":0,
    "exampleString":"a string",
    "exampleVector":[1,2,3],
    "exampleMap":{"a":1,"b":2}
  },
  "anotherInt":2
}
```

#### Useful ConfiguratorJson methods
``` cpp
class ConfiguratorJson{
public:
  /// read and parse file / stream / string
  nlohmann::json to_json();
  std::string to_string(int indent=-1);
  void to_stream(std::ostream& os, int indent=-1);
  void to_file(const std::string& fname, int indent=-1);
  std::vector<uint8_t> to_bson();

  /// write contents of struct to file / stream / string
  void from_json(nlohmann::json& js);
  void from_string(const std::string& str);
  void from_stream(std::istream& is);
  void from_file(const std::string& fname);
  void from_bson(const std::vector<uint8_t>& bson);

  /// equality
  bool operator==(ConfiguratorJson& other);
  bool operator!=(ConfiguratorJson& other);
  bool operator<(ConfiguratorJson& other);
};
```
#### Supported types
* All primitives
* Most std containers: string, vector, set, map, array, pair
* Nested supported types: vector of vector, vector of outfitted struct, etc...
* Any type recognized by nlohmann::json
