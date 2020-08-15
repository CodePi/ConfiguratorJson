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

#include <vector>
#include <map>
#include <set>
#include <array>
#include <string>
#include <fstream>
#include <iomanip>

#include "json.hpp"
#include "Optional.h"

namespace codepi{

class ConfiguratorJson{
public:

    // serialize to json
    nlohmann::json to_json() const{
        nlohmann::json js;
        cfgWriteToJson(js, *this);
        return js;
    }
    std::string to_string(int indent=-1) const{
        return to_json().dump(indent);
    }
    void to_stream(std::ostream& os, int indent=-1) const{
        auto oldwidth = os.width(indent);
        os << to_json();
        os.width(oldwidth);
    }
    void to_file(const std::string& fname, int indent=-1) const{
        std::ofstream ofs(fname);
        if(!ofs) throw std::runtime_error("ConfiguratorJson::to_file: file could not be opened: "+fname);
        to_stream(ofs, indent);
    }
    std::vector<uint8_t> to_bson() const{
        return nlohmann::json::to_bson(to_json());
    }

    // deserialize from json
    void from_json(const nlohmann::json& js){
        cfgSetFromJson(js, *this);
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
    bool operator==(const ConfiguratorJson& other) const { return this->to_json()==other.to_json();}
    bool operator!=(const ConfiguratorJson& other) const { return this->to_json()!=other.to_json();}
    bool operator<(const ConfiguratorJson& other) const { return to_json() < other.to_json();}

    virtual std::string getStructName() const =0;
    virtual ~ConfiguratorJson() =default;

protected:
    enum MFType{CFGJS_INIT_ALL,CFGJS_SET,CFGJS_WRITE_ALL};

    /// Helper method that is called by all of the public methods above.
    ///   This method is automatically generated in subclass using macros below
    ///   Returns the number of variables matched
    virtual int cfgMultiFunction(MFType mfType, const std::string* str,
                                 const nlohmann::json* jsonIn, nlohmann::json* jsonOut)=0;

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
        for(auto& kv : js.items()){
            int rc = cfg.cfgMultiFunction(CFGJS_SET, &kv.key(), &kv.value(), nullptr);
            if(rc==0) throw std::runtime_error("from_json error: "+kv.key()+" not in struct "+cfg.getStructName());
        }
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

    /// cfgSetFromJson for maps with string as key
    template <typename T2>
    static void cfgSetFromJson(const nlohmann::json& js, std::map<std::string, T2>& map) {
        map.clear();
        for(auto& kv : js.items()) {
            T2 val;
            cfgSetFromJson(kv.value(), val);
            map[kv.key()] = std::move(val);
        }
    }

    /// cfgSetFromJson for other maps
    /// Note: json doesn't properly support map with non-string key.  So treat as array of pairs.
    template <typename T1, typename T2>
    static void cfgSetFromJson(const nlohmann::json& js, std::map<T1, T2>& val) { cfgContainerSetFromJson(js, val); }

    /// cfgSetFromJson for pair
    template <typename T1, typename T2>
    static void cfgSetFromJson(const nlohmann::json& js, std::pair<T1, T2>& val) {
        if(js.size()!=2) throw std::runtime_error("cfgSetFromJson: json pair must be size 2");
        // Note: the remove_const is needed because map::value_type is pair<const T1, T2>
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
            insert_helper(container, i, std::move(val));
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
    // cfgWriteToJson(stream, val)
    // Used internally by cfgMultiFunction
    // Writes the contents of val to the json
    // Overloaded for multiple types: string, configurator descendants, bool,
    //   pair, various STL containers, and primitives

    /// cfgWriteToJson for descendants of ConfiguratorJson
    static void cfgWriteToJson(nlohmann::json& js, const ConfiguratorJson& val){
        // Note: the remove_const is needed because cfgMultiFunction is non-const,
        //       but will not modify the object if mfType is CFGJS_WRITE_ALL
        remove_const(val).cfgMultiFunction(CFGJS_WRITE_ALL, nullptr, nullptr, &js);
    }

    /// cfgWriteToJson for Optional<T>
    /// Prints contents of Optional.
    template <typename T>
    static void cfgWriteToJson(nlohmann::json& js, const Optional<T>& val){
        if(!val.isSet()) throw std::runtime_error("cfgWriteToJson Optional<T>: this shouldn't happen");
        cfgWriteToJson(js, (const T&)val);
    }

    /// cfgWriteToJson for vector
    template <typename T>
    static void cfgWriteToJson(nlohmann::json& js, const std::vector<T>& val) { cfgContainerWriteToJsonHelper(js, val); }

    /// cfgWriteToJson for array
    template <typename T, size_t N>
    static void cfgWriteToJson(nlohmann::json& js, const std::array<T, N>& val) { cfgContainerWriteToJsonHelper(js, val); }

    /// cfgWriteToJson for set
    template <typename T>
    static void cfgWriteToJson(nlohmann::json& js, const std::set<T>& val) { cfgContainerWriteToJsonHelper(js, val); }

    /// cfgWriteToJson for map with string as key
    template <typename T2>
    static void cfgWriteToJson(nlohmann::json& js, const std::map<std::string, T2>& val) {
        for(auto& kv : val) {
            nlohmann::json jval;
            cfgWriteToJson(jval, kv.second);
            js[kv.first] = jval;
        }
    }

    /// cfgWriteToJson for other maps
    /// Note: json doesn't properly support map with non-string key.  So treat as array of pairs.
    template <typename T1, typename T2>
    static void cfgWriteToJson(nlohmann::json& js, const std::map<T1, T2>& val) { cfgContainerWriteToJsonHelper(js, val); }

    /// cfgWriteToJson for pair
    template <typename T1, typename T2>
    static void cfgWriteToJson(nlohmann::json& js, const std::pair<T1, T2>& val) {
        nlohmann::json jfirst;
        nlohmann::json jsecond;
        cfgWriteToJson(jfirst, val.first);
        cfgWriteToJson(jsecond, val.second);
        js = {std::move(jfirst), std::move(jsecond)};
    }

    /// cfgContainerWriteToJsonHelper for other containers
    template <typename Container>
    static void cfgContainerWriteToJsonHelper(nlohmann::json& js, const Container& container) {
        js = nlohmann::json::array();
        for(auto& val : container) {
            nlohmann::json j;
            cfgWriteToJson(j, val);
            js.push_back(std::move(j));
        }
    }

    /// cfgWriteToJson for all other types
    /// the enable_if is required to prevent it from matching on ConfiguratorJson descendants
    template <typename T>
    static typename std::enable_if<!std::is_base_of<ConfiguratorJson,T>::value,void>::type
    cfgWriteToJson(nlohmann::json& js, const T& val){
        js = val;
    }

    /// returns true if optional type and value is set
    template<typename T>
    static bool cfgIsSetOrNotOptional(const Optional<T>& opt){
        return opt.isSet();
    }

    /// returns true if not instance of Optional<T>
    template<typename T>
    static bool cfgIsSetOrNotOptional(const T& t){
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
    template<typename T, size_t N, typename U>
    static void insert_helper(std::array<T,N>& arr, size_t i, U&& val){
        if(i>=N) throw std::range_error("insert exceeds array size");
        arr[i] = std::forward<U>(val);
    }

    // inserting into end of container (ignoring index, but should match anyway)
    template<typename Container, typename U>
    static void insert_helper(Container& container, size_t i, U&& val){
        if(container.size()!=i) throw std::runtime_error("insert_helper: insert_helper improperly used");
        container.insert(container.end(),std::forward<U>(val));
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
  structName() { cfgMultiFunction(CFGJS_INIT_ALL,nullptr,nullptr,nullptr); } \
  std::string getStructName() const { return #structName; } \
  int cfgMultiFunction(MFType mfType, const std::string* str, \
    const nlohmann::json* jsonIn, nlohmann::json* jsonOut){ \
    int retVal=0;

// continues cfgMultiFunction method, called for each member variable in struct 
#define CFGJS_ENTRY2(varName, defaultVal) \
  if(mfType==CFGJS_INIT_ALL) { \
    if(cfgIsSetOrNotOptional(varName)) {varName = defaultVal;retVal++;} \
    } else if(mfType==CFGJS_SET && #varName==*str) { cfgSetFromJson(*jsonIn,varName);retVal++;} \
  else if(mfType==CFGJS_WRITE_ALL) { \
    if(cfgIsSetOrNotOptional(varName)) { \
      nlohmann::json jsonTmp;                    \
      cfgWriteToJson(jsonTmp,varName);    \
      (*jsonOut)[#varName] = jsonTmp; retVal++; \
    } \
  }

// alternative to CFGJS_ENTRY2 used when default defaultVal is sufficient
#define CFGJS_ENTRY1(varName) CFGJS_ENTRY2(varName, cfgGetDefaultVal(varName))

// calls cfgMultiFunction method of parent
// allows for inheritance
#define CFGJS_PARENT(parentName) \
  int rc=parentName::cfgMultiFunction(mfType,str,jsonIn,jsonOut); \
  retVal+=rc;

// closes out cfgMultiFunction method
#define CFGJS_TAIL return retVal; }

}  //namespace codepi
