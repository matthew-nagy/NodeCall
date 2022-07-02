#include "Standard_Library.hpp"


namespace nc{   namespace stlib{

//Definitions of all quearies.
//What they do should be selfexplanatory
//The non qdef functions are utility functions used throughout

//One way this could be cleaned up is making macros for fetching data types from a value pointer

bool is_true(argument& val, unique_run_resource& runResource){
    value arg1 = val.getValue(runResource);
    if(arg1->type() == typeid(int)){
        return std::any_cast<int>(*arg1) > 0;
    }
    else if(arg1->type() == typeid(float)){
        return std::any_cast<float>(*arg1) > 0.0;
    }
    else if(arg1->type() == typeid(bool)){
        return std::any_cast<bool>(*arg1);
    }
    else if(arg1->type() == typeid(std::string)){
        return std::any_cast<std::string>(*arg1) != "";
    }
    throw(new INVALID_ARGUMENT_TYPE);
}

namespace q{
    //Mathmatic quearies
    qdef(add){
        value arg1 = args[0].getValue(runResource);
        if(arg1->type() == typeid(int)){
            int tot = std::any_cast<int>(*arg1.get());
            for(size_t i = 1; i < args.size(); i++)
                tot+=std::any_cast<int>(*args[i].getValue(runResource).get());
            return makeVal<int>(tot);
        }
        else if(arg1->type() == typeid(float)){
            float tot = std::any_cast<float>(*arg1.get());
            for(size_t i = 1; i < args.size(); i++)
                tot+=std::any_cast<float>(*args[i].getValue(runResource).get());
            return makeVal<float>(tot);
        }
        else if(arg1->type() == typeid(std::string)){
            std::string tot = std::any_cast<std::string>(*arg1.get());
            for(size_t i = 1; i < args.size(); i++)
                tot+=std::any_cast<std::string>(*args[i].getValue(runResource).get());
                //You can move this one!
            return moveVal<std::string>(std::move(tot));
        }
        throw(new INVALID_ARGUMENT_TYPE);
    }
    qdef(sub){
        value arg1 = args[0].getValue(runResource);
        if(arg1->type() == typeid(int)){
            int tot = std::any_cast<int>(*arg1.get());
            for(size_t i = 1; i < args.size(); i++)
                tot-=std::any_cast<int>(*args[i].getValue(runResource).get());
            return makeVal<int>(tot);
        }
        else if(arg1->type() == typeid(float)){
            float tot = std::any_cast<float>(*arg1.get());
            for(size_t i = 1; i < args.size(); i++)
                tot-=std::any_cast<float>(*args[i].getValue(runResource).get());
            return makeVal<float>(tot);
        }
        throw(new INVALID_ARGUMENT_TYPE);
    }
    qdef(div){
        value arg1 = args[0].getValue(runResource);
        if(arg1->type() == typeid(int)){
            int tot = std::any_cast<int>(*arg1.get());
            for(size_t i = 1; i < args.size(); i++)
                tot/=std::any_cast<int>(*args[i].getValue(runResource).get());
            return makeVal<int>(tot);
        }
        else if(arg1->type() == typeid(float)){
            float tot = std::any_cast<float>(*arg1.get());
            for(size_t i = 1; i < args.size(); i++)
                tot/=std::any_cast<float>(*args[i].getValue(runResource).get());
            return makeVal<float>(tot);
        }
        throw(new INVALID_ARGUMENT_TYPE);
    }
    qdef(mul){
        value arg1 = args[0].getValue(runResource);
        if(arg1->type() == typeid(int)){
            int tot = std::any_cast<int>(*arg1.get());
            for(size_t i = 1; i < args.size(); i++)
                tot*=std::any_cast<int>(*args[i].getValue(runResource).get());
            return makeVal<int>(tot);
        }
        else if(arg1->type() == typeid(float)){
            float tot = std::any_cast<float>(*arg1.get());
            for(size_t i = 1; i < args.size(); i++)
                tot*=std::any_cast<float>(*args[i].getValue(runResource).get());
            return makeVal<float>(tot);
        }
        printf("%s\n", arg1->type().name());
        throw(new INVALID_ARGUMENT_TYPE);
    }
    qdef(mod){
        int modResult = std::any_cast<int>(*args[0].getValue(runResource).get()) % std::any_cast<int>(*args[1].getValue(runResource).get());
        return makeVal<int>(modResult);
    }
    qdef(square_root){
        value arg = args[0].getValue(runResource);
        if(arg->type() == typeid(int)){
            return makeVal<int>(std::sqrt(std::any_cast<int>(*arg.get())));
        }
        else if(arg->type() == typeid(float)){
            return makeVal<float>(std::sqrt(std::any_cast<float>(*arg.get())));
        }
        throw(new INVALID_ARGUMENT_TYPE);
    }


