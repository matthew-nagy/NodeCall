#include "Types.hpp"
#include "Standard_Library.hpp"
#include "tokeniser.hpp"
#include "parser.hpp"

std::vector<std::string> sc = {
    "someNode{",
    "assign(num, 2.0);  #Some comment?",
    "lengthSqr = ((num * num) + (1.0 * 1.0));"
    "print(itos(num), \"thingy\\ttime\");",
    "end();",
    "}"
};

int main(){
    auto l = std::make_shared<nc::additional_library>();
    l->operations = nc::stlib::_standard_operations;
    l->quearies = nc::stlib::_standard_quearies;
    nc::comp::compilation_environment ce;
    ce.addLibrary(l);
    nc::comp::source sourceCode(sc);
    auto t = nc::comp::tokeniseSource(sourceCode, ce);
    nc::comp::parser p(&ce);

    for (size_t i = 0; i < t.size(); i++) {
        p.printState();
        printf("\t\t%s\t%s\n", t[i].representation.c_str(), nc::comp::tokenRep.find(t[i].type)->second.c_str());
        p.giveToken(t[i]);
    }

    std::unique_ptr<nc::program> program = p.getProgram();
    nc::Runtime runtime;
    runtime.loadProgram(std::move(program));
    runtime.enterProgramAt("someNode");
    while (runtime.isRunning()) {}

    return 0;
}