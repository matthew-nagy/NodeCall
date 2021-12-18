#pragma once

#include "NCObjects.hpp"
#include "NCQuearies.hpp"
#include "NCOperations.hpp"
#include <fstream>
#include <string>
#include <stdexcept>

//Allows a script to access objects, functions and quearies not in built
struct NCExtention{
    std::unordered_map<std::string, NCOperationFunc> functions;
    std::unordered_map<std::string, NCQuearyFunc> quearies;
    std::unordered_map<std::string, std::any*> variables;

    NCExtention(
        const std::initializer_list<std::pair<const std::string, NCOperationFunc>>& funcInitilizer,
        const std::initializer_list<std::pair<const std::string, NCQuearyFunc>>& quearyInitalizer,
        const std::initializer_list<std::pair<const std::string, std::any*>>& variableInitalizer
    ):
        functions(funcInitilizer),
        quearies(quearyInitalizer),
        variables(variableInitalizer)
    {}
};

class NodeCall{
public:
    NodeCall() = delete;

    static NCRuntime* compiler(const std::string& filename, const std::initializer_list<NCExtention>& extentions);
    static NCRuntime* compiler(std::ifstream& file, const std::initializer_list<NCExtention>& extentions);
};

namespace ncprivate{
namespace compiler{
    class NC_Compiler_Mismatched_Token_Error : public std::logic_error{};
    class NC_Compiler_Illigal_Special_Token : public std::logic_error{};
    class NC_Compiler_Invalid_Node_Token : public std::logic_error{};

    //Environment in which to compiler node call in. Stores the pre-defined functions, quearies and vairables
    struct CompilerEnvironment{
        NCRuntime* runtime;

        std::unordered_map<std::string, NCOperationFunc> functions;
        std::unordered_map<std::string, NCQuearyFunc> quearies;
        std::unordered_map<std::string, std::any*> variables;
        //TODO; take this out. Just store them as variables
        std::unordered_map<std::string, uint16_t> nodes;

        std::vector<std::any*>& newlyCreatedVariables;
        std::vector<NCQueary*>& newlyCreatedQuearies;

        CompilerEnvironment(std::vector<std::any*>& newlyCreatedVariables, std::vector<NCQueary*>& newlyCreatedQuearies):
            newlyCreatedVariables(newlyCreatedVariables),
            newlyCreatedQuearies(newlyCreatedQuearies)
        {}

        void addExtention(const NCExtention& ex){
            for(auto& [fname, func] : ex.functions)
                functions.insert({fname, func});
            for(auto& [qname, queary] : ex.quearies)
                quearies.insert({qname, queary});
            for(auto& [vname, var] : ex.variables)
                variables.insert({vname, var});
        }
        void addQueary(NCQueary* q){
            newlyCreatedQuearies.emplace_back(q);
        }


        std::any* getVariable(const std::string& name){
            if(variables.count(name) > 0)
                return variables[name];
            else{
                std::any* newVar = new std::any;
                newlyCreatedVariables.emplace_back(newVar);
                variables[name] = newVar;
                return newVar;
            }
        }
        uint16_t getNode(const std::string name){
            if(nodes.count(name) > 0)
                return nodes[name];
            else{
                uint16_t newNodeIndex = runtime->nodes.size();
                runtime->nodes.emplace_back();
                nodes[name] = newNodeIndex;
                return newNodeIndex;
            }
        }
    };

    //Types of token NodeCall recognises
    enum token_type{
        openBracket, closeBracket, openConditionalNode, closeConditionalNode, seperator, terminator, 
        number, string, boolean, function, queary, variable, userDefined, nodeDelimiter
    };
    std::unordered_map<token_type, std::string> tokenToString = {
        {openBracket, "Open Bracket"}, {closeBracket, "Close Bracket"}, {openConditionalNode, "Open Curly"}, {closeConditionalNode, "Close Curly"},
        {seperator, "Comma   "}, {terminator, "Semi-colon"}, {number, "Number "}, {string, "String   "}, {boolean, "Boolean"},
        {function, "Function"}, {queary, "Queary   "}, {variable, "Variable"}, {userDefined, "User Defined"}, {nodeDelimiter, "Node marker"}
    };
    //A token from the lexer. Used in the parser to create the object representation
    struct Token{
        token_type type;
        std::string form;

        void print(){
            printf("|%s\t|%s\n", tokenToString[type].c_str(), form.c_str());
        }

        Token() = default;
        Token(token_type t, std::string&& f):
            type(t),
            form(std::move(f))
        {}
        Token(const Token& copy) = default;
    };

    //Some constants to help parse whats happening in the lexer
    const char commentChar= '#';
    const char baseMarker = '0';
    const char hexMarker = 'x';
    const char binMarker = 'b';
    const char nodeMarkChar = '>';
    const std::unordered_map<char, Token> oneCharTokens = {
        {'(', {openBracket, "("}}, {')', {closeBracket, ")"}}, {'{', {openConditionalNode,"{"}}, {'}', {closeConditionalNode, "}"}},
        {',', {seperator, ","}}, {';', {terminator, ";"}}, {nodeMarkChar, {nodeDelimiter, ">"}}, {'~', {queary, "~"}}, {'^', {queary, "^"}}
    };
    const std::unordered_set<char> whitespace= {' ', '\t'};

    //Lexers are finite state machines. These are the states it could be in
    enum lexer_state{
        lookingForNode, lookingInNode, readingLongToken, readingNonBase10, readingNumber, readingOctal, readingHex, readingBin, readingString
    };

