#include "Types.hpp"
#include "Standard_Library.hpp"
#include "tokeniser.hpp"
#include "parser.hpp"



#include <chrono>
#include <ctime>
#include <cmath>
#include <iostream>

class Timer
{
public:
    void start()
    {
        m_StartTime = std::chrono::system_clock::now();
        m_bRunning = true;
    }

    void stop()
    {
        m_EndTime = std::chrono::system_clock::now();
        m_bRunning = false;
    }

    double elapsedMilliseconds()
    {
        std::chrono::time_point<std::chrono::system_clock> endTime;

        if (m_bRunning)
        {
            endTime = std::chrono::system_clock::now();
        }
        else
        {
            endTime = m_EndTime;
        }

        return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_StartTime).count();
    }

    double elapsedSeconds()
    {
        return elapsedMilliseconds() / 1000.0;
    }

private:
    std::chrono::time_point<std::chrono::system_clock> m_StartTime;
    std::chrono::time_point<std::chrono::system_clock> m_EndTime;
    bool                                               m_bRunning = false;
};

void simple_timed_fib() {
    double tot = 0;
    for (size_t i = 0; i < 500; i++) {
        Timer t;
        t.start();
        float a = 0;
        float b = 1;
        int j = 2;
        while (j < 100) {
            float res = a + b;
            a = b;
            b = res;
            printf("The fibonacci number at index %d is %f\n", j, b);
            j += 1;
        }
        tot += t.elapsedMilliseconds();
    }
    printf("Average time is %lf\n", tot / 500);
    char a;
    std::cin >> a;
}

void ncFibTest() {
    nc::comp::source s = nc::comp::source("fibonacci.txt");
    double tot = 0;
    auto l = std::make_shared<nc::additional_library>();
    l->operations = nc::stlib::_standard_operations;
    l->quearies = nc::stlib::_standard_quearies;
    nc::comp::compilation_environment ce;
    ce.addLibrary(l);
    auto t = nc::comp::tokeniseSource(s, ce);
    std::unique_ptr<nc::program> program = nc::comp::compile(nc::comp::parseTokens(t, &ce));
    nc::Runtime runtime;
    runtime.loadProgram(std::move(program));

    for (size_t i = 0; i < 500; i++) {
        Timer t;
        t.start();
        runtime.enterProgramAt("main");
        while(runtime.getNodesExecuted() == i){}

        tot += t.elapsedMilliseconds();
    }
    printf("Average milliseconds to run = %lf\n", tot / 500);

    //p.writeOutBasicSymbols("WriteOutTest.txt");

    char a;
    std::cin >> a;
}


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
    /*auto l = std::make_shared<nc::additional_library>();
    l->operations = nc::stlib::_standard_operations;
    l->quearies = nc::stlib::_standard_quearies;
    nc::comp::compilation_environment ce;
    ce.addLibrary(l);
    //nc::comp::source sourceCode(sc);
    //nc::comp::source sourceCode("UnitTests/Maths test.txt");
    nc::comp::source sourceCode("bigTest.txt");
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
    printf("Program has finished\n");*/

    simple_timed_fib();
    ncFibTest();

    return 0;
}