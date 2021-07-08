#pragma once
#include <unordered_map>
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <functional>
#include <queue>
#include <stack>
#include <unordered_set>
#include <typeindex>
#include "semaphore.hpp"
#include "NCErrors.h"

namespace nc {
	//Object to store data at runtime. (variables and values)
	class NCObject;
	//Some reference to an NCObject, either by name, direct value, or getting the result of an internal function
	//It can always get you a value
	class NCObjectReference;
	//An object specifically to call at runtime. Has an object to call, references for its arguments, and further internal runtime objects
	//to support conditionals
	struct NCRuntimeObject;



	extern const std::string REG_Node_Name;
	extern const std::string REG_Command_Index;
	extern const std::string REG_Last_Index;
	extern const std::string REG_Global_Stack;
	extern const std::string REG_Global_Queue;
	extern const std::string REG_Created_Container_Lobby;
	extern const std::unordered_set<std::string> REG_Variables;

	//A map of strings to NCObjects, acting as a map to variables and functions defined in the scope
	typedef std::unordered_map<std::string, NCObject> SymbolTable;

	extern SymbolTable getNCSymbolTable(std::string firstNode = "main");


	//A vector of object references to act as arguments to NCFunctions. This way you can pass objects of any kind
	typedef std::vector<NCObjectReference*> NCArgumentList;
	//A list of runtime objects, effectivly a list of commands
	typedef std::vector<NCRuntimeObject> NCRuntimeList;

	//void is nothing. Relinquish sends control up to c++ pausing the program. Terminate wipes the symbol table
	enum NCReturnValue {
		ncr_Void, ncr_Node_Shift, ncr_Relinquish, ncr_Terminate, ncr_Commands_Exhausted
	};

	//Semaphores to synchronise execution between NodeCall program and c++ program
	struct NCThreadControl {
		Semaphore* externalExecutionLock;
		Semaphore* NCInternalLock;

		NCThreadControl() :
			externalExecutionLock(new Semaphore(0)),
			NCInternalLock(new Semaphore(0))
		{}
		~NCThreadControl() {
			delete externalExecutionLock;
			delete NCInternalLock;
		}
	};

	//Higher order packet in order to hold undefined data in NCObjects
	struct NCPacket {
		std::type_index compiledType;
		void* data;

		NCPacket() :
			compiledType(typeid(int))
		{}
	};

	//A pair with a : function to run w/ args, access to the thread control, the symbol table, and an internal extra list of runtime commands for conditionls
	//				: the name of the function. This is used to for the == operator on functions
	using NCFunction = std::pair<std::function<NCReturnValue(NCThreadControl&, NCArgumentList&, SymbolTable&, NCRuntimeList&)>, std::string>;
	//A function to be run within the call of another function to get an NCObject value from somewhere. For example in
	// debug_out(equality_comparisson(7, someVar)), equality_comparisson is an inline function that returns an NCObject of type ncr_Bool
	using NCInlineFunction = std::function<NCObject(NCThreadControl&, NCArgumentList&, SymbolTable&)>;


	enum NCType {
		nct_Int, nct_Float, nct_String, nct_Bool, nct_Type, nct_Func, nct_Packet, nct_Null
	};
	extern const std::unordered_set<NCType> readwritableNCTypes;
	std::string nct_to_str(NCType type)noexcept;
	NCType str_to_nct(std::string str);
	bool nct_is_mathmatical_type(NCType type);

	class NCObjectReference {
	public:
		virtual NCObject& get(SymbolTable& table, NCThreadControl& threadControl) { return __should_never_be_seen; };
		virtual NCObjectReference* makeCopy() { return new NCObjectReference(); }
		virtual ~NCObjectReference() {}

	private:
		static NCObject __should_never_be_seen;
	};

	class NCObject {
	public:
		//For when the object is a form of function
		NCReturnValue operator()(NCThreadControl& threadControl, NCArgumentList& arguments, SymbolTable& table, NCRuntimeList& internalCommands);

		//Assignment of base types to the Object. Overrides the previous value
		//and changes its internal type

		void operator=(const int&			value)noexcept;
		void operator=(const float&			value)noexcept;
		void operator=(const std::string&	value)noexcept;
		void operator=(const bool&			value)noexcept;
		void operator=(const NCType&		value)noexcept;
		void operator=(NCFunction*const&	value)noexcept;
		void operator=(const NCPacket&		value)noexcept;
		void operator=(const NCObject&		value)noexcept;
		void operator=(const void* value)noexcept;

