#pragma once
#ifndef PARALLELSCRIPT_H
#define PARALLELSCRIPT_H
#include "NCCompiler.h"
#include <thread>

namespace nc {

	//Allows for NodeCall programs to be run and manipulated on another thread
	class Script_Thread {
	public:

		//Blocks the current thread while the script is still running
		void blockWhileRunning();

		//Returns true if the script thread is currently running a NodeCal Program
		bool isScriptRunning();

		//Tells the program to hop to a new node of execution
		//Blocks if the program is already running some code
		void sendExecutionToNode(const std::string& nodeName);

		//Changes the layout to be run on
		void changeProgram(const NCNodeLayout& layout, std::string startNode = "main");

		//Once paused, the script will only run once launched
		void launch();

		NCThreadControl* getThreadControl();

		SymbolTable& getSymbolTable();

		Script_Thread(const NCNodeLayout& program, const SymbolTable& additionalSymbols, std::string startNode = "main");

		~Script_Thread();
	private:
		//Thread the script is run in seperate to the main game
		std::thread scriptThread;

		//Runtime the script is run off of
		NodeCallProgram nodeCallProgram;

		//Is true as long as the script should be running
		std::atomic_bool scriptIsRunning;

		//Set to true to shut down the scriptThread
		std::atomic_bool endScriptFlag;

		//Locked by the running of the script
		std::mutex runningMutex;

		//Stops the script thread from starting right away
		Semaphore launchBlocker;

		//Tells the script to run. Should be run on a new thread
		void runScript();
	};

}

#endif