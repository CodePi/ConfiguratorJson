#pragma once

#include "json.hpp"
#include <string>
#include <fstream>
#include <iomanip>

//TODO remove indent

namespace codepi{

class ConfiguratorJson{
public:

    // to
    nlohmann::json to_json(){
        nlohmann::json j;
        cfgMultiFunction(CFGJS_WRITE_ALL, NULL, NULL, NULL, &j, NULL);
        return j;
    }
    std::string to_string(int indent=0){
        return to_json().dump(indent);
    }
    void to_stream(std::ostream& os, int indent=0){
        os << to_json();  //TODO: indent
    }
    void to_file(const std::string& fname, int indent=0){
        std::ofstream ofs(fname);
        to_stream(ofs ,indent);
    }

    // from
    void from_json(const nlohmann::json& j){
        //TODO
    }
    void from_string(const std::string& str) {
        from_json(nlohmann::json::parse(str));
    }
    void from_file(const std::string& fname) {
        std::ifstream ifs(fname);
        from_stream(ifs);
    }
    void from_stream(std::istream& is) {
        nlohmann::json j;
        is >> j;
        from_json(j);
    }

protected:
    enum MFType{CFGJS_INIT_ALL,CFGJS_SET,CFGJS_WRITE_ALL,CFGJS_COMPARE};

    /// Helper method that is called by all of the public methods above.
    ///   This method is automatically generated in subclass using macros below
    ///   Returns the number of variables matched
    virtual int cfgMultiFunction(MFType mfType, std::string* str, std::string* subVar,
                                 nlohmann::json* jsonIn, nlohmann::json* jsonOut,
                                 ConfiguratorJson* other)=0;

    /// returns default value of type T
    template <typename T> static T cfgGetDefaultVal(const T&var){return T();}
    /// overridable method called on parse error
    virtual void throwError(std::string error){ throw std::runtime_error(error); }

    /// cfgSetFromStream for all other types
    /// the enable_if is required to prevent it from matching on Configurator descendants
    template <typename T>
    static typename std::enable_if<!std::is_base_of<ConfiguratorJson,T>::value,void>::type
    cfgSetFromJson(nlohmann::json& js,  T& val, const std::string& subVar=""){
        if(!subVar.empty()) throw std::runtime_error("!subVar.empty()");
        val = js;
    }

    /// cfgWriteToStreamHelper for all other types
    /// the enable_if is required to prevent it from matching on Configurator descendants
    template <typename T>
    static typename std::enable_if<!std::is_base_of<ConfiguratorJson,T>::value,void>::type
    cfgWriteToJsonHelper(nlohmann::json& js, T& val){
        js = val;
    }

    /// cfgCompareHelper for any type with defined operator==
    template <typename T>
    static typename std::enable_if<!std::is_base_of<ConfiguratorJson,T>::value,int>::type
    cfgCompareHelper(T& a, T& b){
        return !(a==b);
    }
};


//////////////////////////////////////////////////////////////////
// Macros to automatically generate the cfgMultiFunction method in
// descendant classes.

// automatically generates subclass constructor and begins cfgMultiFunction method
#define CFGJS_HEADER(structName) \
  structName() { cfgMultiFunction(CFGJS_INIT_ALL,NULL,NULL,NULL,NULL,NULL); } \
  std::string getStructName() { return #structName; } \
  int cfgMultiFunction(MFType mfType, std::string* str, std::string* subVar, \
    nlohmann::json* jsonIn, nlohmann::json* jsonOut,ConfiguratorJson*other){ \
    int retVal=0; \
    structName* otherPtr; \
    if(mfType==CFGJS_COMPARE) {otherPtr = dynamic_cast<structName*>(other); \
      if(!otherPtr) return 1; /*dynamic cast failed, types different*/ }

// continues cfgMultiFunction method, called for each member variable in struct 
#define CFGJS_ENTRY2(varName, defaultVal) \
  if(mfType==CFGJS_INIT_ALL) { \
    /*TODOif(cfgIsSetOrNotOptional(varName))*/ {varName = defaultVal;retVal++;} \
    } else if(mfType==CFGJS_SET && #varName==*str) { cfgSetFromJson(*jsonIn,varName,*subVar);retVal++;} \
  else if(mfType==CFGJS_WRITE_ALL) { \
    /*TODOif(cfgIsSetOrNotOptional(varName))*/ { \
      nlohmann::json jsonTmp;                    \
      cfgWriteToJsonHelper(jsonTmp,varName);    \
      *jsonOut = {#varName, jsonTmp}; retVal++; \
    } \
  } else if(mfType==CFGJS_COMPARE) { \
    retVal+=cfgCompareHelper(this->varName,otherPtr->varName); \
  }

// alternative to CFGJS_ENTRY2 used when default defaultVal is sufficient
#define CFGJS_ENTRY1(varName) CFGJS_ENTRY2(varName, cfgGetDefaultVal(varName))

// calls cfgMultiFunction method of parent
// allows for inheritance
#define CFGJS_PARENT(parentName) \
  int rc=parentName::cfgMultiFunction(mfType,str,subVar,streamIn,streamOut,other); \
  retVal+=rc;


// closes out cfgMultiFunction method
#define CFGJS_TAIL return retVal; }


};  //namespace codepi