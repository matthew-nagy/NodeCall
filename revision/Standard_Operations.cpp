#include "Standard_Library.hpp"


namespace nc{   namespace stlib{

#define opdef(name) void name (argument_list& args, unique_run_resource& runResource)
namespace op{
    //IO
    opdef(print){
        argument_list al;
        for(size_t i = 0; i < args.size(); i++){
            printf("%s", std::any_cast<std::string>(*args[i].getValue(runResource)).c_str());
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

    //Just run quearies
    opdef(nop){
        for(size_t i = 0; i < args.size(); i++)
            args[0].getValue(runResource);
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

    //A way of "calling" without increasing the call stack
    opdef(send_node){
        runResource->sendToNode(std::any_cast<node_index>(*args[0].getValue(runResource)));
    }

    //External control flow
    opdef(terminate){
        runResource->requestTerminate();
    }
    opdef(sleep_for){
        std::shared_ptr<sleep_pack> sleepPack = std::make_shared<sleep_pack>();
        float sToWait = std::any_cast<float>(*args[0].getValue(runResource));
        sleepPack->toWait = sToWait;
        sleepPack->cond = &runResource->getConditionVariable();
        std::any anyPack = sleepPack;

        runResource->blockOnCondition([](const std::any& anyPack){
            sleep_pack& sp = *std::any_cast<std::shared_ptr<sleep_pack>>(anyPack).get();
            //Arbitrary precision
            std::this_thread::sleep_for(std::chrono::milliseconds(int(sp.toWait * 1000.0)));
            sp.cond->notify_one();
        }, sleepPack);
    }
    opdef(trust_wait){
        runResource->blockOnCondition([](const std::any&){}, std::make_any<void*>());
    }

    //List operations
    opdef(list_clear){
        std::vector<std::any>& v = *std::any_cast<std::shared_ptr<std::vector<std::any>>>(*args[0].getValue(runResource)).get();
        v.clear();
    }
    opdef(list_push){
        std::vector<std::any>& v = *std::any_cast<std::shared_ptr<std::vector<std::any>>>(*args[0].getValue(runResource)).get();
        for(size_t i = 1; i < args.size(); i++)
            v.emplace(v.begin(), *args[i].getValue(runResource));
    }
    opdef(list_push_back){
        std::vector<std::any>& v = *std::any_cast<std::shared_ptr<std::vector<std::any>>>(*args[0].getValue(runResource)).get();
        for(size_t i = 1; i < args.size(); i++)
            v.emplace_back(*args[i].getValue(runResource));
    }
}
#undef qdef

const OperationTable _standard_operations = {
    {"print", op::print}, {"println", op::println},
    {"assign", op::assign}, {"nop", op::nop},
    {"if", op::conditional_if}, {"while", op::while_loop}, {"do", op::do_while_loop}, {"call", op::call_node}, {"send_to", op::send_node},
    {"break", op::break_from_conditional}, {"return", op::return_from_node},
    {"end", op::terminate}, {"sleep_for", op::sleep_for}, {"trust_wait", op::trust_wait},
    {"list_clear", op::list_clear}, {"list_push", op::list_push}, {"list_push_back", op::list_push_back}
};

}}