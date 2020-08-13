#include <iostream>
#include "../ConfiguratorJson.h"

using namespace std;
using namespace codepi;

/*struct SubConfig2:public codepi::ConfiguratorJson{
    int k;

    CFGJS_HEADER(SubConfig2)
    CFGJS_ENTRY2(k,9)
    CFGJS_TAIL
};

struct SubConfig1:public SubConfig2{
    int i,j;

    CFGJS_HEADER(SubConfig1)
    CFGJS_ENTRY2(i,7)
    CFGJS_ENTRY1(j)
    CFGJS_PARENT(SubConfig2)
    CFGJS_TAIL
};*/

struct TestConfig : public codepi::ConfiguratorJson{

    int jjj;
    /*std::vector<int> k;
    std::array<int,10> arr;
    std::set<int> intSet;
    std::pair<int, std::string> pair;
    std::pair<std::string, float> pair2;
    std::map<std::string, int> map;
    std::string n;
    SubConfig1 s;
    std::vector<SubConfig1> t;
    std::vector<std::string> strList;
    SubConfig1 u;
    bool b;
    codepi::Optional<int> opt1, opt2, opt3;
    codepi::Optional< std::vector<int> > optvec;*/

    CFGJS_HEADER(TestConfig)
    CFGJS_ENTRY2(jjj,12)
    /*CFGJS_ENTRY1(k)
    CFGJS_ENTRY1(arr)
    CFGJS_ENTRY1(intSet)
    CFGJS_ENTRY1(pair)
    CFGJS_ENTRY1(pair2)
    CFGJS_ENTRY1(map)
    CFGJS_ENTRY2(n,"hello")
    CFGJS_ENTRY1(s)
    CFGJS_ENTRY1(t)
    CFGJS_ENTRY1(strList)
    CFGJS_ENTRY1(u)
    CFGJS_ENTRY1(b)
    CFGJS_ENTRY1(opt1)
    CFGJS_ENTRY1(opt2)
    CFGJS_ENTRY1(opt3)
    CFGJS_ENTRY1(optvec)*/
    CFGJS_TAIL
};


int main() {
    TestConfig tc;
    cout << tc.to_string() << "\n";

}
