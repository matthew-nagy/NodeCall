#include "parser.hpp"



namespace nc {
	namespace comp {


		const token& token_stream::get() {
			if (empty())
				throw(new UNEXPECTED_END_OF_TOKEN_LIST);
			const token& t = tList[currentToken];
			currentToken++;
			return t;
		}
		const token& token_stream::peek()const {
			if (empty())
				throw(new UNEXPECTED_END_OF_TOKEN_LIST);
			return tList[currentToken];
		}
		void token_stream::replace() {
			currentToken -= 1;
		}

		bool token_stream::empty()const {
			return currentToken == tList.size();
		}

		size_t token_stream::getNextIndex()const {
			return currentToken;
		}

		void token_stream::emplaceUntilReturnToScope(std::vector<token>& into, unsigned depth) {
			do {
				if (peek().type == open_bracket)
					depth += 1;
				else if (peek().type == close_bracket)
					depth -= 1;
				into.emplace_back(get());
			} while (depth != 0);
		}
		std::vector<token> token_stream::getLine() {
			std::vector<token> tokens;

			while (peek().type != line_terminator)
				tokens.emplace_back(get());
			//One last one for the semicolon
			tokens.emplace_back(get());

			return tokens;
		}

		token_stream::token_stream(const std::vector<token>& tokens) :
			tList(tokens),
			currentToken(0)
		{}





		void argument_node::printContents(const std::string& priorPrint)const {
			if (std::holds_alternative<call_node>(val)) {
				call_node& cn = std::get<call_node>(val);
				cn.printContents(priorPrint);
			}
			else
				printf("%s'%s'\n", priorPrint.c_str(), valueToken.representation.c_str());
		}

		argument_node& argument_node::operator=(const value& v) {
			val = v;
			return *this;
		}
		argument_node& argument_node::operator=(const call_node& cn) {
			val = cn;
			return *this;
		}
		argument_node& argument_node::operator=(const std::any& a) {
			val = a;
			return *this;
		}

		argument argument_node::getArgument()const {
			if (std::holds_alternative<value>(val))
				return argument(std::get<value>(val));
			else if (std::holds_alternative<std::any>(val))
				return argument(new std::any(std::get<std::any>(val)));
			else if (std::holds_alternative<call_node>(val))
				return argument(std::get<call_node>(val).getQueary());
			else
				throw(new ARGUMENT_NOT_SET);
		}

