#pragma once

#include "json.hpp"
#include <string>
#include <fstream>
#include <iomanip>
#include "Optional.h"

//TODO: add support for containers of objects
//TODO: add support for optional

namespace codepi{

class ConfiguratorJson{
public:

    // to
    nlohmann::json to_json(){
        nlohmann::json j;
        cfgMultiFunction(CFGJS_WRITE_ALL, NULL, NULL, &j, NULL);
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
        to_stream(ofs ,indent);
    }

    // from
    void from_json(nlohmann::json& js){
        for(auto& kv : js.items()){
            cfgMultiFunction(CFGJS_SET, &kv.key(), &kv.value(), NULL, NULL);
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

    bool operator==(ConfiguratorJson& other) { return this->to_json()==other.to_json();}
    bool operator!=(ConfiguratorJson& other) { return this->to_json()!=other.to_json();}

    virtual std::string getStructName()=0;

    void set(const std::string& varName, nlohmann::json & js){
        if(varName=="include"){ // e.g. "include = filename"
            std::string filename;
            //TODO: cfgSetFromStream(js,filename); //get filename
            assert(false);
            //TODO: readFile(filename);  //parse contents of file (recurse)
        }else{ // set varName from contents of stream
            // set value of variable by parsing stream
            int rc=cfgMultiFunction(CFGJS_SET,&varName,&js,NULL,NULL);
            if(rc==0) throwError("Configurator ("+getStructName()+") error, key not recognized: "+varName);
            if(rc>1) throwError("Configurator ("+getStructName()+") error, multiple keys with the same name not allowed: "+varName);
        }
    }

protected:
    enum MFType{CFGJS_INIT_ALL,CFGJS_SET,CFGJS_WRITE_ALL};

    /// Helper method that is called by all of the public methods above.
    ///   This method is automatically generated in subclass using macros below
    ///   Returns the number of variables matched
    virtual int cfgMultiFunction(MFType mfType, const std::string* str,
                                 nlohmann::json* jsonIn, nlohmann::json* jsonOut,
                                 ConfiguratorJson* other)=0;

    /// returns default value of type T
    template <typename T> static T cfgGetDefaultVal(const T&var){return T();}
    /// overridable method called on parse error
    virtual void throwError(std::string error){ throw std::runtime_error(error); }

    void cfgSetFromJson(nlohmann::json& js, ConfiguratorJson& cfg){
        cfg.from_json(js);
    }

    /// cfgSetFromStream for Optional<T>
    template <typename T>
    static void cfgSetFromJson(nlohmann::json& js, Optional<T>& val){
        cfgSetFromJson(js, (T&)val);
    }

    /// cfgSetFromStream for all other types
    /// the enable_if is required to prevent it from matching on Configurator descendants
    template <typename T>
    static typename std::enable_if<!std::is_base_of<ConfiguratorJson,T>::value,void>::type
    cfgSetFromJson(nlohmann::json& js, T& val){
        val = js.get<T>();
    }

    void cfgWriteToJsonHelper(nlohmann::json& js, ConfiguratorJson& val){
        js = val.to_json();
    }

    /// cfgWriteToStreamHelper for Optional<T>
    /// Prints contents of Optional.
    template <typename T>
    static void cfgWriteToJsonHelper(nlohmann::json& js, Optional<T>& opt){
        // shouldn't be able to get this far if not set
        if(!opt.isSet()) throw std::runtime_error("cfgWriteToStreamHelper Optional<T>: this shouldn't happen");
        cfgWriteToJsonHelper(js, (T&)opt);
    }

    /// cfgWriteToStreamHelper for all other types
    /// the enable_if is required to prevent it from matching on Configurator descendants
    template <typename T>
    static typename std::enable_if<!std::is_base_of<ConfiguratorJson,T>::value,void>::type
    cfgWriteToJsonHelper(nlohmann::json& js, T& val){
        js = val;
    }

    static std::string stripSpaces(const std::string& in){
        size_t a=in.find_first_not_of(" \t\r\n"); //find first non-space
        size_t b=in.find_last_not_of(" \t\r\n"); //find last non-space
        if(a==std::string::npos) return ""; //all white spaces
        return in.substr(a,b-a+1); //get rid of leading or trailing spaces
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

};


//////////////////////////////////////////////////////////////////
// Macros to automatically generate the cfgMultiFunction method in
// descendant classes.

// automatically generates subclass constructor and begins cfgMultiFunction method
#define CFGJS_HEADER(structName) \
  structName() { cfgMultiFunction(CFGJS_INIT_ALL,NULL,NULL,NULL,NULL); } \
  std::string getStructName() { return #structName; } \
  int cfgMultiFunction(MFType mfType, const std::string* str, \
    nlohmann::json* jsonIn, nlohmann::json* jsonOut,ConfiguratorJson*other){ \
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


};  //namespace codepi