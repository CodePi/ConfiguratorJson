// Copyright (C) 2020 Paul Ilardi (http://github.com/CodePi)
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, unconditionally.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#pragma once

#include "json.hpp"
#include <vector>
#include <map>
#include <set>
#include <array>
#include <string>
#include <fstream>
#include <iomanip>
#include "Optional.h"

namespace codepi{

class ConfiguratorJson{
public:

    // serialize to json
    nlohmann::json to_json(){
        nlohmann::json j;
        cfgMultiFunction(CFGJS_WRITE_ALL, nullptr, nullptr, &j, nullptr);
        return j;
    }
    std::string to_string(int indent=-1){
        return to_json().dump(indent);
    }
    void to_stream(std::ostream& os, int indent=-1){
        os << std::setw(indent) << to_json();
    }
    void to_file(const std::string& fname, int indent=-1){
        std::ofstream ofs(fname);
        if(!ofs) throw std::runtime_error("ConfiguratorJson::to_file: file could not be opened: "+fname);
        to_stream(ofs, indent);
    }
    std::vector<uint8_t> to_bson() {
        return nlohmann::json::to_bson(to_json());
    }

    // deserialize from json
    void from_json(const nlohmann::json& js){
        for(auto& kv : js.items()){
            cfgMultiFunction(CFGJS_SET, &kv.key(), &kv.value(), nullptr, nullptr);
        }
    }
    void from_string(const std::string& str) {
        nlohmann::json js = nlohmann::json::parse(str);
        from_json(js);
    }
    void from_stream(std::istream& is) {
        nlohmann::json j;
        is >> j;
        from_json(j);
    }
    void from_file(const std::string& fname) {
        std::ifstream ifs(fname);
        if(!ifs) throw std::runtime_error("ConfiguratorJson::from_file: file could not be opened: "+fname);
        from_stream(ifs);
    }
    void from_bson(const std::vector<uint8_t>& bson) {
        nlohmann::json js = nlohmann::json::from_bson(bson);
        from_json(js);
    }

    // comparison
    bool operator==(ConfiguratorJson& other) { return this->to_json()==other.to_json();}
    bool operator!=(ConfiguratorJson& other) { return this->to_json()!=other.to_json();}
    bool operator<(const ConfiguratorJson& other) const { return remove_const(*this).to_json()<remove_const(other).to_json();}

    virtual std::string getStructName()=0;

protected:
    enum MFType{CFGJS_INIT_ALL,CFGJS_SET,CFGJS_WRITE_ALL};

    /// Helper method that is called by all of the public methods above.
    ///   This method is automatically generated in subclass using macros below
    ///   Returns the number of variables matched
    virtual int cfgMultiFunction(MFType mfType, const std::string* str,
                                 const nlohmann::json* jsonIn, nlohmann::json* jsonOut,
                                 ConfiguratorJson* other)=0;

    /// returns default value of type T
    template <typename T> static T cfgGetDefaultVal(const T&var){return T();}
    /// overridable method called on parse error
    virtual void throwError(const std::string& error){ throw std::runtime_error(error); }


    //////////////////////////////////////////////////////////////////
    // cfgSetFromJson(json, val)
    // Used internally by cfgMultiFunction
    // Sets value of val based on contents of json
    // Overloaded for multiple types: string, configurator descendants, bool,
    //   pair, various STL containers, and primitives

    /// cfgSetFromJson for descendants of ConfiguratorJson
    static void cfgSetFromJson(const nlohmann::json& js, ConfiguratorJson& cfg){
        cfg.from_json(js);
    }

    /// cfgSetFromJson for Optional<T>
    template <typename T>
    static void cfgSetFromJson(const nlohmann::json& js, Optional<T>& val){
        cfgSetFromJson(js, (T&)val);
    }

    /// cfgSetFromJson for vector
    template <typename T>
    static void cfgSetFromJson(const nlohmann::json& js, std::vector<T>& val) { cfgContainerSetFromJson(js, val); }

