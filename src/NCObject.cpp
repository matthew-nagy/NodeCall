#include "NCObject.h"


namespace nc {

	const std::string REG_Node_Name = "NODE_NAME";
	const std::string REG_Command_Index = "COMMAND_INDEX";
	const std::string REG_Last_Index = "EXECUTING_INDEX";
	const std::string REG_Global_Stack = "g_stack";
	const std::string REG_Global_Queue = "g_queue";
	const std::string REG_Created_Container_Lobby = "CREATED_CONTAINER_LOBBY";


	const std::unordered_set<std::string> REG_Variables = {
		REG_Node_Name, REG_Command_Index, REG_Last_Index, REG_Global_Stack, REG_Global_Queue, REG_Created_Container_Lobby
	};


	const std::unordered_set<NCType> readwritableNCTypes = {
		nct_Bool, nct_Float, nct_Int, nct_Type, nct_String
	};



	//###################################
	//		Utility functions
	//###################################
	std::string nct_to_str(NCType type)noexcept {
		switch (type) {
		case nct_Int:return"int";
		case nct_Float:return"float";
		case nct_String:return"string";
		case nct_Bool:return"bool";
		case nct_Type:return"NC Type";
		case nct_Func:return"NC Function";
		case nct_Packet: return "Higher Order Packet";
		default:return"Null";
		}
	}

	typedef std::pair<std::string, NCType> pst;
	std::unordered_map<std::string, NCType> stringToTypeMap = {
		pst("int", nct_Int),
		pst("float", nct_Float),
		pst("string", nct_String),
		pst("bool", nct_Bool),
		pst("type", nct_Type),
		pst("function", nct_Func),
		pst("packet", nct_Packet),
		pst("null", nct_Null)
	};

	NCType str_to_nct(std::string str) {
		if (stringToTypeMap.count(str) == 0)
			throw new exception::UnknownType(str);
		return stringToTypeMap[str];
	}

	bool nct_is_mathmatical_type(NCType type) {
		return type == nct_Int || type == nct_Float;
	}


	NCObject NCObjectReference::__should_never_be_seen = makeNCObject<std::string>("<Hidden illigal variable owned by uncallable base NCObjectReference>");


	NCObject& NCNamedObjectReference::get(SymbolTable& table, NCThreadControl& threadControl) {
		return table[name];
	}
	NCObjectReference* NCNamedObjectReference::makeCopy() {
		return new NCNamedObjectReference(name);
	}
	NCNamedObjectReference::NCNamedObjectReference(std::string name) :
		name(name)
	{}


	NCObject& NCSetObjectReference::get(SymbolTable& table, NCThreadControl& threadControl) {
		return *setObject;
	}
	NCObjectReference* NCSetObjectReference::makeCopy() {
		NCObject* copiedObject = new NCObject();
		copiedObject->operator=(*setObject);
		return new NCSetObjectReference(copiedObject);
	}
	NCSetObjectReference::NCSetObjectReference(NCObject* innerObject) :
		setObject(innerObject)
	{}
	NCSetObjectReference::~NCSetObjectReference() {
		delete setObject;
	}


	NCObject& NCInlineFunctionReference::get(SymbolTable& table, NCThreadControl& threadControl) {
		result = std::move(function(threadControl, arguments, table));
		return result;
	}

	NCArgumentList copyArgList(const NCArgumentList& in) {
		NCArgumentList ret;
		for (auto* a : in)
			ret.push_back(a->makeCopy());
		return ret;
	}

	NCObjectReference* NCInlineFunctionReference::makeCopy() {
		std::string funcName = result.asString();
		auto h = new NCInlineFunctionReference(function, copyArgList(arguments), funcName.substr(funcName.size() - 8));
		return h;
	}
	NCInlineFunctionReference::NCInlineFunctionReference(NCInlineFunction function, NCArgumentList arguments, std::string functionName) :
		function(function),
		arguments(arguments),
		result()
	{}

	NCInlineFunctionReference::~NCInlineFunctionReference() {
		for (auto* a : arguments)
			delete a;
	}


