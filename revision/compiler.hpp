#pragma once
#ifndef COMPILER_HPP
#define COMPILER_HPP
#include "Standard_Library.hpp"

namespace nc{   namespace comp{

    ERROR_MAKE(ILLIGAL_NUMERIC);
    ERROR_MAKE(ILLIGAL_STRING);


    enum token_type{
        open_node, close_node, //{ and }
        open_bracket, close_bracket,    //( and )
        argument_seperator, line_terminator,    //, and ;
        integer, floating_point, string, boolean,    //Build in types
        operation, queary, variable,     //Supported features
        assignment, addition, subtraction, multiplication, division, modulo,    //Mathmatical operations
        lsl, lsr, logical_and, logical_or, logical_not, logical_xor,    //Logical operations
        boolean_equality, boolean_inequality, boolean_less, boolean_greater, boolean_lesseq, boolean_greatereq, //Boolean comparisons
        boolean_and, boolean_or,
        null_token   //tee hee
    };

    class compilation_environment {
    public:
        bool isOperation(const std::string& s)const {
            for (auto& l : libraries)
                if (l->operations.count(s) > 0)
                    return true;
            return false;
        }
        bool isQueary(const std::string& s)const {
            for (auto& l : libraries)
                if (l->quearies.count(s) > 0)
                    return true;
            return false;
        }
        bool isKnownVariable(const std::string& s)const {
            for (auto& l : libraries)
                if (l->variables.count(s) > 0)
                    return true;
            if (newVariables.count(s))
                return true;
            return false;
        }

        void requestNewVariable(const std::string& varName) {
            newVariables[varName] = std::make_shared<std::any>();
        }
    private:
        std::vector<std::shared_ptr<additional_library>> libraries;
        VariableTable newVariables;
    };

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

    enum lexer_state{
        looking_for_node,   //Looking for the name of a node to parse
        in_node,            //In a node, looking for the next token to lex
        on_numeric, on_string,  //Going through integers, floating points, and strings
        on_word    //functions, quearies, variables, true and false
    };

    struct token{
        std::string representation;
        unsigned lineNumber;
        token_type type;
    };

    bool isNumericChar(char c) {
        return (c >= '0' && c <= '9') || c == '-' || c == 'x';
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
        return isWhitespace(c) || isComment(c) || isSeperator(c);
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

    class source{
    public:

        char peekChar()const{
            if(currentLineIndex == lines.size())
                return '\0';
            return lines[currentLineIndex][currentCharacterIndex];
        }
        char getChar(){
            char currentChar = peekChar();
            if(currentLineIndex < lines.size()){
                currentCharacterIndex++;
                if(currentCharacterIndex == lines[currentLineIndex].size()){
                    currentCharacterIndex = 0;
                    currentLineIndex++;
                }
            }
            return currentChar;
        }
        void replaceChar(){
            if(currentCharacterIndex == 0){
                currentLineIndex--;
                currentCharacterIndex = lines[currentLineIndex].size() - 1;
            }
            else{
                currentCharacterIndex--;
            }
        }

        void skipWhitespace() {
            while (isWhitespace(peekChar()))
                getChar();
        }
        void skipComment() {
            if (isComment(peekChar()))
                currentLineIndex += 1;
        }

        size_t getLineNumber()const {
            return trueLineNumbers[currentLineIndex];
        }

    private:
        std::vector<std::string> lines;         //All lines with program code on it
        std::vector<unsigned> trueLineNumbers;  //Lines with no code on them are excluded so we keep track here

        size_t currentLineIndex;
        size_t currentCharacterIndex;
    };

    token getNumericToken(source& sourceCode) {
        token toret;
        toret.lineNumber = sourceCode.getLineNumber();
        while (isNumericChar(sourceCode.peekChar()))
            toret.representation += sourceCode.getChar();

        toret.type = getNumericType(toret.representation);

        return toret;
    }
    token getStringToken(source& sourceCode){
        char c = sourceCode.getChar();
        if (c != '"')
            throw(new ILLIGAL_STRING);
        
        token toret;
        toret.type = string;
        toret.lineNumber = sourceCode.getLineNumber();
        
        c = sourceCode.getChar();
        do {
            toret.representation += c;
            c = sourceCode.getChar();
        } while (!isStringChar(c));

        return toret;
    }
    token getSpecialToken(source& sourceCode, compilation_environment& compEnv){
        token toret;
        toret.type = defined_token;
        toret.lineNumber = sourceCode.getLineNumber();

        //Get the entirety of the token
        char c = sourceCode.getChar();
        do {
            toret.representation += c;
            c = sourceCode.getChar();
        } while (!isSpecialTerminatingChar(c));

        //Figure out what type of token it was
        if (compEnv.isOperation(toret.representation)) {
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
        char c1 = sourceCode.getChar();
        if (_single_char_tokens.count(c1) > 0) {
            char c2 = sourceCode.peekChar();
            std::string compString = [c2, c2'\0'];
            if (_double_char_tokens.count(compString) > 0)
                return _double_char_tokens[compString];
            else
                return _single_char_tokens[c1];
        }
        sourceCode.replaceChar();

        if (isNumericChar(c1))
            return getNumericToken(sourceCode);
        else if (isStringChar(c1))
            return getStringToken(sourceCode);
        return getSpecialToken(sourceCode);
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

}}

#endif