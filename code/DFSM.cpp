#include "DFSM.h"

DFSM::DFSM() { InitTable(); }

State DFSM::transition(State current, char input)
{
	//integer/real tokens
	if (input >= '0' && input <= '9') return table[int(current)][0];
	else if (input == '.') return table[int(current)][1];
	
	//keyword/identifier tokens
	else if ((input >= 'a' && input <= 'z') ||
		     (input >= 'A' && input <= 'Z')) return table[int(current)][2];
	else if (input == '_') return table[int(current)][3];
	
	//operator/seperator tokens
	else if (input == ':') return table[int(current)][4];
	else if (input == '=') return table[int(current)][5];
	else if (input == '<' || input == '/') return table[int(current)][6];
	else if (input == '>') return table[int(current)][7];
	else if (input == '+' || input == '-' || input == '*')
		return table[int(current)][8];
	else if (input == '(' || input == ')' || input == ',' ||
		     input == '{' || input == '}' || input == ';')
		return table[int(current)][9];
	else if (input == '$') return table[int(current)][10];
	
	//unknown tokens
	else return P;
}

void DFSM::InitTable()
{
	//transitions for DFSM
	State temp[17][11] = { { B,J,C,Q,D,F,E,G,G,H,I },
						   { B,J,P,P,P,P,P,P,P,P,P },
						   { P,P,L,K,P,P,P,P,P,P,P },
						   { P,P,P,P,P,M,P,P,P,P,P },
						   { P,P,P,P,P,M,P,P,P,P,P },
						   { P,P,P,P,P,P,P,M,P,P,P },
						   { P,P,P,P,P,P,P,P,P,P,P },
						   { P,P,P,P,P,P,P,P,P,P,P },
						   { P,P,P,P,P,P,P,P,P,P,N },
						   { O,P,P,P,P,P,P,P,P,P,P },
						   { P,P,L,K,P,P,P,P,P,P,P },
						   { P,P,L,K,P,P,P,P,P,P,P },
						   { P,P,P,P,P,P,P,P,P,P,P },
						   { P,P,P,P,P,P,P,P,P,P,P },
						   { P,P,P,P,P,P,P,P,P,P,P },
						   { P,P,P,P,P,P,P,P,P,P,P },
						   { P,P,Q,Q,P,P,P,P,P,P,P } };

	for (int i = 0; i < 17; i++)
		for (int j = 0; j < 11; j++)
			table[i][j] = temp[i][j];
}