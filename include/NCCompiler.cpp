#include "NCCompiler.hpp"

//A nice way for users to easily define their own extenions
NCExtention::NCExtention(
    const std::initializer_list<std::pair<const std::string, NCOperationFunc>>& funcInitilizer,
    const std::initializer_list<std::pair<const std::string, NCQuearyFunc>>& quearyInitalizer,
    const std::initializer_list<std::pair<const std::string, std::any*>>& variableInitalizer
):
    functions(funcInitilizer),
    quearies(quearyInitalizer),
    variables(variableInitalizer)
{}

//A way to define extentions from existing maps. Like, the basic NC maps
NCExtention::NCExtention(
    const std::map<std::string, NCOperationFunc>& funcInitilizer,
    const std::map<std::string, NCQuearyFunc>& quearyInitalizer,
    const std::map<std::string, std::any*>& variableInitalizer
):
    functions(funcInitilizer),
    quearies(quearyInitalizer),
    variables(variableInitalizer)
{}




namespace ncprivate{
namespace compiler{
    //Definitions for the environment the source will be compiled in
    //The "compile" function is defined at the bottom of the file

    //Sets up he environment with the runtime it will compile into
    CompilerEnvironment::CompilerEnvironment(NCRuntime* runtime):
        runtime(runtime),
        newlyCreatedVariables(runtime->objects),
        newlyCreatedQuearies(runtime->quearies),
        nodeKeys(runtime->nodeKeys)
    {}

    //Adds an extention to the environment, so you can add to the environment the source will
    //be compiled in
    void CompilerEnvironment::addExtention(const NCExtention& ex){
        for(auto& [fname, func] : ex.functions)
            functions.insert({fname, func});
        for(auto& [qname, queary] : ex.quearies)
            quearies.insert({qname, queary});
        for(auto& [vname, var] : ex.variables)
            variables.insert({vname, var});
    }
    
    //Adds a queary to the created memory list
    void CompilerEnvironment::addQueary(NCQueary* q){
        newlyCreatedQuearies.emplace_back(q);
    }

    //Adds a new variable to the created memory list
    std::any* CompilerEnvironment::getNewVariable(const std::string& name){
        std::any* newVar = new std::any;
        newlyCreatedVariables.emplace_back(newVar);
        variables[name] = newVar;
        return newVar;
    }
    //Either makes a new node, or gets you the index of the requested node
    uint16_t CompilerEnvironment::getNode(const std::string name){
        if(variables.count(name) > 0){
            auto* var = variables[name];
            //If it was seen before hand and mistaken for a variable
            if(!var->has_value()){
                uint16_t newNodeIndex = runtime->nodes.size();
                nodeKeys[name] = newNodeIndex;
                runtime->nodes.emplace_back();
                (*var) = newNodeIndex;
            }
            return std::any_cast<uint16_t>(*var);
        }

        uint16_t newNodeIndex = runtime->nodes.size();
        nodeKeys[name] = newNodeIndex;
        runtime->nodes.emplace_back();
        auto nodeVariable = getNewVariable(name);
        (*nodeVariable) = newNodeIndex;
        //(*variables[name]) = new std::any(newNodeIndex);
        return newNodeIndex;
    }






    const std::map<token_type, std::string> tokenToString = {
        {openBracket, "Open Bracket"}, {closeBracket, "Close Bracket"}, {openConditionalNode, "Open Curly"}, {closeConditionalNode, "Close Curly"},
        {seperator, "Comma   "}, {terminator, "Semi-colon"}, {integer, "Integer "},{floatingPoint, "Float   "}, {string, "String   "}, {boolean, "Boolean"},
        {function, "Function"}, {queary, "Queary   "}, {variable, "Variable"}, {userDefined, "User Defined"}, {nodeDelimiter, "Node marker"},
        {nullToken, "NULL TOKEN"}
    };
    void Token::print()const{
        printf("%u\t|%s\t|%s\n",unsigned(type), (*tokenToString.find(type)).second.c_str(), form.c_str());
    }
    Token::Token(token_type t, std::string&& f):
        type(t),
        form(std::move(f))
    {}



