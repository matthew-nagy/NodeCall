#pragma once

#include "NCObjects.hpp"
#include "NCQuearies.hpp"
#include "NCOperations.hpp"
#include <fstream>
#include <string>
#include <stdexcept>

//Allows a script to access objects, functions and quearies not in built
struct NCExtention{
    std::map<std::string, NCOperationFunc> functions;
    std::map<std::string, NCQuearyFunc> quearies;
    std::map<std::string, std::any*> variables;

    //A nice way for users to easily define their own extenions
    NCExtention(
        const std::initializer_list<std::pair<const std::string, NCOperationFunc>>& funcInitilizer,
        const std::initializer_list<std::pair<const std::string, NCQuearyFunc>>& quearyInitalizer,
        const std::initializer_list<std::pair<const std::string, std::any*>>& variableInitalizer
    );

    //A way to define extentions from existing maps. Like, the basic NC maps
    NCExtention(
        const std::map<std::string, NCOperationFunc>& funcInitilizer,
        const std::map<std::string, NCQuearyFunc>& quearyInitalizer,
        const std::map<std::string, std::any*>& variableInitalizer
    );
};

namespace ncprivate{
namespace compiler{
    //The erros that the lexer, parser and compiler can kick out
#define ERROR_MAKE(NAME) class NAME : public std::logic_error{ public: NAME () : std::logic_error( #NAME ){} }
    ERROR_MAKE(NC_Compiler_Mismatched_Token_Error);             //Some tokens aren't quite right
    ERROR_MAKE(NC_Compiler_Illigal_Special_String_Token);       //A backslash was used, but there is no special token matching that code
    ERROR_MAKE(NC_Compiler_Invalid_Node_Token);                 //Some other rune was seen that is illigal to start a node with
    ERROR_MAKE(NC_Compiler_Invalid_Floating_Point_Token);       //Multiple decimal places have been put- this is illigal
    ERROR_MAKE(NC_Compiler_Invalid_Numeric_Token);              //A non-numeric rune was seen while creating a number
    ERROR_MAKE(NC_Compiler_Unknown_Numeric_Base);               //A non binary/octal/decimal/hexedecimal number was attempted to be used
    ERROR_MAKE(NC_Compiler_Invalid_Digit_For_Numeric_Base);     //A number out of the range of a respective numeric base was seen

    
    ERROR_MAKE(NC_Parser_Uncalled_Queary);                      //A naked queary was written; no ( ) or arguments
    ERROR_MAKE(NC_Parser_Unexpected_Function_Call_Mid_Function);//While expecting arguments or quearies, a function was put
    ERROR_MAKE(NC_Parser_Invalid_Argument_Token);               //Some token that cannot be used as an argument was attempted to be read as an argument
    ERROR_MAKE(NC_Parser_Invalid_Arguments_In_Call);            //While parseing a queary or function, something went wrong reading the arguments
    ERROR_MAKE(NC_Parser_Expected_Comma);                       //While parseing a queary or function, there was a missing ,
    ERROR_MAKE(NC_Parser_Unexpected_EOP);                       //The parser ran out of tokens for a phrase while not finished
    ERROR_MAKE(NC_Parser_Expected_Open_Bracket);                //A queary or function wasn't called properly
    ERROR_MAKE(NC_Parser_Missing_Terminator);                   //MISSING SEMICOLON ON LINE XYZ
    ERROR_MAKE(NC_Parser_Invalid_Terminator);                   //For example "};" is not allowed
    ERROR_MAKE(NC_Parser_Expected_Function);                    //Been told to parse a function, but the first symbol isn't a function?

    ERROR_MAKE(NC_Parser_Expected_Node);                        //At the highest levels of the program, only node names are allowed
    ERROR_MAKE(NC_Parser_Expected_Node_Start);                  //After a nodes name a '>' must appear
#undef ERROR_MAKE
    
    //Environment in which to compiler node call in. Stores the pre-defined functions, quearies and vairables
    struct CompilerEnvironment{
        //Pointer to the runtime this envioronment created
        NCRuntime* runtime;

        //Maps, queries and variables, mapped against the string name they
        //have been given
        std::map<std::string, NCOperationFunc> functions;
        std::map<std::string, NCQuearyFunc> quearies;
        std::map<std::string, std::any*> variables;

        //References to these vectors in the runtime. Any newly created memory for compiling is put here, so that
        //you can delete it all later
        std::vector<std::any*>& newlyCreatedVariables;
        std::vector<NCQueary*>& newlyCreatedQuearies;

