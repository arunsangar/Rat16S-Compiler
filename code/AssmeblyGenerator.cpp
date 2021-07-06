#include "AssmeblyGenerator.h"

void AssemblyGenerator::generateASM(std::string filename)
{
	//initialize variables
	symCounter = 0;
	symTempCounter = 0;
	memAdd = 5000;
	instCounter = 0;

	//open files (reading from token file and writing to ASM file)
	//lineFile is used to keep track of the current line in the 
	//source code, just in case an error needs to be printed
	inFile.open(filename + "tokenized.txt");
	lineFile.open(filename + "lineCount.txt");

	outFile.open(filename + "output.txt");
	rawASM.open(filename + "rawASM.txt");

	//get the first token
	lexer();

	//begin ASM generation
	rat16S();

	//output the instruction table and symbol table to the output file
	printInstTable();
	printSymTable();

	//closes files
	inFile.close();
	lineFile.close();
	outFile.close();
	rawASM.close();
}

void AssemblyGenerator::lexer()
{
	//store the next token and it's lexemme in a global variable
	inFile >> temp.lexemme >> temp.token;
	lineFile >> lineCounter;
}

stEntry AssemblyGenerator::getIdAdd(std::string id, bool declare)
{
	//check the entire symbol table for an entry token that matches id
	//return the entry if found
	//return empty entry if not found
	for (int i = 0; i < symCounter; i++)
		if (symTable[i].token.compare(id) == 0) return symTable[i];

	if(!declare) printErrorMsg("Symbol: " + id + ", not found");
	return stEntry();
}

void AssemblyGenerator::genInst(std::string op, int oprd)
{
	//generate an instruction
	instTable[instCounter].instAdd = instCounter + 1;
	instTable[instCounter].op = op;
	instTable[instCounter].oprd = oprd;
	instCounter++;
}

void AssemblyGenerator::backPatch(int jumpAdd)
{
	//patch a previous jump instruction's operand 
	int address = jumpStack.top();
	jumpStack.pop();
	instTable[address].oprd = jumpAdd;
}

stEntry AssemblyGenerator::createTemp()
{
	std::string name = "t" + std::to_string(symTempCounter);

	symTable[symCounter].token = name;
	symTable[symCounter].memAdd = memAdd;
	symTable[symCounter].type = "integer";
	symCounter++;
	symTempCounter++;
	memAdd++;

	return symTable[symCounter-1];
}

void AssemblyGenerator::rat16S()
{
	if (temp.token.compare("$$") == 0) lexer();
	else printErrorMsg("Expected $$");

	optDeclarationList();

	if (temp.token.compare("$$") == 0) lexer();
	else printErrorMsg("Expected $$");

	statementList();

	if (temp.token.compare("$$") == 0) lexer();
	else printErrorMsg("Expected $$");
}

void AssemblyGenerator::qualifier()
{
	if (temp.token.compare("integer") == 0 ||
		temp.token.compare("boolean") == 0)
	{
		//push the type of variable onto a stack so we can use
		//it for the symbol table
		typeStack.push(temp.token);
		lexer();
	}
	else printErrorMsg("Expected 'integer', 'boolean'");
}

void AssemblyGenerator::optDeclarationList()
{
	if (temp.token.compare("integer") == 0 ||
		temp.token.compare("boolean") == 0) declarationList();
	else if (temp.token.compare("$$") == 0) {}
	else printErrorMsg("Expected 'integer', 'boolean', $$");
}

void AssemblyGenerator::declarationList()
{
	if (temp.token.compare("integer") == 0 ||
		temp.token.compare("boolean") == 0)
	{
		declaration();

		if (temp.token.compare(";") == 0) lexer();
		else printErrorMsg("Expected ;");

		declarationListP();
	}
	else printErrorMsg("Expected 'integer', 'boolean'");
}

