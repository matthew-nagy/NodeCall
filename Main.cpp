#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
size_t created = 0;
size_t deleted = 0;

void * operator new(size_t size)
{
	created += 1;
	void * p = malloc(size);
	return p;
}

void operator delete(void * p)
{
	deleted += 1;
	free(p);
}

#include "NodeCall.h"
#include<iostream>

using namespace nc;

void assign_reassign_print_if_test() {
	NCRuntimeList mainFunction;
	mainFunction.reserve(20);
	mainFunction.emplace_back("println", NCArgumentList{ new NCSetObjectReference(new NCObject(makeNCObject(69420))) }, NCRuntimeList{});
	mainFunction.emplace_back("assign", NCArgumentList{ new NCNamedObjectReference("myNum"), new NCSetObjectReference(new NCObject(makeNCObject( 5))) }, NCRuntimeList{});
	mainFunction.emplace_back("assign", NCArgumentList{ new NCNamedObjectReference("aBoolean"), new NCSetObjectReference(new NCObject(makeNCObject(true))) }, NCRuntimeList{});
	mainFunction.emplace_back("println", NCArgumentList{ new NCSetObjectReference(new NCObject(makeNCObject(std::string("Made it past the primary asignments")))) }, NCRuntimeList{});
	
	NCRuntimeList innerIfThingy;
	innerIfThingy.reserve(10);
	innerIfThingy.emplace_back("println", NCArgumentList{ new NCSetObjectReference(new NCObject(makeNCObject(std::string("Oh pog, In the if statement")))) }, NCRuntimeList{});
	innerIfThingy.emplace_back("assign", NCArgumentList{ new NCNamedObjectReference("myNum"), new NCSetObjectReference(new NCObject(makeNCObject(7))) }, NCRuntimeList{});
	//innerIfThingy.emplace_back("end", NCArgumentList{}, NCRuntimeList());

	mainFunction.emplace_back("if", NCArgumentList{ new NCInlineFunctionReference(reserved::inln::eq_comp, NCArgumentList{ new NCNamedObjectReference("aBoolean"), new NCSetObjectReference(new NCObject(makeNCObject(true))) },"equality") }, innerIfThingy);
	mainFunction.emplace_back("println", NCArgumentList{ new NCSetObjectReference(new NCObject(makeNCObject(std::string("The value started at 5, is now:"))))}, NCRuntimeList{});
	mainFunction.emplace_back("print", NCArgumentList{ new NCNamedObjectReference("myNum") }, NCRuntimeList{});
	NCNodeLayout layout;
	layout["main"] = mainFunction;
	NodeCallProgram myProgram(layout);

	std::atomic_bool programEnded = false;
	myProgram.run(programEnded);

	char a;
	std::cin >> a;
}

void reset_test() {
	NCRuntimeList mainFunction;
	mainFunction.reserve(20);
	mainFunction.emplace_back("assign", NCArgumentList{ new NCNamedObjectReference("someNum"), new NCSetObjectReference(new NCObject(makeNCObject(5))) }, NCRuntimeList{});
	mainFunction.emplace_back("assign", NCArgumentList{ new NCNamedObjectReference("aFloat"), new NCSetObjectReference(new NCObject(makeNCObject<float>(2.32))) }, NCRuntimeList{});
	mainFunction.emplace_back("println", NCArgumentList{ new NCSetObjectReference(new NCObject(makeNCObject<std::string>("The value of my float is "))), new NCNamedObjectReference("aFloat") }, NCRuntimeList{});
	mainFunction.emplace_back("assign", NCArgumentList{ new NCNamedObjectReference("aFloat"), 
		new NCInlineFunctionReference(reserved::inln::math_add, NCArgumentList{ new NCNamedObjectReference("aFloat"), new NCNamedObjectReference("someNum")}, "addition")
	}, NCRuntimeList{});
	mainFunction.emplace_back("println", NCArgumentList{ 
		new NCSetObjectReference(new NCObject(makeNCObject<std::string>("After adding the value "))), 
		new NCNamedObjectReference("someNum"),
		new NCSetObjectReference(new NCObject(makeNCObject<std::string>(" the value is now "))),
		new NCNamedObjectReference("aFloat") }, NCRuntimeList{});
	

	NCNodeLayout layout;
	layout["main"] = mainFunction;
	NodeCallProgram myProgram(layout);

	std::atomic_bool programEnded = false;
	myProgram.run(programEnded);

	programEnded = false;
	myProgram.reset();
	printf("And now having reset it\n");
	myProgram.run(programEnded);

	char a;
	std::cin >> a;
}

