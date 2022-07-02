#pragma once
#ifndef NC_STANDARD_LIBRARY_HPP
#define NC_STANDARD_LIBRARY_HPP

#include "Types.hpp"
#include <cmath>
#include <chrono>

#define ERROR_MAKE(NAME) class NAME : public std::logic_error{ public: NAME () : std::logic_error( #NAME ){} }  

namespace nc{   namespace stlib{  

ERROR_MAKE(INVALID_ARGUMENT_TYPE);

extern const QuearyTable _standard_quearies;
extern const OperationTable _standard_operations;

bool is_true(argument& val, unique_run_resource& runResource);


struct if_pack{
    argument_list triggers;
    std::vector<node_index> resultantNodes;
    node_index elseNode = 0;
    bool hasFinalElse = false;
};
struct while_pack{
    argument trigger;
    node_index node;

    while_pack():
        trigger(new std::any()),
        node(0)
    {}
};
struct sleep_pack{
    std::condition_variable* cond;
    float toWait;  
};

template<class T>
value moveVal(T&& input) {
    return std::make_shared<std::any>(std::make_any<T>(input));
}
template<class T>
value makeVal(T input) {
    return std::make_shared<std::any>(std::make_any<T>(input));
}

#define qdef(name) value name (argument_list& args, unique_run_resource& runResource)
namespace q{
    //Mathmatic quearies
    qdef(add);
    qdef(sub);
    qdef(div);
    qdef(mul);
    qdef(mod);
    qdef(square_root);

    //Logical quearies
    qdef(lshift);
    qdef(rshift);
    qdef(land);
    qdef(lor);
    qdef(lnot);
    qdef(lxor);

    //Binary quearies
    qdef(band);
    qdef(bor);
    qdef(beq);
    qdef(bneq);
    qdef(bless);
    qdef(bgreater);
    qdef(blesseq);
    qdef(bgreatereq);

    //list quearies
    qdef(list_create);
    qdef(list_size);
    qdef(list_index);
    qdef(list_pop);
    qdef(list_pop_front);
    qdef(list_front);
    qdef(list_tail);

    //IO quearies
    qdef(input);

    //Type quearies and conversions
    qdef(getType);

    qdef(ftoi);
    qdef(stoi);
    qdef(btoi);

    qdef(itof);
    qdef(btof);
    qdef(stof);

    qdef(itob);
    qdef(ftob);
    qdef(stob);

    qdef(itos);
    qdef(ftos);
    qdef(btos);
    qdef(ttos);
}

#define opdef(name) void name (argument_list& args, unique_run_resource& runResource)
namespace op{
    //IO
    opdef(print);
    opdef(println);

    //Assignment
    opdef(assign);
    
    //So; what if you want to run a queary without it doing anything?
    opdef(nop);//   :)

    //Internal flow
    opdef(conditional_if);
    opdef(conditional_elif);
    opdef(conditional_else);
    opdef(while_loop);
    opdef(while_node_bottom);
    opdef(break_from_while);
    opdef(break_from_if);
    opdef(call_node);
    opdef(return_from_node);
    //A way of "calling" without increasing the call stack
    opdef(send_node);

    //External control flow
    opdef(terminate);
    opdef(sleep_for);
    opdef(trust_wait);

    //List operations
    opdef(list_clear);
    opdef(list_push);
    opdef(list_push_back);
}

}
}
#endif