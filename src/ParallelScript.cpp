#include "../header/ParallelScript.h"


namespace nc {

	//Blocks the current thread while the script is still running
	void Script_Thread::blockWhileRunning() {
		runningMutex.lock();
		runningMutex.unlock();
	}

	//Returns true if the script thread is currently running a NodeCal Program
	bool Script_Thread::isScriptRunning() {
		return scriptIsRunning;
	}

	//Tells the program to hop to a new node of execution
	//Blocks if the program is already running some code
	void Script_Thread::sendExecutionToNode(const std::string& nodeName) {
		runningMutex.lock();

		nodeCallProgram.restart(nodeName);

		runningMutex.unlock();
	}

	//Changes the layout to be run on
	void Script_Thread::changeProgram(const NCNodeLayout& layout, std::string startNode) {
		runningMutex.lock();

		nodeCallProgram.changeLayout(layout, startNode);

		runningMutex.unlock();
	}

	//Once paused, the script will only run once launched
	void Script_Thread::launch() {
		launchBlocker.incriment();
	}

	NCThreadControl* Script_Thread::getThreadControl() {
		return nodeCallProgram.threadControl;
	}

	Script_Thread::Script_Thread(const NCNodeLayout& program, const SymbolTable& additionalSymbols, std::string startNode) :
		nodeCallProgram(program, startNode),
		scriptIsRunning(false),
		endScriptFlag(false),
		launchBlocker(0)
	{
		scriptThread = std::thread([this]() {runScript(); });
		//Add any extra symbols you need for your script
		for (auto& p : additionalSymbols)
			nodeCallProgram.symbols.emplace(p);
	}

	Script_Thread::~Script_Thread() {
		//Tell the script thread you are finished with it
		endScriptFlag = true;
		//Allow the thread to "launch" to the while condition, which is now false, so the program ends
		launchBlocker.incriment();
		//Now join the thread
		scriptThread.join();
	}


	//Tells the script to run. Should be run on a new thread
	void Script_Thread::runScript() {
		//Hold here until the main thread is ready
		launchBlocker.decriment();

		while (!endScriptFlag) {

			//Set the state to be running a program
			runningMutex.lock();
			scriptIsRunning = true;


			//We aren't taking advantage of a feature of node call here, because reasons
			std::atomic_bool junk_bool = false;		//So we want to basically ignore this bool here
			nodeCallProgram.run(junk_bool);


			//Program finished, it can be messed with externally now
			scriptIsRunning = false;
			runningMutex.unlock();

			//Prevent it from setting off again right away
			launchBlocker.decriment();

		}
	}

}