    //Some constants to help parse whats happening in the lexer
    const char commentChar= '#';
    const char baseMarker = '0';
    const char hexMarker = 'x';
    const char binMarker = 'b';
    const char nodeMarkChar = '>';
    const std::map<char, Token> oneCharTokens = {
        {'(', {openBracket, "("}}, {')', {closeBracket, ")"}}, {'{', {openConditionalNode,"{"}}, {'}', {closeConditionalNode, "}"}},
        {',', {seperator, ","}}, {';', {terminator, ";"}}, {nodeMarkChar, {nodeDelimiter, ">"}}, {'~', {queary, "~"}}, {'^', {queary, "^"}}
    };
    const std::unordered_set<char> whitespace= {' ', '\t'};
    const std::unordered_set<std::string> ignoredTokens = {"else"};

    bool isLastTokenType(std::vector<Token*>& tokens, token_type type){
        if(tokens.size() == 0)
            return false;
        return tokens.back()->type == type;
    }




    //Utility class so that one can consistantly get the next charcter in a multi-line passage passed in. Useful when lexing programs
    //Lets you treat a split string as one long string

    //In theory, this sould have just been one long buffer from a binary read file
    //But its been made, and it works, and I want to move on

    void MultiLineString::takeChar(){
        index += 1;
        if(index >= lines[line].size()){
            line += 1;
            index = 0;
        }
    }
    void MultiLineString::replaceChar(){
        if(index == 0){
            line -= 1;
            index = lines[line].size() - 1;
        }
        else
            index--;
    }
    char MultiLineString::peekChar(){
        return lines[line][index];
    }
    char MultiLineString::peekNextChar(){
        takeChar();
        if(atEnd())
            return '\0';
        char ans = peekChar();
        replaceChar();
        return ans;
    }
    char MultiLineString::peekBack(){
        replaceChar();
        char ans = peekChar();
        takeChar();
        return ans;
    }

    bool MultiLineString::atEnd(){
        return line >= lines.size();
    }

    void MultiLineString::skipLine(){
        line++;
        index = 0;
    }

    size_t MultiLineString::numOfLines(){
        return lines.size();
    }

    size_t MultiLineString::getLineOn(){
        return line + 1;
    }

    MultiLineString::MultiLineString(const std::vector<std::string>& inLines):
        line(0),
        index(0)
    {
        lines.reserve(inLines.size());
        for(auto& s : inLines)
            if(s == "")
                continue;
            else
                lines.emplace_back(s);
    }






    //Given a multi string token, a hint on what it may be, and a compiler environment, this function figures out the exact token type it is, and adds it
    //to the list of tokens
    void addLongToken(unsigned lineOn, bool& onLongToken, bool& seenFloatingPoint, std::string& currentString, token_type& longTokenType, std::vector<Token*>& tokens, CompilerEnvironment& compEnv){
        onLongToken = false;
        //Only keep looking at the token and adding it if it shouldn't be ignored
        if(ignoredTokens.count(currentString) == 0){
            if(longTokenType == floatingPoint)
                seenFloatingPoint = false;
            else if(longTokenType == userDefined){
                if(compEnv.functions.count(currentString) > 0)
                    longTokenType = function;
                else if(compEnv.quearies.count(currentString) > 0)
                    longTokenType = queary;
                else if(compEnv.variables.count(currentString) > 0)
                    longTokenType = variable;
                else if(currentString == "true" || currentString == "false")
                    longTokenType = boolean;
            }
        }
        tokens.emplace_back(new Token(longTokenType, std::move(currentString)));
        tokens[tokens.size()-1]->lineOn = lineOn;
        currentString = "";
        longTokenType = userDefined;
    }

