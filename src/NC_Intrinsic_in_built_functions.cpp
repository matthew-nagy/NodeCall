#include "NCInBuiltFunctions.h"


#define NC_INLINE_ARGUMENTS NCThreadControl& threadControl, NCArgumentList& argList, SymbolTable& symbolTable
namespace nc{
	namespace reserved {
		namespace inln {

			NCObject eq_comp(NC_INLINE_ARGUMENTS) {
				NCObject toret;
				toret = argList[0]->get(symbolTable, threadControl) == argList[1]->get(symbolTable, threadControl);
				return toret;
			}
			NCObject greater_comp(NC_INLINE_ARGUMENTS) {
				NCObject toret;
				toret = argList[1]->get(symbolTable, threadControl) < argList[0]->get(symbolTable, threadControl);
				return toret;
			}
			NCObject less_comp(NC_INLINE_ARGUMENTS) {
				NCObject toret;
				toret = argList[0]->get(symbolTable, threadControl) < argList[1]->get(symbolTable, threadControl);
				return toret;
			}
			NCObject greater_eq_comp(NC_INLINE_ARGUMENTS) {
				NCObject toret;
				toret = eq_comp(threadControl, argList, symbolTable) || greater_comp(threadControl, argList, symbolTable);
				return toret;
			}
			NCObject less_eq_comp(NC_INLINE_ARGUMENTS) {
				NCObject toret;
				toret = eq_comp(threadControl, argList, symbolTable) || less_comp(threadControl, argList, symbolTable);
				return toret;
			}

			NCObject logical_left_shift(NC_INLINE_ARGUMENTS) {
				NCObject toret;
				toret = argList[0]->get(symbolTable, threadControl) << argList[1]->get(symbolTable, threadControl);
				return toret;
			}
			NCObject logical_right_shift(NC_INLINE_ARGUMENTS) {
				NCObject toret;
				toret = argList[0]->get(symbolTable, threadControl) >> argList[1]->get(symbolTable, threadControl);
				return toret;
			}
			NCObject and (NC_INLINE_ARGUMENTS) {
				NCObject toret;
				toret = argList[0]->get(symbolTable, threadControl) && argList[1]->get(symbolTable, threadControl);
				return toret;
			}
			NCObject or (NC_INLINE_ARGUMENTS) {
				NCObject toret;
				toret = argList[0]->get(symbolTable, threadControl) || argList[1]->get(symbolTable, threadControl);
				return toret;
			}
			NCObject not(NC_INLINE_ARGUMENTS) {
				NCObject toret;
				toret = argList[0]->get(symbolTable, threadControl).asBool() ? false : true;
				return toret;
			}
			NCObject logical_xor(NC_INLINE_ARGUMENTS) {
				NCObject toret;
				toret = argList[0]->get(symbolTable, threadControl) ^ argList[1]->get(symbolTable, threadControl);
				return toret;
			}

			NCObject math_add(NC_INLINE_ARGUMENTS) {
				NCObject runningTotal;
				runningTotal = argList[0]->get(symbolTable, threadControl) + argList[1]->get(symbolTable, threadControl);
				for (size_t i = 2; i < argList.size(); i++)
					runningTotal = runningTotal + argList[i]->get(symbolTable, threadControl);
				return runningTotal;
			}
			NCObject math_sub(NC_INLINE_ARGUMENTS) {
				NCObject runningTotal;
				runningTotal = argList[0]->get(symbolTable, threadControl) - argList[1]->get(symbolTable, threadControl);
				for (size_t i = 2; i < argList.size(); i++)
					runningTotal = runningTotal - argList[i]->get(symbolTable, threadControl);
				return runningTotal;
			}
			NCObject math_mul(NC_INLINE_ARGUMENTS) {
				NCObject runningTotal;
				runningTotal = argList[0]->get(symbolTable, threadControl) * argList[1]->get(symbolTable, threadControl);
				for (size_t i = 2; i < argList.size(); i++)
					runningTotal = runningTotal * argList[i]->get(symbolTable, threadControl);
				return runningTotal;
			}
			NCObject math_div(NC_INLINE_ARGUMENTS) {
				NCObject runningTotal;
				runningTotal = argList[0]->get(symbolTable, threadControl) / argList[1]->get(symbolTable, threadControl);
				for (size_t i = 2; i < argList.size(); i++)
					runningTotal = runningTotal / argList[i]->get(symbolTable, threadControl);
				return runningTotal;
			}

