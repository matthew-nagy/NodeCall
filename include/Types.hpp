#pragma once
#ifndef NC_TYPES_HPP
#define NC_TYPES_HPP

#include <any>
#include <memory>
#include <vector>
#include <initializer_list>
#include <unordered_map>
#include <typeindex>
#include <stdint.h>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <string>
#include <stack>
#include <iostream>
#include <fstream>
#include <atomic>
#include <exception>
#include <functional>


#define ERROR_MAKE(NAME) class NAME : public std::logic_error{ public: NAME () : std::logic_error( #NAME ){} }  
namespace nc{
    struct Queary;
    struct Operation;
    class argument;
    class runtime_resources;
    class Runtime;
    class REPL;

    //Some value to be used. May be a constant stored once, may be a variable shared across the program
    typedef std::shared_ptr<std::any> value;
    //A list of arguments to pass to a function
    typedef std::vector<argument> argument_list;
    //A unique pointer to a certain runtime's available resources. Often passed as an argument to function pointers
    typedef std::unique_ptr<runtime_resources> unique_run_resource;
    //A way of pointing to a node of execution. Node Call doesn't nativly support 16 bit ints,
    //so it is easily identifiable by time in program
    typedef uint16_t node_index;
    //A series of operations is all a node is
    typedef std::vector<Operation> node;

    //A queary function performs some computation on its arguments, then returns a result
    typedef std::function<value(argument_list&, unique_run_resource&)> QuearyFunction;
    //An operation is presumed to perform some operation on the global state, facilitated through its arguments
    typedef std::function<void(argument_list&, unique_run_resource&)> OperationFunction;

    //A mapping of queary names to their functions. Used in the compiler, where multiple tables may be provided
    typedef std::unordered_map<std::string, QuearyFunction> QuearyTable;
    //A mapping of operation names to their functions. Used in the compiler, where multiple tables may be provided
    typedef std::unordered_map<std::string, OperationFunction> OperationTable;
    //Mapping for variables
    typedef std::unordered_map<std::string, value> VariableTable;

    //Type of runtime
    enum runtype {
        rt_Serial, rt_Parallel
    };

    //Different ways a node can be called
    enum call_type{
        call_cond_breakable,    //A conditional which 'break' can exit (while loop, do while, etc)
        call_cond_definate,     //A conditional which 'break' would ignore (if, else)
        call_func,              //It was ran as a function
        call_enterance_point    //This node was the enterence point of the program. If you try to get past this, throw an error
    };
    //A method of storing a snapshot of node execution
    struct call_frame{
        call_type exitType;         //How was it this frame was exited?
        node_index index;           //What node was this frame executing?
        uint32_t nextInstruction;   //What was the index of the next instruction to execute
    };

    class argument{
    public:
        value getValue(unique_run_resource& environment);

        argument& operator=(std::any* constant);
        argument& operator=(const value& variable);
        argument& operator=(Queary* queary);

        argument(std::any* constant);
        argument(const value& variable);
        argument(Queary* queary);
    private:
        value assignedValue;
        std::unique_ptr<Queary> innerQueary;
    };

    struct Queary{
        QuearyFunction func;
        argument_list arguments;
        #ifdef _DEBUG 
        unsigned lineNum;
        #endif
        value operator()(unique_run_resource& environment);

        Queary(QuearyFunction func, argument_list&& arguments, unsigned lineNum = 0);
    };

    struct Operation{
        OperationFunction func;
        argument_list arguments;
        #ifdef _DEBUG 
        unsigned lineNum;
        #endif


        void operator()(unique_run_resource& environment);
        
        Operation(OperationFunction func, argument_list&& arguments, unsigned lineNum = 0);
    };

    struct additional_library{
        VariableTable variables;
        QuearyTable quearies;
        OperationTable operations;

        //Standard for node call
        static const std::shared_ptr<additional_library> standardLibrary;
    };

    struct program{
        std::unordered_map<std::string, node_index> nodeMappings;
        std::vector<node> nodes;
        VariableTable ownedVariables;
    };

    //A way of exposing any number of objects to passed in library tables
    class variable_blackboard{
    public:
        template<class T>
        T& get(){
            return map[typeid(T)];
        }

        template<class T>
        void set(T& value){
            map.emplace(typeid(T), value);
        }

        template<class T>
        bool contains()const{
            return map.count(typeid(T)) > 0;
        }

    private:
        std::unordered_map<std::type_index, std::any> map;
    };

    ERROR_MAKE(CANNOT_ENTER_PROGRAM_WHILE_PROGRAM_IS_RUNNING);
    ERROR_MAKE(CANNOT_CHANGE_PROGRAM_WHILE_PROGRAM_IS_RUNNING);

    //The runtime of a Node Call script.
    class Runtime{
        friend class runtime_resources;
        friend class REPL;
    public:
        variable_blackboard& blackboard();

        bool isRunning()const;

        unsigned getNodesExecuted()const;

        void setPrintFunction(void(*newPrintFunction)(const std::string&));

        void pause();

        void enterProgramAt(const std::string& nodeName);

        void loadProgram(const std::shared_ptr<program>& newProgram);

        Runtime(runtype runType);

        ~Runtime();

    private:
        runtype runType;

        void (*printFunction)(const std::string&);

        static void defaultPrint(const std::string& s);

        variable_blackboard internalBlackboard;                 //A type agnostic storage for datatypes. Very useful for extentions
        std::condition_variable conditionVariable;              //Should the script ever need to wait, this is what it waits on
        std::condition_variable launchShutdownVariable;         //Once the script stopped running, wait on this to launch again, or stop running
        std::mutex internalMutex;                               //The runtimes own lock to wait on its conditions

        std::atomic_bool running;           //Is there currently a program being run on the script thread?
        std::atomic_bool shutdownFlag;      //Should the script thread be terminated (ie, is this runtime over)
        std::mutex shutdownMutex;           //Used to sync the script and main threads once a shutdown has been called

        call_frame currentFrame;            //Current state of the program (node, instruction count, etc)
        std::stack<call_frame> callStack;   //Stack of all call frames, so a program can break and return

        std::shared_ptr<program> currentProgram;            //The current program being run on the script thread
        std::unique_ptr<runtime_resources> runtimeResource; //Method of exposing functionality to threads without giving them master control

        std::atomic_uint16_t runsExecuted = 0;                          //Counts how many times this runtime has been used to execute something

        void runLoadedFunction();

        void runProgram();
    };

    //This class has access to the full runtime, but only exposes what Operation's will need
    class runtime_resources{
    public:
        friend class Runtime;
        variable_blackboard& blackboard();
        void call(call_type type, node_index targetIndex);
        void sendToNode(node_index index);
        void requestReturn();
        void requestBreak();
        void requestTerminate();
        void endIf();

        //Prints out using the Runtime's print pointer
        void print(const std::string& s);

        //Probably only used to set up the arguments for a conditional block.
        std::condition_variable& getConditionVariable()const;

        //There is only one condition variable alowed, as this lets the thread certainly be woken up should the script
        //need to be terminated
        void blockOnCondition();
        //As above, but will create another thread which, once the variable is being waited on, shall run the passed in function
        //This lets you inform some primary thread that a script is waiting for it on this variable, while being sure it *is* actually waiting
        void blockOnCondition(void(*onceLockedFunction)(const std::any&), const std::any& argument);
    private:
        Runtime& parent;

        runtime_resources(Runtime& parent);

        void exitOnType(call_type type);
    };

}
#endif