void AssemblyGenerator::declarationListP()
{
	if (temp.token.compare("integer") == 0 ||
		temp.token.compare("boolean") == 0) declarationList();
	else if (temp.token.compare("$$") == 0) {}
	else printErrorMsg("Expected 'integer', 'boolean', $$");
}

void AssemblyGenerator::declaration()
{
	if (temp.token.compare("integer") == 0 ||
		temp.token.compare("boolean") == 0)
	{
		qualifier();
		//this is used to declare variables 
		//so declare is set to true
		ids(true);
	}
	else printErrorMsg("Expected 'integer', 'boolean'");
}

void AssemblyGenerator::ids(bool declare)
{
	//declare is used to mark if this function should add symbols
	//to the symbol table or read values into a symbol
	if (temp.lexemme.compare("identifier") == 0)
	{
		//add item to symbol table
		if (declare)
		{
			stEntry symbol = getIdAdd(temp.token , true);
			if (symbol.memAdd != 0) printErrorMsg("Multiple declarations of: " + temp.token);
			std::string type = typeStack.top();

			symTable[symCounter].token = temp.token;
			symTable[symCounter].memAdd = memAdd;
			symTable[symCounter].type = type;
			symCounter++;
			memAdd++;
			lexer();
			idsP(declare);
		}
		//get input and save it to a memory location
		else
		{
			stEntry symbol = getIdAdd(temp.token);
			genInst("STDIN", -999);
			genInst("POPM", symbol.memAdd);
			lexer();
			idsP(declare);
		}
	}
	else printErrorMsg("Expected identifier");
}

void AssemblyGenerator::idsP(bool declare)
{
	if (temp.token.compare(",") == 0)
	{
		lexer();
		ids(declare);
	}
	else if (temp.token.compare(";") == 0 ||
		temp.token.compare(")") == 0) 
	{
		//if the declarations are done using this qualifier
		//remove it from the type stack
		if (declare) typeStack.pop();
	}
	else printErrorMsg("Expected ,, ;, )");
}

void AssemblyGenerator::statementList()
{
	if (temp.lexemme.compare("identifier") == 0 ||
		temp.token.compare("{") ||
		temp.token.compare("if") ||
		temp.token.compare("printf") ||
		temp.token.compare("scanf") ||
		temp.token.compare("while"))
	{
		statement();
		statementListP();
	}
	else printErrorMsg("Expected identifier, {, if, printf, scanf, while");
}

void AssemblyGenerator::statementListP()
{
	if (temp.lexemme.compare("identifier") == 0 ||
		temp.token.compare("{") == 0 ||
		temp.token.compare("if") == 0 ||
		temp.token.compare("printf") == 0 ||
		temp.token.compare("scanf") == 0 ||
		temp.token.compare("while") == 0) statementList();

	else if (temp.token.compare("}") == 0 ||
			 temp.token.compare("$$") == 0) {}

	else printErrorMsg("Expected identifier, {, if, printf, scanf, while, }, $$");
}

void AssemblyGenerator::statement()
{
	if (temp.lexemme.compare("identifier") == 0) assign();
	
	else if (temp.token.compare("{") == 0) compound();
	
	else if (temp.token.compare("if") == 0) ifstmt();

	else if (temp.token.compare("printf") == 0) write();
	
	else if (temp.token.compare("scanf") == 0) read();
	
	else if (temp.token.compare("while") == 0) whilestmt();

	else printErrorMsg("Expected identifier, {, if, printf, scanf, while");
}

void AssemblyGenerator::compound()
{
	if (temp.token.compare("{") == 0)
	{
		lexer();
		statementList();

		if (temp.token.compare("}") == 0) lexer();
		else printErrorMsg("Expected }");
	}
	else printErrorMsg("Expected {");
}

