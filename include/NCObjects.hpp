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


namespace ncprivate{
    union NCArgUnion{
        //A queary that can be run
        NCQueary* queary;
        //Not unique to this union, accessed in many places
        std::any* object;
        //unique to this union
        std::any* constant;

        NCArgUnion();
    };
}
//Types an argument can be
enum NCArgType{ncat_Constant, ncat_Queary, ncat_Object};

//Used to represent an argument to Node Call functions
struct NCArgument{
    //Gets the value of this argument
    std::any& getValue(NC_Runtime_Log& runtimeLog);
    //Explains what type of argument this object is
    NCArgType type;

    //This is used to produce duplicates of an argument. Both point to the same data- This is not a copy!
    NCArgument& operator=(const NCArgument& right);

    NCArgument& operator=(const std::any& constant){
        type = ncat_Constant;
        argUnion.constant = new std::any(constant);
        return *this;
    }
    NCArgument& operator=(NCQueary* queary){
        type = ncat_Queary;
        argUnion.queary = queary;
        return *this;
    }
    NCArgument& operator=(std::any* object){
        type = ncat_Object;
        argUnion.object = object;
        return *this;
    }

    //Constructor as a queary
    // NCArgument(NCQueary* queary);
    //COnstructor as a shared object
    //NCArgument(std::any* object);
    //Constructor as a singular constant
    // NCArgument(const std::any& constant);

    //If this is a singluar constant, than its memory must now be freed
    ~NCArgument();
private:
    ncprivate::NCArgUnion argUnion;
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
    void linkTo(uint16_t nodeNumber){
        currentFrame.nodeNumber = nodeNumber;
        currentFrame.commandNumber = 0;
    }

    //call as it it were a function, with a functionally marked call frame
    void callFunctionally(uint16_t nodeNumber){
        callFrames.emplace(currentFrame);
        currentFrame.commandNumber = 0;
        currentFrame.nodeNumber = nodeNumber;
        currentFrame.callType = nc_call_Function;
    }

    //This is some if or loop. Create a new call framne marked as conditional
    void callConditional(uint16_t nodeNumber){
        callFrames.emplace(currentFrame);
        currentFrame.commandNumber = 0;
        currentFrame.nodeNumber = nodeNumber;
        currentFrame.callType = nc_call_Conditional;
    }

    void terminate(){
        //Tell the environment it should stop
        calledTerminate = true;
        //End the node, the environment will now see that flag
        currentFrame.commandNumber = nodes[currentFrame.nodeNumber].size();
    }

    void returnFunc(){
        popCallFramesUntil(nc_call_Function);
    }
    void returnConditional(){
        popCallFramesUntil(nc_call_Conditional);
    }

    void run(){
        while(!calledTerminate){
            while(currentFrame.commandNumber < nodes[currentFrame.nodeNumber].size()){
                NCFunction& instruction = nodes[currentFrame.nodeNumber][currentFrame.commandNumber];
                currentFrame.commandNumber++;
                instruction(*this, runtimeLog);
            }
        }
    }

    NCRuntime():
        currentFrame({0, 0, nc_call_Function})
    {}

    ~NCRuntime(){
        for(std::any* v : objects)
            delete v;
        for(NCQueary* q : quearies)
            delete q;
    }

private:
    void popCallFramesUntil(NCCallType type){
        while(callFrames.top().callType != type)
            callFrames.pop();
        currentFrame = callFrames.top();
        callFrames.pop();
    }

    std::vector<std::vector<NCFunction>> nodes;
    //These are just the objects and queries owned by this runtime;some others may be stored elsewhere
    std::vector<std::any*> objects;
    std::vector<NCQueary*> quearies;

    std::stack<NCCallFrame> callFrames;

    NCCallFrame currentFrame;
    NC_Runtime_Log runtimeLog;
    std::atomic_bool calledTerminate = false;
};

//Possible functions
/*

    assign
    println
    if
    else    (used with a sort of global "what did the last if do" variable)
    while
    link_to (as with old version of nc, moves execution)
    call_node (new; pushes the current node and instruction number onto a call stack. 
                    A sort of "call as function", but with all avriables still global)
    return
    
    create_list
    push
    pop
    index

    pause
    end
    throw

//Possible quearies
    add
    sub
    mul
    div
    mod
    lg_>>
    lg_<<
    lg_&&
    lg_||
    lg_!
    lg_^
    and
    or
    not
    cmp==
    cmp!=
    cmp_<
    cmp_>
    cmp_<=
    cmp_>=

    size
    get_type

    input

    to_int
    to_float
    to_string
    to_bool
*/