		Queary* call_node::getQueary()const {
			if (!std::holds_alternative<QuearyFunction>(func))
				throw(new OPERATION_CANNOT_BE_USED_AS_QUEARY);
			argument_list argList;
			for (auto& an : arguments)
				argList.emplace_back(an.getArgument());
			return new Queary(std::get<QuearyFunction>(func), std::move(argList), lineNum);
		}
		void call_node::printContents(const std::string& priorPrint)const {
			printf("%s'%s'\n", priorPrint.c_str(), functionToken.representation.c_str());
			for (size_t i = 0; i < arguments.size(); i++)
				arguments[i].printContents(priorPrint + "\t");
		}
		void call_node::getOperationFrom(node& commands)const {
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


		ParserPack::ParserPack(std::vector<token>& tokens, const std::shared_ptr<compilation_environment>& compEnv) :
			compEnv(compEnv),
			tokens(tokens)
		{}

		
		argument_node getVariableArgNode(const token& t, compilation_environment& env);
		argument_node getConstantArgNode(const token& t, compilation_environment& env);
		argument_node getQuearyArgNode(token_stream& ts, compilation_environment& env);
		argument_node getBracketedArgNode(token_stream& ts, compilation_environment& env);
		argument_node getArgumentNode(token_stream& ts, compilation_environment& env);


		void emplaceArguments(token_stream& ts, compilation_environment& env, std::vector<argument_node>& arguments) {
			if (ts.get().type != open_bracket)throw(new EXPECTED_OPEN_BRACKET);
			if (ts.peek().type == close_bracket) {
				ts.get();
				return;
			}
			bool expectToBeOver = false;
			while (ts.peek().type != close_bracket && !expectToBeOver) {
				arguments.emplace_back(getArgumentNode(ts, env));
				if (ts.peek().type != argument_seperator)
					expectToBeOver = true;
				else
					ts.get();
			}
			if (!(ts.get().type == close_bracket && expectToBeOver))
				throw(new EXPECTED_ARGUMENT);
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
				else if (t.representation == "false")argNode = std::make_any<bool>(false);
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

		argument_node getQuearyArgNode(token_stream& ts, compilation_environment& env) {
			call_node cn;
			cn.func = env.getQueary(ts.peek().representation);
			cn.lineNum = ts.peek().lineNumber;
			cn.functionToken = ts.get();

			emplaceArguments(ts, env, cn.arguments);

			argument_node argNode;
			argNode.val = cn;
			argNode.valueToken = cn.functionToken;
			return argNode;
		}

		argument_node getBracketedArgNode(token_stream& ts, compilation_environment& env) {
			if (ts.get().type != open_bracket)
				throw(new EXPECTED_OPEN_BRACKET);
			argument_node left = getArgumentNode(ts, env);
			if (ts.peek().type != close_bracket) {
				if (ts.peek().type != queary) {
					call_node cn;
					cn.lineNum = ts.peek().lineNumber;
					cn.functionToken = ts.peek();
					cn.func = env.getQueary(ts.get().representation);
					cn.arguments.emplace_back(left);
					cn.arguments.emplace_back(getArgumentNode(ts, env));
					if (ts.get().type != close_bracket)
						throw(new EXPECTED_CLOSE_BRACKET);

					argument_node bArg;
					bArg.val = cn;
					bArg.valueToken = cn.functionToken;
					return bArg;
				}
			}
			return left;
		}
		argument_node getArgumentNode(token_stream& ts, compilation_environment& env) {
			if (ts.peek().type == queary)
				return getQuearyArgNode(ts, env);
			else if (ts.peek().type == variable)
				return getVariableArgNode(ts.get(), env);
			else if (ts.peek().type == open_bracket)
				return getBracketedArgNode(ts, env);
			else
				return getConstantArgNode(ts.get(), env);
		}

		call_node getEndIf() {
			call_node cn;
			cn.func = stlib::op::break_from_if;
			cn.lineNum = -1;
			return cn;
		}
		call_node getEndWhile(std::shared_ptr<stlib::while_pack> whilePack) {
			call_node ew;
			ew.func = stlib::op::while_node_bottom;
			ew.arguments.emplace_back();
			ew.arguments.back() = std::make_any< std::shared_ptr<stlib::while_pack>>(whilePack);
			ew.lineNum = -1;
			return ew;
		}
		call_node getWhileBreak() {
			call_node wb;
			wb.func = stlib::op::break_from_while;
			wb.lineNum = -1;
			return wb;
		}


		void emplaceIfElseStreamNodes(ParserPack& pack, std::vector<call_node>& cn) {
			bool inIf = true;
			cn.emplace_back();
			call_node& node = cn.back();
			node.func = stlib::op::conditional_if;
			node.lineNum = pack.tokens.peek().lineNumber;
			node.functionToken = pack.tokens.peek();
			node.arguments.emplace_back();
			std::shared_ptr<stlib::if_pack> ip = std::make_shared<stlib::if_pack>();
			node.arguments.back() = std::make_any<std::shared_ptr<stlib::if_pack>>(ip);

			//Used to track how many nodes are spawned for this. They all need unique names, or the node mappings become messed up
			int deep = 0;
			while (inIf) {
				node_index conditionalIndex = pack.nextIndex;
				pack.tokens.get();
				if (!ip->hasFinalElse) {
					if (pack.tokens.get().type != open_bracket)
						throw(new EXPECTED_OPEN_BRACKET);
					ip->triggers.emplace_back(getArgumentNode(pack.tokens, *pack.compEnv).getArgument());
					if (pack.tokens.get().type != close_bracket)
						throw(new EXPECTED_CLOSE_BRACKET);
					ip->resultantNodes.emplace_back(conditionalIndex);
				}
				else
					ip->elseNode = conditionalIndex;

				pack.nodeMappings["#l" + std::to_string(node.lineNum) + "_icb" + std::to_string(deep)] = conditionalIndex;
				pack.nextIndex += 1;

				auto conditionalNode = getNode(pack);
				//Leave if node afterwards
				conditionalNode.emplace_back(getEndIf());
				pack.syntaxTree.emplace_back(conditionalNode);

				if (pack.tokens.peek().representation == "else") {
					if (ip->hasFinalElse)
						throw(new ELSE_HAS_NO_PARENT);
					else
						ip->hasFinalElse = true;
				}
				else if (pack.tokens.peek().representation != "elif") {
					inIf = false;
				}
				deep++;
			}
		}
		void emplaceWhileNodes(ParserPack& pack, std::vector<call_node>& cn) {
			cn.emplace_back();
			call_node& node = cn.back();
			node.func = stlib::op::while_loop;
			node.functionToken = pack.tokens.peek();
			node.lineNum = pack.tokens.get().lineNumber;
			node.arguments.emplace_back();
			std::shared_ptr<stlib::while_pack> wp = std::make_shared<stlib::while_pack>();
			node.arguments.back() = std::make_any<std::shared_ptr<stlib::while_pack>>(wp);

			if (pack.tokens.get().type != open_bracket)
				throw(new EXPECTED_OPEN_BRACKET);
			argument_node whileArg = getArgumentNode(pack.tokens, *pack.compEnv);
			wp->trigger = whileArg.getArgument();
			if (pack.tokens.get().type != close_bracket)
				throw(new EXPECTED_CLOSE_BRACKET);


			std::vector<call_node> whileNode = getNode(pack);
			whileNode.emplace_back(getEndWhile(wp));
			whileNode.emplace_back(getWhileBreak());


			wp->node = pack.nextIndex;
			pack.nodeMappings["#l" + std::to_string(node.lineNum) + "_wcb"] = pack.nextIndex;
			pack.nextIndex++;
			pack.syntaxTree.emplace_back(std::move(whileNode));
		}
		void emplaceAssignmentCallNode(ParserPack& pack, std::vector<call_node>& cn) {
			argument_node left = getArgumentNode(pack.tokens, *pack.compEnv);
			if (pack.tokens.peek().representation != "=")
				throw(new EXPECTED_ASSIGNMENT_OPERATOR);
			cn.emplace_back();
			cn.back().func = stlib::op::assign;
			cn.back().functionToken = pack.tokens.peek();
			cn.back().lineNum = pack.tokens.get().lineNumber;
			cn.back().arguments.emplace_back(left);
			cn.back().arguments.emplace_back(getArgumentNode(pack.tokens, *pack.compEnv));

			if (pack.tokens.get().type != line_terminator)
				throw(new EXPECTED_LINE_TERMINATOR);
		}
		void emplaceGenericCallNode(ParserPack& pack, std::vector<call_node>& cn) {
			OperationFunction func = pack.compEnv->getOpereration(pack.tokens.peek().representation);

			cn.emplace_back();
			cn.back().func = func;
			cn.back().functionToken = pack.tokens.peek();
			cn.back().lineNum = pack.tokens.get().lineNumber;

			emplaceArguments(pack.tokens, *pack.compEnv, cn.back().arguments);

			if (pack.tokens.get().type != line_terminator)
				throw(new EXPECTED_EOL);
		}
		void emplaceCallNode(ParserPack& pack, std::vector<call_node>& cn) {
			if (pack.tokens.peek().type == variable) {
				emplaceAssignmentCallNode(pack, cn);
				return;
			}
			OperationFunction func = pack.compEnv->getOpereration(pack.tokens.peek().representation);
			if (func.target<OperationFunction>() == stlib::op::conditional_if.target<OperationFunction>()) {
				emplaceIfElseStreamNodes(pack, cn);
				return;
			}
			else if (func.target<OperationFunction>() == stlib::op::while_loop.target<OperationFunction>()) {
				emplaceWhileNodes(pack, cn);
				return;
			}
			else {
				emplaceGenericCallNode(pack, cn);
			}
		}
		//From open node to end node
		std::vector<call_node> getNode(ParserPack& pack) {
			std::vector<call_node> cn;
			if (pack.tokens.get().type != open_node)
				throw(new EXPECTED_OPEN_NODE);
			while (pack.tokens.peek().type != close_node)
				emplaceCallNode(pack, cn);
			pack.tokens.get();
			return cn;
		}

		ParserPack parseTokens(std::vector<token>& tokens, const std::shared_ptr<compilation_environment>& compEnv) {
#ifdef NC_COMPILE_PRINTING
			printf("IN PARSE TOKENS\n");
#endif
			ParserPack pack(tokens, compEnv);
			while (pack.tokens.peek().type != null_token) {
				if (pack.tokens.peek().type != variable)
					throw(new EXPECTED_NODE);
				const token& t = pack.tokens.get();
				pack.syntaxTree.emplace_back(getNode(pack));

				//Set the rep up after, so any conditional nodes inside don't clash
				pack.nodeMappings[t.representation] = pack.nextIndex;
				*compEnv->getVariable(t.representation) = pack.nextIndex;
				pack.nextIndex += 1;
			}

			return pack;
		}

		std::unique_ptr<program> compile(const ParserPack& pack, bool printout) {
			if(printout)
				printf("Compiling\n");

			std::unique_ptr<program> p = std::make_unique<program>();
			p->nodeMappings = pack.nodeMappings;
			p->ownedVariables = pack.compEnv->getNewVariables();

			if(printout)
				printf("Tree is:\n");

			for (size_t i = 0; i < pack.syntaxTree.size(); i++) {
				p->nodes.emplace_back();
				if (printout)
					printf(">");
				for (size_t j = 0; j < pack.syntaxTree[i].size(); j++) {
					pack.syntaxTree[i][j].getOperationFrom(p->nodes.back());
					if (printout)
						pack.syntaxTree[i][j].printContents("\t");
				}
			}
			return p;
		}
	}


	std::shared_ptr<comp::compilation_environment> createEnvironment(const std::vector<std::shared_ptr<additional_library>>& libraries) {
		std::shared_ptr<comp::compilation_environment> env = std::make_shared<comp::compilation_environment>();
		for (auto& lib : libraries)
			env->addLibrary(lib);
		return env;
	}

	std::shared_ptr<program> compileProgram(std::shared_ptr<comp::compilation_environment> environment, const std::vector<std::string>& sourceCode, bool printDetail) {
		comp::source programSource(sourceCode);
		auto tokens = comp::tokeniseSource(programSource, *environment);

		if(printDetail){
			printf("Tokens>>\n");
			for(auto& t : tokens)
				printf("%s\t%s\n", t.representation.c_str(), comp::tokenRep.find(t.type)->second.c_str());
		}

		comp::ParserPack parsePack = comp::parseTokens(tokens, environment);

		return comp::compile(parsePack, printDetail);
	}
	std::shared_ptr<program> compileProgram(std::shared_ptr<comp::compilation_environment> environment, const std::string& programPath, bool printDetail) {
		std::ifstream sourceFile;
		sourceFile.open(programPath, std::ios::in);
		std::vector<std::string> sourceCode;
		//Just a guess to start off with, but this will prevent the first few resize and copies
		sourceCode.reserve(20);
		std::string line;
		while (std::getline(sourceFile, line)) {
			sourceCode.emplace_back(std::move(line));
		}
		sourceFile.close();

		return compileProgram(environment, sourceCode, printDetail);
	}
}