    //Logical quearies
    qdef(lshift){
        int argl = std::any_cast<int>(*args[0].getValue(runResource));
        int argr = std::any_cast<int>(*args[1].getValue(runResource));
        return makeVal<int>(argl << argr);
    }
    qdef(rshift){
        int argl = std::any_cast<int>(*args[0].getValue(runResource));
        int argr = std::any_cast<int>(*args[1].getValue(runResource));
        return makeVal<int>(argl >> argr);
    }
    qdef(land){
        value arg1 = args[0].getValue(runResource);
        if(arg1->type() == typeid(int)){
            int argl = std::any_cast<int>(*arg1);
            int argr = std::any_cast<int>(*args[1].getValue(runResource));
            return makeVal<int>(argl & argr);
        }
        throw(new INVALID_ARGUMENT_TYPE);
    }
    qdef(lor){
        value arg1 = args[0].getValue(runResource);
        if(arg1->type() == typeid(int)){
            int argl = std::any_cast<int>(*arg1);
            int argr = std::any_cast<int>(*args[1].getValue(runResource));
            return makeVal<int>(argl | argr);
        }
        throw(new INVALID_ARGUMENT_TYPE);
    }
    qdef(lnot){
        value arg1 = args[0].getValue(runResource);
        if(arg1->type() == typeid(int)){
            int argl = std::any_cast<int>(*arg1);
            return makeVal<int>(~argl);
        }
        else if(arg1->type() == typeid(float)){
            return makeVal<bool>(std::any_cast<float>(*args[0].getValue(runResource))  == 0.0);
        }
        else if(arg1->type() == typeid(bool)){
            return makeVal<bool>(!std::any_cast<float>(*args[0].getValue(runResource)));
        }
        throw(new INVALID_ARGUMENT_TYPE);
    }
    qdef(lxor){
        value arg1 = args[0].getValue(runResource);
        if(arg1->type() == typeid(int)){
            int argl = std::any_cast<int>(*arg1);
            int argr = std::any_cast<int>(*args[1].getValue(runResource));
            return makeVal<int>(argl ^ argr);
        }
        else if(arg1->type() == typeid(bool)){
            bool argl = std::any_cast<bool>(*arg1);
            bool argr = std::any_cast<bool>(*args[1].getValue(runResource));
            return makeVal<bool>(argl ^ argr);
        }
        throw(new INVALID_ARGUMENT_TYPE);
    }


    //Binary quearies
    qdef(band){
        for(size_t i = 0; i < args.size(); i++)
            if(! is_true(args[i], runResource))
                return makeVal<bool>(false);
        return makeVal<bool>(true);
    }
    qdef(bor){
        for(size_t i = 0; i < args.size(); i++)
            if(is_true(args[i], runResource))
                return makeVal<bool>(true);
        return makeVal<bool>(false);
    }

