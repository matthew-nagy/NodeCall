#include "repl.hpp"

namespace nc {

	void REPL::giveCommandPrompt(const std::string& line) {
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

	REPL::REPL(const std::shared_ptr<Runtime>& targetRuntime, const std::shared_ptr<comp::compilation_environment>& compileEnvironment) :
		connectedRuntime(targetRuntime),
		compEnv(compileEnvironment)
	{
		//Set aside a new node just for the repl
		connectedRuntime->currentProgram->nodeMappings[replNodeName] = connectedRuntime->currentProgram->nodes.size();
		connectedRuntime->currentProgram->nodes.emplace_back();
	}

	void REPL::parseNodeRequest(const std::string& request) {
		std::string availableMappings = ">Found Node Mappings\n";
		for (auto& [nName, _] : connectedRuntime->currentProgram->nodeMappings)
			if (nName.substr(0, request.size()) == request && nName[0] != '#')
				availableMappings += "\t" + nName + "\n";

		connectedRuntime->printFunction(availableMappings);
	}
	void REPL::parseVariableRequest(const std::string& request) {
		std::string availableMappings = ">Found Variables\n";
		auto mappings = compEnv->getVariables(request);
		for (auto& s : mappings)
			availableMappings += "\t" + s.first + "\n";

		connectedRuntime->printFunction(availableMappings);
	}
	void REPL::parseQuearyRequest(const std::string& request) {
		std::string availableMappings = ">Found Queary functions\n";
		auto mappings = compEnv->getQuearys(request);
		for (auto& s : mappings)
			availableMappings += "\t" + s.first + "\n";

		connectedRuntime->printFunction(availableMappings);
	}
	void REPL::parseFunctionRequest(const std::string& request) {
		std::string availableMappings = ">Found Operational Functions\n";
		auto mappings = compEnv->getOperations(request);
		for (auto& s : mappings)
			availableMappings += "\t" + s.first + "\n";

		connectedRuntime->printFunction(availableMappings);
	}

	void REPL::evaluateCommands(const std::string& commands) {
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

	const char REPL::metaCommand = '/';
	const std::string REPL::replNodeName = "#REPL_node";


}