    bool isLastTokenType(std::vector<Token>& tokens, token_type type){
        if(tokens.size() == 0)
            return false;
        return tokens.back().type == type;
    }

    //Utility class so that one can consistantly get the next charcter in a multi-line passage passed in. Useful when lexing programs
    class MultiLineString{
    public:
        void takeChar(){
            index += 1;
            if(index >= lines[line].size()){
                line += 1;
                index = 0;
            }
        }
        void replaceChar(){
            if(index == 0){
                line -= 1;
                index = lines[line].size() - 1;
            }
            else
                index--;
        }
        char peekChar(){
            return lines[line][index];
        }
        char peekNextChar(){
            takeChar();
            if(atEnd())
                return '\0';
            char ans = peekChar();
            replaceChar();
            return ans;
        }
        char peekBack(){
            replaceChar();
            char ans = peekChar();
            takeChar();
            return ans;
        }

        bool atEnd(){
            printf("Checking end, %zu against %zu\n", line, lines.size());
            return line >= lines.size();
        }

        void skipLine(){
            line++;
            index = 0;
        }

        size_t numOfLines(){
            return lines.size();
        }

        MultiLineString(const std::vector<std::string>& lines):
            lines(lines),
            line(0),
            index(0)
        {}

    private:
        const std::vector<std::string>& lines;
        size_t line;
        size_t index;
    };

    //Given a multi string token, a hint on what it may be, and a compiler environment, this function figures out the exact token type it is, and adds it
    //to the list of tokens
    void addLongToken(bool& onLongToken, std::string& currentString, token_type longTokenType, std::vector<Token>& tokens, CompilerEnvironment& compEnv){
        onLongToken = false;
        //Ensure it is *actually* user defined. It may be a known 
        if(longTokenType == userDefined){
            if(compEnv.functions.count(currentString) > 0)
                longTokenType = function;
            else if(compEnv.quearies.count(currentString) > 0)
                longTokenType = queary;
            else if(compEnv.variables.count(currentString) > 0)
                longTokenType = variable;
        }
        tokens.emplace_back(longTokenType, std::move(currentString));
        currentString = "";
    }

    std::vector<Token> lexer(MultiLineString& string, CompilerEnvironment& compEnv){
        lexer_state state = lookingForNode;
        std::string currentString = "";
        std::vector<Token> tokens;
        tokens.reserve(string.numOfLines() * 6); //Assming an average of a function, open,close brackets, semicolon and 2 arguments. Some will have more, some less
        bool inNode = false;
        bool onLongToken = false;
        bool seenDecimalPoint = false;
        token_type longTokenType;

        while(!string.atEnd()){
            printf("Start loop\n");
            if(string.peekChar() == commentChar){
                string.skipLine();
                continue;
            }
            else if(whitespace.count(string.peekChar()) && state != readingString){
                if(onLongToken)
                    addLongToken(onLongToken, currentString, longTokenType, tokens, compEnv);
            }
            else if(oneCharTokens.count(string.peekChar()) > 0 && state != readingString){
                //Edge case for node declerations
                if(string.peekChar() == '>'){
                    char next = string.peekNextChar();
                    if(next == '='){
                        string.replaceChar();
                        currentString = ">";
                        longTokenType = userDefined;
                    }
                    else if(isLastTokenType(tokens, seperator)){
                        tokens.emplace_back(queary, ">");
                    }
                    else{
                        inNode = !inNode;
                        tokens.emplace_back(nodeDelimiter, ">");
                    }
                }
                else{
                    if(onLongToken)
                        addLongToken(onLongToken, currentString, longTokenType, tokens, compEnv);

                    tokens.emplace_back((*oneCharTokens.find(string.peekChar())).second);
                    if(string.peekChar() == nodeMarkChar)
                        inNode = !inNode;
                    
                    state = inNode ? lookingInNode : lookingForNode;
                }
            }
            else{
                //It should be true that two multi character tokens are never directly adjacent, with no space or other token between
                //In fact, even having them with a space shouldn't be a thing. Therefore assume that the character being peeked at is part of
                //whatever character we were working on.
                switch(state){
                case lookingForNode:
                    if( (string.peekChar() >= 'a' && string.peekChar() <='z') || 
                        (string.peekChar() >= 'A' && string.peekChar() <= 'Z')){
                            state = readingLongToken;
                            currentString += string.peekChar();
                    }
                    else
                        throw(new NC_Compiler_Invalid_Node_Token);
                    break;
                case lookingInNode:
                    currentString += string.peekChar();
                    switch(string.peekChar()){
                        case '\"': state = readingString; break;
                        case '0' : state = readingNonBase10; break;
                        default:
                            if(string.peekChar() >= '1' && string.peekChar() <= '9')
                                state = readingNumber;
                            else
                                state = readingLongToken;
                    }
                    break; 

                //Split these us and do stuff
                case readingString:
                    if(string.peekChar() == '"'){
                        currentString += string.peekChar();
                        addLongToken(onLongToken, currentString, longTokenType, tokens, compEnv);
                        state = lookingInNode;
                    }
                    //Could be a special case
                    else if(string.peekChar() == '\\'){
                        string.takeChar();
                        switch(string.peekChar()){
                            case 't': currentString += '\t'; break;
                            case 'n': currentString += '\n'; break;
                            case '\\': currentString += '\\';break;
                            case '\"': currentString += '\"';break;
                            default:
                                throw(new NC_Compiler_Illigal_Special_Token);
                        }
                    }
                    else
                        currentString += string.peekChar();
                case readingLongToken:
                case readingNonBase10:
                    break;
                    
                }
            }

            string.takeChar();
        }

        return tokens;
    }

}
}