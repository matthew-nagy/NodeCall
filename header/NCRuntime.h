#pragma once
#include "NCObject.h"
#include <memory>
#include <fstream>

#ifndef NC_RUNTIME
#define NC_RUNTIME true
namespace nc {

	struct NCRuntimeObject {
		NCObjectReference* command;
		NCArgumentList arguments;
		//A list of "internal" commands. Think how the if() function takes {} of commands (same with while)
		NCRuntimeList internalCommands;

		NCReturnValue run(SymbolTable& table, NCThreadControl& threadControl);

		NCRuntimeObject(std::string commandName, NCArgumentList arguments, const NCRuntimeList& internalCommands);
		NCRuntimeObject(NCObjectReference* command, NCArgumentList arguments, const NCRuntimeList& internalCommands);

		NCRuntimeObject(const NCRuntimeObject& __copy);

		~NCRuntimeObject();
	};

	typedef std::unordered_map<std::string, NCRuntimeList> NCNodeLayout;
	struct NodeCallProgram {
		NCNodeLayout layout;
		SymbolTable symbols;
		NCThreadControl* threadControl;


		void run(std::atomic_bool& programHasEnded);

		void reset(std::string startNode = "main");

		void readInBasicSymbols(std::ifstream& file);
		void readInBasicSymbols(const std::string& fileDestination);

		void writeOutBasicSymbols(std::ofstream& file);
		void writeOutBasicSymbols(const std::string& fileDestination);

		NodeCallProgram(const NCNodeLayout& layout, std::string firstNode = "main");

		~NodeCallProgram();

		static NCReturnValue runCommands(
			NCRuntimeList& commands,
			SymbolTable& table,
			NCThreadControl* threadControl
		);
	};

}

#endif