    //Lexes a token that is one character long. Called with many references to lexer variables
    lexer_state handleOneCharToken(bool& onLongToken, bool& seenDecimalPoint, bool& inNode, MultiLineString& string, std::string& currentString, token_type& longTokenType, std::vector<Token*>& tokens, CompilerEnvironment& compEnv){
        if(onLongToken)
            addLongToken(string.getLineOn(), onLongToken, seenDecimalPoint, currentString, longTokenType, tokens, compEnv);
        //Edge case for node declerations
        if(string.peekChar() == '>'){
            char next = string.peekNextChar();
            if(next == '='){
                string.replaceChar();
                currentString = ">";
                longTokenType = userDefined;
            }
            else if(isLastTokenType(tokens, seperator)){
                tokens.emplace_back(new Token(queary, ">"));
                tokens[tokens.size()-1]->lineOn = string.getLineOn();
            }
            else{
                inNode = !inNode;
                tokens.emplace_back(new Token(nodeDelimiter, ">"));
                tokens[tokens.size()-1]->lineOn = string.getLineOn();
            }
        }
        else{

            tokens.emplace_back(new Token((*oneCharTokens.find(string.peekChar())).second));
            tokens[tokens.size()-1]->lineOn = string.getLineOn();
            if(string.peekChar() == nodeMarkChar)
                inNode = !inNode;
    
        }
        return inNode ? lookingInNode : lookingForNode;
    }
    
    //A switch statement to handle the progression of the lexer's finite state machine
    
    //This switch statement should probably be multiple functions, instead of one long one, but
    //again, this already works and I want to make something else closer to the game 
    
    void lexerSwitch(lexer_state& state, bool& onLongToken, MultiLineString& string, std::string& currentString, token_type& longTokenType, bool& seenDecimalPoint, std::vector<Token*>& tokens, CompilerEnvironment& compEnv){
        //It should be true that two multi character tokens are never directly adjacent, with no space or other token between
        //In fact, even having them with a space shouldn't be a thing. Therefore assume that the character being peeked at is part of
        //whatever character we were working on.
        switch(state){
        case lookingForNode:
            onLongToken = true;
            if( (string.peekChar() >= 'a' && string.peekChar() <='z') || 
                (string.peekChar() >= 'A' && string.peekChar() <= 'Z')){
                    state = readingLongToken;
                    currentString += string.peekChar();
            }
            else
                throw(new NC_Compiler_Invalid_Node_Token);
            break;
        case lookingInNode:
            onLongToken = true;
            currentString += string.peekChar();
            switch(string.peekChar()){
                case '\"': state = readingString; longTokenType = token_type::string; break;
                case baseMarker : state = readingNonBase10; longTokenType = token_type::integer; break;
                default:
                    if((string.peekChar() >= '1' && string.peekChar() <= '9') || string.peekChar() == '-'){
                        state = readingNumber;
                        longTokenType = token_type::integer;
                    }
                    else{
                        state = readingLongToken;
                    }
            }
            break; 

        //Split these us and do stuff
        case readingString:
            if(string.peekChar() == '"'){
                currentString += string.peekChar();
                addLongToken(string.getLineOn(), onLongToken, seenDecimalPoint, currentString, longTokenType, tokens, compEnv);
                state = lookingInNode;
                longTokenType = userDefined;
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
                        throw(new NC_Compiler_Illigal_Special_String_Token);
                }
            }
            else
                currentString += string.peekChar();
            break;
        case readingLongToken:
            currentString += string.peekChar();
            break;
        case readingNumber:
            if(string.peekChar() == '.'){
                if(seenDecimalPoint){
                    printf("%s + '%c'\n", currentString.c_str(), string.peekChar());
                    throw new(NC_Compiler_Invalid_Floating_Point_Token);
                }
                seenDecimalPoint = true;
                longTokenType = floatingPoint;
            }
            else if(string.peekChar() > '9' || string.peekChar() < '0')
                throw new(NC_Compiler_Invalid_Numeric_Token);
            currentString += string.peekChar();
            break;
        case readingNonBase10:
            switch(string.peekChar()){
                case binMarker:
                    state = readingBin;
                    break;
                case hexMarker:
                    state = readingHex;
                    break;
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                    state = readingOctal;
                    break;
                default:
                    throw(new NC_Compiler_Unknown_Numeric_Base);
            }
            currentString += string.peekChar();
            break;
        case readingOctal:
            if(string.peekChar() > '7' || string.peekChar() < '0')
                throw(new NC_Compiler_Invalid_Digit_For_Numeric_Base);
            currentString += string.peekChar();
            break;
        case readingBin:
            if(string.peekChar() != '1' && string.peekChar() != '0')
                throw(new NC_Compiler_Invalid_Digit_For_Numeric_Base);
            currentString += string.peekChar();
            break;
        case readingHex:
            if((string.peekChar() > '9' || string.peekChar() < '0') && (string.peekChar() >'F' || string.peekChar() < 'A'))
                throw(new NC_Compiler_Invalid_Digit_For_Numeric_Base);
            currentString += string.peekChar();
            break;
        default:
            break;
            
        }
            
    }

