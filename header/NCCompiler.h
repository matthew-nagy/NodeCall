#pragma once
#include "NCInBuiltFunctions.h"
#include <fstream>

extern size_t created;
extern size_t deleted;

namespace nc {

	NCNodeLayout* getNCProgramFromSource(std::vector<std::string>& source);
	NCNodeLayout* getNCProgramFromFile(const std::string& fullFileName);
	NCNodeLayout* getNCProgramFromFile(std::ifstream& file);


	class NodeCallSourceControl {
	public:
		void loadProgramIn(std::string programName, std::string fileName) {}
		void loadProgramIn(std::string programName, const std::vector<std::string>& source) {}
		void setSourceLocation(std::string sourceLocation) {}
	private:
		std::string sourceLocation;
		std::unordered_map<std::string, NCNodeLayout*> loadedPrograms;
	};

}