	NCReturnValue NCObject::operator()(NCThreadControl& threadControl, NCArgumentList& arguments, SymbolTable& table, NCRuntimeList& internalCommands) {
		if (myType == nct_Func)
			return _ncfunction.first(threadControl, arguments, table, internalCommands);
		else
			throw new exception::NonFunctionCalled(nct_to_str(myType));
	}

	void NCObject::operator=(const int&			value)noexcept {
		myType = nct_Int;
		_int = value;
	}
	void NCObject::operator=(const float&			value)noexcept {
		myType = nct_Float;
		_float = value;
	}
	void NCObject::operator=(const std::string&	value)noexcept {
		_string = value;
		myType = nct_String;
	}
	void NCObject::operator=(const bool&			value)noexcept {
		myType = nct_Bool;
		_bool = value;
	}
	void NCObject::operator=(const NCType&		value)noexcept {
		myType = nct_Type;
		_nctype = value;
	}
	void NCObject::operator=(const NCFunction&	value)noexcept {
		myType = nct_Func;
		_ncfunction = value;
	}
	void NCObject::operator=(const NCPacket&		value)noexcept {
		myType = nct_Packet;
		_ncpacket = value;
	}
	void NCObject::operator=(const NCObject&		value)noexcept {
		myType = value.getType();
		switch (myType) {
#define NCSWITCHSET(nct, varname) case nct : varname = value. varname; break;
			NCSWITCHSET(nct_Int, _int)
				NCSWITCHSET(nct_Float, _float)
				NCSWITCHSET(nct_String, _string)
				NCSWITCHSET(nct_Bool, _bool)
				NCSWITCHSET(nct_Type, _nctype)
				NCSWITCHSET(nct_Func, _ncfunction)
		default:
			myType = nct_Null;
#undef NCSWITCHSET
		}
	}
	void NCObject::operator=(const void* value)noexcept {
		if (value == nullptr)
			myType = nct_Null;
		else
			this->operator=(std::string((char*)value));
	}

#define MATHMATICAL_FUNCTION_WITH_OP(OPERATOR, OPERATOR_STRING) case nct_Int:\
			if (value.getType() == nct_Int)\
				toret = _int OPERATOR value._int;\
			else if (value.getType() == nct_Float)\
				toret = _int OPERATOR value._float;\
			else throw new exception::MathmaicalTypeMissmatch(nct_to_str(myType), nct_to_str(value.getType()), OPERATOR_STRING);\
			break;\
		case nct_Float:\
			if (value.getType() == nct_Int)\
				toret = _float OPERATOR value._int;\
			else if (value.getType() == nct_Float)\
				toret = _float OPERATOR value._float;\
			else throw new exception::MathmaicalTypeMissmatch(nct_to_str(myType), nct_to_str(value.getType()), OPERATOR_STRING);\
			break;

