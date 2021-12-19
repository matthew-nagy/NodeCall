#include "NCOperations.hpp"

namespace ncprivate{
namespace ops{
#define opdef(NAME) void NAME (NCRuntime& environment, std::vector<NCArgument>& args, NC_Runtime_Log& runLog, unsigned lineNumber)

    opdef(assign){
        args[0].getValue(runLog) = args[1].getValue(runLog);
    }

    opdef(print){
        std::vector<NCArgument> argVec;
        argVec.emplace_back();
        for(size_t i = 0; i < args.size(); i++){
            argVec[0] = args[i];
            printf("%s", std::any_cast<std::string>(queary::tfstring(argVec, runLog, lineNumber)).c_str());
        }

    }
    opdef(println){
        print(environment, args, runLog, lineNumber);
        printf("\n");
    }

    //If the first argument resolves to a boolean true, the second argument is treated
    //as a node index and execution is sent there
    opdef(conditional_if){
        if(std::any_cast<bool>(args[0].getValue(runLog))){
            environment.callConditional(std::any_cast<uint16_t>(args[1].getValue(runLog)));
        }
    }
    //As with if, but should the condition be false, treats argument 3 as the else's node
    //index. Execution is sent there
    opdef(conditional_if_else){
        if(std::any_cast<bool>(args[0].getValue(runLog))){
            environment.callConditional(std::any_cast<uint16_t>(args[1].getValue(runLog)));
        }
        else{
            environment.callConditional(std::any_cast<uint16_t>(args[2].getValue(runLog)));
        }
    }

    //Used for the top of do_whiles
    opdef(link_conditionally){
        environment.callConditional(std::any_cast<uint16_t>(args[0].getValue(runLog)));
    }
    //Used to just link
    opdef(link_to){
        environment.linkTo(std::any_cast<uint16_t>(args[0].getValue(runLog)));
    }
    //Call a node like a function
    opdef(call_node){
        environment.callFunctionally(std::any_cast<uint16_t>(args[0].getValue(runLog)));
    }

    opdef(return_from){
        environment.returnFunc();
    }
    opdef(break_from){
        environment.returnConditional();
    }
    opdef(end){
        environment.terminate();
    }


     //Add a macro controlled type safety to this
    opdef(new_vector){
        args[0].getValue(runLog) = std::vector<std::any>();
    }
    opdef(push_front){
        auto* vec = std::any_cast<std::vector<std::any>>(&args[0].getValue(runLog));
        vec->insert(vec->begin(), args[1].getValue(runLog));
    }
    opdef(push_back){
        std::any_cast<std::vector<std::any>>(&args[0].getValue(runLog))->emplace_back(args[1].getValue(runLog));
    }
    opdef(erase){
        auto* vec = std::any_cast<std::vector<std::any>>(&args[0].getValue(runLog));
        vec->erase(vec->begin() + std::any_cast<int>(args[1].getValue(runLog)));
    }
    opdef(clear){
        std::any_cast<std::vector<std::any>>(&args[0].getValue(runLog))->clear();
    }

}

    // const std::vector<NCOperationFunc> standard_functions = {
    //     ops::assign, ops::print, ops::println,
    //     ops::conditional_if, ops::conditional_if_else, ops::link_conditionally, ops::link_to,
    //     ops::call_node, ops::return_from, ops::break_from, ops::end,
    //     ops::new_vector, ops::push_front, ops::push_back, ops::erase, ops::clear
    // };

    const std::map<std::string, NCOperationFunc> standard_functions = {
        {"assign", ops::assign}, {"print", ops::print}, {"println", ops::println},
        {"if", ops::conditional_if}, {"if_else", ops::conditional_if_else},
        {"link_conditionaly",ops::link_conditionally}, {"link_to", ops::link_to},
        {"call_node", ops::call_node}, {"return", ops::return_from}, {"break", ops::break_from},
        {"end", ops::end}, {"new_vector", ops::new_vector}, {"push_front", ops::push_front},
        {"push_back", ops::push_back}, {"erase", ops::erase}, {"clear", ops::clear}
    };

}
#undef opdef
