#include "LexicalAnalyzer.h"

void LexicalAnalyzer::lexicalAnalysis(std::string filename)
{
	removeComments(filename);

	//open input file (text file w/ comments removed)
	//open output file (token/lexemme file)
	std::ifstream inFile(filename + "RC.txt");
	std::ofstream outFile(filename + "tokenized.txt");
	std::ofstream lineFile(filename + "lineCount.txt");

	//declare temporary variables
	std::string lexemme = "";
	State current = A, previous = A;
	char temp = 'a';
	int lineCounter = 1;

	//create the finite state machine
	DFSM dfsm; 

	//loop until there are no more characters in the file
	while (inFile.get(temp))
	{

		//get another character if char is a new line or it is a space and current 
		//state is A	(the lexar will output unecessary unknown tokens for them)
		while ((current == A && (temp == '\n' || temp == '\t'
			|| int(temp) == 32)) && !inFile.eof())
		{
			if (temp == '\n') lineCounter++;
			inFile.get(temp);
		}

		//save previous state and transition using DFSM
		previous = current;
		current = dfsm.transition(current, temp);

		//if current state is unknown, check if previous state was an accepting
		//state or not
		if (current == P)
		{

			//previous = integer accepting state
			if (previous == B)  outFile << "   integer\t" << lexemme << "\n";

			//previous = identifier accepting state
			else if (previous == C) outFile << "identifier\t" << lexemme << "\n";

			//previous = keyword/identifier accepting state
			else if (previous == L)
			{
				if (isKeyword(lexemme)) outFile << "   keyword\t" << lexemme << "\n";
				else outFile << "identifier\t" << lexemme << "\n";
			}

			//previous = a seperator accepting state
			else if (previous == D || previous == H || previous == N)  
				outFile << " seperator\t" << lexemme << "\n";

			//previous = an operator accepting state
			else if (previous == E || previous == F || previous == G || previous == M)
				outFile << "  operator\t" << lexemme << "\n";

			//previous = real accepting state
			else if (previous == O) outFile << "      real\t" << lexemme << "\n"; 
			
			//if previous state was not an accepting state (unknown token)
			else 
			{
				lexemme += temp;
				outFile << "   unknown\t" << lexemme << "\n";
				lexemme.clear();
				current = A;
				continue;
			}

			lineFile << std::to_string(lineCounter) + "\n";
			//clear the temporary storage lexemme for next one
			lexemme.clear();
			//move file pointer back one character
			inFile.unget();
			//change state back to starting state
			current = A;
		}
		
		//current state = known state (i.e. not end of token)
		//add the character to the lexemme
		else { lexemme += tolower(temp); }
	}

	//close the input/output files
	inFile.close();
	outFile.close();
	lineFile.close();
}

void LexicalAnalyzer::removeComments(std::string filename)
{
	//open the commented code file and an output file for raw code
	std::ifstream inFile(filename + ".txt");
	std::ofstream outFile(filename + "RC.txt");

	//temporary variable for storing input character
	char temp = 'a';

	//loop until no more characters available
	while (inFile.get(temp))
	{
		//get characters within '[' and ']'
		if (temp == '[') while (temp != ']') inFile >> temp;
		//send the rest to the output file
		else outFile << temp;
	}

	//close input/output files
	inFile.close();
	outFile.close();
}

//return true if the lexemme is a keyword
bool LexicalAnalyzer::isKeyword(std::string word)
{
	for (int i = 0; i < 14; i++) if (word.compare(keywords[i]) == 0) return true;
	return false;
}