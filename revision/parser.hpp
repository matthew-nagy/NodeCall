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
	ERROR_MAKE(QUEARY_MUST_BE_CALLED);
	ERROR_MAKE(QUEARY_HAS_MISMATCHED_BRACKETS);
	ERROR_MAKE(MISSING_SEPERATOR_BETWEEN_ARGUMENTS);
	ERROR_MAKE(INVALID_BOOLEAN);
	ERROR_MAKE(INVALID_TOKEN_TYPE_FOR_ARG);
	ERROR_MAKE(INVALID_BINARY_DIGIT);
	ERROR_MAKE(INVALID_HEX_DIGIT);

	//A way of getting storing tokens for parsing
	class token_stream {
	public:

		const token& get() {
			if (empty())
				throw(new UNEXPECTED_END_OF_TOKEN_LIST);
			const token& t = tList[currentToken];
			currentToken++;
			return t;
		}
		const token& peek()const {
			if (empty())
				throw(new UNEXPECTED_END_OF_TOKEN_LIST);
			return tList[currentToken];
		}
		void replace() {
			currentToken -= 1;
		}

		bool empty()const {
			return currentToken == tList.size();
		}

		size_t getNextIndex()const {
			return currentToken;
		}

		void emplaceUntilReturnToScope(std::vector<token>& into, unsigned depth) {
			do{
				if (peek().type == open_bracket)
					depth += 1;
				else if (peek().type == close_bracket)
					depth -= 1;
				into.emplace_back(get());
			} while (depth != 0);
		}

		token_stream(const std::vector<token>& tokens):
			tList(tokens),
			currentToken(0)
		{}

	private:
		const std::vector<token>& tList;
		size_t currentToken;
	};


	struct argument_node;

	//A way of storing a function token's value, and its arguments
	struct call_node {
		std::variant<OperationFunction, QuearyFunction> func;
		std::vector<argument_node> arguments;
		unsigned lineNum = 0;

		Queary* getQueary()const;
		void getOperationFrom(node& commands);
	};

	//A way of storing an argument, be it constant, variable or a queary (via a call_node)
	struct argument_node {
		std::variant<value, std::any, call_node> val;

		argument_node& operator=(const value& v) {
			val = v;
			return *this;
		}
		argument_node& operator=(const call_node& cn) {
			val = cn;
			return *this;
		}
		argument_node& operator=(const std::any& a) {
			val = a;
			return *this;
		}

		argument getArgument()const {
			if (std::holds_alternative<value>(val))
				return argument(std::get<value>(val));
			else if (std::holds_alternative<std::any>(val))
				return argument(new std::any(std::get<std::any>(val)));
			else if (std::holds_alternative<call_node>(val))
				return argument(std::get<call_node>(val).getQueary());
			else
				throw(new ARGUMENT_NOT_SET);
		}
	};

	Queary* call_node::getQueary()const {
		if (!std::holds_alternative<QuearyFunction>(func))
			throw(new OPERATION_CANNOT_BE_USED_AS_QUEARY);
		argument_list argList;
		for (auto& an : arguments)
			argList.emplace_back(an.getArgument());
		return new Queary(std::get<QuearyFunction>(func), std::move(argList), lineNum);
	}
	void call_node::getOperationFrom(node& commands) {
		if (!std::holds_alternative<OperationFunction>(func))
			throw(new QUEARY_CANNOT_BE_USED_AS_OPERATION);
		argument_list argList;
		for (auto& an : arguments)
			argList.emplace_back(an.getArgument());

		commands.emplace_back(std::get<OperationFunction>(func), std::move(argList), lineNum);
	}





	/*
		Functions to get the integer value from a token. These tokens can be in binary, hex, or decimal
	*/
	int parseBinary(const std::string& s) {
		int val = 0;
		for (size_t i = 0; i < s.size(); i++) {
			if (s[i] == '1')
				val += 1;
			else if (s[i] != '0')
				throw(new INVALID_BINARY_DIGIT);
			val = val << 1;
		}
		return val;
	}
	int parseHex(const std::string& s) {
		int val = 0;
		for (size_t i = 0; i < s.size(); i++) {
			if (s[i] >= '0' && s[i] <= '9')
				val += s[i] - '9';
			else if (s[i] >= 'A' && s[i] <= 'F')
				val += 10 + (s[i] - 'A');
			else
				throw(new INVALID_HEX_DIGIT);
			val = val << 4;
		}
		return val;
	}
	int parseString(const std::string& s) {
		if (s.size() >= 2) {
			if (s[1] == 'x')
				return parseHex(s.substr(2));
			else if (s[2] == 'b')
				return parseBinary(s.substr(2));
		}
		return std::atoi(s.c_str());
	}



	/*
		Methods of getting argument nodes. So this covers constants, variables, and other quearies
	*/

	argument_node getVariableArgNode(const token& t, compilation_environment& env){
		argument_node argNode;
		argNode = env.getVariable(t.representation);
		return argNode;
	}
	argument_node getConstantArgNode(const token& t, compilation_environment& env){
		argument_node argNode;
		if (t.type == boolean) {
			if (t.representation == "true")argNode = std::make_any<bool>(true);
			else if (t.representation == "false")argNode = std::make_any<bool>(true);
			else throw(new INVALID_BOOLEAN);
		}
		else if (t.type == floating_point)
			argNode = std::make_any<float>(std::atof(t.representation.c_str()));
		else if (t.type == string)
			argNode = std::make_any<std::string>(t.representation);
		else if (t.type == integer)
			argNode = std::make_any<int>(parseString(t.representation));
		else
			throw(new INVALID_TOKEN_TYPE_FOR_ARG);

		return argNode;
	}



	argument_node getQuearyArgNode(const std::vector<token>& tokens, compilation_environment& env) {
		//A queary is stored in a call node
		call_node callNode;
		//Set up a stream for easier parsing, then record the line number for the queary
		token_stream tStream(tokens);
		callNode.lineNum = tStream.peek().lineNumber;

		//Check if the first token is actually* a queary or not
		if (tStream.peek().type != queary)
			throw(new TOKEN_CANNOT_BE_USED_AS_A_QUEARY);
		//And if it is, get its pointer
		callNode.func = env.getQueary(tStream.get().representation);
		if (tStream.get().type != open_bracket)
			throw(new QUEARY_MUST_BE_CALLED);

		bool endBraketHit = false;
		while (!tStream.empty()) {
			if (endBraketHit)
				throw(new QUEARY_HAS_MISMATCHED_BRACKETS);
			//Get the next token, and 
			token nextToken = tStream.get();
			if (nextToken.type == queary) {
				std::vector<token> subQuearyTokens = { nextToken };
				if (tStream.peek().type != open_bracket)
					throw(new QUEARY_MUST_BE_CALLED);
				tStream.emplaceUntilReturnToScope(subQuearyTokens, 0);
				callNode.arguments.emplace_back(getQuearyArgNode(subQuearyTokens, env));
			}
			else if (nextToken.type == variable) {
				callNode.arguments.emplace_back(getVariableArgNode(nextToken, env));
			}
			else if (nextToken.type == close_bracket)//This should be the last one
				endBraketHit = true;
			else {
				callNode.arguments.emplace_back(getConstantArgNode(nextToken, env));
			}

			if (!tStream.empty()) {
				if (tStream.peek().type != argument_seperator)
					throw(new MISSING_SEPERATOR_BETWEEN_ARGUMENTS);
				else
					tStream.get();
			}
		}

		argument_node argNode;
		argNode = callNode;

		return argNode;
	}

}}


#endif