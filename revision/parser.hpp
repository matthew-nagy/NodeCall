#ifndef NC_PARSER_HPP
#define NC_PARSER_HPP

#include "tokeniser.hpp"
#include <variant>

namespace nc {	namespace comp {
	
	ERROR_MAKE(UNEXPECTED_END_OF_TOKEN_LIST);
	ERROR_MAKE(ARGUMENT_NOT_SET);
	ERROR_MAKE(QUEARY_CANNOT_BE_USED_AS_OPERATION);
	ERROR_MAKE(OPERATION_CANNOT_BE_USED_AS_QUEARY);
	ERROR_MAKE(TOKEN_CANNOT_BE_USED_AS_A_QUEARY);
	ERROR_MAKE(TOKEN_CANNOT_BE_USED_AS_ARGUMENT);
	ERROR_MAKE(MISSING_SEPERATOR_BETWEEN_ARGUMENTS);
	ERROR_MAKE(INVALID_BOOLEAN);
	ERROR_MAKE(INVALID_TOKEN_TYPE_FOR_ARG);
	ERROR_MAKE(INVALID_BINARY_DIGIT);
	ERROR_MAKE(INVALID_HEX_DIGIT);
	ERROR_MAKE(NON_GENERIC_OPERATION);

	struct argument_node;
	struct call_node;

	//A way of getting storing tokens for parsing
	class token_stream {
	public:

		const token& get();
		const token& peek()const;
		void replace();

		bool empty()const;

		size_t getNextIndex()const;

		void emplaceUntilReturnToScope(std::vector<token>& into, unsigned depth);
		std::vector<token> getLine();

		token_stream(const std::vector<token>& tokens);

	private:
		const std::vector<token>& tList;
		size_t currentToken;
	};


	//A way of storing a function token's value, and its arguments
	struct call_node {
		std::variant<OperationFunction, QuearyFunction> func;
		token functionToken;
		std::vector<argument_node> arguments;
		unsigned lineNum = 0;

		void printContents(const std::string& priorPrint)const;

		Queary* getQueary()const;
		void getOperationFrom(node& commands)const;
	};

	//A way of storing an argument, be it constant, variable or a queary (via a call_node)
	struct argument_node {
		mutable std::variant<value, std::any, call_node> val;
		token valueToken;

		void printContents(const std::string& priorPrint)const;

		argument_node& operator=(const value& v);
		argument_node& operator=(const call_node& cn);
		argument_node& operator=(const std::any& a);

		argument getArgument()const;
	};


	ERROR_MAKE(EXPECTED_NODE);
	ERROR_MAKE(EXPECTED_OPEN_NODE);
	ERROR_MAKE(EXPECTED_OPERATION);
	ERROR_MAKE(EXPECTED_OPEN_BRACKET);
	ERROR_MAKE(EXPECTED_CLOSE_BRACKET);
	ERROR_MAKE(EXPECTED_ARGUMENT);
	ERROR_MAKE(EXPECTED_ARGUMENT_SEPERATOR);
	ERROR_MAKE(EXPECTED_ASSIGNMENT_OPERATOR);
	ERROR_MAKE(EXPECTED_LINE_TERMINATOR);
	ERROR_MAKE(EXPECTED_QUEARY);
	ERROR_MAKE(EXPECTED_EOL);
	ERROR_MAKE(USED_EXPIRED_NODE_PARSE);
	ERROR_MAKE(ELSE_HAS_NO_PARENT);

	struct ParserPack {
		std::unordered_map<std::string, node_index> nodeMappings;
		node_index nextIndex = 0;
		std::shared_ptr<compilation_environment> compEnv;
		std::vector<std::vector<call_node>> syntaxTree;
		token_stream tokens;

		ParserPack(std::vector<token>& tokens, const std::shared_ptr<compilation_environment>& compEnv);
	};


	//From open node to end node
	std::vector<call_node> getNode(ParserPack& pack);
	
	ParserPack parseTokens(std::vector<token>& tokens, const std::shared_ptr<compilation_environment>& compEnv);

	std::unique_ptr<program> compile(const ParserPack& pack, bool printout = false);


	std::shared_ptr<comp::compilation_environment> createEnvironment(const std::vector<std::shared_ptr<additional_library>>& libraries);

	std::shared_ptr<program> compileProgram(std::shared_ptr<comp::compilation_environment> environment, const std::vector<std::string>& sourceCode);
	std::shared_ptr<program> compileProgram(std::shared_ptr<comp::compilation_environment> environment, const std::string& programPath);

}}


#endif