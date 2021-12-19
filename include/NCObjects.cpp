#include "NCObjects.hpp"

namespace ncprivate{
    NCArgUnion::NCArgUnion(){}
}


NCArgument& NCArgument::operator=(const NCArgument& right){
    type = right.type;
    switch(type){
        case ncat_Constant:
            argUnion.constant = right.argUnion.constant;
            break;
        case ncat_Queary:   
            argUnion.queary = right.argUnion.queary;
            break;
        case ncat_Object:
            argUnion.object = right.argUnion.object;
            break;
    }

    return *this;
}


NCArgument::~NCArgument(){
    if(type == ncat_Constant){
        delete argUnion.constant;
    }
}

std::any& NCArgument::getValue(NC_Runtime_Log& runtimeLog){
    switch(type){
        case ncat_Constant:
            return *argUnion.constant;
        case ncat_Queary:
            return argUnion.queary->operator()(runtimeLog);
        case ncat_Object:
            return *argUnion.object;
    }
}

std::any& NCQueary::operator()(NC_Runtime_Log& runtimeLog){
    this->hiddenValue = func(arguments, runtimeLog, lineNum);
    return this->hiddenValue;
}
void NCFunction::operator()(NCRuntime& runtime, NC_Runtime_Log& runtimeLog){
    func(runtime, arguments, runtimeLog, lineNumber);
}

#include "NCQuearies.hpp"
#include "NCOperations.hpp"
#include "NCCompiler.hpp"
int main(){

    std::ifstream file("newTest.ncs");
    std::vector<std::string> source;
    std::string line;
    while(std::getline(file, line))
        source.emplace_back(std::move(line));
    ncprivate::compiler::MultiLineString s(source);

    std::vector<std::any*> newlyCreatedVariables;
    std::vector<NCQueary*> newlyCreatedQuearies;

    ncprivate::compiler::CompilerEnvironment env(newlyCreatedVariables, newlyCreatedQuearies);
    env.addExtention(NCExtention(ncprivate::standard_functions, ncprivate::standard_quearies, std::map<std::string, std::any*>{}));
    auto tokens = ncprivate::compiler::lexer(s, env);

    printf("\n\nLexed\n\n");

    for(auto& t : tokens)
        t.print();
    return 0;
}