void embbed_test() {
	NCRuntimeList mainFunction;
	mainFunction.reserve(20);
	mainFunction.emplace_back("assign", NCArgumentList{ new NCNamedObjectReference("num"), new NCSetObjectReference(new NCObject(makeNCObject(10))) }, NCRuntimeList{});
	mainFunction.emplace_back("print_and_incriment", NCArgumentList{ new NCNamedObjectReference("num") }, NCRuntimeList{});
	mainFunction.emplace_back("print_and_incriment", NCArgumentList{ new NCNamedObjectReference("num") }, NCRuntimeList{});
	mainFunction.emplace_back("print_and_incriment", NCArgumentList{ new NCNamedObjectReference("num") }, NCRuntimeList{});
	mainFunction.emplace_back("print_and_incriment", NCArgumentList{ new NCNamedObjectReference("num") }, NCRuntimeList{});
	

	NCNodeLayout layout;
	layout["main"] = mainFunction;
	NodeCallProgram myProgram(layout);
	myProgram.symbols["print_and_incriment"] = new NCFunction([](NCThreadControl& tc, NCArgumentList& args, SymbolTable& table, NCRuntimeList&)->NCReturnValue {
		NCObject& target = args[0]->get(table, tc);
		printf("%d\n", target.asInt());
		target = target.asInt() + 1;
		return ncr_Void;
	}, "printAndIncrimentEmbedded");

	std::atomic_bool programEnded = false;
	myProgram.run(programEnded);

	char a;
	std::cin >> a;
}

void compile_from_source_test() {
	std::vector<std::string> source = {
		"main>",
		"   println(\"Howdy, I am here\");",
		"   println(5);",
		"   println(0.386);",
		"   println(True);",
		"   println(:type);",
		"   assign ( num1, 5);",
		"   assign ( num2, 2);",
		"   assign ( result, add ( num1, num2));",
		"   println(\"Assignment has been done, we have everything\");",
		//"	throw();",
		"   println(add(str(num1), \" + \", str(num2), \" = \", str(result)));",
		">"
	};

	printf("About to compile\n");
	NCNodeLayout* layout = getNCProgramFromSource(source);
	printf("Mem in use after compile is %u\n", created - deleted);
	printf("Compiled, onto running\n\n");
	NodeCallProgram p(*layout);
	std::atomic_bool hasEnded = false;
	p.run(hasEnded);

	char a;
	std::cin >> a;
	delete layout;
}

void full_operator_test() {
	NCNodeLayout* layout = getNCProgramFromFile("Tests/full_operator_test.nc");
	NodeCallProgram p(*layout);
	std::atomic_bool hasEnded = false;
	p.run(hasEnded);

	char a;
	std::cin >> a;

	delete layout;
}

#include <chrono>
#include <ctime>
#include <cmath>

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
		int a = 0;
		int b = 1;
		int j = 2;
		while (j < 100) {
			int res = a + b;
			a = b;
			b = res;
			printf("The fibonacci number at index %d is %d\n", j, b);
			j += 1;
		}
		tot += t.elapsedMilliseconds();
	}
	printf("Average time is %lf\n", tot / 500);
}

void fibonacci_test() {
	NCNodeLayout* layout = getNCProgramFromFile("Tests/fibonacci_test.nc");
	double tot = 0;
	for (size_t i = 0; i < 500; i++) {
		NodeCallProgram p(*layout);
		std::atomic_bool hasEnded = false;
		Timer t;
		t.start();
		p.run(hasEnded);
		tot += t.elapsedMilliseconds();
	}
	printf("Average milliseconds to run = %lf\n", tot / 500);

	//p.writeOutBasicSymbols("WriteOutTest.txt");

	char a;
	std::cin >> a;

	delete layout;
}