    std::vector<Token*> lexer(MultiLineString& string, CompilerEnvironment& compEnv){
        lexer_state state = lookingForNode;
        std::string currentString = "";
        std::vector<Token*> tokens;
        tokens.reserve(string.numOfLines() * 6); //Assming an average of a function, open,close brackets, semicolon and 2 arguments. Some will have more, some less
        bool inNode = false;
        bool onLongToken = false;
        bool seenDecimalPoint = false;
        token_type longTokenType = userDefined;

        while(!string.atEnd()){
            if(string.peekChar() == commentChar){
                string.skipLine();
                continue;
            }
            else if(whitespace.count(string.peekChar()) && state != readingString){
                if(onLongToken)
                    addLongToken(string.getLineOn(), onLongToken, seenDecimalPoint, currentString, longTokenType, tokens, compEnv);
            }
            else if(oneCharTokens.count(string.peekChar()) > 0 && state != readingString){
                state = handleOneCharToken(onLongToken, seenDecimalPoint, inNode, string, currentString, longTokenType, tokens, compEnv);
            }
            else{
                //It should be true that two multi character tokens are never directly adjacent, with no space or other token between
                //In fact, even having them with a space shouldn't be a thing. Therefore assume that the character being peeked at is part of
                //whatever character we were working on.
                lexerSwitch(state, onLongToken, string, currentString, longTokenType, seenDecimalPoint, tokens, compEnv);
            }

            string.takeChar();
        }

        return tokens;
    }


    //Adds a new argument to the given vector, based off the type and form of the given token in this environment
    void parseArgument(const Token& token, std::vector<NCArgument>& into, CompilerEnvironment& env){
        into.emplace_back();
        NCArgument& newArg = into[into.size() - 1];

        switch(token.type){
            case integer:
                newArg = std::atoi(token.form.c_str());
                break;
            case floatingPoint:
                newArg = float(std::atof(token.form.c_str()));
                break;
            case token_type::string:
                //Remove the starting and ending " chars
                newArg = token.form.substr(1, token.form.size() - 2);
                break;
            case token_type::boolean:
                newArg = token.form == "true" ? true : false;
                break;
            case userDefined:
                //Make it point to the same variable
                if(env.variables.count(token.form) > 0){
                    newArg = env.variables[token.form];
                }
                else if(env.quearies.count(token.form) > 0)
                    throw(new NC_Parser_Uncalled_Queary);
                else if(env.functions.count(token.form) > 0)
                    throw(new NC_Parser_Unexpected_Function_Call_Mid_Function);
                else{//Need to make a new variable!
                    newArg = env.getNewVariable(token.form);
                }
                break;
            default:
                printf("Argument token is -> ");
                token.print();
                throw(new NC_Parser_Invalid_Argument_Token);
        }
    }
    
    void parseQueary(const std::vector<Token*>& tokens, std::vector<NCArgument>& into, CompilerEnvironment& env);