void AssemblyGenerator::assign()
{
	if (temp.lexemme.compare("identifier") == 0)
	{
		save = temp;

		stEntry symbol = getIdAdd(save.token);
		if (symbol.memAdd == 0) printErrorMsg("Symbol: " + save.token + ", not found");

		lexer();

		if (temp.token.compare(":=") == 0) lexer();
		else printErrorMsg("Expected :=");

		//if assigning an identifier make sure it is of the same type
		if (temp.lexemme.compare("identifier") == 0)
		{
			stEntry symbol2 = getIdAdd(temp.token);

			if (symbol.memAdd == 0) printErrorMsg("Symbol: " + save.token + ", not found");
			else if (symbol.type.compare(symbol2.type) == 0) expression();
			else printErrorMsg("Cannot convert " + symbol2.type + " to " + symbol.type);
		}
		else expression();

		//generate instruction to save the evaluated expression's value 
		//into the saved identifier
		genInst("POPM", symbol.memAdd);

		if (temp.token.compare(";") == 0) lexer();
		else printErrorMsg("Expected ;");
	}
	else printErrorMsg("Expected identifier");
}

void AssemblyGenerator::ifstmt()
{
	if (temp.token.compare("if") == 0)
	{
		lexer();

		if (temp.token.compare("(") == 0) lexer();
		else printErrorMsg("Expected (");

		condition();

		if (temp.token.compare(")") == 0) lexer();
		else printErrorMsg("Expected )");

		statement();
		ifstmtP();
	}
	else printErrorMsg("Expected if");
}

void AssemblyGenerator::ifstmtP()
{
	//Labels are used here because the program may end with an
	//if statement. In this case the jump instruction would 
	//be back patched with the last instruction in the if/if-else.
	//I used a label so there will be an extra instruction for
	//the jump instruction to jump to

	if (temp.token.compare("endif") == 0)
	{
		//generate label and back patch last jump instruction
		//to jump to this label's instruction address
		genInst("LABEL", -999);
		backPatch(instCounter);
		lexer();
	}

	else if (temp.token.compare("else") == 0)
	{
		//generate a jump instruction to unconditionally 
		//jump over the else, if the 'if' statement has executed
		genInst("JUMP", -999);
		//back patch the last 'jumpz' statement to jump to 
		//the instruction after the unconditional 'jump' 
		backPatch(instCounter + 1);
		//now push the unconditional 'jump' address onto the jump stack
		jumpStack.push(instCounter - 1);

		lexer();
		statement();

		//generate a label since this may be the last statement in the program
		genInst("LABEL", -999);

		if (temp.token.compare("endif") == 0)
		{
			lexer();
			//back patch the uncoditional jump
			backPatch(instCounter);
		}
		else printErrorMsg("Expected endif");
	}
	else printErrorMsg("Expected else, endif");
}

void AssemblyGenerator::write()
{

	if (temp.token.compare("printf") == 0)
	{
		lexer();
		if (temp.token.compare("(") == 0) lexer();
		else printErrorMsg("Expected (");

		//expression will push a number to TOS so we can
		//generate a write statement immediately after that
		expression();
		genInst("STDOUT", -999);

		if (temp.token.compare(")") == 0) lexer();
		else printErrorMsg("Expected )");

		if (temp.token.compare(";") == 0) lexer();
		else printErrorMsg("Expected ;");
	}
	else printErrorMsg("Expected printf");
}

void AssemblyGenerator::read()
{
	if (temp.token.compare("scanf") == 0)
	{
		lexer();
		if (temp.token.compare("(") == 0) lexer();
		else printErrorMsg("Expected (");

		//this is for reading values into identifiers
		//so declare is set to false
		ids(false);

		if (temp.token.compare(")") == 0) lexer();
		else printErrorMsg("Expected )");

		if (temp.token.compare(";") == 0) lexer();
		else printErrorMsg("Expected ;");
	}
	else printErrorMsg("Expected scanf");
}

