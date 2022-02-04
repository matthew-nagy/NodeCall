#include "Standard_Library.hpp"


namespace nc{   namespace stlib{


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

template<class T>
value moveVal(T&& input){
    return std::make_shared<std::any>(std::make_any<T>(input));
}
template<class T>
value makeVal(T input){
    return std::make_shared<std::any>(std::make_any<T>(input));
}

#define qdef(name) value name (argument_list& args, unique_run_resource& runResource)
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
            return makeVal<int>(!argl);
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
    bool leftLessRight(argument& arg1, argument& arg2, unique_run_resource& runResource){}
    bool leftMoreRight(argument& arg1, argument& arg2, unique_run_resource& runResource){}

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

    qdef(list_size){
        return makeVal<int>(getAnyVecPtr(args[0], runResource)->size());
    }
    qdef(list_index){
        int index = std::any_cast<int>(*args[1].getValue(runResource));
        return std::make_shared<std::any>((*getAnyVecPtr(args[0], runResource))[index]);
    }
    qdef(list_pop);
    qdef(list_pop_front);
    qdef(list_front);
    qdef(list_tail);


    //IO quearies
    qdef(input){
        std::string userInput = "";
        std::getline(std::cin, userInput);
        return moveVal<std::string>(std::move(userInput));
    }


    //Type quearies and conversions
    qdef(getType);

    qdef(ftoi);
    qdef(stoi);
    qdef(btoi);

    qdef(itof);
    qdef(btof);
    qdef(stof);

    qdef(itob);
    qdef(ftob);
    qdef(stob);

    qdef(itos);
    qdef(ftos);
    qdef(btos);
}
#undef qdef


    //AND OTHERS
    const QuearyTable _standard_quearies = {
        {"add", q::add}, {"sub", q::sub}, {"mul", q::mul}, {"div", q::div}, {"mod", q::mod}, {"sqrt", q::square_root},
        {"lshift", q::lshift}, {"rshift", q::rshift}, {"&", q::land}, {"|", q::lor}, {"!", q::lnot}, {"^", q::lxor}
    };

}}