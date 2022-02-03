#pragma once
#ifndef NC_STANDARD_LIBRARY_HPP
#define NC_STANDARD_LIBRARY_HPP

#include "Types.hpp"

namespace nc{   namespace stlib{    

extern const QuearyTable _standard_quearies;
extern const OperationTable _standard_operations;


    typedef value(*QuearyFunction)(const argument_list&, unique_run_resource&);
    typedef void(*OperationFunction)(const argument_list&, unique_run_resource&);

#define qdef(name) value name (const argument_list& args, unique_run_resource& runResource)
namespace q{
    //Mathmatic quearies
    qdef(add);
    qdef(sub);
    qdef(div);
    qdef(mul);
    qdef(mod);

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
    qdef(bnot);
    qdef(beq);
    qdef(bneq);
    qdef(bless);
    qdef(bgreater);
    qdef(blesseq);
    qdef(bgreatereq);

    //list quearies
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
}
#undef qdef

#define opdef(name) void name (const argument_list& args, unique_run_resource& runResource)
namespace op{
    //IO
    opdef(print);
    opdef(println);

    //Assignment
    opdef(assign);

    //Internal flow
    opdef(conditional_if);
    opdef(while_loop);
    opdef(do_while_loop);
    opdef(break_from_conditional);
    opdef(call_node);
    opdef(return_from_node);

    //External control flow
    opdef(terminate);
    opdef(sleep_for);
    opdef(trust_wait);

    //List operations
    opdef(list_create);
    opdef(list_clear);
    opdef(list_push);
    opdef(list_push_back);
}
#undef qdef

}
}
#endif