	NCObject NCObject::operator+(const NCObject& value) {
		NCObject toret;
		switch (myType) {
			MATHMATICAL_FUNCTION_WITH_OP(+, "+")
		case nct_String:
			if (value.getType() == nct_String)
				toret = _string + value._string;
			else throw new exception::MathmaicalTypeMissmatch(nct_to_str(myType), nct_to_str(value.getType()), "+");
			break;
		default:
			throw new exception::MathmaicalTypeMissmatch(nct_to_str(myType), nct_to_str(value.getType()), "+");
		}

		return toret;
	}
	NCObject NCObject::operator-(const NCObject& value) {
		NCObject toret;
		switch (myType) {
			MATHMATICAL_FUNCTION_WITH_OP(-, "-")
		default:
			throw new exception::MathmaicalTypeMissmatch(nct_to_str(myType), nct_to_str(value.getType()), "-");
		}
		return toret;
	}
	NCObject NCObject::operator*(const NCObject& value) {
		NCObject toret;
		switch (myType) {
			MATHMATICAL_FUNCTION_WITH_OP(*, "*")
		default:
			throw new exception::MathmaicalTypeMissmatch(nct_to_str(myType), nct_to_str(value.getType()), "*");
		}
		return toret;
	}
	NCObject NCObject::operator/(const NCObject& value) {
		NCObject toret;
		switch (myType) {
			MATHMATICAL_FUNCTION_WITH_OP(/ , "/")
		default:
			throw new exception::MathmaicalTypeMissmatch(nct_to_str(myType), nct_to_str(value.getType()), "/");
		}
		return toret;
	}
#undef MATHMATICAL_FUNCTION_WITH_OP
	NCObject NCObject::operator >> (const NCObject& value) {
		NCObject toret;
		if (myType == nct_Int && value.getType() == nct_Int) {
			toret = _int >> value._int;
		}
		else
			throw new exception::MathmaicalTypeMissmatch(nct_to_str(myType), nct_to_str(value.getType()), ">>");
		return toret;
	}
	NCObject NCObject::operator<<(const NCObject& value) {
		NCObject toret;
		if (myType == nct_Int && value.getType() == nct_Int) {
			toret = _int << value._int;
		}
		else
			throw new exception::MathmaicalTypeMissmatch(nct_to_str(myType), nct_to_str(value.getType()), "<<");
		return toret;
	}
	NCObject NCObject::operator&&(const NCObject& value) {
		NCObject toret;
		if (myType == nct_Bool && value.getType() == nct_Bool) {
			toret = _bool && value._bool;
		}
		else if (myType == ncr_Void || value.getType() == ncr_Void) {
			toret = false;
		}
		else {
			toret = _int && value._int;
		}
		return toret;
	}
	NCObject NCObject::operator||(const NCObject& value) {
		NCObject toret;
		if (myType == nct_Bool && value.getType() == nct_Bool)
			toret = _bool || value._bool;
		else if (myType == ncr_Void && value.getType() == ncr_Void)
			toret = false;
		else
			toret = _int || value._int;
		return toret;
	}
	NCObject NCObject::operator^(const NCObject& value) {
		NCObject toret;
		if (myType == nct_Bool && value.getType() == nct_Bool)
			toret = _bool ^ value._bool;
		else if (myType == nct_Int && value.getType() == nct_Int)
			toret = _int ^ value._int;
		else
			toret = false;
		return toret;
	}

	bool NCObject::operator==(const int&			value)const noexcept {
		if (myType == nct_Int)
			return _int == value;
		else if (myType == nct_Float)
			return _float == value;
		return false;
	}
	bool NCObject::operator==(const float&		value)const noexcept {
		if (myType == nct_Int)
			return _int == value;
		else if (myType == nct_Float)
			return _float == value;
		return false;
	}
	bool NCObject::operator==(const std::string&	value)const noexcept {
		if (myType == nct_String)
			return _string == value;
		return false;
	}
	bool NCObject::operator==(const bool&			value)const noexcept {
		if (myType == nct_Bool)
			return _bool == value;
		if (myType == nct_Null)
			return value == false;
		return value ? _int > 0 : _int == 0;
	}
	bool NCObject::operator==(const NCType&		value)const noexcept {
		if (myType == nct_Type)
			return _nctype == value;
		return false;
	}
	bool NCObject::operator==(const NCFunction&	value)const noexcept {
		if (myType == nct_Func)
			return value.second == _ncfunction.second;
		return false;

	}
	bool NCObject::operator==(const NCPacket&		value)const noexcept {
		if (myType != nct_Packet)
			return false;
		return _ncpacket.data == value.data;
	}
	bool NCObject::operator==(const NCObject&		value)const noexcept {
		switch (myType) {
		case nct_Int: return value == _int;
		case nct_Float: return value == _float;
		case nct_String: return value == _string;
		case nct_Bool: return value == _bool;
		case nct_Func: return value == _ncfunction;
		case nct_Type: return value == _nctype;
		default:
			return value == nullptr;
		}
	}
	bool NCObject::operator==(const void*			value)const noexcept {
		if (myType == nct_Null)
			return true;
		return false;
	}



	//Won't define for other types because it seems like a pain :P
	bool NCObject::operator<(const NCObject& value)const {
		if (myType == value.getType() && value.getType() == nct_String)
			return _string.size() < value._string.size();
		else
			return asFloat() < value.asFloat();
	}


	NCType NCObject::getType()const noexcept {
		return myType;
	}