void equiptNodeWithFibAndTest() {
	NCObject runFibonacciObj;
	//NCReturnValue(NCThreadControl&, NCArgumentList&, SymbolTable&, NCRuntimeList&)
	auto fibFunc = NCFunction([](NCThreadControl&, NCArgumentList&, SymbolTable&, NCRuntimeList&)->NCReturnValue {
		int a = 0;
		int b = 1;
		int j = 2;
		while (j < 100) {
			int res = a + b;
			a = b;
			b = res;
			printf("The fibonacci number at index %d is %d\n", j, b);
			j += 1;
		}
		return ncr_Void;
	}, "runFibonacci");
	runFibonacciObj = &fibFunc;

	NCNodeLayout* layout = getNCProgramFromFile("Tests/linked_fibonacci.nc");
	double tot = 0;
	for (size_t i = 0; i < 500; i++) {
		NodeCallProgram p(*layout);
		p.symbols["runFibonacci"] = runFibonacciObj;
		std::atomic_bool hasEnded = false;
		Timer t;
		t.start();
		p.run(hasEnded);
		tot += t.elapsedMilliseconds();
	}

	printf("Average elapsed time for the embedded fibonacci was %f\n", tot / 500.0);

	delete layout;

	char a;
	std::cin >> a;
}

void run_with_name(std::string name, bool holdAfterCall = true) {
	NCNodeLayout* layout = getNCProgramFromFile(name);
	NodeCallProgram p(*layout);
	std::atomic_bool hasEnded = false;
	p.run(hasEnded);

	if (holdAfterCall) {
		char a;
		std::cin >> a;
	}

	delete layout;
}

void parallelism_test() {
	std::vector<std::string> program = {
		"main>",
		"  assign(a, 5);",
		"  println(\"a should be 5:\", a);",
		">"
	};
	NCNodeLayout* layout = getNCProgramFromSource(program);
	Script_Thread thread(*layout, SymbolTable());

	thread.launch();
	thread.blockWhileRunning();
	thread.sendExecutionToNode("main");
	thread.launch();
	
	std::vector<std::string> secondProgram = {
		"main>",
		"  println(\"a should still be an accessable variable, here is its value\");",
		"  println(a);",
		">"
	};
	NCNodeLayout* secondLayout = getNCProgramFromSource(secondProgram);

	thread.blockWhileRunning();
	thread.changeProgram(*secondLayout);
	thread.launch();
	thread.blockWhileRunning();

	delete layout;
	delete secondLayout;

}

int main() {
	printf("%u ncobject\n", sizeof(nc::NCObject));
	printf("%u inline func reference\n", sizeof(nc::NCInlineFunctionReference));
	printf("%u named obj reference\n", sizeof(nc::NCNamedObjectReference));
	printf("%u obj reference\n", sizeof(nc::NCObjectReference));
	printf("%u packet\n", sizeof(nc::NCPacket));
	printf("%u runtime object\n", sizeof(nc::NCRuntimeObject));
	printf("%u ncprogram\n", sizeof(nc::NodeCallProgram));
	printf("%u set obj ref\n", sizeof(nc::NCSetObjectReference));
	printf("%u thread control\n", sizeof(nc::NCThreadControl));
	printf("%u func\n", sizeof(nc::NCFunction));
	printf("%u argList\n", sizeof(nc::NCArgumentList));
	printf("%u inline func\n", sizeof(nc::NCInlineFunction));
	printf("%u node layout\n", sizeof(nc::NCNodeLayout));
	printf("%u runtimeLIst\n", sizeof(nc::NCRuntimeList));
	printf("%u cc lobby\n", sizeof(nc::Created_Container_Lobby));
	printf("%u symbol table\n", sizeof(nc::SymbolTable));
	printf("%u int\n", sizeof(int));
	printf("%u float\n", sizeof(float));
	printf("%u string\n", sizeof(std::string));

	printf("At start mem in use is %u\n", created - deleted);
	//assign_reassign_print_if_test();
	//reset_test();
	//embbed_test();
	//compile_from_source_test();
	//full_operator_test();
	//fibonacci_test();
	//equiptNodeWithFibAndTest();
	//simple_timed_fib();
	//run_with_name("Tests/fibonacci_test.nc", false);

	parallelism_test();

	printf("Mem still in use is %u\n", created - deleted);

	char a;
	std::cin >> a;
	return 0;
}