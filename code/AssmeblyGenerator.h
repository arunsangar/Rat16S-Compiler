#pragma once

#include "SyntaxAnalyzer.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <stack>

//entry for symbol table
struct stEntry
{
	std::string token, type;
	int memAdd;
};

//entry for instruction table
struct instEntry
{
	int instAdd, oprd;
	std::string op;
};

class AssemblyGenerator 
{
public:

	//call this function with a filename to generate 3AC code
	//from the tokenized version of the file
	void generateASM(std::string filename);

private:

	//gets a token from the tokenzied file
	void lexer();

	//gets the symbol table entry of the token that matches id
	stEntry getIdAdd(std::string id, bool declare = false);
	//genereate an instruction given operation and operand
	void genInst(std::string op, int oprd);
	//patch a jump instruction with an instruction address
	void backPatch(int jumpAdd);
	//create a temporary variable in the symbol table and 
	//return the variable name
	stEntry createTemp();

	//top-down parser functions
	void rat16S();
	void qualifier();
	void optDeclarationList();
	void declarationList();
	void declarationListP();
	void declaration();
	void ids(bool delcare);
	void idsP(bool declare);
	void statementList();
	void statementListP();
	void statement();
	void compound();
	void assign();
	void ifstmt();
	void ifstmtP();
	void write();
	void read();
	void whilestmt();
	void condition();
	void relop();
	void expression();
	void expressionP();
	void term();
	void termP();
	void factor();
	void primary();

	//helper functions for showing information
	void printInstTable();
	void printSymTable();
	void printErrorMsg(std::string msg);

	//input and output files
	std::ifstream inFile;
	std::ifstream lineFile;
	std::ofstream outFile;
	std::ofstream rawASM;

	//temporary variables to be used during IC generation
	pair temp, save;
	int symCounter, memAdd, instCounter, symTempCounter, lineCounter;
	
	//Symbol Table
	stEntry symTable[100];
	//Instruction Table
	instEntry instTable[500];

	//used for holding data type when declaring variables
	std::stack<std::string> typeStack;
	//used for holding address of last jump instruction
	std::stack<int> jumpStack;
};