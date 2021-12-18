#pragma once
#include "NCObjects.hpp"
#include "NCQuearies.hpp"
#include "NCOperations.hpp"
#include <fstream>
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

    //Environment in which to compiler node call in. Stores the pre-defined functions, quearies and vairables
    struct CompilerEnvironment{
        NCRuntime* runtime;

        std::unordered_map<std::string, NCOperationFunc> functions;
        std::unordered_map<std::string, NCQuearyFunc> quearies;
        std::unordered_map<std::string, std::any*> variables;
        std::unordered_map<std::string, uint16_t> nodes;

        std::vector<std::any*>& newlyCreatedVariables;
        std::vector<NCQueary*>& newlyCreatedQuearies;

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
    //A token from the lexer. Used in the parser to create the object representation
    struct Token{
        token_type type;
        std::string form;

        Token() = default;
        Token(token_type t, std::string&& f):
            type(t),
            form(std::move(f))
        {}
    };

    //Some constants to help parse whats happening in the lexer
    const char binMarker = 'b';
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
        lookingForNode, lookingInNode, readingLongToken, readingNonBase10, readingString
    };

    //Utility class so that one can consistantly get the next charcter in a multi-line passage passed in. Useful when lexing programs
    class MultiLineString{
    public:
        char peekChar(){
            return lines[line][index];
        }
        char takeChar(){
            char val = peekChar();
            index += 1;
            if(index == lines[line].size()){
                line += 1;
                index = 0;
            }
            return val;
        }
        void replaceChar(){
            if(index == 0){
                line -= 1;
                index = lines[line].size() - 1;
            }
            else
                index--;
        }
        char lookBack(){
            replaceChar();
            return takeChar();
        }

        bool atEnd(){
            return (line == lines.size() - 1) && (index == lines[line].size());
        }

        MultiLineString(const std::vector<std::string>& lines):
            lines(lines)
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
        bool inNode = false;
        bool onLongToken = false;
        token_type longTokenType;

        while(!string.atEnd()){
            if(whitespace.count(string.peekChar()) && state != readingString){
                string.takeChar();
                if(onLongToken)
                    addLongToken(onLongToken, currentString, longTokenType, tokens, compEnv);
            }
            else if(oneCharTokens.count(string.peekChar()) > 0 && state != readingString){
                //Edge case for node declerations
                if(string.peekChar() == '>'){
                    string.takeChar();
                    char next = string.peekChar();
                    string.replaceChar();
                    if(next == '='){
                        string.replaceChar();
                        currentString = ">";
                        longTokenType = userDefined;
                    }
                    else if(tokens[tokens.size()-1].type != seperator){
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

                    tokens.emplace_back(oneCharTokens.find(string.peekChar()));
                    if(string.takeChar() == nodeMarkChar)
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
                    //Make sure its alphabetical pls
                    break;
                case lookingInNode:
                    //Unsure what this could be; number, user, etc
                    break; 

                //Split these us and do stuff
                case readingString:
                case readingLongToken:
                case readingNonBase10:
                    break;
                    
                }
            }
        }
    }

}
}