void AssemblyGenerator::whilestmt()
{
	if (temp.token.compare("while") == 0)
	{
		//save address and create label so we can jump back later
		genInst("LABEL", -999);
		int address = instCounter;

		lexer();

		if (temp.token.compare("(") == 0) lexer();
		else printErrorMsg("Expected (");

		//this will create a 'jumpz' instruction
		condition();

		if (temp.token.compare(")") == 0) lexer();
		else printErrorMsg("Expected )");

		statement();

		//jump back to start of while loop
		genInst("JUMP", address);
		//create a label for 'jumpz'
		genInst("LABEL", -999);
		//back patch the 'jumpz'
		backPatch(instCounter);
	}
	else printErrorMsg("Expected while");
}

void AssemblyGenerator::condition()
{
	//if the token is an identifier, check if it's a boolean
	int isBool = 0;
	if (temp.lexemme.compare("identifier") == 0)
	{
		stEntry symbol = getIdAdd(temp.token);
		//if it is a boolean
		if (symbol.type.compare("boolean") == 0) isBool = 2;
		//if it is not a boolean
		else isBool = 1;
		//a third option for isBool is needed for error handling
	}

	//this is used to prevent arthimetic from being done on bools
	if (temp.token.compare("true") == 0 ||
		temp.token.compare("false") == 0 || isBool == 2)
	{
		expression();

		//checking if booleans are equal or not is allowed
		if (temp.token.compare("=") == 0 || temp.token.compare("/=") == 0)
		{
			save = temp;
			relop();

			if (temp.lexemme.compare("identifier") == 0)
			{
				stEntry symbol2 = getIdAdd(temp.token);

				if (symbol2.type.compare("boolean") == 0) expression();
				else printErrorMsg("Expected boolean");
			}
			else if (temp.token.compare("true") == 0 ||
				temp.token.compare("false") == 0) expression(); 
			else printErrorMsg("Expected true, false, identifier(boolean)");

			if (save.token.compare("=") == 0) genInst("EQU", -999);
			else
			{
				//the extra 'pushi' and 'equ' instructions flip the 
				//boolean on TOS
				genInst("EQU", -999);
				genInst("PUSHI", 0);
				genInst("EQU", -999);
			}

			jumpStack.push(instCounter);
			genInst("JUMPZ", -999);
		}
		//checking if booleans are greater than / less than is not allowed
		else if (temp.token.compare(">") == 0 || temp.token.compare("<") == 0 ||
				 temp.token.compare("=>") == 0 || temp.token.compare("<=") == 0)
			printErrorMsg("Attempting do arithmetic on boolean values");

		jumpStack.push(instCounter);
		genInst("JUMPZ", -999);
	}

	else if (temp.token.compare("-") == 0 ||
			 temp.lexemme.compare("integer") == 0 ||
			 temp.token.compare("(") == 0 || isBool == 1)
	{
		expression();

		//save the relational operator
		save = temp;
		
		relop();

		if (temp.lexemme.compare("identifier") == 0)
		{
			stEntry symbol2 = getIdAdd(temp.token);

			if (symbol2.type.compare("boolean") == 0) 
				printErrorMsg("Attempting to do arithmetic on boolean values");
		}
		else if (temp.token.compare("true") == 0 ||
				 temp.token.compare("false") == 0) 
				 printErrorMsg("Attempting to do arithmetic on boolean values");

		expression();

		//check which relational operator was saved
		//generate corresponding instructions
		if (save.token.compare("=") == 0) genInst("EQU", -999);
		else if (save.token.compare("<") == 0) genInst("LES", -999);
		else if (save.token.compare(">") == 0) genInst("GRT", -999);
		else if (save.token.compare("/=") == 0) 
		{
			//the extra 'pushi' and 'equ' instructions flip the 
			//boolean on TOS
			genInst("EQU", -999);
			genInst("PUSHI", 0);
			genInst("EQU", -999);
		}
		else if (save.token.compare("=>") == 0 || save.token.compare("<=") == 0)
		{
			//create temp variables to store the two values on TOS
			stEntry symbol1 = createTemp(), symbol2 = createTemp();

			//put the values on TOS into memory
			genInst("POPM", symbol2.memAdd);
			genInst("POPM", symbol1.memAdd);
			//push them back onto the TOS
			genInst("PUSHM", symbol1.memAdd);
			genInst("PUSHM", symbol2.memAdd);

			//check if they are equal
			genInst("EQU", -999);
			//if they are equal, skip the greater than comparison
			genInst("PUSHI", 0);
			genInst("EQU", -999);
			genInst("JUMPZ", instCounter + 6);

			//push stored values back onto TOS
			genInst("PUSHM", symbol1.memAdd);
			genInst("PUSHM", symbol2.memAdd);
			//check if the second value is greater than / less than
			save.token.compare("=>") == 0 ? genInst("GRT", -999) : genInst("LES", -999);
			
		}

		jumpStack.push(instCounter);
		genInst("JUMPZ", -999);

	}
	else printErrorMsg("Expected identifier, true, false, -, integer, (");
}