	int NCObject::asInt()const {
		switch (myType) {
		case nct_Int:return _int;
		case nct_Float: return _float;
		case nct_String: return std::atoi(_string.c_str());
		case nct_Bool: return _bool ? 1 : 0;
		case nct_Type: return _nctype;
		default:
			throw new exception::InvalidTypeCast(nct_to_str(myType), "int");
		}
	}
	float NCObject::asFloat()const {
		switch (myType) {
		case nct_Int:return _int;
		case nct_Float: return _float;
		case nct_String: return std::atof(_string.c_str());
		case nct_Bool: return _bool ? 1.0 : 0.0;
		case nct_Type: return _nctype;
		default:
			throw new exception::InvalidTypeCast(nct_to_str(myType), "float");
		}
	}
	std::string NCObject::asString()const {
		switch (myType) {
		case nct_Int:return std::to_string(_int);
		case nct_Float: return std::to_string(_float);
		case nct_String: return _string;
		case nct_Bool: return _bool ? "True" : "False";
		case nct_Type: return nct_to_str(_nctype);
		case nct_Func:return "Function with name '" + _ncfunction.second + "'";
		case nct_Null:return "NullValue";
		default:
			throw new exception::InvalidTypeCast(nct_to_str(myType), "string");
		}
	}
	bool NCObject::asBool()const {
		switch (myType) {
		case nct_Int:return _int > 0;
		case nct_Float: return _float > 0;
		case nct_String: if (_string == "True") return true;
						 else if (_string == "False") return false;
						 else throw new exception::InvalidTypeCast(nct_to_str(myType), "bool (value wasn't True or False)");
		case nct_Bool: return _bool;
		case nct_Type: return _nctype != nct_Null;
		case nct_Null: return false;
		default:
			throw new exception::InvalidTypeCast(nct_to_str(myType), "bool");
		}
	}
	NCType NCObject::asNCType()const {
		switch (myType) {
		case nct_Type: return _nctype;
		default:
			throw new exception::InvalidTypeCast(nct_to_str(myType), "NodeCallType");
		}
	}
	NCFunction NCObject::asNCFunction()const {
		switch (myType) {
		case nct_Func:return _ncfunction;
		default:
			throw new exception::InvalidTypeCast(nct_to_str(myType), "NodeCallFunction");
		}
	}
	NCPacket NCObject::asNCPacket()	const {
		if (myType != nct_Packet)
			throw new exception::InvalidTypeCast(nct_to_str(myType), "NCPacket");
		return _ncpacket;
	}


	void NCObject::writeToFile(std::ofstream& file)const {
		size_t stringSize;
		file.write((char*)&myType, sizeof(NCType));
		switch (myType) {
		case nct_Int:
		case nct_Float:
		case nct_Bool:
		case nct_Type:
			//All of these are just the single size, so lets write them out as a 4
			file.write((char*)&_int, sizeof(int));
			break;
		case nct_String:
			//A bit special, need to write out the entire string
			stringSize = _string.size();
			file.write((char*)&stringSize, sizeof(size_t));
			file.write(&_string[0], stringSize);
			break;
		default:
			throw new exception::InvalidTypeForFileIO(nct_to_str(myType));
		}
	}

	//#define NC_OBJECT_TRACK

	NCObject::NCObject() {
#ifdef NC_OBJECT_TRACK
		printf("Object created)\n");
#endif
	}
	//Constructs off input from a file
	NCObject::NCObject(std::ifstream& file) {
		size_t stringSize;
		file.read((char*)&myType, sizeof(NCType));
		switch (myType) {
			file.read((char*)&myType, sizeof(NCType));
			switch (myType) {
			case nct_Int:
			case nct_Float:
			case nct_Bool:
			case nct_Type:
				//All of these are just the single size, so lets write them out as a 4
				file.read((char*)&_int, sizeof(int));
				break;
			case nct_String:
				//A bit special, need to write out the entire string
				file.read((char*)&stringSize, sizeof(size_t));
				_string.resize(stringSize);
				file.read(&_string[0], stringSize);
				break;
			default:
				throw new exception::InvalidTypeForFileIO(nct_to_str(myType));
			}
		}
	}
	NCObject::NCObject(const NCObject& other){
		//Now copy the value and type across
		this->operator=(other);

#ifdef NC_OBJECT_TRACK
		printf("Object copied\n");
#endif
	}
	NCObject::~NCObject() {
#ifdef NC_OBJECT_TRACK
		printf("Destroyed\n");
#endif
	}

}