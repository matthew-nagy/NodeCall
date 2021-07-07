#include "NCInBuiltFunctions.h"

nc::Created_Container_Lobby* getSetLobbyFrom(nc::SymbolTable& table) {
	void* store = table[nc::REG_Created_Container_Lobby].asNCPacket().data;
	return (nc::Created_Container_Lobby*)store;
}

#define NC_IN_BUILT_ARGUMENTS NCThreadControl& threadControl, NCArgumentList& args, SymbolTable& table, NCRuntimeList& internalRuntimeArgs
namespace nc {
	namespace reserved {
		namespace func {
			//Erases a variable from the symbol table
			NCReturnValue erase_symbol(NC_IN_BUILT_ARGUMENTS) {
				table.erase(table.find(args[0]->get(table, threadControl).asString()));
				return ncr_Void;
			}
			//Send execution to a different node. Arg[0] better be a string or this'll all crap
			NCReturnValue link_to(NC_IN_BUILT_ARGUMENTS) {
				table[REG_Node_Name] = args[0]->get(table, threadControl);
				table[REG_Command_Index] = 0;
				return ncr_Node_Shift;
			}
			//Relinquish control up to the c++ code
			NCReturnValue relinquish(NC_IN_BUILT_ARGUMENTS) {
				return ncr_Relinquish;
			}
			//End the program
			NCReturnValue terminate(NC_IN_BUILT_ARGUMENTS) {
				printf("Termination called\n");
				return ncr_Terminate;
			}
			//Throws an exception letting me observe the state of the program
			NCReturnValue throw_exception(NC_IN_BUILT_ARGUMENTS) {
				throw new exception::Requested;
				return ncr_Void;
			}
			//Assign the value of arg[1] to arg[0]
			NCReturnValue assign(NC_IN_BUILT_ARGUMENTS) {
				args[0]->get(table, threadControl) = args[1]->get(table, threadControl);
				return ncr_Void;
			}
			//If statement
			NCReturnValue if_statement(NC_IN_BUILT_ARGUMENTS) {
				if (args[0]->get(table, threadControl).asBool()) {
					NCReturnValue ncret = NodeCallProgram::runCommands(internalRuntimeArgs, table, &threadControl);
					//If it stopped because the commands have been exhausted, just rerturn void
					return ncret == ncr_Commands_Exhausted ? ncr_Void : ncret;
				}
				return ncr_Void;
			}
			//While loop
			NCReturnValue while_loop(NC_IN_BUILT_ARGUMENTS) {
				NCReturnValue ncrv;
				while (args[0]->get(table, threadControl).asBool()) {
					ncrv = NodeCallProgram::runCommands(internalRuntimeArgs, table, &threadControl);
					if (ncrv == ncr_Node_Shift)
						return ncr_Node_Shift;
					if (ncrv == ncr_Terminate)
						return ncr_Terminate;
				}

				return ncr_Void;
			}
			//Do While loop
			NCReturnValue do_while_loop(NC_IN_BUILT_ARGUMENTS) {
				NCReturnValue ncrv = NodeCallProgram::runCommands(internalRuntimeArgs, table, &threadControl);
				if (ncrv == ncr_Node_Shift)
					return ncr_Node_Shift;
				if (ncrv == ncr_Terminate)
					return ncr_Terminate;
				while (args[0]->get(table, threadControl).asBool()) {
					ncrv = NodeCallProgram::runCommands(internalRuntimeArgs, table, &threadControl);
					if (ncrv == ncr_Node_Shift)
						return ncr_Node_Shift;
					if (ncrv == ncr_Terminate)
						return ncr_Terminate;
				}

				return ncr_Void;
			}
			//Print value
			NCReturnValue print_out(NC_IN_BUILT_ARGUMENTS) {
				for (auto& arg : args) {
					NCObject& gotValue = arg->get(table, threadControl);
					printf("%s", gotValue.asString().c_str());
				}
				return ncr_Void;
			}
			//Print with a newln char
			NCReturnValue print_line_out(NC_IN_BUILT_ARGUMENTS) {
				print_out(threadControl, args, table, internalRuntimeArgs);
				printf("\n");
				return ncr_Void;
			}

			/*###################################################
			Functions specifically for queue, stack and vectors
			####################################################*/

