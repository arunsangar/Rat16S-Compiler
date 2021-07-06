//------------------------------------------------------------------------------------
//Name: Arun Sangar
//Course: CPSC 323c
//Date last modified: 2/19/2016
//Project Name: Rat16S Compiler
//Filename: main.cpp
//
//Description: Entry point to Rat16S compiler on a text file
//-----------------------------------------------------------------------------------

#include "LexicalAnalyzer.h"
#include "SyntaxAnalyzer.h"
#include "AssmeblyGenerator.h"

int main()
{
	std::string filename;
	std::cout << "Enter a filename (without .txt): ";
	std::cin >> filename;

	LexicalAnalyzer testA;
	testA.lexicalAnalysis(filename);

	//SyntaxAnalyzer testB;
	//testB.syntaxAnalysis(filename + "tokenized.txt");

	AssemblyGenerator testC;
	testC.generateASM(filename);

	return 0;
}