#pragma once
#include<vector>
#include<string>
#include "NCObjects.hpp"
#include "NCQuearies.hpp"

namespace ncprivate{
namespace ops{
#define opdef(NAME) void NAME (NCRuntime& environment, std::vector<NCArgument>& args, NC_Runtime_Log& runLog, unsigned lineNumber)

    opdef(assign);
    opdef(print);
    opdef(println);

    //If the first argument resolves to a boolean true, the second argument is treated
    //as a node index and execution is sent there
    opdef(conditional_if);
    //As with if, but should the condition be false, treats argument 3 as the else's node
    //index. Execution is sent there
    opdef(conditional_if_else);

    //Used for the top of do_whiles
    opdef(link_conditionally);
    //Used to just link
    opdef(link_to);
    //Call a node like a function
    opdef(call_node);

    opdef(return_from);
    opdef(break_from);
    opdef(end);


     //Add a macro controlled type safety to this
    opdef(new_vector);
    opdef(push_front);
    opdef(push_back);
    opdef(erase);
    opdef(clear);

}

    extern const std::unordered_map<std::string, NCOperationFunc> standard_functions;

}

/*

    if
    if_else (a boolean with 2 possible node indices to jump to, worry about theparse later)
    
    while and do_while are put at the bottom of their node, with if and ifelse to do it
    So they are more macro-y than anything else

    link_to_coinditionally (creates a new call frame)
    link_to


    call_node
    return_from
    end_conditional_section (maybe break in the parser?)
    end

    new_vector
    push_front
    push_back
    erase
    clear

*/