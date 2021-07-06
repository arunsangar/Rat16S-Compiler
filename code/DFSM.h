#pragma once

typedef enum State { A, B, C, D, E, F, G, H, I, 
					 J, K, L, M, N, O, P, Q };

class DFSM
{
public:

	DFSM();

	State transition(State current, char input);

private:

	void InitTable();

	State table[17][11];
};