		//Mathmatical functions that can be run between objects, and it gets run on internal values.
		//If the operation makes no sense for a given value it throws an MathmaicalTypeMissmatch.

		NCObject operator+(const NCObject& value);
		NCObject operator-(const NCObject& value);
		NCObject operator*(const NCObject& value);
		NCObject operator/(const NCObject& value);
		NCObject operator >> (const NCObject& value);
		NCObject operator<<(const NCObject& value);
		NCObject operator&&(const NCObject& value);
		NCObject operator||(const NCObject& value);
		NCObject operator^(const NCObject& value);

		//Equality operators between base types and other NCObjects. Equality is checked against internal types.
		//Allows for different types, such as int == float, or float == bool (not equal 0), and these use c++'s equality functions.
		//If it really makes no sense it either defaults to checking the int value or straight returns false. Please use common sense.
		//If worst comes to worst you could just check the types form within the NodeCall script

		bool operator==(const int&			value)const noexcept;
		bool operator==(const float&		value)const noexcept;
		bool operator==(const std::string&	value)const noexcept;
		bool operator==(const bool&			value)const noexcept;
		bool operator==(const NCType&		value)const noexcept;
		bool operator==(const NCFunction&	value)const noexcept;
		bool operator==(const NCPacket&		value)const noexcept;
		bool operator==(const NCObject&		value)const noexcept;
		bool operator==(const void*			value)const noexcept;

		//Won't define for other types because it seems like a pain :P
		bool operator<(const NCObject& value)const;

		//Getter and setter functions for data about the object

		NCType getType()const noexcept;


		int			asInt()			const;
		float		asFloat()		const;
		std::string asString()		const;
		bool		asBool()		const;
		NCType		asNCType()		const;
		NCFunction*	asNCFunction()	const;
		NCPacket	asNCPacket()	const;

		void writeToFile(std::ofstream& file)const;

		//Empty constructor. Generally for direct values and temporay variables
		NCObject();
		//Constructs off input from a file
		NCObject(std::ifstream& file);
		//Copy constructor. Copies name, type, and value from @param other
		NCObject(const NCObject& other);
		~NCObject();

	private:
		union {
			int _int;
			float _float;
			bool _bool;
			NCType _nctype;
		};
		std::string _string;
		NCFunction* _ncfunction;
		NCPacket _ncpacket;
		NCType myType = nct_Null;
	};

	template<class T>
	NCObject makeNCObject(T value) {
		NCObject toret;
		toret = value;
		return toret;
	}

	//Gets an NCObject from the symbol table based off the variable's name
	class NCNamedObjectReference : public NCObjectReference {
	public:
		NCObject& get(SymbolTable& table, NCThreadControl& threadControl)override;
		NCObjectReference* makeCopy()override;
		NCNamedObjectReference(std::string name);
	private:
		std::string name;
	};
	//Always returns the same hidden NCObject. This is useful for hard values in the code that you don't want updating on the symbol table
	class NCSetObjectReference : public NCObjectReference {
	public:
		NCObject& get(SymbolTable& table, NCThreadControl& threadControl)override;
		NCObjectReference* makeCopy()override;
		NCSetObjectReference(NCObject* innerObject);
		~NCSetObjectReference();
	private:
		NCObject* setObject;
	};
	//Runs an inline function on internal arguments to return a new NCObject that holds the correct value.
	class NCInlineFunctionReference : public NCObjectReference {
	public:
		NCObject& get(SymbolTable& table, NCThreadControl& threadControl)override;
		NCObjectReference* makeCopy()override;
		NCInlineFunctionReference(NCInlineFunction function, NCArgumentList arguments, std::string functionName);
		~NCInlineFunctionReference();
	private:
		//get returns a reference so you store the value here between calls
		NCObject result;
		NCArgumentList arguments;
		NCInlineFunction function;
	};

	struct Created_Container_Lobby {
		std::list < std::stack<nc::NCObject>*> stacks;
		std::list < std::queue<nc::NCObject>*> queues;
		std::list < std::vector<nc::NCObject>*> vectors;

		void deleteAll() {
			for (auto* s : stacks)
				delete s;
			for (auto* q : queues)
				delete q;
			for (auto* v : vectors)
				delete v;
		}
	};
}