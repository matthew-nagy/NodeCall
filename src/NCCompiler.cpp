#include "../header/NCCompiler.h"

namespace nc {

	namespace cmpl {

		class Multi_String_Indexer {
		public:
			size_t getSize()const {
				return fullSize;
			}

			char operator[](size_t index)const {
				size_t stringIndex = 0;
				while (index >= strings[stringIndex].size()) {
					index -= strings[stringIndex].size();
					stringIndex += 1;
				}
				return strings[stringIndex][index];
			}

			Multi_String_Indexer(const std::vector<std::string>& strings) :
				strings(strings)
			{
				fullSize = 0;
				for (std::string s : strings)
					fullSize += s.size();
			}

		private:
			const std::vector<std::string>& strings;
			size_t fullSize;
		};

		bool isWhitespace(char c) {
			return c == ' ' || c == '\t' || c == '\n';
		}

		bool allWhitespace(const std::string& s) {
			bool sIsAllWhitespace = true;
			for (auto& c : s)
				if (isWhitespace(c) == false)
					sIsAllWhitespace = false;
			return sIsAllWhitespace;
		}

		bool isComment(const std::string& s) {
			size_t index = 0;
			//Boi ain't this just a funky for loop -_-
			while (index < s.size()) {
				if (s[index] == '#')
					return true;
				index++;
			}
			return false;
		}

		void clearBlankLinesFromSource(std::vector<std::string>& source) {
			for (size_t i = 0; i < source.size(); i++) {
				if (allWhitespace(source[i]) || source[i] == "" || isComment(source[i])) {
					source.erase(source.begin() + i);
					i--;
				}
			}
		}

		void stripWhitespaceFromArgs(std::string& arg) {
			while (isWhitespace(arg[arg.size() - 1]))
				arg.pop_back();
			while (isWhitespace(arg[0]))
				arg.erase(arg.begin());
		}
		void stripWhitespaceFromArgs(std::vector<std::string>& args) {
			for (std::string& arg : args) {
				while (isWhitespace(arg[arg.size() - 1]))
					arg.pop_back();
				while (isWhitespace(arg[0]))
					arg.erase(arg.begin());
			}
		}

		bool hasBracketsIn(const std::string& s) {
			for (auto& c : s)
				if (c == '(')
					return true;
			return false;
		}

		int getHex(const std::string& s) {
			int ret = 0;
			for (size_t i = 2; i < s.size(); i++) {
				ret *= 16;
				if (s[i] >= '0' && s[i] <= '9')
					ret += s[i] - '0';
				else if (s[i] >= 'A' && s[i] <= 'F')
					ret += 10 + s[i] - 'A';
				else
					throw new exception::InvalidNumeric(s);
			}
			return ret;
		}

		int getBinary(const std::string& s) {
			int ret = 0;
			for (size_t i = 2; i < s.size(); i++) {
				ret = ret << 1;
				if (s[i] == '1')
					ret += 1;
				else if (s[i] != '0')
					throw new exception::InvalidNumeric(s);
			}
			return ret;
		}

		int get0StartedIntValue(const std::string& s) {
			if (s.size() == 1)
				return 0;
			if (s[1] == 'x')
				return getHex(s);
			if (s[1] == 'b')
				return getBinary(s);
			throw new exception::InvalidNumeric(s);
			return 0;
		}

		NCArgumentList getNCArgumentList(const std::vector<std::string>& arguments);
		NCRuntimeList getNCNode(const std::vector<std::string>& nodeSource);

		//Returns the string contained between opener and ender of the same level. Assumes it starts 1 past opener, and incriments index past ender
		std::string getContainedString(const Multi_String_Indexer& line, size_t& index, char opener, char ender) {
			unsigned levelsIn = 1;
			std::string toret = "";

			while (levelsIn > 0) {
				char c = line[index];

				index++;
				if (c == opener)
					levelsIn++;
				else if (c == ender)
					levelsIn--;
				toret += c;
			}

			toret.pop_back();//Get rid of trailing ')'

			return toret;
		}

