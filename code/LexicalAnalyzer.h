#pragma once

#include "DFSM.h"

#include <iostream>
#include <fstream>
#include <string>

class LexicalAnalyzer
{
public:

	void lexicalAnalysis(std::string filename);

private:

	void removeComments(std::string filename);
	bool isKeyword(std::string word);

	std::string keywords[14] = { "function", "integer", "boolean",
		"real", "if", "endif", "else", "return", "printf",
		"scanf", "while", "true", "false", "int" };
};