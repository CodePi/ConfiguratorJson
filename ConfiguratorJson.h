#pragma once

#include "json.hpp"
#include <string>
#include <iomanip>

//TODO remove indent

namespace codepi{

class ConfiguratorJson{
public:
    std::string to_string(int indent=0){
        return to_json().dump(indent);
    }
    nlohmann::json to_json(){
        nlohmann::json j;
        return j;
    }

protected:
    enum MFType{CFGJS_INIT_ALL,CFGJS_SET,CFGJS_WRITE_ALL,CFGJS_COMPARE};

    /// Helper method that is called by all of the public methods above.
    ///   This method is automatically generated in subclass using macros below
    ///   Returns the number of variables matched
    virtual int cfgMultiFunction(MFType mfType, std::string* str, std::string* subVar,
                                 std::istream* streamIn, std::ostream* streamOut, int indent,
                                 ConfiguratorJson* other)=0;

    /// return i*2 spaces, for printing
    static std::string cfgIndentBy(int i);
    /// returns default value of type T
    template <typename T> static T cfgGetDefaultVal(const T&var){return T();}
    /// overridable method called on parse error
    virtual void throwError(std::string error){ throw std::runtime_error(error); }

    /// cfgSetFromStream for all other types
    /// the enable_if is required to prevent it from matching on Configurator descendants
    template <typename T>
    static typename std::enable_if<!std::is_base_of<ConfiguratorJson,T>::value,void>::type
    cfgSetFromStream(std::istream& ss,  T& val, const std::string& subVar=""){
        if(!subVar.empty()) { //subVar should be empty
            ss.setstate(std::ios::failbit); //set fail bit to trigger error handling
            return;
        }
        ss>>std::setbase(0)>>val;
    }

    /// cfgWriteToStreamHelper for all other types
    /// the enable_if is required to prevent it from matching on Configurator descendants
    template <typename T>
    static typename std::enable_if<!std::is_base_of<ConfiguratorJson,T>::value,void>::type
    cfgWriteToStreamHelper(std::ostream& stream, T& val, int indent){
        stream<<val;
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
  structName() { cfgMultiFunction(CFGJS_INIT_ALL,NULL,NULL,NULL,NULL,0,NULL); } \
  std::string getStructName() { return #structName; } \
  int cfgMultiFunction(MFType mfType, std::string* str, std::string* subVar, \
    std::istream* streamIn, std::ostream* streamOut,int indent,ConfiguratorJson*other){ \
    int retVal=0; \
    structName* otherPtr; \
    if(mfType==CFGJS_COMPARE) {otherPtr = dynamic_cast<structName*>(other); \
      if(!otherPtr) return 1; /*dynamic cast failed, types different*/ }

// continues cfgMultiFunction method, called for each member variable in struct 
#define CFGJS_ENTRY2(varName, defaultVal) \
  if(mfType==CFGJS_INIT_ALL) { \
    /*TODOif(cfgIsSetOrNotOptional(varName))*/ {varName = defaultVal;retVal++;} \
    } else if(mfType==CFGJS_SET && #varName==*str) { cfgSetFromStream(*streamIn,varName,*subVar);retVal++;} \
  else if(mfType==CFGJS_WRITE_ALL) { \
    /*TODOif(cfgIsSetOrNotOptional(varName))*/ { \
      *streamOut<</*TODOcfgIndentBy(indent)<<*/#varName<<"="; \
      cfgWriteToStreamHelper(*streamOut,varName,indent); \
      *streamOut<<std::endl;retVal++; \
      if(streamOut->fail()) \
        throwError("ConfiguratorJson ("+getStructName()+") error, can't write variable: "+#varName); \
    } \
  } else if(mfType==CFGJS_COMPARE) { \
    retVal+=cfgCompareHelper(this->varName,otherPtr->varName); \
  }

// alternative to CFGJS_ENTRY2 used when default defaultVal is sufficient
#define CFGJS_ENTRY1(varName) CFGJS_ENTRY2(varName, cfgGetDefaultVal(varName))

// calls cfgMultiFunction method of parent
// allows for inheritance
#define CFGJS_PARENT(parentName) \
  int rc=parentName::cfgMultiFunction(mfType,str,subVar,streamIn,streamOut,indent,other); \
  retVal+=rc;


// closes out cfgMultiFunction method
#define CFGJS_TAIL return retVal; }


};  //namespace codepi