		std::vector<std::string> splitOnLevel(const std::string& line, char opener, char ender, char splitter) {
			std::vector<std::string> ret;
			std::string current = "";
			short levelOn = 0;
			size_t index = 0;
			bool inString = false;

			while (index != line.size()) {
				char check = line[index];
				index++;

				if (check == opener)
					levelOn++;
				else if (check == ender)
					levelOn--;

				if (check == '\"') {
					if (inString)
						inString = false;
					else
						inString = true;
				}

				if (levelOn == 0 && check == splitter && inString == false) {
					ret.push_back(current);
					current = "";
				}
				else
					current += check;


			}

			if (current != "")
				ret.push_back(current);

			return ret;
		}

		struct ParsedFunctionCall {
			std::string command;
			std::vector<std::string> arguments;
		};

		ParsedFunctionCall getFunctionCall(const Multi_String_Indexer& s, size_t& index) {
			std::string commandName = "";
			bool haveCommand = false;
			char c;
			while (!haveCommand) {
				c = s[index];

				index++;

				if (c == '(')
					haveCommand = true;
				else
					commandName += c;
			}
			size_t ii = index;
			auto arguments = splitOnLevel(getContainedString(s, index, '(', ')'), '(', ')', ',');
			stripWhitespaceFromArgs(arguments);
			stripWhitespaceFromArgs(commandName);

			ParsedFunctionCall ret;
			ret.command = commandName;
			ret.arguments = arguments;
			return ret;
		}

		NCInlineFunctionReference* getNCInlineFunction(const std::string& arg) {
			auto holder = std::vector<std::string>{ arg };
			Multi_String_Indexer msi(holder);
			size_t index = 0;

			auto functionCall = getFunctionCall(msi, index);

			return new NCInlineFunctionReference(reserved::inline_function_names[functionCall.command], getNCArgumentList(functionCall.arguments), "Inline return expression from '" + arg + "'");
		}

		NCObjectReference* getNCStringArg(const std::string& arg) {
			size_t index = 1;//first is '"'
			char currentChar, lastChar;
			std::string hardCodedReference = "";
			currentChar = lastChar = '\0';
			bool ended = false;
			while (index != arg.size() && !ended) {
				lastChar = currentChar;
				currentChar = arg[index];
				index++;

				if (currentChar == '"' && lastChar != '\\')
					ended = true;
				else
					hardCodedReference += currentChar;
			}

			if (!(ended && index == arg.size()))
				throw new exception::UnclosedStringOnCompilation(arg);

			return new NCSetObjectReference(new NCObject(makeNCObject<std::string>(hardCodedReference)));
		}
		//Could add hex
		NCObjectReference* getNCNumericArg(const std::string& arg) {
			bool hasPoint = false;
			bool hasBaseMarker = false;
			for (size_t i = 0; i < arg.size(); i++) {
				char c = arg[i];
				if (c == '-' && i == 0 && arg.size() > 1)
					continue;
				else if (c < '0' || c > '9') {
					if (c != '.' && c != 'b' && c != 'x')
						throw new exception::InvalidNumeric(arg);
					else if (hasPoint || hasBaseMarker)//Can only have one of either of the two, and ONE at that
						throw new exception::InvalidNumeric(arg);
					else if (c == '.')
						hasPoint = true;
					else
						hasBaseMarker = true;
				}
			}
			if (hasPoint)
				return new NCSetObjectReference(new NCObject(makeNCObject<float>(std::atof(arg.c_str()))));
			else {
				int val;
				if (arg[0] == '0')
					val = get0StartedIntValue(arg);
				else
					val = std::atoi(arg.c_str());
				return new NCSetObjectReference(new NCObject(makeNCObject<int>(val)));
			}

		}
		NCObjectReference* getNCSymbolArg(const std::string& arg) {
			if (hasBracketsIn(arg)) {//Oh damn its an inline function oh crap oh heck
				return getNCInlineFunction(arg);
			}
			else
				return new NCNamedObjectReference(arg);
		}
		NCObjectReference* getNCTypeArg(std::string arg) {
			//Make sure it doesn't have the starting ':' character
			arg.erase(arg.begin());
			NCType typeStringConversion = str_to_nct(arg);
			return new NCSetObjectReference(new NCObject(makeNCObject(typeStringConversion)));
		}

