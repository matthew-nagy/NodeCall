#include "tokeniser.hpp"

namespace nc {
    namespace comp {

        const std::unordered_map<token_type, std::string> tokenRep = {
            {open_node, "open node"}, {close_node, "close node"},
            {open_bracket, "open bracket"}, {close_bracket, "close bracket"},
            {argument_seperator, "argument seperator"}, {line_terminator, "line terminator"},
            {integer, "int"}, {floating_point, "float"}, {string, "string"}, {boolean, "bool"},
            {operation, "operation"}, {queary, "queary"}, {variable, "variable"},
            {assignment, "assignment"}, {addition, "addition"}, {subtraction, "subtraction"}, {multiplication, "mult"}, {division, "div"}, {modulo, "mod"},
            {lsl, "shift left"}, {lsr, "shift right"}, {logical_and, "and"}, {logical_or, "or"}, {logical_not, "not"}, {logical_xor, "xor"},
            {boolean_equality, "equals"}, {boolean_inequality, "not equals"}, {boolean_less, "less"}, {boolean_greater, "greater"},
            {boolean_lesseq, "less or equal"}, {boolean_greatereq, "greater or equal"},
            {boolean_and, "and"}, {boolean_or, "or"},
            {null_token, "null"}
        };



        bool compilation_environment::isOperation(const std::string& s)const {
            for (auto& l : libraries)
                if (l->operations.count(s) > 0)
                    return true;
            return false;
        }
        bool compilation_environment::isQueary(const std::string& s)const {
            for (auto& l : libraries)
                if (l->quearies.count(s) > 0)
                    return true;
            return false;
        }
        bool compilation_environment::isKnownVariable(const std::string& s)const {
            for (auto& l : libraries)
                if (l->variables.count(s) > 0)
                    return true;
            if (newVariables.count(s))
                return true;
            return false;
        }

        void compilation_environment::requestNewVariable(const std::string& varName) {
            printf("Creating new variable %s\n", varName.c_str());
            newVariables[varName] = std::make_shared<std::any>();
        }

        VariableTable compilation_environment::getNewVariables()const {
            return newVariables;
        }

        void compilation_environment::addLibrary(const std::shared_ptr<additional_library>& lib) {
            libraries.emplace_back(lib);
        }
        void compilation_environment::addVariables(const std::initializer_list <std::pair <std::string, value>>& initilizer) {
            for (auto& v : initilizer)
                newVariables.emplace(v);
        }
        void compilation_environment::addVariable(const std::string& name, const value& val) {
            newVariables.emplace(name, val);
        }

        OperationFunction compilation_environment::getOpereration(const std::string& s) {
            for (auto& l : libraries)
                if (l->operations.count(s) > 0)
                    return l->operations.find(s)->second;
            throw(new UNKNOWN_OPERATION_REQUESTED);
        }
        QuearyFunction compilation_environment::getQueary(const std::string& s) {
            for (auto& l : libraries)
                if (l->quearies.count(s) > 0)
                    return l->quearies.find(s)->second;
            printf("%s\n", s.c_str());
            throw(new UNKNOWN_QUEARY_REQUESTED);
        }
        value compilation_environment::getVariable(const std::string& s) {
            printf("variable name is >'%s'<\n", s.c_str());
            for (auto& l : libraries)
                if (l->variables.count(s) > 0)
                    return l->variables.find(s)->second;
            if (newVariables.count(s) > 0)
                return newVariables.find(s)->second;
            printf("%s\n", s.c_str());
            throw(new UNKNOWN_VARIABLE_REQUESTED);
        }

        std::vector<std::pair<std::string, OperationFunction>> compilation_environment::getOperations(const std::string& start) {
            std::vector<std::pair<std::string, OperationFunction>> operations;

            for (auto& l : libraries)
                for (auto& pair : l->operations)
                    if (pair.first.substr(0, start.size()) == start)
                        operations.emplace_back(pair);

            return operations;
        }
        std::vector<std::pair<std::string, QuearyFunction>> compilation_environment::getQuearys(const std::string& start) {
            std::vector<std::pair<std::string, QuearyFunction>> quearies;

            for (auto& l : libraries)
                for (auto& pair : l->quearies)
                    if (pair.first.substr(0, start.size()) == start)
                        quearies.emplace_back(pair);

            return quearies;
        }
        std::vector<std::pair<std::string, value>> compilation_environment::getVariables(const std::string& start) {
            std::vector<std::pair<std::string, value>> variables;

            for (auto& l : libraries)
                for (auto& pair : l->variables)
                    if (pair.first.substr(0, start.size()) == start)
                        variables.emplace_back(pair);
            for (auto& pair : newVariables)
                if (pair.first.substr(0, start.size()) == start)
                    variables.emplace_back(pair);

            return variables;
        }