        //Sets up he environment with the runtime it will compile into
        CompilerEnvironment(NCRuntime* runtime);

        //Adds an extention to the environment, so you can add to the environment the source will
        //be compiled in
        void addExtention(const NCExtention& ex);

        //Adds a queary to the created memory list
        void addQueary(NCQueary* q);

        //Adds a new variable to the created memory list
        std::any* getNewVariable(const std::string& name);

        //Either makes a new node, or gets you the index of the requested node
        uint16_t getNode(const std::string name);
        
        //Compiles the given source in the environment
        NCRuntime* compile(const std::vector<std::string>& lines);
    };

    //Types of token NodeCall recognises
    enum token_type{
        openBracket, closeBracket, openConditionalNode, closeConditionalNode, seperator, terminator, 
        integer, floatingPoint, string, boolean, function, queary, variable, userDefined, nodeDelimiter,
        nullToken
    };
    //A token from the lexer. Used in the parser to create the object representation
    struct Token{
        token_type type;
        std::string form;
        unsigned lineOn;

        void print()const;

        Token() = default;
        Token(token_type t, std::string&& f);
        Token(const Token& copy) = default;
    };
    
    //Lexers are finite state machines. These are the states it could be in
    enum lexer_state{
        lookingForNode, lookingInNode, readingLongToken, readingNonBase10, readingNumber, readingOctal, readingHex, readingBin, readingString
    };

    bool isLastTokenType(std::vector<Token*>& tokens, token_type type);

    //Utility class so that one can consistantly get the next charcter in a multi-line passage passed in. Useful when lexing programs
    class MultiLineString{
    public:
        void takeChar();
        void replaceChar();
        char peekChar();
        char peekNextChar();
        char peekBack();

        bool atEnd();

        void skipLine();

        size_t numOfLines();

        size_t getLineOn();

        MultiLineString(const std::vector<std::string>& inLines);

    private:
        std::vector<std::string> lines;
        size_t line;
        size_t index;
    };

    //Given a multi string token, a hint on what it may be, and a compiler environment, this function figures out the exact token type it is, and adds it
    //to the list of tokens
    void addLongToken(unsigned lineOn, bool& onLongToken, bool& seenFloatingPoint, std::string& currentString, token_type& longTokenType, std::vector<Token*>& tokens, CompilerEnvironment& compEnv);

    //Lexes a token that is one character long. Called with many references to lexer variables
    lexer_state handleOneCharToken(bool& onLongToken, bool& seenDecimalPoint, bool& inNode, MultiLineString& string, std::string& currentString, token_type& longTokenType, std::vector<Token*>& tokens, CompilerEnvironment& compEnv);
    
    //A switch statement to handle the progression of the lexer's finite state machine
    void lexerSwitch(lexer_state& state, bool& onLongToken, MultiLineString& string, std::string& currentString, token_type& longTokenType, bool& seenDecimalPoint, std::vector<Token*>& tokens, CompilerEnvironment& compEnv);

    std::vector<Token*> lexer(MultiLineString& string, CompilerEnvironment& compEnv);


    //Adds a new argument to the given vector, based off the type and form of the given token in this environment
    void parseArgument(const Token& token, std::vector<NCArgument>& into, CompilerEnvironment& env);

    //Given a function call, gets the arguments until the close brackets
    void parseCall(const std::vector<Token*>& tokens, std::vector<NCArgument>& arguments, CompilerEnvironment& env, size_t& index);

    //Given a string of tokens, create your new queary argument
    void parseQueary(const std::vector<Token*>& tokens, std::vector<NCArgument>& into, CompilerEnvironment& env);

    void parseFunction(const std::vector<Token*>& tokens, CompilerEnvironment& env, unsigned thisNodeIndex, std::vector<std::vector<NCFunction>>& nodes);

    uint16_t parseNodeAndGetIndex(const std::string& name, const std::vector<Token*>& tokens, CompilerEnvironment& env, std::vector<std::vector<NCFunction>>& nodes, size_t& index, token_type layerEnterType, token_type layerExitType);

    void parser(const std::vector<Token*>& tokens, CompilerEnvironment& environment, std::vector<std::vector<NCFunction>>& nodes);

}
}

class NodeCall{
public:
    static NCRuntime* compile(std::ifstream& file, const std::initializer_list<NCExtention*>& extentions);
};