		NCArgumentList getNCArgumentList(const std::vector<std::string>& arguments) {
			NCArgumentList toret;

			for (const std::string& arg : arguments) {
				if (arg == "")
					throw new exception::MissingArgOnCompilation;
				else if (arg == "True" || arg == "true")
					toret.push_back(new NCSetObjectReference(new NCObject(makeNCObject(true))));
				else if (arg == "False" || arg == "false")
					toret.push_back(new NCSetObjectReference(new NCObject(makeNCObject(false))));
				else if (arg[0] == '"')
					toret.push_back(getNCStringArg(arg));
				else if (arg[0] == ':')
					toret.push_back(getNCTypeArg(arg));
				//Remember to check for negative numbers
				else if ((arg[0] >= '0' && arg[0] <= '9') || arg[0] == '-')
					toret.push_back(getNCNumericArg(arg));
				else
					toret.push_back(getNCSymbolArg(arg));
			}

			return toret;
		}

		NCRuntimeObject getNCRuntimeFromLine(const Multi_String_Indexer& line, size_t& index) {
			ParsedFunctionCall fc = getFunctionCall(line, index);
			std::string internalCommands;

			if (index >= line.getSize()) {
				printf("unexpected end of segment after function, probably a missing ';'\n");
				throw new exception::UnexpectedEndOfSection("getting runtime from line");
			}
			char c = line[index];
			index++;
			if (c == '{') {
				internalCommands = getContainedString(line, index, '{', '}');
				stripWhitespaceFromArgs(internalCommands);
			}

			return NCRuntimeObject(fc.command, getNCArgumentList(fc.arguments), getNCNode(std::vector<std::string>{internalCommands}));

		}

		NCRuntimeList getNCNode(const std::vector<std::string>& nodeSource) {
			Multi_String_Indexer node(nodeSource);
			NCRuntimeList list;
			size_t index = 0;
			while (index < node.getSize()) {
				list.push_back(getNCRuntimeFromLine(node, index));
			}
			return list;
		}
	}

	NCNodeLayout* getNCProgramFromSource(std::vector<std::string>& source) {
		cmpl::clearBlankLinesFromSource(source);
		size_t lineNumber = 0;
		NCNodeLayout* program = new NCNodeLayout;
		std::vector<std::string> currentNodeSource;
		std::string currentNodeName = "";
		//Ballpark guess of node size for average program.
		currentNodeSource.reserve(source.size() / 10);

		while (lineNumber < source.size()) {
			if (source[lineNumber][source[lineNumber].size() - 1] != '>')
				throw new exception::MissingNodeDeclarer(source[lineNumber]);
			else {
				currentNodeName = source[lineNumber];
				currentNodeName.pop_back();
				cmpl::stripWhitespaceFromArgs(std::vector<std::string>{currentNodeName});
				lineNumber++;
				while (source[lineNumber] != ">") {
					currentNodeSource.push_back(source[lineNumber]);
					lineNumber++;
					if (lineNumber == source.size())
						throw new exception::UnexpectedEndOfSection(currentNodeName);
				}
				//Go past the ">" line
				lineNumber++;
				program->operator[](currentNodeName) = cmpl::getNCNode(currentNodeSource);
				currentNodeName = "";
				currentNodeSource.clear();
			}
		}

		return program;
	}
	NCNodeLayout* getNCProgramFromFile(const std::string& fullFileName) {
		std::ifstream file;
		file.open(fullFileName);

		NCNodeLayout* layout = getNCProgramFromFile(file);
		file.close();

		return layout;
	}
	NCNodeLayout* getNCProgramFromFile(std::ifstream& file) {
		std::vector<std::string> source;
		std::string line;
		while (std::getline(file, line))
			source.push_back(line);

		return getNCProgramFromSource(source);
	}
}