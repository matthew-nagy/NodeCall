#include "Types.hpp"
#include "Standard_Library.hpp"
#include "tokeniser.hpp"
#include "parser.hpp"

std::vector<std::string> sc = {
    "someNode{",
    "println(\"Executing...\");"
    "assign(num, 2.0);  #Some comment?",
    "lengthSqr = ((1.0 * 1.0) + (num * num));",
    "   ",
    "      #comment test",
    "",
    "  println(\"About to heccin do the ifffff\");",
    "           if(equals(1, 2)){",
    "println(\"Wow here we be!\");",
    "}else{println(\"Panic at the disco\");}",
    "println(ftos(lengthSqr), \" thingy\\ttime\");",
    "print(\"ham salad!\", \"\\n\");",
    "end();",
    "}",
    "nextNode{",
    "println(\"This thing sure can parse!\");",
    "}"
};

int main(){
    auto l = std::make_shared<nc::additional_library>();
    l->operations = nc::stlib::_standard_operations;
    l->quearies = nc::stlib::_standard_quearies;
    nc::comp::compilation_environment ce;
    ce.addLibrary(l);
    //nc::comp::source sourceCode(sc);
    nc::comp::source sourceCode("UnitTests/Maths test.txt");
    auto t = nc::comp::tokeniseSource(sourceCode, ce);
    //nc::comp::parser p(&ce);

    for (size_t i = 0; i < t.size(); i++) {
        //p.printState();
        printf("\t\t%s\t%s\n", t[i].representation.c_str(), nc::comp::tokenRep.find(t[i].type)->second.c_str());
       // p.giveToken(t[i]);
    }

    std::unique_ptr<nc::program> program = nc::comp::compile(nc::comp::parseTokens(t, &ce));
    nc::Runtime runtime;
    runtime.loadProgram(std::move(program));
    //runtime.enterProgramAt("someNode");
    runtime.enterProgramAt("main");
    printf("Executing program...\n");
    while (runtime.isRunning()) {}
    printf("Program has finished\n");

    return 0;
}