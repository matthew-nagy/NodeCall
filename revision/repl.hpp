#pragma once
#ifndef NC_REPL_HPP
#define NC_REPL_HPP

#include "parser.hpp"

namespace nc {


	class REPL {
		friend class Runtime;
	public:

		void giveCommandPrompt(const std::string& line) {
			if (line.size() == 0)
				return;

			if (line[0] == '/' && line.size() > 1) {
				std::string request = line.substr(2);
				switch (line[1]) {
				case 'n':
					parseNodeRequest(request);
					break;
				case 'v':
					parseVariableRequest(request);
					break;
				case 'q':
					parseQuearyRequest(request);
					break;
				case 'f':
					parseFunctionRequest(request);
					break;
				default:
					connectedRuntime->printFunction("Unknown prompt request '" + line.substr(1, 1) + "'\n");
					break;
				}
			}
			else {
				evaluateCommands(line);
			}
		}

		REPL(const std::shared_ptr<Runtime>& targetRuntime, const std::shared_ptr<comp::compilation_environment>& compileEnvironment):
			connectedRuntime(targetRuntime),
			compEnv(compileEnvironment)
		{
			//Set aside a new node just for the repl
			connectedRuntime->currentProgram->nodeMappings[replNodeName] = connectedRuntime->currentProgram->nodes.size();
			connectedRuntime->currentProgram->nodes.emplace_back();
		}

	private:
		std::shared_ptr<Runtime> connectedRuntime;
		std::shared_ptr<comp::compilation_environment> compEnv;

		static const char metaCommand;
		static const std::string replNodeName;

		void parseNodeRequest(const std::string& request) {
			std::string availableMappings = ">Found Node Mappings\n";
			for (auto& [nName, _] : connectedRuntime->currentProgram->nodeMappings)
				if (nName.substr(0, request.size()) == request && nName[0] != '#')
					availableMappings += "\t" + nName + "\n";

			connectedRuntime->printFunction(availableMappings);
		}
		void parseVariableRequest(const std::string& request) {

		}
		void parseQuearyRequest(const std::string& request) {
		
		}
		void parseFunctionRequest(const std::string& request) {
		
		}

		void evaluateCommands(const std::string& commands) {
			comp::source pseudoSource(std::vector<std::string>{ "{", commands, "end();", "}"});
			std::vector<comp::token> resultingTokens = comp::tokeniseSource(pseudoSource, *compEnv);
			comp::ParserPack pack(resultingTokens, compEnv);
			
			auto tree = comp::getNode(pack);

			//Obtain the node set aside in the runtime for repl
			auto& replNode = connectedRuntime->currentProgram->nodes[connectedRuntime->currentProgram->nodeMappings[replNodeName]];// = commandNode;
			replNode.clear();
			
			//Compile the given commands into the repl
			for (auto& c : tree)
				c.getOperationFrom(replNode);

			connectedRuntime->enterProgramAt(replNodeName);
		}
	};

	const char REPL::metaCommand = '/';
	const std::string REPL::replNodeName = "#REPL_node";


}


#endif