        compilation_environment::compilation_environment(const std::initializer_list<std::shared_ptr<additional_library>>& libs) {
            for (auto& p : libs) {
                libraries.emplace_back(p);
            }
        }
        compilation_environment::compilation_environment(const compilation_environment& copy) {
            //Reserve size to speed emplacing up
            libraries.reserve(copy.libraries.size());
            newVariables.reserve(copy.newVariables.size());

            for (auto& l : copy.libraries)
                libraries.emplace_back(l);
            for (auto& pair : copy.newVariables)
                newVariables.emplace(pair);
        }





        //Mapping of single characters to the token the represent
        const std::unordered_map<char, token_type> _single_char_tokens = {
            {'{', open_node}, {'}', close_node},
            {'(', open_bracket}, {')', close_bracket},
            {',', argument_seperator}, {';', line_terminator},
            {'=', assignment}, {'+', addition}, {'-', subtraction}, {'*', multiplication}, {'/', division}, {'%', modulo},
            {'&', logical_and}, {'|', logical_or}, {'!', logical_not}, {'^', logical_xor},
            {'<', boolean_less}, {'>', boolean_greater},
            {'\0', null_token}
        };
        //Mapping of double characters to the tokens they represent. This lets you tell the different betwen two '='s and a '=='
        const std::unordered_map<std::string, token_type> _double_char_tokens = {
            {"==", boolean_equality}, {"!=", boolean_inequality},
            {"<<", lsl}, {">>", lsr},
            {">=", boolean_greatereq}, {"<=", boolean_lesseq},
            {"&&", boolean_and}, {"||", boolean_or}
        };


        token::token(const std::string& s, unsigned lineNum, token_type type) :
            representation(s),
            lineNumber(lineNum),
            type(type)
        {}


        bool isNumericChar(char c) {
            return (c >= '0' && c <= '9') || c == '-' || c == 'x' || c == '.';
        }
        bool isStringChar(char c) {
            return c == '"' || c == '\'';
        }
        bool isWhitespace(char c) {
            return c == ' ' || c == '\t';
        }
        bool isComment(char c) {
            return c == '#';
        }
        bool isSeperator(char c) {
            return c == ',';
        }

        bool isSpecialTerminatingChar(char c) {
            return isWhitespace(c) || isComment(c) || isSeperator(c) || c == '\0' || _single_char_tokens.count(c) > 0;
        }

        token_type getNumericType(const std::string& str) {
            bool seenFloatingPoint = false;
            for (size_t i = 0; i < str.size(); i++)
                if (str[i] == '.') {
                    if (seenFloatingPoint)
                        throw(new ILLIGAL_NUMERIC);
                    else
                        seenFloatingPoint = true;
                }

            return seenFloatingPoint ? floating_point : integer;
        }



        char source::peek()const {
            if (currentLineIndex == lines.size())
                return '\0';
            return lines[currentLineIndex][currentCharacterIndex];
        }
        char source::get() {
            char currentChar = peek();
            if (currentLineIndex < lines.size()) {
                currentCharacterIndex++;
                if (currentCharacterIndex == lines[currentLineIndex].size()) {
                    currentCharacterIndex = 0;
                    currentLineIndex++;
                }
            }
            return currentChar;
        }
        void source::replace() {
            if (currentCharacterIndex == 0) {
                currentLineIndex--;
                currentCharacterIndex = lines[currentLineIndex].size() - 1;
            }
            else {
                currentCharacterIndex--;
            }
        }

        void source::skipWhitespace() {
            while (isWhitespace(peek()))
                get();
        }
        void source::skipComment() {
            if (isComment(peek())) {
                currentLineIndex += 1;
                currentCharacterIndex = 0;
            }
        }

        size_t source::getLineNumber()const {
            if (currentLineIndex >= trueLineNumbers.size())
                return -1;
            return trueLineNumbers[currentLineIndex];
        }

        source::source(const std::string& name) {
            std::ifstream file;
            file.open(name, std::ios::in);
            printf("Source is at >'%s'\n", name.c_str());
            createFromLines(lineFromFile(file));

            for (auto& s : lines)
                printf("\t%s\n", s.c_str());
        }

        source::source(std::ifstream& file) {
            createFromLines(lineFromFile(file));
        }

        source::source(const std::vector<std::string>& code) {
            createFromLines(code);
        }


