#include "NCObjects.hpp"

#define ERROR_MAKE(NAME) class NAME : public std::logic_error{ public: NAME () : std::logic_error( #NAME ){} }
    ERROR_MAKE( NC_Runtime_Null_Argument_Used );
#undef ERROR_MAKE

NCArgument& NCArgument::operator=(const NCArgument& right){
    type = right.type;
    switch(type){
        case ncat_Constant:
            constantVal = right.constantVal;
            break;
        case ncat_Queary:   
            quearyPtr = right.quearyPtr;
            break;
        case ncat_Object:
            objectPtr = right.objectPtr;
            break;
        default:
            throw(new NC_Runtime_Null_Argument_Used);
    }

    return *this;
}


NCArgument::NCArgument(const NCArgument& cpyf){
    *this = cpyf;
}

std::any& NCArgument::getValue(NC_Runtime_Log& runtimeLog){
    switch(type){
        case ncat_Constant:
            return constantVal;
        case ncat_Queary:
            return quearyPtr->operator()(runtimeLog);
        case ncat_Object:
            return *objectPtr;
        default:
            throw(new NC_Runtime_Null_Argument_Used);
    }
}

std::any& NCQueary::operator()(NC_Runtime_Log& runtimeLog){
    this->hiddenValue = func(arguments, runtimeLog, lineNum);
    return this->hiddenValue;
}
void NCFunction::operator()(NCRuntime& runtime, NC_Runtime_Log& runtimeLog){
    func(runtime, arguments, runtimeLog, lineNumber);
}




//Link to with no trace. No new call frame
void NCRuntime::linkTo(uint16_t nodeNumber){
    currentFrame.nodeNumber = nodeNumber;
    currentFrame.commandNumber = 0;
}

//call as it it were a function, with a functionally marked call frame
void NCRuntime::callFunctionally(uint16_t nodeNumber){
    callFrames.emplace(currentFrame);
    currentFrame.commandNumber = 0;
    currentFrame.nodeNumber = nodeNumber;
    currentFrame.callType = nc_call_Function;
}

//This is some if or loop. Create a new call framne marked as conditional
void NCRuntime::callConditional(uint16_t nodeNumber){
    callFrames.emplace(currentFrame);
    currentFrame.commandNumber = 0;
    currentFrame.nodeNumber = nodeNumber;
    currentFrame.callType = nc_call_Conditional;
}

void NCRuntime::terminate(){
    //Tell the environment it should stop
    calledTerminate = true;
    //End the node, the environment will now see that flag
    currentFrame.commandNumber = nodes[currentFrame.nodeNumber].size();
}

void NCRuntime::returnFunc(){
    if(callFrames.size() == 0){
        calledTerminate = true;
        currentFrame.commandNumber = nodes[currentFrame.nodeNumber].size();
        return;
    }
    //Go up to the last function call
    while(callFrames.top().callType != nc_call_Function){
        callFrames.pop();
        if(callFrames.size() == 0){
            calledTerminate = true;
            currentFrame.commandNumber = nodes[currentFrame.nodeNumber].size();
            return;
        }
    }
    currentFrame = callFrames.top();
    callFrames.pop();
}

void NCRuntime::returnConditional(){
    currentFrame = callFrames.top();
    callFrames.pop();
}

void NCRuntime::run(){
    while(!calledTerminate){
        while(currentFrame.commandNumber < nodes[currentFrame.nodeNumber].size()){
            NCFunction& instruction = nodes[currentFrame.nodeNumber][currentFrame.commandNumber];
            currentFrame.commandNumber++;
            instruction(*this, runtimeLog);
        }
    }
    //Make sure it can be run again
    calledTerminate = false;
    while(callFrames.size() > 0)
        callFrames.pop();
}

NCRuntime::NCRuntime():
    currentFrame({0, 0, nc_call_Function})
{}

NCRuntime::~NCRuntime(){
    for(std::any* v : objects)
        delete v;
    for(NCQueary* q : quearies)
        delete q;
}