    //Given a function call, gets the arguments until the close brackets
    void parseCall(const std::vector<Token*>& tokens, std::vector<NCArgument>& arguments, CompilerEnvironment& env, size_t& index){
        bool argumentTurn = true;
        while(tokens[index]->type != closeBracket){
            if(argumentTurn){
                if(tokens[index + 1]->type == openBracket){ //Parse this new queary
                    std::vector<Token*> subQuearyTokens;
                    subQuearyTokens.emplace_back(tokens[index]);
                    index++;
                    unsigned levelsDeep = 0;
                    do{
                        if(tokens[index]->type == openBracket)
                            levelsDeep++;
                        else if(tokens[index]->type == closeBracket)
                            levelsDeep--;
                        subQuearyTokens.emplace_back(tokens[index]);
                        index++;
                        if(index == tokens.size())
                            throw(new NC_Parser_Unexpected_EOP);
                    }while(levelsDeep != 0);
                    //Will be incrimented at the bottom, so decriment here to keep the ','
                    index--;

                    parseQueary(subQuearyTokens, arguments, env);
                }
                else
                    parseArgument(*tokens[index], arguments, env);
                argumentTurn = false;
            }
            else{
                //Put a comma or something else, make sure next token is an argument
                if(tokens[index]->type == seperator)
                    argumentTurn = true;
                else{    //Every argument should be seperated
                    tokens[index]->print();
                    throw(new NC_Parser_Expected_Comma);
                }
            }

            index++;
        }
        //You can't end a call with ",)". You can however, call using ()
        if(argumentTurn && (tokens[index]->type != closeBracket)){
            throw(new NC_Parser_Invalid_Arguments_In_Call);
        }
        //Move it past the close brackets
        index++;
    }
    
    //Given a string of tokens, create your new queary argument
    void parseQueary(const std::vector<Token*>& tokens, std::vector<NCArgument>& into, CompilerEnvironment& env){
        if(tokens.size() < 3)//Tokens for:  Queary_type  (  )    Thats 3. Maybe one won't have an argument as its a c++ constant? Or static function?
            throw(new NC_Parser_Invalid_Arguments_In_Call);

        into.emplace_back();
        NCArgument& newQueary = into[into.size() - 1];
        NCQueary* thisQueary = new NCQueary;
        //Register this queary for deletion at the end
        env.addQueary(thisQueary);
        newQueary = thisQueary;

        thisQueary->func = env.quearies[tokens[0]->form];
        if(tokens[1]->type != openBracket)
            throw(new NC_Parser_Expected_Open_Bracket);
        

        size_t index = 2;
        parseCall(tokens, thisQueary->arguments, env, index);
        //And now the queary is finished
    }

    uint16_t parseNodeAndGetIndex(const std::string& name, const std::vector<Token*>& tokens, CompilerEnvironment& env, std::vector<std::vector<NCFunction>>& nodes, 
                                    size_t& index, token_type layerEnterType = nullToken, token_type layerExitType = nodeDelimiter);

    void parseFunction(const std::vector<Token*>& tokens, CompilerEnvironment& env, unsigned thisNodeIndex, std::vector<std::vector<NCFunction>>& nodes){
        auto& functions = nodes[thisNodeIndex];

        functions.emplace_back();
        NCFunction& thisFunc = functions[functions.size() - 1];
        thisFunc.func = env.functions[tokens[0]->form];
        thisFunc.lineNumber = tokens[0]->lineOn;
        std::string nodeName = "CN-"+std::to_string(thisFunc.lineNumber);
        if(tokens[1]->type != openBracket)
            throw(new NC_Parser_Expected_Open_Bracket);
        

        size_t index = 2;
        parseCall(tokens, thisFunc.arguments, env, index);

        //Well, now we either have out function done, or a conditional
        if(tokens[index]->type != terminator){
            if(tokens[index]->type == openConditionalNode){
                unsigned numOfNodes = 0;
                //What if it has more!!! The answer is that you keep on keep going on
                while(tokens[index]->type == openConditionalNode){
                    thisFunc.arguments.emplace_back();
                    index++;
                    std::string thisName = nodeName + "-" + std::to_string(numOfNodes);
                    numOfNodes++;
                    thisFunc.arguments[thisFunc.arguments.size() - 1] = parseNodeAndGetIndex(thisName, tokens, env, nodes, index, openConditionalNode, closeConditionalNode);

                    //Don't crash the while loop, just break here
                    if(index == tokens.size())
                        break;
                }
            }
            else
                throw(new NC_Parser_Missing_Terminator);
        }

    }

