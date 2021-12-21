#include "include/NCCompiler.hpp"

#include <iostream>
int main(){
    std::vector<std::string> scripts = {
        "conditional_tests.ncs",
        "maths_test.ncs"
    };

    printf("Available test scripts to run are:\n");
    for(size_t i = 0; i < scripts.size(); i++){
        printf("\t%zu: %s\n", i + 1, scripts[i].c_str());
    }
    printf("Please enter an integer indicating which test you wish to run\n\t>");

    std::string answer;
    std::cin >> answer;

    std::ifstream file("test_scripts/" + scripts[std::atoi(answer.c_str()) - 1]);
    NCRuntime* runtime = NodeCall::compile(file, {});
    runtime->run();
    delete runtime;
}