        bool source::lineValid(const std::string& line) {
            if (line == "")return false;
            for (size_t i = 0; i < line.size(); i++) {
                if (isComment(line[i]))//Entire line is a comments
                    return false;
                else if (!isWhitespace(line[i]))//Some non whitespace character ready to read
                    return true;
            }
            return false;//No character was found
        }

        std::string source::getStrippedLine(const std::string& line) {
            return line;
        }

        void source::createFromLines(const std::vector<std::string>& code) {
            for (size_t i = 0; i < code.size(); i++) {

                if (lineValid(code[i])) {
                    for (size_t j = 0; j < code[i].size(); j++) {
                        if (!isWhitespace(code[i][j])) {
                            lines.emplace_back(getStrippedLine(code[i].substr(j)));
                            trueLineNumbers.emplace_back(i);
                            break;
                        }
                    }
                }

            }

            currentCharacterIndex = currentLineIndex = 0;
        }

        std::vector<std::string> source::lineFromFile(std::ifstream& file) {
            std::string line;
            std::vector<std::string> lines;
            while (std::getline(file, line))
                lines.emplace_back(std::move(line));
            return lines;
        }



        token getNumericToken(source& sourceCode) {
            token toret;
            toret.lineNumber = sourceCode.getLineNumber();
            while (isNumericChar(sourceCode.peek()))
                toret.representation += sourceCode.get();

            toret.type = getNumericType(toret.representation);

            return toret;
        }
        token getStringToken(source& sourceCode) {
            char c = sourceCode.get();
            if (!isStringChar(c))
                throw(new ILLIGAL_STRING);

            char openChar = c;

            token toret;
            toret.type = string;
            toret.lineNumber = sourceCode.getLineNumber();

            c = sourceCode.get();
            do {
                printf("%s\n", (std::string("") + c).c_str());
                if (c == '\\') {
                    char replacement = 'n';
                    switch (sourceCode.peek()) {
                    case '\\':
                        replacement = '\\';
                        break;
                    case 'n':
                        replacement = '\n';
                        break;
                    case 't':
                        replacement = '\t';
                        break;
                    case '"':
                        replacement = '"';
                        break;
                    case '\'':
                        replacement = '\'';
                        break;
                    }
                    if (replacement != 'n') {
                        c = replacement;
                        sourceCode.get();
                    }
                }
                toret.representation += c;
                c = sourceCode.get();
            } while (c != openChar);

            return toret;
        }
        token getSpecialToken(source& sourceCode, compilation_environment& compEnv) {
            token toret;
            toret.lineNumber = sourceCode.getLineNumber();

            //Get the entirety of the token
            char c = sourceCode.get();
            do {
                toret.representation += c;
                c = sourceCode.get();
            } while (!isSpecialTerminatingChar(c));
            sourceCode.replace();

            //Figure out what type of token it was
            if (toret.representation == "true" || toret.representation == "false") {
                toret.type = boolean;
            }
            else if (compEnv.isOperation(toret.representation)) {
                toret.type = operation;
            }
            else if (compEnv.isQueary(toret.representation)) {
                toret.type = queary;
            }
            else {
                if (!compEnv.isKnownVariable(toret.representation)) {
                    compEnv.requestNewVariable(toret.representation);
                }
                toret.type = variable;
            }

            return toret;
        }

        token getTokenFromSource(source& sourceCode, compilation_environment& compEnv) {
            unsigned lineNum = sourceCode.getLineNumber();
            char c1 = sourceCode.get();
            if (_single_char_tokens.count(c1) > 0) {
                char c2 = sourceCode.peek();
                std::string compString = "  ";
                compString[0] = c1;
                compString[1] = c2;
                if (_double_char_tokens.count(compString) > 0) {
                    //Take that second one too then
                    sourceCode.get();
                    return token(compString, lineNum, _double_char_tokens.find(compString)->second);
                }
                else {
                    return token(std::string() + c1, lineNum, _single_char_tokens.find(c1)->second);
                }
            }
            sourceCode.replace();

            if (isNumericChar(c1))
                return getNumericToken(sourceCode);
            else if (isStringChar(c1))
                return getStringToken(sourceCode);
            return getSpecialToken(sourceCode, compEnv);
        }

        std::vector<token> tokeniseSource(source& sourceCode, compilation_environment& compEnv) {
            std::vector<token> tokens;

            do {
                tokens.emplace_back(getTokenFromSource(sourceCode, compEnv));
                sourceCode.skipWhitespace();
                sourceCode.skipComment();
            } while (tokens.back().type != null_token);

            return tokens;
        }

    }
}