void AssemblyGenerator::relop()
{
	if (temp.token.compare("=") == 0 ||
		temp.token.compare("<") == 0 ||
		temp.token.compare(">") == 0 ||
		temp.token.compare("/=") == 0 ||
		temp.token.compare("=>") == 0 ||
		temp.token.compare("<=") == 0) lexer();
	else printErrorMsg("Expected =, <, >, /=, =>, <=");
}

void AssemblyGenerator::expression()
{
	//this is used for preventing arithmetic on bools
	if (temp.lexemme.compare("identifier") == 0 ||
		temp.token.compare("true") == 0 ||
		temp.token.compare("false") == 0 ||
		temp.token.compare("-") == 0 ||
		temp.lexemme.compare("integer") == 0 ||
		temp.token.compare("(") == 0)
	{
		term();
		expressionP();
	}
	else printErrorMsg("Expected identifier, true, false, -, integer, (");
}

void AssemblyGenerator::expressionP()
{
	if (temp.token.compare("+") == 0 ||
		temp.token.compare("-") == 0)
	{
		std::string op = temp.token;

		lexer();

		if (temp.lexemme.compare("identifier") == 0)
		{
			stEntry symbol = getIdAdd(temp.token);
			if (symbol.type.compare("boolean") == 0)
				printErrorMsg("Attempting to do arithmetic on boolean values");
		}

		term();

		//generate add/sub instruction
		op.compare("+") == 0 ? genInst("ADD", -999) : genInst("SUB", -999);

		expressionP();
	}
	else if (temp.token.compare(";") == 0 ||
		temp.token.compare(")") == 0 ||
		temp.token.compare("=") == 0 ||
		temp.token.compare("/=") == 0 ||
		temp.token.compare("<") == 0 ||
		temp.token.compare(">") == 0 ||
		temp.token.compare("=>") == 0 ||
		temp.token.compare("<=") == 0) {}
	else printErrorMsg("Expected +, -, ;, ), =, /=, <, >, =>, <=");
}

void AssemblyGenerator::term()
{
	//this is used to prevent arithmetic on bools
	if (temp.lexemme.compare("identifier") == 0 ||
		temp.token.compare("true") == 0 ||
		temp.token.compare("false") == 0 ||
		temp.token.compare("-") == 0 ||
		temp.lexemme.compare("integer") == 0 ||
		temp.token.compare("(") == 0)
	{
		factor();
		termP();
	}

	else printErrorMsg("Expected identifier, true, false, -, integer, (");
}

void AssemblyGenerator::termP()
{
	if (temp.token.compare("*") == 0 ||
		temp.token.compare("/") == 0)
	{
		std::string op = temp.token;

		lexer();

		if (temp.lexemme.compare("identifier") == 0)
		{
			stEntry symbol = getIdAdd(temp.token);
			if (symbol.type.compare("boolean") == 0)
				printErrorMsg("Attempting to do arithmetic on boolean values");
		}

		factor();
		termP();

		//generate add/sub instruction
		op.compare("*") == 0 ? genInst("MUL", -999) : genInst("DIV", -999);

		expressionP();
	}
	else if (temp.token.compare(";") == 0 ||
		temp.token.compare(")") == 0 ||
		temp.token.compare("+") == 0 ||
		temp.token.compare("-") == 0 ||
		temp.token.compare("=") == 0 ||
		temp.token.compare("/=") == 0 ||
		temp.token.compare(">") == 0 ||
		temp.token.compare("<") == 0 ||
		temp.token.compare("=>") == 0 ||
		temp.token.compare("<=") == 0) {}
	else printErrorMsg("Expected *, /, ;, ), +, -, =, /=, >, <, =>, <=");
}

