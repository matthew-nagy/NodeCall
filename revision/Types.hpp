#pragma once
#ifndef NC_TYPES_HPP
#define NC_TYPES_HPP

#include<any>
#include<memory>
#include<vector>
#include<initializer_list>
#include<unordered_map>
#include<typeindex>
#include <stdint.h>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <string>

namespace nc{
    struct Queary;
    class Operation;
    class argument;
    class runtime_resources;
    class Runtime;

    typedef std::shared_ptr<std::any> value;
    typedef std::vector<argument> argument_list;
    typedef std::unique_ptr<runtime_resources> unique_run_resource;
    typedef uint16_t node_index;

    typedef value(*QuearyFunction)(const argument_list&, unique_run_resource&);
    typedef void(*OperationFunction)(const argument_list&, unique_run_resource&);

    typedef std::unordered_map<std::string, QuearyFunction> QuearyTable;
    typedef std::unordered_map<std::string, OperationFunction> OperationTable;

    enum call_type{
        call_cond_breakable,
        call_cond_definate,
        call_func
    };

    class argument{
    public:
        std::any* getValue(unique_run_resource& environment){
            return (innerQueary == nullptr) ? assignedValue.get() : (*innerQueary)(environment).get();
        }

        argument& operator=(std::any* constant){
            assignedValue = std::make_shared<std::any>(constant);
            innerQueary = nullptr;
        }
        argument& operator=(const value& variable){
            assignedValue = variable;
            innerQueary = nullptr;
        }
        argument& operator=(Queary* queary){
            innerQueary.reset(queary);
        }
    
        argument(std::any* constant):
            assignedValue(std::make_shared<std::any>(constant)),
            innerQueary(nullptr)
        {}
        argument(const value& variable):
            assignedValue(variable),
            innerQueary(nullptr)
        {}
        argument(Queary* queary):
            innerQueary(queary)
        {}
    private:
        value assignedValue;
        std::unique_ptr<Queary> innerQueary;
    };

    struct Queary{
        QuearyFunction func;
        const argument_list arguments;
        #ifdef _DEBUG 
        unsigned lineNum;
        #endif
        value operator()(unique_run_resource& environment)const{
            return func(arguments, environment);
        }

        Queary(QuearyFunction func, const argument_list& arguments):
            func(func),
            arguments(arguments)
        {}
        Queary(QuearyFunction func, std::initializer_list<argument> arguments):
            func(func),
            arguments(arguments)
        {}
    };

    struct Operation{
        OperationFunction func;
        const argument_list arguments;
        #ifdef _DEBUG 
        unsigned lineNum;
        #endif

        void operator()(unique_run_resource& environment){
            func(arguments, environment);
        }
    };

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

    class Runtime{
        friend class runtime_resources;
    public:
        variable_blackboard& blackboard(){
            return internalBlackboard;
        }

        runtime_resources getRuntimeResource(){
            return runtime_resources(*this);
        }
    private:
        variable_blackboard internalBlackboard;
        std::condition_variable conditionVariable;
        std::mutex internalMutex;
    };

    //This class has access to the full runtime, but only exposes what Operation's will need
    class runtime_resources{
    public:
        friend class Runtime;
        variable_blackboard& blackboard(){
            return parent.blackboard();
        }
        void call(call_type type, node_index targetIndex);
        void requestReturn();
        void requestBreak();
        void requestTerminate();

        //Probably only used to set up the arguments for a conditional block.
        std::condition_variable& getConditionVariable()const{
            return parent.conditionVariable;
        }

        //There is only one condition variable alowed, as this lets the thread certainly be woken up should the script
        //need to be terminated
        void blockOnCondition(){
            std::unique_lock l(parent.internalMutex);
            parent.conditionVariable.wait(l);
        }
        //As above, but will create another thread which, once the variable is being waited on, shall run the passed in function
        //This lets you inform some primary thread that a script is waiting for it on this variable, while being sure it *is* actually waiting
        void blockOnCondition(void(*onceLockedFunction)(const std::any&), const std::any& argument){
            std::unique_lock l(parent.internalMutex);
            std::thread([](void(*olf)(const std::any&), const std::any& arg, std::mutex& mutex){
                std::unique_lock ownerExecutionFinishedLock(mutex);
                ownerExecutionFinishedLock.unlock();    //We know that the script is waiting, with only this thread active from that
                //Therefore while it is bad for to unlock a unique lock, we can be certain that this is safe, as no other thread should be
                //trying to access the lock. Furthermore, this is needed, because the passed in lambda may need to get that lock anyway
                olf(arg);
            }).detach();
            parent.conditionVariable.wait(l);
        }
    private:
        Runtime& parent;

        runtime_resources(Runtime& parent):
            parent(parent)
        {}
    };
}
#endif