    bool areArgsEqual(argument& arg1, argument& arg2, unique_run_resource& runResource){
        value v1 = arg1.getValue(runResource);
        value v2 = arg2.getValue(runResource);
        if(v1->type() == typeid(int)){
            return std::any_cast<int>(*v1) == std::any_cast<int>(*v2);
        }
        else if(v1->type() == typeid(float)){
            return std::any_cast<float>(*v1) == std::any_cast<float>(*v2);
        }
        else if(v1->type() == typeid(bool)){
            return std::any_cast<bool>(*v1) == std::any_cast<bool>(*v2);
        }
        else if(v1->type() == typeid(std::string)){
            return std::any_cast<std::string>(*v1) == std::any_cast<std::string>(*v2);
        }
        else if(v1->type() == typeid(std::type_index)){
            return std::any_cast<std::type_index>(*v1) == std::any_cast<std::type_index>(*v2);
        }
        throw(new INVALID_ARGUMENT_TYPE);
    }
    qdef(beq){
        return makeVal<bool>(areArgsEqual(args[0], args[1], runResource));
    }
    qdef(bneq){
        return makeVal<bool>(!areArgsEqual(args[0], args[1], runResource));
    }
    
    //TO DO
    bool leftLessRight(argument& arg1, argument& arg2, unique_run_resource& runResource){
        value v1 = arg1.getValue(runResource);
        value v2 = arg2.getValue(runResource);

        if(v1->type() == typeid(int)){
            return std::any_cast<int>(*v1) < std::any_cast<int>(*v2);
        }
        else if(v1->type() == typeid(float)){
            return std::any_cast<float>(*v1) < std::any_cast<float>(*v2);
        }
        throw(new INVALID_ARGUMENT_TYPE);
    }
    bool leftMoreRight(argument& arg1, argument& arg2, unique_run_resource& runResource){
        value v1 = arg1.getValue(runResource);
        value v2 = arg2.getValue(runResource);

        if(v1->type() == typeid(int)){
            return std::any_cast<int>(*v1) > std::any_cast<int>(*v2);
        }
        else if(v1->type() == typeid(float)){
            return std::any_cast<float>(*v1) > std::any_cast<float>(*v2);
        }
        throw(new INVALID_ARGUMENT_TYPE);}

    qdef(bless){
        return makeVal<bool>(leftLessRight(args[0], args[1], runResource));
    }
    qdef(bgreater){
        return makeVal<bool>(leftMoreRight(args[0], args[1], runResource));
    }
    qdef(blesseq){
        return makeVal<bool>(!leftMoreRight(args[0], args[1], runResource));
    }
    qdef(bgreatereq){
        return makeVal<bool>(!leftLessRight(args[0], args[1], runResource));
    }


    //list quearies
    std::shared_ptr<std::vector<std::any>> getAnyVecPtr(argument& arg, unique_run_resource& runResource){
        return std::any_cast<std::shared_ptr<std::vector<std::any>>>(*arg.getValue(runResource));
    }

    qdef(list_create){
        return std::make_shared<std::any>(std::make_any<std::vector<std::any>>());
    }
    qdef(list_size){
        return makeVal<int>(getAnyVecPtr(args[0], runResource)->size());
    }
    qdef(list_index){
        int index = std::any_cast<int>(*args[1].getValue(runResource));
        return std::make_shared<std::any>((*getAnyVecPtr(args[0], runResource))[index]);
    }
    qdef(list_pop){
        std::shared_ptr<std::vector<std::any>> vec = getAnyVecPtr(args[0], runResource);
        value toret = std::make_shared<std::any>(vec->back());
        vec->pop_back();
        return toret;
    }
    qdef(list_pop_front){
        std::shared_ptr<std::vector<std::any>> vec = getAnyVecPtr(args[0], runResource);
        value toret = std::make_shared<std::any>(vec->front());
        vec->erase(vec->begin());
        return toret;
    }
    qdef(list_front){
        std::shared_ptr<std::vector<std::any>> vec = getAnyVecPtr(args[0], runResource);
        value toret = std::make_shared<std::any>(vec->front());
        return toret;
    }
    qdef(list_tail){
        std::shared_ptr<std::vector<std::any>> vec = getAnyVecPtr(args[0], runResource);
        value toret = std::make_shared<std::any>(vec->back());
        return toret;
    }