    /// cfgSetFromJson for array
    template <typename T, size_t N>
    static void cfgSetFromJson(const nlohmann::json& js, std::array<T, N>& val) { cfgContainerSetFromJson(js, val); }

    /// cfgSetFromJson for set
    template <typename T>
    static void cfgSetFromJson(const nlohmann::json& js, std::set<T>& val) { cfgContainerSetFromJson(js, val); }

    /// cfgSetFromJson for map
    template <typename T1, typename T2>
    static void cfgSetFromJson(const nlohmann::json& js, std::map<T1, T2>& val) { cfgContainerSetFromJson(js, val); }

    /// cfgSetFromJson for pair
    template <typename T1, typename T2>
    static void cfgSetFromJson(const nlohmann::json& js, std::pair<T1, T2>& val) {
        assert(js.size()==2);
        cfgSetFromJson(js.at(0), remove_const(val.first));
        cfgSetFromJson(js.at(1), val.second);
    }


    /// cfgContainerSetFromJson helper function for other containers
    template <typename Container>
    static void cfgContainerSetFromJson(const nlohmann::json& js, Container& container) {
        clear_helper(container);
        int i=0;
        for(nlohmann::json jval : js) {
            typename Container::value_type val;
            cfgSetFromJson(jval,val);
            insert_helper(container, i, val);
            i++;
        }
    }

    /// cfgSetFromJson for all other types
    /// the enable_if is required to prevent it from matching on ConfiguratorJson descendants
    template <typename T>
    static typename std::enable_if<!std::is_base_of<ConfiguratorJson,T>::value,void>::type
    cfgSetFromJson(const nlohmann::json& js, T& val){
        val = js.get<T>();
    }

    //////////////////////////////////////////////////////////////////
    // cfgWriteToJsonHelper(stream, val)
    // Used internally by cfgMultiFunction
    // Writes the contents of val to the json
    // Overloaded for multiple types: string, configurator descendants, bool,
    //   pair, various STL containers, and primitives

    /// cfgWriteToJsonHelper for descendants of ConfiguratorJson
    static void cfgWriteToJsonHelper(nlohmann::json& js, ConfiguratorJson& val){
        js = val.to_json();
    }

    /// cfgWriteToJsonHelper for Optional<T>
    /// Prints contents of Optional.
    template <typename T>
    static void cfgWriteToJsonHelper(nlohmann::json& js, Optional<T>& val){
        // shouldn't be able to get this far if not set
        if(!val.isSet()) throw std::runtime_error("cfgWriteToJsonHelper Optional<T>: this shouldn't happen");
        cfgWriteToJsonHelper(js, (T&)val);
    }

    /// cfgWriteToJsonHelper for vector
    template <typename T>
    static void cfgWriteToJsonHelper(nlohmann::json& js, std::vector<T>& val) { cfgContainerWriteToJsonHelper(js, val); }

    /// cfgWriteToJsonHelper for array
    template <typename T, size_t N>
    static void cfgWriteToJsonHelper(nlohmann::json& js, std::array<T, N>& val) { cfgContainerWriteToJsonHelper(js, val); }

    /// cfgWriteToJsonHelper for set
    template <typename T>
    static void cfgWriteToJsonHelper(nlohmann::json& js, std::set<T>& val) { cfgContainerWriteToJsonHelper(js, val); }

    /// cfgWriteToJsonHelper for map
    template <typename T1, typename T2>
    static void cfgWriteToJsonHelper(nlohmann::json& js, std::map<T1, T2>& val) { cfgContainerWriteToJsonHelper(js, val); }

    /// cfgWriteToJsonHelper for pair
    template <typename T1, typename T2>
    static void cfgWriteToJsonHelper(nlohmann::json& js, std::pair<T1, T2>& val) {
        nlohmann::json jkey;
        nlohmann::json jval;
        cfgWriteToJsonHelper(jkey, val.first);
        cfgWriteToJsonHelper(jval, val.second);
        js = {std::move(jkey), std::move(jval)};
    }

