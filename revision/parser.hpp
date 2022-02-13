#ifndef NC_PARSER_HPP
#define NC_PARSER_HPP

#include "tokeniser.hpp"
#include <variant>

/*
	*
	*The parse is scathingly bad. It loops over an entire section, copies the tokens, then runs the next function on those tokens. This could result
	* in it all being copied up to 3 or 4 times. NodeCall 1's parser was much better in its inline recursive nature.
	* Honestly I could have copied and pasted it with some minor changes. Why did I not do that. At some point this should be done
	* 
*/

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
		std::vector<token> getLine() {
			std::vector<token> tokens;

			while (peek().type != line_terminator)
				tokens.emplace_back(get());
			//One last one for the semicolon
			tokens.emplace_back(get());

			return tokens;
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
		token functionToken;
		std::vector<argument_node> arguments;
		unsigned lineNum = 0;

		void printContents(const std::string& priorPrint);

		Queary* getQueary()const;
		void getOperationFrom(node& commands);
	};

	//A way of storing an argument, be it constant, variable or a queary (via a call_node)
	struct argument_node {
		std::variant<value, std::any, call_node> val;
		token valueToken;

		void printContents(const std::string& priorPrint) {
			if (std::holds_alternative<call_node>(val)) {
				call_node& cn = std::get<call_node>(val);
				cn.printContents(priorPrint);
			}
			else
				printf("%s'%s'\n", priorPrint.c_str(), valueToken.representation.c_str());
		}

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
	void call_node::printContents(const std::string& priorPrint) {
		printf("%s'%s'\n", priorPrint.c_str(), functionToken.representation.c_str());
		for (size_t i = 0; i < arguments.size(); i++)
			arguments[i].printContents(priorPrint + "\t");
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

	argument_node getVariableArgNode(const token& t, compilation_environment& env) {
		argument_node argNode;
		argNode = env.getVariable(t.representation);
		argNode.valueToken = t;
		return argNode;
	}
	argument_node getConstantArgNode(const token& t, compilation_environment& env) {
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

		argNode.valueToken = t;
		return argNode;
	}


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

	class parser {
	public:

		void giveToken(const token& t) {
			if (t.type == null_token)
				return;

			switch (currentState) {
			case look_for_node:
				if (t.type != variable)
					throw(new EXPECTED_NODE);
				//You have found a node, set its value
				(*compEnv->getVariable(t.representation)) = nextIndex;
				nodeMappings[t.representation] = nextIndex;
				nextIndex += 1;
				callNodes.emplace_back();
				currentState = opening_node;
				break;
			case opening_node:
				if (t.type != open_node)
					throw(new EXPECTED_OPEN_NODE);
				currentState = look_for_operation;
				break;
			case look_for_operation:
				if (t.type == close_node) {
					currentState = look_for_node;
					break;
				}
				else {
					callNodes.back().emplace_back();
					currentCallNode.push(&callNodes.back().back());

					//function call
					if (t.type == operation) {
						currentCallNode.top()->functionToken = t;
						currentCallNode.top()->func = compEnv->getOpereration(t.representation);
						currentState = opening_operation;
					}
					//Assignment of some sort
					else if (t.type == variable) {
						currentCallNode.top()->func = stlib::op::assign;
						printProgram();
						argNodes.emplace(&currentCallNode.top()->arguments);
						argNodes.top()->emplace_back();
						argNodes.top()->back() = compEnv->getVariable(t.representation);
						argNodes.top()->back().valueToken = t;
						currentState = starting_assignment;
						printProgram();
					}
					else {
						throw(new EXPECTED_OPERATION);
					}
				}
				break;
			case opening_operation:
				if (t.type != open_bracket)
					throw(new EXPECTED_OPEN_BRACKET);
				currentState = look_for_argument;
				argNodes.push(&currentCallNode.top()->arguments);
				break;
			case look_for_argument:
				switch (t.type) {
				case variable:
					argNodes.top()->emplace_back(getVariableArgNode(t, *compEnv));
					currentState = looking_for_seperator;
					break;
				case queary:
					argNodes.top()->emplace_back();
					argNodes.top()->back().val = call_node();
					currentCallNode.emplace(&std::get<call_node>(argNodes.top()->back().val));
					currentCallNode.top()->func = compEnv->getQueary(t.representation);
					currentCallNode.top()->functionToken = t;
					argNodes.emplace(&currentCallNode.top()->arguments);
					currentState = opening_queary;
					quearyDepth += 1;
					break;
				case close_bracket://Let the seperator handle this
					exitFunction();
					break;
				default:
					argNodes.top()->emplace_back(getConstantArgNode(t, *compEnv));
					currentState = looking_for_seperator;
				}
				break;
			case opening_queary:
				if (t.type != open_bracket)
					throw(new EXPECTED_OPEN_BRACKET);
				currentState = look_for_argument;
				break;
			case looking_for_seperator:
				if (t.type == close_bracket) {
					//We aren't pushing to these arguments anymore
					exitFunction();
					break;
				}
				if (t.type != argument_seperator)
					throw(new EXPECTED_ARGUMENT_SEPERATOR);
				currentState = look_for_argument;
				break;
			case starting_assignment:
				if (t.type != assignment)
					throw(new EXPECTED_ASSIGNMENT_OPERATOR);
				currentState = assignment_r_arg;
				currentCallNode.top()->functionToken = t;
				break;
			case assignment_r_arg:
				if (t.type == open_bracket) {
					openBrackets(false);
				}
				else {
					argNodes.top()->emplace_back();
					if (t.type == variable) {
						argNodes.top()->back() = compEnv->getVariable(t.representation);
						argNodes.top()->back().valueToken = t;
					}
					else
						argNodes.top()->back() = getConstantArgNode(t, *compEnv);
					currentState = possible_end_assignment_or_queary;
				}
				break;
			case left_arg:
				if (t.type == open_bracket) {
					openBrackets(true);
				}
				else {
					argNodes.top()->emplace_back();
					if (t.type == variable) {
						argNodes.top()->back() = compEnv->getVariable(t.representation);
						argNodes.top()->back().valueToken = t;
					}
					else
						argNodes.top()->back() = getConstantArgNode(t, *compEnv);
					currentState = middleOperator;
				}
				break;
			case possible_end_assignment_or_queary:
				if (t.type == line_terminator)
					currentState = closing_operation;
				else if (_inlineable_queary_type.count(t.type) > 0)
					currentState = middleOperator;
				else
					throw(new EXPECTED_EOL);
			case middleOperator:
				if (_inlineable_queary_type.count(t.type) == 0)
					throw(new EXPECTED_QUEARY);
				currentCallNode.top()->func = compEnv->getQueary(t.representation);
				currentCallNode.top()->functionToken = t;
				currentState = right_arg;
				break;
			case right_arg:
				if (t.type == open_bracket) {
					openBrackets(false);
				}
				else{
					argNodes.top()->emplace_back();
					if (t.type == variable) {
						argNodes.top()->back() = compEnv->getVariable(t.representation);
						argNodes.top()->back().valueToken = t;
					}
					else
						argNodes.top()->back() = getConstantArgNode(t, *compEnv);
					currentState = closing_operation;
				}
				break;
			case closing_operation:
				if (bracketDepth > 0) {
					if (t.type != close_bracket)
						throw(new EXPECTED_CLOSE_BRACKET);
					else {
						bracketDepth -= 1;
						printf("close bracket (%u)\n", bracketDepth);
						argNodes.pop();
						currentCallNode.pop();
						if (bracketsWereOnLeft.top())
							currentState = middleOperator;
						bracketsWereOnLeft.pop();
					}
				}
				else {
					if (t.type != line_terminator)
						throw(new EXPECTED_LINE_TERMINATOR);
					emptyStacks();
					currentState = look_for_operation;
				}
			}
		}

		void printState() {
			printf("%s\n", statePrintValues.find(currentState)->second.c_str());
		}

		void printProgram() {
			for (size_t i = 0; i < callNodes.size(); i++) {
				printf(">");
				for (size_t j = 0; j < callNodes[i].size(); j++)
					callNodes[i][j].printContents("\t");
			}
		}

		std::unique_ptr<program> getProgram() {
			std::unique_ptr<program> p = std::make_unique<program>();
			p->nodeMappings = nodeMappings;
			p->ownedVariables = compEnv->getNewVariables();
			for (size_t i = 0; i < callNodes.size(); i++) {
				p->nodes.emplace_back();
				for (size_t j = 0; j < callNodes[i].size(); j++) {
					callNodes[i][j].getOperationFrom(p->nodes.back());
				}
			}

			return p;
		}

		parser(compilation_environment* compEnv):
			compEnv(compEnv)
		{}

	private:
		enum State {
			look_for_node, opening_node, 
			look_for_operation, opening_operation, closing_operation,
			look_for_argument, looking_for_seperator, opening_queary,
			starting_assignment, assignment_r_arg, possible_end_assignment_or_queary,
			left_arg, middleOperator, right_arg
		};

		static const std::unordered_map<State, std::string> statePrintValues;

		State currentState = look_for_node;
		unsigned quearyDepth = 0;
		unsigned bracketDepth = 0;
		std::stack<bool> bracketsWereOnLeft;

		std::stack<std::vector<argument_node>*> argNodes;
		std::stack<call_node*> currentCallNode;
		std::vector<std::vector<call_node>> callNodes;
		std::unordered_map<std::string, node_index> nodeMappings;
		node_index nextIndex = 0;
		compilation_environment* compEnv;

		void emptyStacks() {
			while (bracketsWereOnLeft.size() > 0)
				bracketsWereOnLeft.pop();
			while (argNodes.size() > 0)
				argNodes.pop();
			while (currentCallNode.size() > 0)
				currentCallNode.pop();
		}

		void openBrackets(bool onLeft) {
			bracketDepth++;
			currentCallNode.top()->arguments.emplace_back();
			currentCallNode.top()->arguments.back() = call_node();
			currentCallNode.emplace(&std::get<call_node>(currentCallNode.top()->arguments.back().val));
			argNodes.emplace(&currentCallNode.top()->arguments);
			bracketsWereOnLeft.emplace(onLeft);
			currentState = left_arg;
			printf("Open bracket (%u)\n", bracketDepth);
		}

		void exitFunction() {
			argNodes.pop();
			if (quearyDepth == 0) {
				currentState = closing_operation;
			}
			else {
				quearyDepth -= 1;
				//Return up to either the next queary or the operation
				currentCallNode.pop();
				currentState = looking_for_seperator;
			}
		}
	};
	/*look_for_node, opening_node,
		look_for_operation, opening_operation, closing_operation,
		look_for_argument, looking_for_seperator, opening_queary,
		starting_assignment, left_arg, middleOperator, right_arg*/
	const std::unordered_map<parser::State, std::string> parser::statePrintValues = {
		{look_for_node, "looking for node"}, {opening_node, "opening a node"},
		{look_for_operation, "looking for operation"}, {opening_operation, "opening an operation"},
		{closing_operation, "closing out operation"}, {look_for_argument, "looking for an argument to op/q"},
		{looking_for_seperator, "looking for argument seperator"}, {opening_queary, "opening a queary"},
		{starting_assignment, "Looking for assignment operator"}, {assignment_r_arg, "First arg after assignment"},
		{possible_end_assignment_or_queary, "Assignment could end, or could have operator"}, {left_arg, "Looking for left operational argument"},
		{middleOperator, "Looking for a binary operator"}, {right_arg, "Looking for right operational argument"}
	};


	/*
		Methods of getting argument nodes. So this covers constants, variables, and other quearies
	*/
	/*

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


	call_node getGenericCallNode(const std::vector<token>& tokens, compilation_environment& env) {
		if(tokens.size() < 4 || tokens.back().type != line_terminator)
			throw(new NON_GENERIC_OPERATION);
		if (tokens[tokens.size() - 2].type != close_bracket)
			throw(new OPERATION_BRAKETS_NOT_CLOSED);
		if (tokens[1].type != open_bracket)
			throw(new OPERATION_MUST_BE_CALLED);

		call_node cnode;

		cnode.func = env.getOpereration(tokens.front().representation);
		std::vector<token> argTokens;
		for (size_t i = 2; i < tokens.size() - 1; i++) {
			bool openBraket = false;
			if (tokens[i].type == close_bracket) {
				openBraket = true;
				argTokens.emplace_back(tokens[i]);
			}
			if (tokens[i].type == argument_seperator || openBraket) {
				switch (argTokens[0].type) {
				case queary:
					cnode.arguments.emplace_back(getQuearyArgNode(argTokens, env));
					break;
				case variable:
					if (argTokens.size() > 1)
						throw(new EXPECTED_ARGUMENT_SEPERATOR);
					cnode.arguments.emplace_back(getVariableArgNode(argTokens.front(), env));
					break;
				default:
					if (argTokens.size() > 1)
						throw(new EXPECTED_ARGUMENT_SEPERATOR);
					cnode.arguments.emplace_back(getConstantArgNode(argTokens.front(), env));
				}
				argTokens.clear();
			}
			else {
				argTokens.emplace_back(tokens[i]);
			}
		}

		return cnode;
	}

	call_node getIfElifElseCallNode(token_stream& tokens, compilation_environment& env);
	call_node getWhileCallNode(token_stream& tokens, compilation_environment& env);
	call_node getDoWhileCallNode(token_stream& tokens, compilation_environment& env);

	call_node getNextCallNode(token_stream& tokens, compilation_environment& env) {
		const token& firstToken = tokens.peek();
		if (firstToken.type != operation)
			throw(new EXPECTED_OPERATOR);

		if (firstToken.representation == "if")
			return getIfElifElseCallNode(tokens, env);
		else if (firstToken.representation == "while")
			return getWhileCallNode(tokens, env);
		else if (firstToken.representation == "do")
			return getDoWhileCallNode(tokens, env);
		else
			return getGenericCallNode(tokens.getLine(), env);
	}
	std::vector<call_node> getNodeSyntaxTree(const std::vector<token>& tokens, compilation_environment& env) {
		token_stream ts(tokens);
		std::vector<call_node> calls;

		while (!ts.empty())
			calls.emplace_back(getNextCallNode(ts, env));

		return calls;
	}

	std::vector<std::vector<call_node>> getAbstractSyntaxTree(const std::vector<token>& tokens, compilation_environment& env) {
		std::vector<std::vector<call_node>> nodes;
		std::vector<token> nodeTokens;
		unsigned nodeDepth = 0;
		for (size_t i = 0; i < tokens.size(); i++) {
			nodeTokens.emplace_back(tokens[i]);
			if (tokens[i].type == open_node)
				nodeDepth++;
			else if (tokens[i].type == close_node) {
				nodeDepth -= 1;
				nodes.emplace_back(getNodeSyntaxTree(nodeTokens, env));
				nodeTokens.clear();
			}
		}

		if (nodeDepth != 0)
			throw(new UNEXPECTED_EOF);

		return nodes;
	}
	*/
}}


#endif