			//Push an element onto a queue, stack or vector
			NCReturnValue push_element(NC_IN_BUILT_ARGUMENTS) {
				return ncr_Void;
			}
			//Erases an element from a vector
			NCReturnValue erase_element(NC_IN_BUILT_ARGUMENTS) { 
				auto& pack = args[0]->get(table, threadControl).asNCPacket();
				if (pack.compiledType != typeid(std::vector<NCObject>))
					throw new exception::InvalidTypeCast("some data", "pointer to vector of NCObject");
				std::vector<NCObject>* vec = (std::vector<NCObject>*)pack.data;
				for (size_t i = 1; i < args.size(); i++) {
					int increase = args[i]->get(table, threadControl).asInt();
					vec->erase(vec->begin() + increase);
				}
				return ncr_Void;
			}
			//Clear a queue, stack or vector
			NCReturnValue clear(NC_IN_BUILT_ARGUMENTS) {
				for (size_t i = 0; i < args.size(); i++) {
					NCPacket& pack = args[i]->get(table, threadControl).asNCPacket();
					if (pack.compiledType == typeid(std::queue<NCObject>))
						while (!((std::queue<NCObject>*)pack.data)->empty())
							((std::queue<NCObject>*)pack.data)->pop();
					if (pack.compiledType == typeid(std::stack<NCObject>))
						while (!((std::stack<NCObject>*)pack.data)->empty())
							((std::stack<NCObject>*)pack.data)->pop();
					if (pack.compiledType == typeid(std::vector<NCObject>))
						((std::vector<NCObject>*)pack.data)->clear();
				}
				return ncr_Void;
			}
			//Makes a new queue
			NCReturnValue make_queue(NC_IN_BUILT_ARGUMENTS) { 
				for (size_t i = 0; i < args.size(); i++) {
					NCPacket pack;
					pack.compiledType = typeid(std::queue<NCObject>);
					std::queue<NCObject>* newQue = new std::queue<NCObject>;
					getSetLobbyFrom(table)->queues.push_back(newQue);
					pack.data = (void*)newQue;
					args[i]->get(table, threadControl) = pack;
				}
				return ncr_Void;
			}
			//Makes a new stack
			NCReturnValue make_stack(NC_IN_BUILT_ARGUMENTS) {
				for (size_t i = 0; i < args.size(); i++) {
					NCPacket pack;
					pack.compiledType = typeid(std::stack<NCObject>);
					std::stack<NCObject>* newStack = new std::stack<NCObject>;
					getSetLobbyFrom(table)->stacks.push_back(newStack);
					pack.data = (void*)newStack;
					args[i]->get(table, threadControl) = pack;
				}
				return ncr_Void;
			}
			//Makes a new vector of nc object
			NCReturnValue make_list(NC_IN_BUILT_ARGUMENTS) {
				for (size_t i = 0; i < args.size(); i++) {
					NCPacket pack;
					pack.compiledType = typeid(std::vector<NCObject>);
					std::vector<NCObject>* newVec = new std::vector<NCObject>;
					getSetLobbyFrom(table)->vectors.push_back(newVec);
					pack.data = (void*)newVec;
					args[i]->get(table, threadControl) = pack;
				}
				return ncr_Void;
			}
		}

		NCFunction funcPointerToNCFunction(NCReturnValue(*funcPointer)(NCThreadControl&, NCArgumentList&, SymbolTable&, NCRuntimeList&), std::string name) {
			return NCFunction(
				[funcPointer](NC_IN_BUILT_ARGUMENTS) ->NCReturnValue {
				return funcPointer(threadControl, args, table, internalRuntimeArgs);
			}, name
			);
		}

		std::vector<NCFunction> function_names = {
			funcPointerToNCFunction(func::link_to, "link_to"),
			funcPointerToNCFunction(func::relinquish, "relinquish"),
			funcPointerToNCFunction(func::terminate, "end"),
			funcPointerToNCFunction(func::throw_exception, "throw"),
			funcPointerToNCFunction(func::assign, "assign"),
			funcPointerToNCFunction(func::if_statement, "if"),
			funcPointerToNCFunction(func::while_loop, "while"),
			funcPointerToNCFunction(func::do_while_loop, "do_while"),
			funcPointerToNCFunction(func::print_out, "print"),
			funcPointerToNCFunction(func::print_line_out, "println"),

			funcPointerToNCFunction(func::push_element, "push"),
			funcPointerToNCFunction(func::erase_element, "erase"),
			funcPointerToNCFunction(func::clear, "clear"),
			funcPointerToNCFunction(func::make_queue, "new_queue"),
			funcPointerToNCFunction(func::make_stack, "new_stack"),
			funcPointerToNCFunction(func::make_list, "new_list"),
		};

	}

	SymbolTable getNCSymbolTable(std::string firstNode) {
		SymbolTable symbolTable;

		symbolTable[REG_Command_Index] = 0;

		symbolTable[REG_Last_Index] = 0;

		symbolTable[REG_Node_Name] = firstNode;

		NCPacket pack; pack.data = new Created_Container_Lobby;
		NCPacket stackPack; stackPack.compiledType = typeid(std::stack<NCObject>); stackPack.data = (void*)new std::stack<NCObject>;
		NCPacket queuePack; queuePack.compiledType = typeid(std::queue<NCObject>); queuePack.data = (void*)new std::queue<NCObject>;
		((Created_Container_Lobby*)pack.data)->stacks.push_back((std::stack<NCObject>*)stackPack.data);
		((Created_Container_Lobby*)pack.data)->queues.push_back((std::queue<NCObject>*)queuePack.data);

		symbolTable[REG_Global_Stack] = stackPack;

		symbolTable[REG_Global_Queue] = queuePack;

		symbolTable[REG_Created_Container_Lobby] = pack;

		for (NCFunction ncfunction : reserved::function_names) {
			symbolTable[ncfunction.second] = ncfunction;
		}


		return symbolTable;
	}
}

#undef NC_IN_BUILT_ARGUMENTS