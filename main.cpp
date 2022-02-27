#include "include/NodeCall2.hpp"

const std::vector<std::string> c_unitTestPaths = {
    "conditional_tests",
    "logic_tests",
    "maths_test"
};

const std::string c_exitCommand = "exit";

int main(){

    bool testing = true;
    std::string request;

    auto environment = nc::createEnvironment({nc::additional_library::standardLibrary});
    nc::Runtime runtime;

    while(testing){
        printf("Please enter the number of the script to run, or %s to quit:\n", c_exitCommand.c_str());
        
        for(size_t i = 0; i < c_unitTestPaths.size(); i++){
            printf("%zu:\t%s\n", i + 1, c_unitTestPaths[i].c_str());
        }
        
        printf("<<");
        std::cin >> request;

        if(request == c_exitCommand){
            testing = false;
        }
        else{
            int index = std::atoi(request.c_str()) + -1;
            if(index < 0 || index >= c_unitTestPaths.size()){
                printf("That is not a valid index\n");
            }
            else{
                auto program = nc::compileProgram(environment, "test_scripts/" + c_unitTestPaths[index] + ".dat", true);
                runtime.loadProgram(program);
                runtime.enterProgramAt("main");
                
                //Short busy wait for the test to finish
                while(runtime.isRunning()){}
            }
        }
    }

    return 0;
}