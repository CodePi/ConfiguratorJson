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

    /// serialize to json (string, stream, file, or bson)
    nlohmann::ordered_json to_json() const{
        nlohmann::ordered_json js;
        const_cast<ConfiguratorJson*>(this)->cfgMultiFunction(CFGJS_WRITE_ALL, nullptr, nullptr, &js);
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
        if(!ofs) throwError("ConfiguratorJson::to_file: file could not be opened: "+fname);
        to_stream(ofs, indent);
    }
    std::vector<uint8_t> to_bson() const{
        return nlohmann::ordered_json::to_bson(to_json());
    }
    friend std::ostream& operator<<(std::ostream& os, const ConfiguratorJson& cfg) {
        os << cfg.to_json();
        return os;
    }
    friend void to_json(nlohmann::ordered_json& js, const ConfiguratorJson& cfg) {
        js = cfg.to_json();
    }

    /// deserialize from json (string, stream, file, or bson)
    void from_json(const nlohmann::ordered_json& js) {
        for(auto& kv : js.items()){
            int num_matches = cfgMultiFunction(CFGJS_SET, &kv.key(), &kv.value(), nullptr);
            if(num_matches==0 && !allow_keys_not_in_struct()) {
                throwError("from_json error: \"" + kv.key() + "\" not a member of " + getStructName());
            }
        }
    }
    void from_string(const std::string& str)         { from_json(nlohmann::ordered_json::parse(str)); }
    void from_string(const char* str)                { from_json(nlohmann::ordered_json::parse(str)); }
    void from_string(const char* str, size_t n)      { from_json(nlohmann::ordered_json::parse(str, str+n)); }
    void from_stream(std::istream& is)               { from_json(nlohmann::ordered_json::parse(is)); }
    void from_bson(const std::vector<uint8_t>& bson) { from_json(nlohmann::ordered_json::from_bson(bson)); }
    void from_file(const std::string& fname) {
        std::ifstream ifs(fname);
        if(!ifs) throwError("ConfiguratorJson::from_file: file could not be opened: "+fname);
        from_stream(ifs);
    }
    friend std::istream& operator>>(std::istream& is, ConfiguratorJson& cfg) {
        cfg.from_stream(is);
        return is;
    }
    friend void from_json(const nlohmann::ordered_json& js, ConfiguratorJson& cfg) { cfg.from_json(js); }

    /// comparison
    bool operator==(const ConfiguratorJson& other) const { return this->to_json() == other.to_json();}
    bool operator!=(const ConfiguratorJson& other) const { return this->to_json() != other.to_json();}
    bool operator< (const ConfiguratorJson& other) const { return this->to_json() <  other.to_json();}

    /// returns struct name, automatically generated by macros in descendant classes
    virtual std::string getStructName() const =0;

    /// virtual destructor that can be overridden in descendant classes
    virtual ~ConfiguratorJson() =default;

protected:
    enum MFType{CFGJS_INIT_ALL,CFGJS_SET,CFGJS_WRITE_ALL};

    /// Helper method that is called by all of the public methods above.
    ///   This method is automatically generated in subclass using macros below
    ///   Returns the number of variables matched
    virtual int cfgMultiFunction(MFType mfType, const std::string* str,
                                 const nlohmann::ordered_json* jsonIn, nlohmann::ordered_json* jsonOut)=0;

    /// returns default value of type T
    template <typename T> static T cfgGetDefaultVal(const T&var){return T();}
    /// overridable method called on parse error (note: override must be const)
    virtual void throwError(const std::string& error) const { throw std::runtime_error(error); }
    /// overridable method used to control whether json keys that don't match members are allowed
    /// Note: override also needs to be const
    virtual bool allow_keys_not_in_struct() const { return false; }

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

};

// from_json for Optional class
template <typename T>
void from_json(const nlohmann::ordered_json& js, Optional<T>& val) {
    if(js.empty()) val.unset();
    else js.get_to(val.get());
}

// to_json for Optional class
template <typename T>
void to_json(nlohmann::ordered_json& js, const Optional<T>& val) {
    if(val.isSet()) js = val.get();
    else js={};
}