    uint16_t parseNodeAndGetIndex(const std::string& name, const std::vector<Token*>& tokens, CompilerEnvironment& env, std::vector<std::vector<NCFunction>>& nodes, size_t& index, token_type layerEnterType, token_type layerExitType){
        std::vector<Token*> thisNode;
        unsigned deep = 1;
        while(deep > 0){
            if(tokens[index]->type == layerEnterType)
                deep++;
            else if(tokens[index]->type == layerExitType)
                deep--;
            thisNode.emplace_back(tokens[index]);
            index++;
        }
        //Remove the last limiter
        thisNode.pop_back();

        uint16_t thisNodeIndex = env.getNode(name);

        unsigned i = 0;
        //Get the node this will write its functions to
        std::vector<Token*> thisCall;
        bool onConditionals = false;
        
        //The final closing token was already removed
        while(i < thisNode.size()){
            thisCall.emplace_back(thisNode[i]);
            if(thisNode[i]->type == terminator){
                if(!onConditionals){
                    parseFunction(thisCall, env, thisNodeIndex, nodes);
                    thisCall.clear();
                }
            }
            else if(thisNode[i]->type == openConditionalNode)
                onConditionals = true;
            else if(thisNode[i]->type == closeConditionalNode){
                bool calling = true;
                if(i != (thisNode.size() - 1))
                    if(thisNode[i+1]->type == openConditionalNode)
                        calling = false;
                //Unless there is another conditional right away, just continue onwards
                if(calling){
                    parseFunction(thisCall, env, thisNodeIndex, nodes);
                    thisCall.clear();
                    onConditionals = false;
                }
            }
            i++;
        }

        std::vector<NCFunction>& node = nodes[thisNodeIndex];
        //If its a conditional, it needs to break at the end. 
        //Should WHILE and DO_WHILE ever get added, the logic for adding conditionl breaks at the end would go here
        node.emplace_back();
        NCFunction& finalFunc = node.back();
        finalFunc.lineNumber = 0xBAADF00D;
        if(layerExitType == closeConditionalNode)
            finalFunc.func = ops::break_from;
        else
            finalFunc.func = ops::return_from;


        return thisNodeIndex;
    }

    void parser(const std::vector<Token*>& tokens, CompilerEnvironment& environment, std::vector<std::vector<NCFunction>>& nodes){
        size_t index = 0;
        while(index < tokens.size()){
            if(tokens[index]->type == userDefined){
                std::string nodeName = tokens[index]->form;
                index++;
                if(tokens[index]->type == nodeDelimiter){
                    //Scooch past the delimiter
                    index++;
                    parseNodeAndGetIndex(nodeName, tokens, environment, nodes, index);
                }
                else
                    throw(new NC_Parser_Expected_Node_Start);
            }
            else
                throw(new NC_Parser_Expected_Node);
        }
    }




    NCRuntime* CompilerEnvironment::compile(const std::vector<std::string>& lines){
        MultiLineString source(lines);

        auto tokens = lexer(source, *this);

        parser(tokens, *this, this->runtime->nodes);

        for(auto t : tokens)
            delete t;

        return this->runtime;
    }

}
}



NCRuntime* NodeCall::compile(std::ifstream& file, const std::initializer_list<NCExtention*>& extentions){
    std::vector<std::string> source;
    std::string line;
    while(std::getline(file, line))
        source.emplace_back(std::move(line));
    
    NCExtention* baselineNodeCallFunctionality = new NCExtention(
        ncprivate::standard_functions,
        ncprivate::standard_quearies,
        std::map<std::string, std::any*>{}
    );
    ncprivate::compiler::CompilerEnvironment env(new NCRuntime);
    env.addExtention(*baselineNodeCallFunctionality);
    for(auto& e : extentions)
        env.addExtention(*e);

    auto runtime = env.compile(source);

    delete baselineNodeCallFunctionality;
    return runtime;
}