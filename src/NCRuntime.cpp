#include "../header/NCRuntime.h"

namespace nc {

	NCReturnValue NCRuntimeObject::run(SymbolTable& table, NCThreadControl& threadControl) {
		return command->get(table, threadControl)(threadControl, arguments, table, internalCommands);
	}

	//#define NC_OBJECT_REFERENCE_MEMORY_TRACKER

	NCRuntimeObject::NCRuntimeObject(std::string commandName, NCArgumentList arguments, const NCRuntimeList& internalCommands) :
		command(new NCNamedObjectReference(commandName)),
		arguments(arguments),
		internalCommands(std::move(internalCommands))
	{
#ifdef NC_OBJECT_REFERENCE_MEMORY_TRACKER
		printf("Runtime %p created from command name %s\n", (void*)command, commandName.c_str());
#endif
	}
	NCRuntimeObject::NCRuntimeObject(NCObjectReference* command, NCArgumentList arguments, const NCRuntimeList& internalCommands) :
		command(command->makeCopy()),
		arguments(arguments),
		internalCommands(std::move(internalCommands))
	{
#ifdef NC_OBJECT_REFERENCE_MEMORY_TRACKER
		printf("Runtime %p created from command %p\n", (void*)this->command, (void*)command);
#endif
	}

	NCArgumentList makeCopyOfList(const NCArgumentList& in) {
		NCArgumentList ret;
		for (auto* a : in)
			ret.push_back(a->makeCopy());
		return ret;
	}

	NCRuntimeObject::NCRuntimeObject(const NCRuntimeObject& __copy) :
		arguments(makeCopyOfList(__copy.arguments)),
		command(__copy.command->makeCopy())
	{
#ifdef NC_OBJECT_REFERENCE_MEMORY_TRACKER
		printf("Copied runtime object with pointer %p\n", (void*)__copy.command);
#endif
		for (const NCRuntimeObject& ncro : __copy.internalCommands)
			internalCommands.push_back(NCRuntimeObject(ncro));
	}

	NCRuntimeObject::~NCRuntimeObject() {
#ifdef NC_OBJECT_REFERENCE_MEMORY_TRACKER
		printf("Deleting runtime %p\n", (void*)this->command);
#endif
		delete command;
		for (auto* a : arguments)
			delete a;
	}

	void NodeCallProgram::run(std::atomic_bool& programHasEnded) {
		printf("In run\n");
		NCReturnValue currentReturn = ncr_Void;
		while (currentReturn != ncr_Terminate && currentReturn != ncr_Commands_Exhausted)
			currentReturn = NodeCallProgram::runCommands(layout[symbols[REG_Node_Name].asString()], symbols, threadControl);

		programHasEnded = true;
		//Allow the external program to resume
		threadControl->externalExecutionLock->incriment();
	}

	NCReturnValue NodeCallProgram::runCommands(
		NCRuntimeList& commands,
		SymbolTable& table,
		NCThreadControl* threadControl
	) {
		NCReturnValue currentReturn = ncr_Void;
		bool hasCommandsLeft = true;
		// Change access from map grabs to references
		NCObject& commandIndex = table[REG_Command_Index];
		int commandIndexPriorToRunning = table[REG_Command_Index].asInt();
		commandIndex = 0;
		NCObject& lastIndex = table[REG_Last_Index];
		

		while (currentReturn != ncr_Terminate && currentReturn != ncr_Node_Shift && hasCommandsLeft) {
			while (currentReturn == ncr_Void && hasCommandsLeft) {
				//Index of current command to be run
				int programCounter = commandIndex.asInt();
				//Fetch the command

				NCRuntimeObject& command = commands[programCounter];
				//Incriment program counter 

				lastIndex = commandIndex;
				commandIndex = programCounter + 1;
				//Execute
				currentReturn = command.run(table, *threadControl);

				if (commandIndex.asInt() >= commands.size()) {
					hasCommandsLeft = false;
					if (currentReturn == ncr_Void)
						currentReturn = ncr_Commands_Exhausted;
				}
			}
			if (currentReturn == ncr_Relinquish) {
				//Allow the external program to start up again
				threadControl->externalExecutionLock->incriment();
				//External program will incriment the internal sem when ready
				threadControl->NCInternalLock->decriment();
				currentReturn = ncr_Void;
			}
		}
		commandIndex = commandIndexPriorToRunning;
		return currentReturn;
	}

	void NodeCallProgram::reset(std::string startNode) {
		auto createds = (Created_Container_Lobby*)symbols[REG_Created_Container_Lobby].asNCPacket().data;
		createds->deleteAll();
		delete createds;
		delete threadControl;
		threadControl = new NCThreadControl();
		symbols.clear();
		symbols = getNCSymbolTable(startNode);
	}


	void NodeCallProgram::readInBasicSymbols(std::ifstream& file) {
		while (!file.eof()) {
			size_t keySize;
			file.read((char*)&keySize, sizeof(size_t));
			std::string key = "";
			key.resize(keySize);
			file.read(&key[0], keySize);
			//Emplaces by key, NCObject deals with the file
			symbols.emplace(key, file);
		}
	}
	void NodeCallProgram::readInBasicSymbols(const std::string& fileDestination) {
		std::ifstream file;
		file.open(fileDestination, std::ios::in | std::ios::binary);
		readInBasicSymbols(file);
		file.close();
	}

	void NodeCallProgram::writeOutBasicSymbols(std::ofstream& file) {
		for (auto& pair : symbols) {
			//If it is a legal type and also not a set up environment variable, write it out
			if (readwritableNCTypes.count(pair.second.getType()) > 0 && REG_Variables.count(pair.first) == 0) {
				size_t size = pair.first.size();
				file.write((char*)&size, sizeof(size_t));
				file.write(&pair.first[0], size);
				pair.second.writeToFile(file);
			}
		}
	}
	void NodeCallProgram::writeOutBasicSymbols(const std::string& fileDestination) {
		std::ofstream file;
		file.open(fileDestination, std::ios::out | std::ios::binary);
		writeOutBasicSymbols(file);
		file.close();
	}


	NodeCallProgram::NodeCallProgram(const NCNodeLayout& layout, std::string firstNode) :
		layout(std::move(layout)),
		symbols(getNCSymbolTable(firstNode)),
		threadControl(new NCThreadControl())
	{}

	NodeCallProgram::~NodeCallProgram() {
		delete threadControl;
		auto createds = (Created_Container_Lobby*)symbols[REG_Created_Container_Lobby].asNCPacket().data;
		createds->deleteAll();
		delete createds;
	}

}