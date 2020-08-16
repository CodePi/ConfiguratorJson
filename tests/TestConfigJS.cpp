#include <iostream>
#include <set>
#include "../ConfiguratorJson/ConfiguratorJson.h"

using namespace std;
using namespace codepi;

struct SubConfig2:public codepi::ConfiguratorJson{
    int k;

    CFGJS_HEADER(SubConfig2)
    CFGJS_ENTRY_DEF(k,9)
    CFGJS_TAIL
};

struct SubConfig1:public SubConfig2{
    int i,j;

    CFGJS_HEADER(SubConfig1)
    CFGJS_ENTRY_DEF(i,7)
    CFGJS_ENTRY(j)
    CFGJS_PARENT(SubConfig2)
    CFGJS_TAIL
};

struct TestConfig : public codepi::ConfiguratorJson{

    int jjj;
    std::string n;
    std::vector<int> k;
    std::array<int,10> arr;
    std::set<int> intSet;
    std::pair<int, std::string> pair;
    std::pair<std::string, float> pair2;
    std::map<std::string, int> map;
    SubConfig1 s;
    SubConfig1 u;
    std::vector<SubConfig1> subvec;
    std::array<SubConfig1, 10> subarr;
    std::set<SubConfig1> subset;
    std::map<string, SubConfig1> submap;
    std::vector<std::string> strList;
    bool b;
    codepi::Optional<int> opt1, opt2, opt3;
    codepi::Optional< std::vector<int> > optvec;
    codepi::Optional<SubConfig1> optsc1;
    SubConfig2 sc2;
    std::map<int, string> intstrmap;

    CFGJS_HEADER(TestConfig)
    CFGJS_ENTRY_DEF(jjj,12)
    CFGJS_ENTRY_DEF(n,"hello")
    CFGJS_MULTIENTRY10(k,arr,intSet,pair,pair2,map,n,s,u,subvec)
    CFGJS_MULTIENTRY5(subarr,subset,submap,strList,b)
    CFGJS_MULTIENTRY5(opt1,opt2,opt3,optvec,optsc1)
    CFGJS_MULTIENTRY2(sc2, intstrmap)
    CFGJS_TAIL
};

int main() {
    TestConfig tc;
    tc.jjj = 17;
    tc.k = {1,2,3,4,5};
    tc.arr = {1,2,3,4,5,6,7,8,9,10};
    tc.intSet = {3,1,4,1,5,9};
    tc.pair = {1,"hello"};
    tc.pair2 = {"goodbye", 7};
    tc.map = {{"a",123}, {"b", 456}};
    tc.n = "foo";
    tc.s.i = 5;
    tc.s.j = 6;
    tc.s.k = 7;
    tc.subvec = {tc.s, tc.u};
    tc.subarr.fill(tc.s);
    tc.subset = {tc.s, tc.u};
    tc.submap = {{"a", tc.s}, {"b", tc.u}};
    tc.strList = {"a","b","c"};
    tc.b = true;
    tc.opt2 = 2;
    tc.optvec = {1,2,3,4,5};
    tc.optsc1 = tc.s;
    tc.intstrmap = {{1,"a"},{2,"b"}};
    string str = tc.to_string();
    cout << str << "\n";

    // test serialization/deserialization
    TestConfig tc2;
    tc2.from_string(str);
    cout << tc2 << "\n";
    assert(tc==tc2);

    // sizes of serialization
    cout << "json size: " << str.size() << "\n";
    cout << "bson size: " << tc.to_bson().size() << "\n";
    cout << "ubjson size: " << nlohmann::json::to_ubjson(tc.to_json()).size() << "\n";
    cout << "cbor size: " << nlohmann::json::to_cbor(tc.to_json()).size() << "\n";
    cout << "msgpack size: " << nlohmann::json::to_msgpack(tc.to_json()).size() << "\n";

    // test conversion to binary
    TestConfig tc3, tc4, tc5, tc6;
    tc3.from_bson(tc.to_bson());
    tc4.from_json(nlohmann::json::from_ubjson(nlohmann::json::to_ubjson(tc.to_json())));
    tc5.from_json(nlohmann::json::from_cbor(nlohmann::json::to_cbor(tc.to_json())));
    tc6.from_json(nlohmann::json::from_msgpack(nlohmann::json::to_msgpack(tc.to_json())));
    assert(tc==tc3);
    assert(tc==tc4);
    assert(tc==tc5);
    assert(tc==tc6);

    return 0;
}
