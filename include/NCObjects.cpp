#include "NCObjects.hpp"

NCArgument& NCArgument::operator=(const NCArgument& right){
    type = right.type;
    switch(type){
        case ncat_Constant:
            constantVal = right.constantVal;
            break;
        case ncat_Queary:   
            quearyPtr = right.quearyPtr;
            break;
        case ncat_Object:
            objectPtr = right.objectPtr;
            break;
    }

    return *this;
}


NCArgument::NCArgument(const NCArgument& cpyf){
    *this = cpyf;
}

std::any& NCArgument::getValue(NC_Runtime_Log& runtimeLog){
    switch(type){
        case ncat_Constant:
            return constantVal;
        case ncat_Queary:
            return quearyPtr->operator()(runtimeLog);
        case ncat_Object:
            return *objectPtr;
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
    NCExtention basicNodeCall(ncprivate::standard_functions, ncprivate::standard_quearies, std::map<std::string, std::any*>{});
    printf("Compiling\n");
    NCRuntime* runtime = NodeCall::compile(file, {&basicNodeCall});
    printf("Compiled, running\n");

    (*runtime).run();
    delete runtime;
    return 0;
}