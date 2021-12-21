#pragma once

#include <any>
#include <string>
#include <map>
#include <vector>
#include <stack>
#include "NCLog.hpp"
#include <atomic>

struct NCQueary;
struct NCArgument;
struct NCRuntime;

//A queary that looks at its arguments, and returns a new argument
typedef std::any(*NCQuearyFunc)(std::vector<NCArgument>& args, NC_Runtime_Log& runLog, unsigned lineNumber);
//AN operation on the node call runtime and data
typedef void(*NCOperationFunc)(NCRuntime& environment, std::vector<NCArgument>& args, NC_Runtime_Log& runLog, unsigned lineNumber);

//Types an argument can be
enum NCArgType{ncat_Constant, ncat_Queary, ncat_Object, ncat_nullType};

//Used to represent an argument to Node Call functions
struct NCArgument{
    //Gets the value of this argument
    std::any& getValue(NC_Runtime_Log& runtimeLog);
    //Explains what type of argument this object is
    NCArgType type = ncat_nullType;

    //This is used to produce duplicates of an argument. Both point to the same data- This is not a copy!
    NCArgument& operator=(const NCArgument& right);

    NCArgument& operator=(const std::any& constant){
        type = ncat_Constant;
        constantVal = constant;
        return *this;
    }
    NCArgument& operator=(NCQueary* queary){
        type = ncat_Queary;
        quearyPtr = queary;
        return *this;
    }
    NCArgument& operator=(std::any* object){
        type = ncat_Object;
        objectPtr = object;
        return *this;
    }

    //Constructor as a queary
    // NCArgument(NCQueary* queary);
    //COnstructor as a shared object
    //NCArgument(std::any* object);
    //Constructor as a singular constant
    // NCArgument(const std::any& constant);
    NCArgument() = default;
    NCArgument(const NCArgument& cpyf);
private:
    std::any* objectPtr;
    NCQueary* quearyPtr;
    std::any constantVal;
};

//Allows operations to be done on arguments before passing it to an NCFunction
struct NCQueary{
    //The queary function being used to transform he arguments
    NCQuearyFunc func;
    //The arguments this queary uses
    std::vector<NCArgument> arguments;
    //Line this queary is on
    unsigned lineNum;

    //Runs the function on the given arguments to resolve a queary
    std::any& operator()(NC_Runtime_Log& runtimeLog);



    NCQueary() = default;
private:
    //To speed up other code, all arguments must turn into references to std::any. Therefore, although we create
    //a new std::any each time, we need to copy it to here first, to prevent copying this, and other any's later
    std::any hiddenValue;
};

struct NCFunction{
    NCOperationFunc func;
    std::vector<NCArgument> arguments;
    unsigned lineNumber;

    void operator()(NCRuntime& runtime, NC_Runtime_Log& runtimeLog);
};

enum NCCallType{
    nc_call_Function, nc_call_Conditional
};

struct NCCallFrame{
    uint16_t nodeNumber;
    uint16_t commandNumber;
    NCCallType callType;
};


namespace ncprivate{
namespace compiler{
    struct CompilerEnvironment;
}}


struct NCRuntime{
friend struct ncprivate::compiler::CompilerEnvironment;

    //Link to with no trace. No new call frame
    void linkTo(uint16_t nodeNumber);

    //call as it it were a function, with a functionally marked call frame
    void callFunctionally(uint16_t nodeNumber);

    //This is some if or loop. Create a new call framne marked as conditional
    void callConditional(uint16_t nodeNumber);

    void terminate();

    void returnFunc();
    void returnConditional();

    void run();

    NCRuntime();

    ~NCRuntime();

private:

    std::vector<std::vector<NCFunction>> nodes;
    //These are just the objects and queries owned by this runtime;some others may be stored elsewhere
    std::vector<std::any*> objects;
    std::vector<NCQueary*> quearies;

    std::stack<NCCallFrame> callFrames;

    NCCallFrame currentFrame;
    NC_Runtime_Log runtimeLog;
    std::atomic_bool calledTerminate = false;
};


