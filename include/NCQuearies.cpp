#include "NCQuearies.hpp"

#define qdef(name) std::any name (std::vector<NCArgument>& args, NC_Runtime_Log& runLog, unsigned lineNumber)
namespace ncprivate{
namespace queary{
    qdef(add){
        std::any& arg1 = args[0].getValue(runLog);
        std::any& arg2 = args[1].getValue(runLog);
        std::any answer;

        if(arg1.type() == typeid(int)){
            answer = std::any_cast<int>(arg1) + std::any_cast<int>(arg2);
        }
        else if(arg1.type() == typeid(float)){
            answer = std::any_cast<float>(arg1) + std::any_cast<float>(arg2);
        }
        else if(arg1.type() == typeid(std::string)){
            answer = std::any_cast<std::string>(arg1) + std::any_cast<std::string>(arg2);
        }
        else
            runLog.addLog(ncras_Severe_Error, lineNumber, "Cannot perform addition on non identical types, not of int, float or string");

        return answer;
    }
    
    #define qdef_int_float_maths(NAME, OP, DETAIL)\
    std::any NAME (std::vector<NCArgument>& args, NC_Runtime_Log& runLog, unsigned lineNumber){   \
        std::any& arg1 = args[0].getValue(runLog);    \
        std::any& arg2 = args[1].getValue(runLog);    \
        std::any answer;    \
        \
        if(arg1.type() == typeid(int)){ \
            answer = std::any_cast<int>(arg1) OP std::any_cast<int>(arg2);  \
        }   \
        else if(arg1.type() == typeid(float)){  \
            answer = std::any_cast<float>(arg1) OP std::any_cast<float>(arg2);  \
        }   \
        else{    \
            runLog.addLog(ncras_Severe_Error, lineNumber, "Cannot " #DETAIL " two non identical, numeric types");\
        }\
        \
        return answer;  \
    }

    qdef_int_float_maths(sub, -, subtract)
    qdef_int_float_maths(mul, *, perform multiplication)
    qdef_int_float_maths(div, /, perform division)

    #define qdef_int_logic(NAME, OP, DETAIL)    \
    std::any NAME (std::vector<NCArgument>& args, NC_Runtime_Log& runLog, unsigned lineNumber){ \
        std::any& arg1 = args[0].getValue(runLog);    \
        std::any& arg2 = args[1].getValue(runLog);    \
        std::any answer;    \
        if(arg1.type() == typeid(int)){ \
            answer = std::any_cast<int>(arg1) OP std::any_cast<int>(arg2);   \
        }   \
        else    \
            runLog.addLog(ncras_Severe_Error, lineNumber, "Cannot perform " #DETAIL " operation on non integer types"); \
        \
        return answer;  \
    }
    qdef_int_logic(mod, %, modulo)
    qdef_int_logic(rshift, >>, bitwise right shift)
    qdef_int_logic(lshift, <<, bitwise left shift)
    
    
    qdef(land){
        std::any& arg1 = args[0].getValue(runLog);
        std::any& arg2 = args[1].getValue(runLog);
        std::any answer;
        if(arg1.type() == typeid(int)){
            answer = std::any_cast<int>(arg1) & std::any_cast<int>(arg2);
        }
        else if(arg1.type() == typeid(bool)){
            answer = std::any_cast<bool>(arg1) & std::any_cast<bool>(arg2);
        }
        else
            runLog.addLog(ncras_Severe_Error, lineNumber, "Cannot perform logical and operation on non integer or boolean types");

        return answer;
    }
    qdef(lor){
        std::any& arg1 = args[0].getValue(runLog);
        std::any& arg2 = args[1].getValue(runLog);
        std::any answer;
        if(arg1.type() == typeid(int)){
            answer = std::any_cast<int>(arg1) | std::any_cast<int>(arg2);
        }
        else if(arg1.type() == typeid(bool)){
            answer = std::any_cast<bool>(arg1) | std::any_cast<bool>(arg2);
        }
        else
            runLog.addLog(ncras_Severe_Error, lineNumber, "Cannot perform logical or operation on non integer or boolean types");

        return answer;
    }
    qdef(lnot){
        std::any& arg1 = args[0].getValue(runLog);
        std::any answer;
        if(arg1.type() == typeid(int)){
            answer = !std::any_cast<int>(arg1);
        }
        else if(arg1.type() == typeid(bool)){
            answer = !std::any_cast<bool>(arg1);
        }
        else
            runLog.addLog(ncras_Severe_Error, lineNumber, "Cannot perform logical not operation on non integer or boolean types");
            

        return answer;
    }
    qdef(lxor){
        std::any& arg1 = args[0].getValue(runLog);
        std::any& arg2 = args[1].getValue(runLog);
        std::any answer;
        if(arg1.type() == typeid(int)){
            answer = std::any_cast<int>(arg1) ^ std::any_cast<int>(arg2);
        }
        else if(arg1.type() == typeid(bool)){
            answer = std::any_cast<bool>(arg1) ^ std::any_cast<bool>(arg2);
        }
        else
            runLog.addLog(ncras_Severe_Error, lineNumber, "Cannot perform logical xor operation on non integer or boolean types");

        return answer;
    }
    qdef(band){
        std::any& arg1 = args[0].getValue(runLog);
        std::any& arg2 = args[1].getValue(runLog);
        std::any answer;
        if(arg1.type() == typeid(int)){
            answer = (std::any_cast<int>(arg1) > 0) && (std::any_cast<int>(arg2) > 0);
        }
        else if(arg1.type() == typeid(float)){
            answer = (std::any_cast<float>(arg1) > 0) && (std::any_cast<float>(arg2) > 0);
        }
        else if(arg1.type() == typeid(bool)){
            answer = std::any_cast<bool>(arg1) && std::any_cast<bool>(arg2);
        }
        else
            runLog.addLog(ncras_Severe_Error, lineNumber, "Attempted to boolean and on non identical, or non supported types");

        return answer;
    }
    qdef(bor){
        std::any& arg1 = args[0].getValue(runLog);
        std::any& arg2 = args[1].getValue(runLog);
        std::any answer;
        if(arg1.type() == typeid(int)){
            answer = (std::any_cast<int>(arg1) > 0) || (std::any_cast<int>(arg2) > 0);
        }
        else if(arg1.type() == typeid(float)){
            answer = (std::any_cast<float>(arg1) > 0) || (std::any_cast<float>(arg2) > 0);
        }
        else if(arg1.type() == typeid(bool)){
            answer = std::any_cast<bool>(arg1) || std::any_cast<bool>(arg2);
        }
        else
            runLog.addLog(ncras_Severe_Error, lineNumber, "Attempted to boolean or on non identical, or non supported types");

        return answer;
    }
    qdef(bnot){
        std::any& arg1 = args[0].getValue(runLog);
        std::any answer;
        if(arg1.type() == typeid(int)){
            answer = (std::any_cast<int>(arg1) > 0);
        }
        else if(arg1.type() == typeid(float)){
            answer = (std::any_cast<float>(arg1) > 0);
        }
        else if(arg1.type() == typeid(bool)){
            answer = std::any_cast<bool>(arg1);
        }
        else
            runLog.addLog(ncras_Severe_Error, lineNumber, "Attempted to boolean not on non supported types");

        return answer;
    }
    
#define qopdef(name, OP, TYPENAME) std::any name (std::vector<NCArgument>& args, NC_Runtime_Log& runLog, unsigned lineNumber){ \
        std::any& a1 = args[0].getValue(runLog);    \
        std::any& a2 = args[1].getValue(runLog);    \
        std::any answer;    \
        if(a1.type() != a2.type())  \
            answer = false; \
        else if( a1.type() == typeid( int ) ) answer = std::any_cast< int >( a1 ) OP std::any_cast< int >( a2 );    \
        else if( a1.type() == typeid( float ) ) answer = std::any_cast< float >( a1 ) OP std::any_cast< float >( a2 );  \
        else if( a1.type() == typeid( std::string ) ) answer = std::any_cast< std::string >( a1 ) OP std::any_cast< std::string >( a2 );    \
        else if( a1.type() == typeid( bool ) ) answer = std::any_cast< bool >( a1 ) OP std::any_cast< bool >( a2 );   \
        else if( a1.type() == typeid( std::type_index ) ) answer = std::any_cast< std::type_index >( a1 ) OP std::any_cast< std::type_index >( a2 );    \
        else    \
            runLog.addLog(ncras_Severe_Error, lineNumber, "Attempted to boolean " #TYPENAME " on non identical, or not supported types");  \
    \
       return answer;\
    }

    qopdef(beq, ==, equals)
    qopdef(bneq, !=, not equals)
    qopdef(bless, <, less than)
    qopdef(bgreater, >, greater than)
    qopdef(blesseq, <=, less than or equal)
    qopdef(bgreatereq, >=, greater than or equal)
#undef qopdef

    qdef(size){
        std::any& arg1 = args[0].getValue(runLog);
        std::any answer;
        if(arg1.type() == typeid(std::vector<std::any>))
            answer = std::any_cast<std::vector<std::any>>(&arg1)->size();
        else{
            runLog.addLog(ncras_Badly_Formed_Function, lineNumber, "Called 'size' on non vector argument; defaulted to 0");
            answer = 0;
        }

        return answer;
    }
    qdef(index){
        std::any& arg1 = args[0].getValue(runLog);
        std::any& arg2 = args[1].getValue(runLog);
        std::any answer;
        if(arg2.type() == typeid(int)){
            if(arg1.type() == typeid(std::vector<std::any>)){
                auto& vec = *std::any_cast<std::vector<std::any>>(&arg1);
            }
            else{
                runLog.addLog(ncras_Badly_Formed_Function, lineNumber, "Non-vector objects are not subscriptable");
                answer = 0;
            }
        }
        else{
            runLog.addLog(ncras_Badly_Formed_Function, lineNumber, "Cannot get a vector element with a non-integer index; defaulted to 0");
            answer = 0;
        }

        return answer;
    }

    #define qdef_vec_op(NAME, CARRYOUTOP)   \
    std::any NAME (std::vector<NCArgument>& args, NC_Runtime_Log& runLog, unsigned lineNumber){   \
        std::any& arg1 = args[0].getValue(runLog);    \
        std::any answer;    \
        if(arg1.type() == typeid(std::vector<std::any>)){   \
            auto& vec = *std::any_cast<std::vector<std::any>>(&arg1);   \
            if(vec.size() == 0){    \
                runLog.addLog(ncras_Severe_Error, lineNumber, "Called '" #NAME "' on vector with size 0");    \
                answer = 0; \
            }   \
            else{   \
                CARRYOUTOP ;  \
            }   \
        }   \
        else{   \
            runLog.addLog(ncras_Badly_Formed_Function, lineNumber, "Called '" #NAME "' on non vector argument; defaulted to 0");  \
            answer = 0; \
        }   \
        \
        return answer;  \
    }

    qdef_vec_op(pop, answer = vec.back(); vec.pop_back())
    qdef_vec_op(front, answer = vec[0])
    qdef_vec_op(tail, answer = vec[0])

    qdef(input){
        std::any answer;
        
        std::string in;
        std::cin >> in;

        answer.emplace<std::string>(std::move(in));

        return answer;
    }

    qdef(getType){
        std::any& arg1 = args[0].getValue(runLog);
        std::any answer;
        answer = std::type_index(arg1.type());
        return answer;
    }

        #define qdef_type_func(NAME, TYPE, INTCASE, FLOATCASE, BOOLCASE, STRINGCASE)  \
    std::any NAME (std::vector<NCArgument>& args, NC_Runtime_Log& runLog, unsigned lineNumber){ \
        std::any& arg1 = args[0].getValue(runLog);    \
        std::any answer;    \
        if(arg1.type() == typeid(TYPE)){ \
            runLog.addLog(ncras_Point_Of_Note, lineNumber, "In " #TYPE ", no need for type conversion");    \
            answer = arg1;  \
        }   \
        else if(arg1.type() == typeid(int)){    \
            answer = INTCASE ;\
        }   \
        else if(arg1.type() == typeid(float)){    \
            answer = FLOATCASE ;\
        }   \
        else if(arg1.type() == typeid(bool)){    \
            answer = BOOLCASE ;\
        }   \
        else if(arg1.type() == typeid(std::string)){    \
            answer = STRINGCASE ;\
        }   \
        else{   \
            runLog.addLog(ncras_Severe_Error, lineNumber, "In " #TYPE ", cannot convert between non base types");    \
            printf("Bad type '%s'!\n", arg1.type().name()); \
            answer = TYPE ();   \
        }   \
        return answer;   \
    }

    qdef_type_func(tfint,    int,         0,    int(std::any_cast<float>(arg1)), int(std::any_cast<bool>(arg1)), std::atoi(std::any_cast<std::string>(arg1).c_str()))
    qdef_type_func(tffloat,  float,       float(std::any_cast<int>(arg1)),  0.0, float(int(std::any_cast<bool>(arg1))), std::atof(std::any_cast<std::string>(arg1).c_str()))
    qdef_type_func(tfbool,   bool,        std::any_cast<int>(arg1) != 0, std::any_cast<float>(arg1)  > 0.0, false, std::any_cast<std::string>(arg1) == "true"  ? true : false); //Maybe in future log if its neither
    qdef_type_func(tfstring, std::string, std::to_string(std::any_cast<int>(arg1)), std::to_string(std::any_cast<float>(arg1)), std::to_string(std::any_cast<bool>(arg1)), "")
}

    const std::map<std::string, NCQuearyFunc> standard_quearies = {
        {"add", queary::add}, {"sub", queary::sub}, {"mul", queary::mul}, {"div", queary::div}, {"mod",queary::mod}, {">>", queary::rshift}, {"<<", queary::lshift},
        {"&", queary::land}, {"|", queary::lor}, {"~", queary::lnot}, {"^", queary::lxor}, {"&&", queary::band}, {"||", queary::bor}, {"!",queary::bnot},
        {"==",queary::beq}, {"!=",queary::bneq}, {"<",queary::bless}, {">",queary::bgreater}, {"<=",queary::blesseq}, {">=",queary::bgreatereq},
        {"size",queary::size}, {"index",queary::index}, {"pop",queary::pop}, {"front",queary::front}, {"tail",queary::tail},
        {"input",queary::input},
        {"typeof",queary::getType}, {"int",queary::tfint}, {"float",queary::tffloat}, {"bool",queary::tfbool}, {"string",queary::tfstring}
    };

}
#undef qdef
#undef qdef_int_float_maths
#undef qdef_int_logic
#undef qdef_vec_op
#undef qdef_type_func
