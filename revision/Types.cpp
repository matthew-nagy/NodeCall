#include "Types.hpp"

namespace nc{

value argument::getValue(unique_run_resource& environment){
    return (innerQueary == nullptr) ? assignedValue : (*innerQueary)(environment);
}

argument& argument::operator=(std::any* constant){
    assignedValue = std::make_shared<std::any>(constant);
    innerQueary = nullptr;
    return *this;
}
argument& argument::operator=(const value& variable){
    assignedValue = variable;
    innerQueary = nullptr;
    return *this;
}
argument& argument::operator=(Queary* queary){
    innerQueary.reset(queary);
    return *this;
}

argument::argument(std::any* constant):
    assignedValue(std::make_shared<std::any>(constant)),
    innerQueary(nullptr)
{}
argument::argument(const value& variable):
    assignedValue(variable),
    innerQueary(nullptr)
{}
argument::argument(Queary* queary):
    innerQueary(queary)
{}




value Queary::operator()(unique_run_resource& environment){
    return func(arguments, environment);
}

Queary::Queary(QuearyFunction func, argument_list&& arguments, unsigned lineNumber):
    func(func),
    arguments(std::move(arguments))
{
#ifdef _DEBUG
    lineNum = lineNumber;
#endif
}



void Operation::operator()(unique_run_resource& environment){
    func(arguments, environment);
}
Operation::Operation(OperationFunction func, argument_list&& arguments, unsigned lineNumber):
    func(func),
    arguments(std::move(arguments))
{
#ifdef _DEBUG
    lineNum = lineNumber;
#endif
}





variable_blackboard& Runtime::blackboard(){
    return internalBlackboard;
}

bool Runtime::isRunning()const{
    return running;
}

void Runtime::pause(){
    running = false;
    //Wakes up the script thread if it was waiting
    conditionVariable.notify_one();
}

void Runtime::enterProgramAt(const std::string& nodeName){
    if(running)
        throw(new CANNOT_ENTER_PROGRAM_WHILE_PROGRAM_IS_RUNNING);
    while(callStack.size() > 0)
        callStack.pop();
    currentFrame.exitType = call_enterance_point;
    currentFrame.nextInstruction = 0;
    currentFrame.index = currentProgram->nodeMappings[nodeName];

    running = true;
    launchShutdownVariable.notify_one();
}

void Runtime::loadProgram(std::unique_ptr<program>&& newProgram){
    if(running)
        throw(new CANNOT_CHANGE_PROGRAM_WHILE_PROGRAM_IS_RUNNING);
    currentProgram = std::move(newProgram);
}

Runtime::Runtime():
    running(false),
    shutdownFlag(false),
    runtimeResource(new runtime_resources(*this))
{
    std::unique_lock<std::mutex> launchLock(internalMutex);
    //Send of the script thread
    std::thread([this](){
        this->runProgram();
    }).detach();
    conditionVariable.wait(launchLock);
}

Runtime::~Runtime(){
    shutdownFlag = true;    //Tell script to shutdown
    pause();                //Make sure its no longer running so it can register that flag
    launchShutdownVariable.notify_one();    //Launch it again in case it got stuck somewhere ig
    std::unique_lock<std::mutex> sync(shutdownMutex);   //now take the mutex that the script holds throughout its lifetime. This proves it has finished
}

void Runtime::runProgram(){
    //This lock will persist until the thread shuts down
    std::unique_lock<std::mutex> shutdownMutex;
    conditionVariable.notify_one();
    //This lock allows this thread to wait while there is nothing to run
    std::unique_lock<std::mutex> myLock(internalMutex);
    while(!shutdownFlag){
        launchShutdownVariable.wait(myLock);
        while(running){
            Operation& op = currentProgram->nodes[currentFrame.index][currentFrame.nextInstruction];
            currentFrame.nextInstruction++;
            op(runtimeResource);
        }
    }
}
    





variable_blackboard& runtime_resources::blackboard(){
    return parent.blackboard();
}
void runtime_resources::call(call_type type, node_index targetIndex){
    parent.callStack.push(parent.currentFrame);
    parent.currentFrame.exitType = type;
    sendToNode(targetIndex);
}
void runtime_resources::sendToNode(node_index index){
    parent.currentFrame.index = index;
    parent.currentFrame.nextInstruction = 0;
}
void runtime_resources::requestReturn(){
    exitOnType(call_func);
}
void runtime_resources::requestBreak(){
    exitOnType(call_cond_breakable);
}

void runtime_resources::exitOnType(call_type type){
    if(parent.currentFrame.exitType != type){
        while(parent.callStack.top().exitType != type)
            parent.callStack.pop();
        parent.callStack.pop();
    }
    parent.currentFrame = parent.callStack.top();
    parent.callStack.pop();
}
void runtime_resources::requestTerminate(){
    parent.pause();
}

//Probably only used to set up the arguments for a conditional block.
std::condition_variable& runtime_resources::getConditionVariable()const{
    return parent.conditionVariable;
}

//There is only one condition variable alowed, as this lets the thread certainly be woken up should the script
//need to be terminated
void runtime_resources::blockOnCondition(){
    std::unique_lock l(parent.internalMutex);
    parent.conditionVariable.wait(l);
}
//As above, but will create another thread which, once the variable is being waited on, shall run the passed in function
//This lets you inform some primary thread that a script is waiting for it on this variable, while being sure it *is* actually waiting
void runtime_resources::blockOnCondition(void(*onceLockedFunction)(const std::any&), const std::any& argument){
    std::unique_lock l(parent.internalMutex);
    std::thread([](void(*olf)(const std::any&), const std::any& arg, std::mutex& mutex){
        std::unique_lock ownerExecutionFinishedLock(mutex);
        ownerExecutionFinishedLock.unlock();    //We know that the script is waiting, with only this thread active from that
        //Therefore while it is bad for to unlock a unique lock, we can be certain that this is safe, as no other thread should be
        //trying to access the lock. Furthermore, this is needed, because the passed in lambda may need to get that lock anyway
        olf(arg);
    }, onceLockedFunction, std::ref(argument), std::ref(parent.internalMutex)).detach();
    parent.conditionVariable.wait(l);
}

runtime_resources::runtime_resources(Runtime& parent):
    parent(parent)
{}


}