//////////////////////////////////////////////////////////////////
// Macros to automatically generate the cfgMultiFunction method in
// descendant classes.

// automatically generates subclass constructor and begins cfgMultiFunction method
#define CFGJS_HEADER(structName) \
  structName() { cfgMultiFunction(CFGJS_INIT_ALL,nullptr,nullptr,nullptr); } \
  CFGJS_HEADER_NO_CTOR(structName)

// same as CFGJS_HEADER, but without generating a constructor
#define CFGJS_HEADER_NO_CTOR(structName)                                    \
  std::string getStructName() const { return #structName; }                 \
  int cfgMultiFunction(MFType mfType, const std::string* str,               \
    const nlohmann::ordered_json* jsonIn, nlohmann::ordered_json* jsonOut){ \
      int retVal=0;

// continues cfgMultiFunction method, called for each member variable in struct 
#define CFGJS_ENTRY_DEF(varName, defaultVal)                                         \
  if(mfType==CFGJS_INIT_ALL) {                                                       \
    if(cfgIsSetOrNotOptional(varName)) {                                             \
      varName = defaultVal; retVal++;                                                \
    }                                                                                \
  } else if(mfType==CFGJS_SET && #varName==*str) {                                   \
    jsonIn->get_to(varName); retVal++;                                               \
  } else if(mfType==CFGJS_WRITE_ALL) {                                               \
    if(cfgIsSetOrNotOptional(varName)) {                                             \
      (*jsonOut)[#varName] = varName; retVal++;                                      \
    }                                                                                \
  }

// alternative to CFGJS_ENTRY_DEF used when default defaultVal is sufficient
#define CFGJS_ENTRY(varName) CFGJS_ENTRY_DEF(varName, cfgGetDefaultVal(varName))

// multientry
#define CFGJS_MULTIENTRY1(v1)                              CFGJS_ENTRY(v1)
#define CFGJS_MULTIENTRY2(v1,v2)                           CFGJS_ENTRY(v1) CFGJS_MULTIENTRY1(v2)
#define CFGJS_MULTIENTRY3(v1,v2,v3)                        CFGJS_ENTRY(v1) CFGJS_MULTIENTRY2(v2,v3)
#define CFGJS_MULTIENTRY4(v1,v2,v3,v4)                     CFGJS_ENTRY(v1) CFGJS_MULTIENTRY3(v2,v3,v4)
#define CFGJS_MULTIENTRY5(v1,v2,v3,v4,v5)                  CFGJS_ENTRY(v1) CFGJS_MULTIENTRY4(v2,v3,v4,v5)
#define CFGJS_MULTIENTRY6(v1,v2,v3,v4,v5,v6)               CFGJS_ENTRY(v1) CFGJS_MULTIENTRY5(v2,v3,v4,v5,v6)
#define CFGJS_MULTIENTRY7(v1,v2,v3,v4,v5,v6,v7)            CFGJS_ENTRY(v1) CFGJS_MULTIENTRY6(v2,v3,v4,v5,v6,v7)
#define CFGJS_MULTIENTRY8(v1,v2,v3,v4,v5,v6,v7,v8)         CFGJS_ENTRY(v1) CFGJS_MULTIENTRY7(v2,v3,v4,v5,v6,v7,v8)
#define CFGJS_MULTIENTRY9(v1,v2,v3,v4,v5,v6,v7,v8,v9)      CFGJS_ENTRY(v1) CFGJS_MULTIENTRY8(v2,v3,v4,v5,v6,v7,v8,v9)
#define CFGJS_MULTIENTRY10(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10) CFGJS_ENTRY(v1) CFGJS_MULTIENTRY9(v2,v3,v4,v5,v6,v7,v8,v9,v10)

// calls cfgMultiFunction method of parent
// allows for inheritance
#define CFGJS_PARENT(parentName) \
  int rc=parentName::cfgMultiFunction(mfType,str,jsonIn,jsonOut); \
  retVal+=rc;

// closes out cfgMultiFunction method
#define CFGJS_TAIL return retVal; }

}  //namespace codepi
