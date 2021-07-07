#pragma once
#include "NCRuntime.h"

#define NC_INLINE_ARGUMENTS NCThreadControl& threadControl, NCArgumentList& argList, SymbolTable& symbolTable
namespace nc {
	namespace reserved {
		namespace inln {

			NCObject eq_comp(NC_INLINE_ARGUMENTS);
			NCObject greater_comp(NC_INLINE_ARGUMENTS);
			NCObject less_comp(NC_INLINE_ARGUMENTS);
			NCObject greater_eq_comp(NC_INLINE_ARGUMENTS);
			NCObject less_eq_comp(NC_INLINE_ARGUMENTS);

			NCObject logical_left_shift(NC_INLINE_ARGUMENTS);
			NCObject logical_right_shift(NC_INLINE_ARGUMENTS);
			NCObject and (NC_INLINE_ARGUMENTS);
			NCObject or (NC_INLINE_ARGUMENTS);
			NCObject not(NC_INLINE_ARGUMENTS);
			NCObject logical_xor(NC_INLINE_ARGUMENTS);

			NCObject math_add(NC_INLINE_ARGUMENTS);
			NCObject math_sub(NC_INLINE_ARGUMENTS);
			NCObject math_mul(NC_INLINE_ARGUMENTS);
			NCObject math_div(NC_INLINE_ARGUMENTS);

			NCObject get_type(NC_INLINE_ARGUMENTS);
			NCObject input(NC_INLINE_ARGUMENTS);
			NCObject get_element(NC_INLINE_ARGUMENTS);
			NCObject pop_element(NC_INLINE_ARGUMENTS);
			NCObject get_size(NC_INLINE_ARGUMENTS);

			NCObject to_int(NC_INLINE_ARGUMENTS);
			NCObject to_float(NC_INLINE_ARGUMENTS);
			NCObject to_string(NC_INLINE_ARGUMENTS);
			NCObject to_bool(NC_INLINE_ARGUMENTS);

		}

#undef NC_INLINE_ARGUMENTS

#define NC_IN_BUILT_ARGUMENTS NCThreadControl& threadControl, NCArgumentList& args, SymbolTable& table, NCRuntimeList& internalRuntimeArgs

		namespace func {
			/*###################################################
			Basic functions to be used
			####################################################*/
			//Erases a variable from the symbol table
			NCReturnValue erase_symbol(NC_IN_BUILT_ARGUMENTS);
			//Send execution to a different node. Arg[0] better be a string or this'll all crap
			NCReturnValue link_to(NC_IN_BUILT_ARGUMENTS);
			//Relinquish control up to the c++ code
			NCReturnValue relinquish(NC_IN_BUILT_ARGUMENTS);
			//End the program
			NCReturnValue terminate(NC_IN_BUILT_ARGUMENTS);
			//Throws an exception letting me observe the state of the program
			NCReturnValue throw_exception(NC_IN_BUILT_ARGUMENTS);
			//Assign the value of arg[1] to arg[0]
			NCReturnValue assign(NC_IN_BUILT_ARGUMENTS);
			//If statement
			NCReturnValue if_statement(NC_IN_BUILT_ARGUMENTS);
			//While loop
			NCReturnValue while_loop(NC_IN_BUILT_ARGUMENTS);
			//Do While loop
			NCReturnValue do_while_loop(NC_IN_BUILT_ARGUMENTS);
			//Print value
			NCReturnValue print_out(NC_IN_BUILT_ARGUMENTS);
			//Print with a newln char
			NCReturnValue print_line_out(NC_IN_BUILT_ARGUMENTS);


			/*###################################################
			Functions specifically for queue, stack and vectors
			####################################################*/

			//Push an element onto a queue, stack or vector
			NCReturnValue push_element(NC_IN_BUILT_ARGUMENTS);
			//Erases an element from a vector
			NCReturnValue erase_element(NC_IN_BUILT_ARGUMENTS);
			//Clear a queue, stack or vector
			NCReturnValue clear(NC_IN_BUILT_ARGUMENTS);
			//Makes a new queue
			NCReturnValue make_queue(NC_IN_BUILT_ARGUMENTS);
			//Makes a new stack
			NCReturnValue make_stack(NC_IN_BUILT_ARGUMENTS);
			//Makes a new vector of nc object
			NCReturnValue make_list(NC_IN_BUILT_ARGUMENTS);

		}

		extern std::vector<NCFunction> function_names;
		extern std::unordered_map <std::string, NCInlineFunction> inline_function_names;
	}
}
//After externing in NCObject.h is defined in this files .cpp (Specifically the one for reserved functions
//SymbolTable getNCSymbolTable(std::string firstNode);

#undef NC_IN_BUILT_ARGUMENTS