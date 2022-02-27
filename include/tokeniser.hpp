#pragma once
#ifndef NC_TOKENISER_HPP
#define NC_TOKENISER_HPP
#include "Standard_Library.hpp"
#include <unordered_set>
#include <initializer_list>
#include <fstream>

namespace nc{   namespace comp{

    ERROR_MAKE(ILLIGAL_NUMERIC);
    ERROR_MAKE(ILLIGAL_STRING);
    ERROR_MAKE(UNKNOWN_OPERATION_REQUESTED);
    ERROR_MAKE(UNKNOWN_QUEARY_REQUESTED);
    ERROR_MAKE(UNKNOWN_VARIABLE_REQUESTED);


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

    extern const std::unordered_map<token_type, std::string> tokenRep;

    class compilation_environment {
    public:
        bool isOperation(const std::string& s)const;
        bool isQueary(const std::string& s)const;
        bool isKnownVariable(const std::string& s)const;

        void requestNewVariable(const std::string& varName);

        VariableTable getNewVariables()const;

        void addLibrary(const std::shared_ptr<additional_library>& lib);
        void addVariables(const std::initializer_list <std::pair <std::string, value>>& initilizer);
        void addVariable(const std::string& name, const value& val);

        OperationFunction getOpereration(const std::string& s);
        QuearyFunction getQueary(const std::string& s);
        value getVariable(const std::string& s);
        
        std::vector<std::pair<std::string, OperationFunction>> getOperations(const std::string& start);
        std::vector<std::pair<std::string, QuearyFunction>> getQuearys(const std::string& start);
        std::vector<std::pair<std::string, value>> getVariables(const std::string& start);

        compilation_environment() = default;
        compilation_environment(const std::initializer_list<std::shared_ptr<additional_library>>& libs);
        compilation_environment(const compilation_environment& copy);
    private:
        std::vector<std::shared_ptr<additional_library>> libraries;
        VariableTable newVariables;
    };

    //Mapping of single characters to the token the represent
    extern const std::unordered_map<char, token_type> _single_char_tokens;
    //Mapping of double characters to the tokens they represent. This lets you tell the different betwen two '='s and a '=='
    extern const std::unordered_map<std::string, token_type> _double_char_tokens;

    struct token{
        std::string representation;
        unsigned lineNumber;
        token_type type;

        token() = default;

        token(const std::string& s, unsigned lineNum, token_type type);
    };


    class source{
    public:

        char peek()const;
        char get();
        void replace();

        void skipWhitespace();
        void skipComment();

        size_t getLineNumber()const;

        source(const std::string& name);

        source(std::ifstream& file);

        source(const std::vector<std::string>& code);

    private:
        std::vector<std::string> lines;         //All lines with program code on it
        std::vector<unsigned> trueLineNumbers;  //Lines with no code on them are excluded so we keep track here

        size_t currentLineIndex;
        size_t currentCharacterIndex;

        bool lineValid(const std::string& line);

        std::string getStrippedLine(const std::string& line);
        
        void createFromLines(const std::vector<std::string>& code);

        std::vector<std::string> lineFromFile(std::ifstream& file);
    };

    std::vector<token> tokeniseSource(source& sourceCode, compilation_environment& compEnv);

}}

#endif