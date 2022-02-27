#pragma once
#ifndef NC_REPL_HPP
#define NC_REPL_HPP

#include "parser.hpp"

namespace nc {


	class REPL {
		friend class Runtime;
	public:

		void giveCommandPrompt(const std::string& line);

		REPL(const std::shared_ptr<Runtime>& targetRuntime, const std::shared_ptr<comp::compilation_environment>& compileEnvironment);

	private:
		std::shared_ptr<Runtime> connectedRuntime;
		std::shared_ptr<comp::compilation_environment> compEnv;

		static const char metaCommand;
		static const std::string replNodeName;

		void parseNodeRequest(const std::string& request);
		void parseVariableRequest(const std::string& request);
		void parseQuearyRequest(const std::string& request);
		void parseFunctionRequest(const std::string& request);

		void evaluateCommands(const std::string& commands);
	};

	const char REPL::metaCommand = '/';
	const std::string REPL::replNodeName = "#REPL_node";


}


#endif