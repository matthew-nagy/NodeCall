#pragma once

#include "NCCompiler.hpp"
#include "NCObjects.hpp"
#include <mutex>

class NCRepl{
public:
    
    NCRuntime* compile(std::ifstream& file, const std::initializer_list<NCExtention*>& extentions){
        compilationMutex.lock();
        auto compilerOutput= NodeCall::compileWithEnvironment(file, extentions);
        runtime = compilerOutput.first;
        env = compilerOutput.second;
        compilationMutex.unlock();
        return runtime;
    }

    void execute(std::string input){
        compilationMutex.lock();
        if(input[0] == '>'){
            executeMacro(input);
        }
        else{
            printf("Got '%s'\n", input.c_str());
            executeCommand(input);
        }
        compilationMutex.unlock();
    }

private:
    ncprivate::compiler::CompilerEnvironment* env;
    NCRuntime* runtime;
    std::mutex compilationMutex;

    void executeMacro(const std::string& input){
        std::string operationalQueary;
        if(input.size() >= 4 )
            operationalQueary = input.substr(3);
        else
            operationalQueary = "";
        switch(input[1]){
            case 'f':
                printf(" | Available functions:\n");
                for(auto& [FuncName, _] : env->functions){
                    bool printOut = false;
                    if(operationalQueary == "")
                        printOut = true;
                    else if(input.size() >= operationalQueary.size()){
                        printOut = FuncName.substr(0, operationalQueary.size()) == operationalQueary;
                    }
                    
                    if(printOut)
                        printf(" | \t%s\n", FuncName.c_str());
                }
                printf(" |\n");
                break;
            case 'q':
                printf(" | Available quearies:\n");
                for(auto& [FuncName, _] : env->quearies){
                    bool printOut = false;
                    if(operationalQueary == "")
                        printOut = true;
                    else if(input.size() >= operationalQueary.size()){
                        printOut = FuncName.substr(0, operationalQueary.size()) == operationalQueary;
                    }
                    
                    if(printOut)
                        printf(" | \t%s\n", FuncName.c_str());
                }
                printf(" |\n");
                break;
            case 'v':
                printf(" | Available variables:\n");
                for(auto& [FuncName, _] : env->variables){
                    bool printOut = false;
                    if(operationalQueary == "")
                        printOut = true;
                    else if(input.size() >= operationalQueary.size()){
                        printOut = FuncName.substr(0, operationalQueary.size()) == operationalQueary;
                    }
                    
                    if(printOut)
                        printf(" | \t%s\n", FuncName.c_str());
                }
                printf(" |\n");
                break;
            case 'r':
                runtime->run(operationalQueary);
                break;
            default:
                printf(" | Unknown macro, available options are\n");
                printf(" | \t|>f : Displays available functions\n");
                printf(" | \t|>q : Displays available quearies\n");
                printf(" | \t|>v : Displays available variables\n");
                printf(" | \t|>r : Runs the current runtime from the given node.\n");
                printf(" |\n");
        }
    }

    void executeCommand(const std::string& input){
        NCRuntime* miniRuntime = env->compile(std::vector<std::string>{"cmdMain>", input, ">"});
        miniRuntime->run("cmdMain");
        delete miniRuntime;
    }
};
