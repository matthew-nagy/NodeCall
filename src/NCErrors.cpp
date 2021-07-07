#include "../header/NCErrors.h"

namespace nc {
	namespace exception {
		NonFunctionCalled::NonFunctionCalled(std::string typeName) :
			typeName(typeName)
		{}

		const char* NonFunctionCalled::what()const noexcept {
			return std::string("Attempted to call variable as a function. Was of type '" + typeName + "'.").c_str();
		}



		MathmaicalTypeMissmatch::MathmaicalTypeMissmatch(std::string lType, std::string rType, std::string operation) :
			lType(lType),
			rType(rType),
			operation(operation)
		{}

		const char* MathmaicalTypeMissmatch::what()const noexcept {
			return std::string("Error in attempting to perform maths due to type issues running '" + operation +
				"'. Left object is of type '" + lType + "'." +
				" Right object is of type '" + rType + "'.").c_str();
		}


		InvalidTypeCast::InvalidTypeCast(std::string currentType, std::string desiredType) :
			currentType(currentType),
			desiredType(desiredType)
		{}
		const char* InvalidTypeCast::what()const noexcept {
			return std::string("Attempted to convert variable of type '" + currentType + "' into a variable of type '" + desiredType + "'.").c_str();
		}


		const char* MissingArgOnCompilation::what()const noexcept {
			return std::string("An argument is expected").c_str();
		}

		UnclosedStringOnCompilation::UnclosedStringOnCompilation(std::string argument) :
			argument(argument)
		{}
		const char* UnclosedStringOnCompilation::what()const noexcept {
			return std::string("Error with string in argument '" + argument + "'").c_str();
		}

		UndefinedSymbol::UndefinedSymbol(std::string symbol) :
			symbol(symbol)
		{}
		const char* UndefinedSymbol::what()const noexcept {
			return std::string("Error, no symbol '" + symbol + "' has been found").c_str();
		}


		UnknownType::UnknownType(std::string requested) :
			requested(requested)
		{}
		const char* UnknownType::what()const noexcept {
			return std::string("Unknown NCType of type '" + requested + "' asked for").c_str();
		}

		MissingNodeDeclarer::MissingNodeDeclarer(std::string line) :
			line(line)
		{}
		const char* MissingNodeDeclarer::what()const noexcept {
			return std::string("Issue with line '" + line + "', missing '>' symbol").c_str();
		}


		UnexpectedEndOfSection::UnexpectedEndOfSection(std::string sectionName) :
			sectionName(sectionName)
		{}
		const char* UnexpectedEndOfSection::what()const noexcept {
			return std::string("Section '" + sectionName + "' finished early").c_str();
		}


		InvalidNumeric::InvalidNumeric(std::string name) :
			name(name)
		{}
		const char* InvalidNumeric::what()const noexcept {
			return std::string(name + " is not a valid int or float value").c_str();
		}

		InvalidTypeForFileIO::InvalidTypeForFileIO(std::string currentType) :
			currentType(currentType)
		{}
		const char* InvalidTypeForFileIO::what() const noexcept {
			return std::string("Cannot write out Objects of type " + currentType).c_str();

		}
	}
}