			NCObject get_type(NC_INLINE_ARGUMENTS) {
				auto& subject = argList[0]->get(symbolTable, threadControl);
				NCObject toret;
				toret = subject.getType();
				return toret;
			}
			NCObject input(NC_INLINE_ARGUMENTS) {
				std::string value;
				std::getline(std::cin, value);
				return makeNCObject<std::string>(value);
			}
			NCObject get_element(NC_INLINE_ARGUMENTS) {
				int index = argList[1]->get(symbolTable, threadControl).asInt();
				NCPacket& pack = argList[0]->get(symbolTable, threadControl).asNCPacket();
				if (pack.compiledType != typeid(std::vector<NCObject>))
					throw new exception::InvalidTypeCast("pack of something", "pack of vector of nc object");

				std::vector<NCObject>* vec = (std::vector<NCObject>*)pack.data;
				if (index >= vec->size())
					//Index out of size
					throw new exception::Requested();
				return vec->operator[](index);
			}
			NCObject pop_element(NC_INLINE_ARGUMENTS) {
				int index = argList[1]->get(symbolTable, threadControl).asInt();
				NCPacket& pack = argList[0]->get(symbolTable, threadControl).asNCPacket();
				if (pack.compiledType != typeid(std::queue<NCObject>) && pack.compiledType != typeid(std::stack<NCObject>))
					throw new exception::InvalidTypeCast("pack of something", "pack of stack/queue of nc object");

				if (pack.compiledType == typeid(std::queue<NCObject>)) {
					const NCObject ret = ((std::queue<NCObject>*)pack.data)->back();
					((std::queue<NCObject>*)pack.data)->pop();
					return ret;
				}
				//else is a stack
				const NCObject& ret = ((std::stack<NCObject>*)pack.data)->top();
				((std::queue<NCObject>*)pack.data)->pop();
				return ret;
			}

			NCObject to_int(NC_INLINE_ARGUMENTS) {
				auto& subject = argList[0]->get(symbolTable, threadControl);
				NCObject toret;
				toret = subject.asInt();
				return toret;
			}
			NCObject to_float(NC_INLINE_ARGUMENTS) {
				auto& subject = argList[0]->get(symbolTable, threadControl);
				NCObject toret;
				toret = subject.asFloat();
				return toret;
			}
			NCObject to_string(NC_INLINE_ARGUMENTS) {
				auto& subject = argList[0]->get(symbolTable, threadControl);
				NCObject toret;
				toret = subject.asString();
				return toret;
			}
			NCObject to_bool(NC_INLINE_ARGUMENTS) {
				auto& subject = argList[0]->get(symbolTable, threadControl);
				NCObject toret;
				toret = subject.asBool();
				return toret;
			}

		}
		using namespace inln;
		typedef std::pair<std::string, NCInlineFunction> ifp;
		std::unordered_map <std::string, NCInlineFunction> inline_function_names = {
			ifp("equality", eq_comp),
			ifp("grtr", greater_comp),
			ifp("less", less_comp),
			ifp("grtr_eq", greater_eq_comp),
			ifp("less_eq", less_eq_comp),

			ifp("left_shift", logical_left_shift),
			ifp("right_shift", logical_right_shift),
			ifp("and", and),
			ifp("or", or ),
			ifp("not", not),
			ifp("logical_xor", logical_xor),

			ifp("add", math_add),
			ifp("sub", math_sub),
			ifp("mul", math_mul),
			ifp("div", math_div),

			ifp("typeof", get_type),
			ifp("input", input),
			ifp("pop", pop_element),
			ifp("get_index", get_element),

			ifp("int", to_int),
			ifp("float", to_float),
			ifp("str", to_string),
			ifp("bool", to_bool)
		};
	}
}

#undef NC_INLINE_ARGUMENTS