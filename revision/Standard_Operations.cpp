#include "Standard_Library.hpp"


namespace nc{   namespace stlib{

#define opdef(name) void name (argument_list& args, unique_run_resource& runResource)
namespace op{
    //IO
    opdef(print){
        argument_list al;
        for(size_t i = 0; i < args.size(); i++){
            printf("%s", std::any_cast<std::string>(*args[i].getValue(runResource)));
        }
    }
    opdef(println){
        print(args, runResource);
        printf("\n");
    }

    //Assignment
    opdef(assign){
        (*args[0].getValue(runResource)) = (*args[1].getValue(runResource));
    }

    //Internal flow
    opdef(conditional_if){
        if_pack& ifs = *std::any_cast<std::shared_ptr<if_pack>>(args[0].getValue(runResource)).get();
        for(size_t i = 0; i < ifs.triggers.size(); i++){
            if(std::any_cast<bool>(*ifs.triggers[i].getValue(runResource))){
                runResource->call(call_cond_definate, ifs.resultantNodes[i]);
                return;
            }
        }
        if(ifs.hasFinalElse){
            runResource->call(call_cond_definate, ifs.elseNode);
        }
    }
    //The bottom of the resulting node should have a special if pack at the bottom to send it back to the same node
    opdef(while_loop){
        while_top_pack& wtp = *std::any_cast<std::shared_ptr<while_top_pack>>(*args[0].getValue(runResource)).get();
        if(std::any_cast<bool>(*wtp.trigger.getValue(runResource))){
            runResource->call(call_cond_breakable, wtp.node);
        }
    }
    opdef(do_while_loop){
        runResource->call(call_cond_breakable, std::any_cast<node_index>(*args[0].getValue(runResource)));
    }
    opdef(break_from_conditional){
        runResource->requestBreak();
    }
    opdef(call_node){
        runResource->call(call_func, std::any_cast<node_index>(*args[0].getValue(runResource)));
    }
    opdef(return_from_node){
        runResource->requestReturn();
    }

    //External control flow
    opdef(terminate);
    opdef(sleep_for);
    opdef(trust_wait);

    //List operations
    opdef(list_clear);
    opdef(list_push);
    opdef(list_push_back);
}
#undef qdef

}}