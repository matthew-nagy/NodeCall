#pragma once
#ifndef COMPILER_HPP
#define COMPILER_HPP
#include "Standard_Library.hpp"

namespace nc{   namespace comp{

    enum token_type{
        open_node, close_node, //{ and }
        open_bracket, close_bracket,    //( and )
        argument_seperator, line_terminator,    //, and ;
        integer, floatingPoint, string, boolean,    //Build in types
        function, queary, variable,     //Supported features
        assignment, addition, subtraction, multiplication, division, modulo,    //Mathmatical operations
        lsl, lsr, logical_and, logical_or, logical_not, logical_xor,    //Logical operations
        boolean_equality, boolean_inequality, boolean_less, boolean_greater, boolean_lesseq, boolean_greatereq, //Boolean comparisons
        boolean_and, boolean_or,
        nullToken   //tee hee
    };

    //Mapping of single characters to the token the represent
    const std::unordered_map<char, token_type> _single_char_tokens = {
        {'{', open_node}, {'}', close_node}, 
        {'(', open_bracket}, {')', close_bracket},
        {',', argument_seperator}, {';', line_terminator},
        {'=', assignment}, {'+', addition}, {'-', subtraction}, {'*', multiplication}, {'/', division}, {'%', modulo},
        {'&', logical_and}, {'|', logical_or}, {'!', logical_not}, {'^', logical_xor},
        {'<', boolean_less}, {'>', boolean_greater}
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

}}

#endif