void AssemblyGenerator::factor()
{
	if (temp.token.compare("-") == 0)
	{	
		lexer();

		//if the number has a negative sign in front of it
		//make the number a negative number
		if (temp.lexemme.compare("integer") == 0)
		{
			int x = stoi(temp.token);
			x -= x * 2;
			temp.token = std::to_string(x);
		}
		else printErrorMsg("Expected integer after '-'");

		primary();
	}
	else if (temp.lexemme.compare("identifier") == 0 ||
			 temp.lexemme.compare("integer") == 0 ||
			 temp.token.compare("(") == 0 ||
			 temp.token.compare("true") == 0 ||
			 temp.token.compare("false") == 0) primary();
	else printErrorMsg("Expected -, identifier, integer, (, true, false");
}

void AssemblyGenerator::primary()
{
	if (temp.lexemme.compare("identifier") == 0)
	{
		//generate instruction to push a value from memory onto TOS
		stEntry symbol = getIdAdd(temp.token);
		genInst("PUSHM", symbol.memAdd);
		lexer();
	}
	else if (temp.lexemme.compare("integer") == 0)
	{
		//generate instruction to push an integer onto TOS
		genInst("PUSHI", std::stoi(temp.token));
		lexer();
	}	
	else if (temp.token.compare("(") == 0)
	{
		//(<Expression>)
		lexer();
		expression();
		if (temp.token.compare(")") == 0) lexer();
		else printErrorMsg("Expected )");
	}
	else if (temp.token.compare("true") == 0)
	{
		//generate instruction to push 1 onto TOS for true
		genInst("PUSHI", 1);
		lexer();
	}
	else if (temp.token.compare("false") == 0)
	{
		//generate instruction to push 0 onto TOS for false
		genInst("PUSHI", 0);
		lexer();
	}
	else printErrorMsg("Expected identifier, integer, (, true, false");
}

void AssemblyGenerator::printInstTable()
{
	for (int i = 0; i < instCounter; i++)
	{
		outFile << std::setw(3) << instTable[i].instAdd 
			<< std::setw(7) << instTable[i].op;
		if (instTable[i].oprd == -999) outFile << std::endl;
		else outFile << std::setw(12) << instTable[i].oprd << std::endl;
	}
	outFile << "\n\n";
	for (int i = 0; i < instCounter; i++)
	{
		rawASM << std::setw(7) << instTable[i].op;
		if (instTable[i].oprd == -999) rawASM << std::endl;
		else rawASM << std::setw(12) << instTable[i].oprd << std::endl;
	}
}

void AssemblyGenerator::printSymTable()
{
	outFile << "  Type  " << "  Token Name" << "  Memory Address\n";
	outFile << "--------------------------------------\n";
	for (int i = 0; i < symCounter; i++)
	{
		outFile << std::setw(8) << symTable[i].type 
			<< std::setw(12) << symTable[i].token 
			<< std::setw(10) << symTable[i].memAdd << std::endl;
	}
}

void AssemblyGenerator::printErrorMsg(std::string msg)
{
		printInstTable();
		outFile << "----------Error Message----------\n";
		outFile << "        Error: " + msg + "\n";
		outFile << "Current Token: " + temp.token + "\n";
		outFile << "      Lexemme: " + temp.lexemme + "\n";
		outFile << "       Line #: " + std::to_string(lineCounter) + "\n\n";
		printSymTable();
		exit(-1);
}
