#pragma once
#include "NCObjects.hpp"
#include "NCLog.hpp"
#include <typeindex>
#include <iostream>

#define qdef(name) std::any name (std::vector<NCArgument>& args, NC_Runtime_Log& runLog, unsigned lineNumber)
namespace ncprivate{
namespace queary{
    qdef(add);
    qdef(sub);
    qdef(div);
    qdef(mul);
    qdef(mod);

    qdef(lshift);
    qdef(rshift);

    qdef(land);
    qdef(lor);
    qdef(lnot);
    qdef(lxor);

    qdef(band);
    qdef(bor);
    qdef(bnot);
    qdef(beq);
    qdef(bneq);
    qdef(bless);
    qdef(bgreater);
    qdef(blesseq);
    qdef(bgreatereq);

    qdef(size);
    qdef(index);
    qdef(pop);
    qdef(front);
    qdef(tail);

    qdef(input);

    qdef(getType);
    qdef(tfint);
    qdef(tffloat);
    qdef(tfbool);
    qdef(tfstring);

/*
queary::land, queary::lor, queary::lnot, queary::lxor, queary::band, queary::bor, queary::bnot,
        queary::beq, queary::bneq, queary::bless, queary::bgreater, queary::blesseq, queary::bgreatereq,
        queary::size, queary::index, queary::pop, queary::front, queary::tail,
        queary::input,
        queary::getType, queary::tfint, queary::tffloat, queary::tfbool, queary::tfstring
*/
}

 extern const std::map<std::string, NCQuearyFunc> standard_quearies;

}

#undef qdef