    /// cfgContainerWriteToJsonHelper for other containers
    template <typename Container>
    static void cfgContainerWriteToJsonHelper(nlohmann::json& js, Container& container) {
        js = nlohmann::json::array();
        for(auto& val : container) {
            nlohmann::json j;
            cfgWriteToJsonHelper(j, remove_const(val));
            js.push_back(std::move(j));
        }
    }

    /// cfgWriteToJsonHelper for all other types
    /// the enable_if is required to prevent it from matching on ConfiguratorJson descendants
    template <typename T>
    static typename std::enable_if<!std::is_base_of<ConfiguratorJson,T>::value,void>::type
    cfgWriteToJsonHelper(nlohmann::json& js, T& val){
        js = val;
    }

    /// returns true if optional type and value is set
    template<typename T>
    static bool cfgIsSetOrNotOptional(Optional<T>& opt){
        return opt.isSet();
    }

    /// returns true if not instance of Optional<T>
    template<typename T>
    static bool cfgIsSetOrNotOptional(T& t){
        return true;
    }

    //////////////////////////////
    // Helper function to distinguish between stl arrays and other containers

    // clear array by filling it with default values
    template<typename T, size_t N>
    static void clear_helper(std::array<T,N>& arr){
        arr.fill(T());
    }

    // clear container
    template<typename Container>
    static void clear_helper(Container& container){
        container.clear();
    }

    // inserting into array by index
    template<typename T, size_t N>
    static void insert_helper(std::array<T,N>& arr, size_t i, T& val){
        if(i>=N) throw std::range_error("insert exceeds array size");
        arr[i] = val;
    }

    // inserting into end of container (ignoring index, but should match anyway)
    template<typename Container, typename T>
    static void insert_helper(Container& container, size_t i, T& val){
        assert(container.size()==i);
        container.insert(container.end(),val);
    }

    /// helper function for cfgSetFromStream for pairs
    /// workaround: a map's value_type is pair<const T1, T2> this casts off the const
    template <typename T>
    static T& remove_const(const T& val){
        return const_cast<T&>(val);
    }
};

//////////////////////////////////////////////////////////////////
// Macros to automatically generate the cfgMultiFunction method in
// descendant classes.

// automatically generates subclass constructor and begins cfgMultiFunction method
#define CFGJS_HEADER(structName) \
  structName() { cfgMultiFunction(CFGJS_INIT_ALL,nullptr,nullptr,nullptr,nullptr); } \
  std::string getStructName() { return #structName; } \
  int cfgMultiFunction(MFType mfType, const std::string* str, \
    const nlohmann::json* jsonIn, nlohmann::json* jsonOut,ConfiguratorJson*other){ \
    int retVal=0; \
    structName* otherPtr;

// continues cfgMultiFunction method, called for each member variable in struct 
#define CFGJS_ENTRY2(varName, defaultVal) \
  if(mfType==CFGJS_INIT_ALL) { \
    if(cfgIsSetOrNotOptional(varName)) {varName = defaultVal;retVal++;} \
    } else if(mfType==CFGJS_SET && #varName==*str) { cfgSetFromJson(*jsonIn,varName);retVal++;} \
  else if(mfType==CFGJS_WRITE_ALL) { \
    if(cfgIsSetOrNotOptional(varName)) { \
      nlohmann::json jsonTmp;                    \
      cfgWriteToJsonHelper(jsonTmp,varName);    \
      (*jsonOut)[#varName] = jsonTmp; retVal++; \
    } \
  }

// alternative to CFGJS_ENTRY2 used when default defaultVal is sufficient
#define CFGJS_ENTRY1(varName) CFGJS_ENTRY2(varName, cfgGetDefaultVal(varName))

// calls cfgMultiFunction method of parent
// allows for inheritance
#define CFGJS_PARENT(parentName) \
  int rc=parentName::cfgMultiFunction(mfType,str,jsonIn,jsonOut,other); \
  retVal+=rc;

// closes out cfgMultiFunction method
#define CFGJS_TAIL return retVal; }

}  //namespace codepi
