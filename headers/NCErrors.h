#pragma once
#include <string>
#include <exception>

namespace nc {

	namespace exception {

		//An exception to be thrown when a non-function type NCObject is called like a function
		class NonFunctionCalled : public std::exception {
		public:
			NonFunctionCalled(std::string typeName);
			const char* what()const noexcept override;
		private:
			//Name and proper type of the variable
			std::string typeName;
		};

		//An exception to be thrown when a non-sensical mathmatical operation is attempted to be performed
		//on incorrect type. Example, bool / string
		class MathmaicalTypeMissmatch : public std::exception {
		public:
			MathmaicalTypeMissmatch(std::string lType, std::string rType, std::string operation);
			const char* what()const noexcept override;
		private:
			//Name and type of the two operands, and the operation
			std::string lType, rType, operation;
		};

		//An exeption thrown when an invalid type conversion is asked for. There is no way to convert a bool to a type after, for example
		class InvalidTypeCast : public std::exception {
		public:
			InvalidTypeCast(std::string currentType, std::string desiredType);
			const char* what()const noexcept override;
		private:
			std::string currentType, desiredType;
		};

		//A exception to be thrown when an empty argument is given on compilation
		class MissingArgOnCompilation : public std::exception {
		public:
			const char* what()const noexcept override;
		};
		//A exception to be thrown when a string is messed up on compilation
		class UnclosedStringOnCompilation : public std::exception {
		public:
			UnclosedStringOnCompilation(std::string argument);
			const char* what()const noexcept override;
		private:
			std::string argument;
		};
		//A exception to be thrown when an incorrect symbol is asked for on compilation
		class UndefinedSymbol : public std::exception {
		public:
			UndefinedSymbol(std::string symbol);
			const char* what()const noexcept override;
		private:
			std::string symbol;
		};
		//A exception to be thrown when an unknown NCType asked for on compilation
		class UnknownType : public std::exception {
		public:
			UnknownType(std::string requested);
			const char* what()const noexcept override;
		private:
			std::string requested;
		};
		//A exception to be thrown when a node is initilised incorrectly
		class MissingNodeDeclarer : public std::exception {
		public:
			MissingNodeDeclarer(std::string line);
			const char* what()const noexcept override;
		private:
			std::string line;
		};
		//A exception to be thrown when a node is initilised incorrectly
		class UnexpectedEndOfSection : public std::exception {
		public:
			UnexpectedEndOfSection(std::string sectionName);
			const char* what()const noexcept override;
		private:
			std::string sectionName;
		};
		//A exception to be thrown when the script contains issues in its numbers, ie 32.4z12
		class InvalidNumeric : public std::exception {
		public:
			InvalidNumeric(std::string name);
			const char* what()const noexcept override;
		private:
			std::string name;
		};
		//A exception to be thrown when the script contains issues in its numbers, ie 32.4z12
		class Requested : public std::exception {
		public:
			const char* what()const noexcept { return nullptr; }
		}; 

		class InvalidTypeForFileIO : public std::exception {
		public:
			InvalidTypeForFileIO(std::string currentType);
			const char* what() const noexcept override;
		private:
			std::string currentType;
		};
	}

}