    //IO quearies
    qdef(input){
        std::string userInput = "";
        std::getline(std::cin, userInput);
        return moveVal<std::string>(std::move(userInput));
    }


    //Type quearies and conversions
    qdef(getType){
        return makeVal<std::type_index>(args[0].getValue(runResource)->type());
    }

#define ConvFunc(type, wantedType, functionStart, functionEnd) return makeVal< wantedType > ( functionStart( std::any_cast<type>(*args[0].getValue(runResource)) functionEnd ))

    qdef(ftoi){
        ConvFunc(float, int, int , );
    }
    qdef(stoi){
        ConvFunc(std::string, int, std::atoi, .c_str());
    }
    qdef(btoi){
        ConvFunc(bool, int, int, );
    }

    qdef(itof){
        ConvFunc(int, float, float, );
    }
    qdef(btof){
        ConvFunc(bool, float, float, );
    }
    qdef(stof){
        ConvFunc(std::string, float, std::atof, .c_str());
    }

    bool stob(const std::string& s){
        return s == "true";
    }

    qdef(itob){
        ConvFunc(int, bool, bool, );
    }
    qdef(ftob){
        ConvFunc(float, bool, bool, );
    }
    qdef(stob){
        ConvFunc(std::string, bool, stob, );
    }

    qdef(itos){
        ConvFunc(int, std::string, std::to_string, );
    }
    qdef(ftos){
        ConvFunc(float, std::string, std::to_string, );
    }
    qdef(btos){
        ConvFunc(bool, std::string, ([](bool myBool) {return myBool ? "true" : "false"; }), );
    }
    qdef(ttos){
        return makeVal<std::string>(std::any_cast<std::type_index>(*args[0].getValue(runResource)).name());
    }
}


    //The names of all the quaries, and their function pointers
    const QuearyTable _standard_quearies = {
        {"add", q::add}, {"sub", q::sub}, {"mul", q::mul}, {"div", q::div}, {"mod", q::mod}, {"sqrt", q::square_root},
        {"+", q::add}, {"-", q::sub}, {"*", q::mul}, {"/", q::div}, {"%", q::mod},

        {"lshift", q::lshift}, {"rshift", q::rshift},
        {"<<", q::lshift}, {">>", q::rshift}, {"&", q::land}, {"|", q::lor}, {"!", q::lnot}, {"^", q::lxor},

        {"and", q::band}, {"or", q::bor}, {"equals", q::beq}, {"not_equals", q::bneq}, {"not", q::lnot},
        {"&&", q::band}, {"||", q::bor}, {"==", q::beq}, {"!=", q::bneq},

        {"less", q::bless}, {"greater", q::bgreater}, {"less_eq", q::blesseq}, {"greater_eq", q::bgreatereq},
        {"<", q::bless}, {">", q::bgreater}, {"<=", q::blesseq}, {">=", q::bgreatereq},

        {"list_create", q::list_create}, {"list_size", q::list_size}, {"list_index", q::list_index}, {"list_pop", q::list_pop},
        {"list_pop_front", q::list_pop_front}, {"list_front", q::list_front}, {"list_tail", q::list_tail},
        {"input", q::input},
        {"typeof", q::getType},
        {"foti", q::ftoi}, {"stoi", q::stoi}, {"btoi", q::btoi},
        {"itof", q::itof}, {"btof", q::btof}, {"stof", q::stof},
        {"itob", q::itob}, {"ftob", q::ftob}, {"stob", q::stob},
        {"itos", q::itos}, {"ftos", q::ftos}, {"btos", q::btos}, {"ttos", q::ttos}
    };

}}