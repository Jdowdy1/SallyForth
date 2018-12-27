// File: Sally.cpp
//
// CMSC 341 Spring 2017 Project 2
//
// Implementation of member functions of Sally Forth interpreter
//

#include <iostream>
#include <string>
#include <list>
#include <stack>
#include <stdexcept>
#include <cstdlib>
using namespace std;

#include "Sally.h"


// Basic Token constructor. Just assigns values.
//
Token::Token(TokenKind kind, int val, string txt) {
	m_kind = kind;
	m_value = val;
	m_text = txt;
}


// Basic SymTabEntry constructor. Just assigns values.
//
SymTabEntry::SymTabEntry(TokenKind kind, int val, operation_t fptr) {
	m_kind = kind;
	m_value = val;
	m_dothis = fptr;
}


// Constructor for Sally Forth interpreter.
// Adds built-in functions to the symbol table.
//
Sally::Sally(istream& input_stream) :
	istrm(input_stream)  // use member initializer to bind reference
{

	symtab["DUMP"] = SymTabEntry(KEYWORD, 0, &doDUMP);

	symtab["+"] = SymTabEntry(KEYWORD, 0, &doPlus);
	symtab["-"] = SymTabEntry(KEYWORD, 0, &doMinus);
	symtab["*"] = SymTabEntry(KEYWORD, 0, &doTimes);
	symtab["/"] = SymTabEntry(KEYWORD, 0, &doDivide);
	symtab["%"] = SymTabEntry(KEYWORD, 0, &doMod);
	symtab["NEG"] = SymTabEntry(KEYWORD, 0, &doNEG);

	symtab["."] = SymTabEntry(KEYWORD, 0, &doDot);
	symtab["SP"] = SymTabEntry(KEYWORD, 0, &doSP);
	symtab["CR"] = SymTabEntry(KEYWORD, 0, &doCR);

	symtab["DUP"] = SymTabEntry(KEYWORD, 0, &doDUP);
	symtab["DROP"] = SymTabEntry(KEYWORD, 0, &doDROP);
	symtab["SWAP"] = SymTabEntry(KEYWORD, 0, &doSWAP);
	symtab["ROT"] = SymTabEntry(KEYWORD, 0, &doROT);

	symtab["SET"] = SymTabEntry(KEYWORD, 0, &doSET);
	symtab["@"] = SymTabEntry(KEYWORD, 0, &doAT);
	symtab["!"] = SymTabEntry(KEYWORD, 0, &doStore);

	symtab["<"] = SymTabEntry(KEYWORD, 0, &doLess);
	symtab[">"] = SymTabEntry(KEYWORD, 0, &doGreat);
	symtab["<="] = SymTabEntry(KEYWORD, 0, &doLessEq);
	symtab[">="] = SymTabEntry(KEYWORD, 0, &doGreatEq);
	symtab["=="] = SymTabEntry(KEYWORD, 0, &doEq);
	symtab["!="] = SymTabEntry(KEYWORD, 0, &doNotEq);

	symtab["AND"] = SymTabEntry(KEYWORD, 0, &doAnd);
	symtab["NOT"] = SymTabEntry(KEYWORD, 0, &doNot);
	symtab["OR"] = SymTabEntry(KEYWORD, 0, &doOr);

	symtab["IFTHEN"] = SymTabEntry(KEYWORD, 0, &doIfThen);
	symtab["ELSE"] = SymTabEntry(KEYWORD, 0, &doElse);
	symtab["ENDIF"] = SymTabEntry(KEYWORD, 0, &doEndIf);

	symtab["DO"] = SymTabEntry(KEYWORD, 0, &doDo);
	symtab["UNTIL"] = SymTabEntry(KEYWORD, 0, &doUntil);


}


// This function should be called when tkBuffer is empty.
// It adds tokens to tkBuffer.
//
// This function returns when an empty line was entered 
// or if the end-of-file has been reached.
//
// This function returns false when the end-of-file was encountered.
// 
// Processing done by fillBuffer()
//   - detects and ignores comments.
//   - detects string literals and combines as 1 token
//   - detetcs base 10 numbers
// 
//
bool Sally::fillBuffer() {
	string line;     // single line of input
	int pos;         // current position in the line
	int len;         // # of char in current token
	long int n;      // int value of token
	char *endPtr;    // used with strtol()


	while (true) {    // keep reading until empty line read or eof

					  // get one line from standard in
					  //
		getline(istrm, line);

		// if "normal" empty line encountered, return to mainLoop
		//
		if (line.empty() && !istrm.eof()) {
			return true;
		}

		// if eof encountered, return to mainLoop, but say no more
		// input available
		//
		if (istrm.eof()) {
			return false;
		}


		// Process line read

		pos = 0;                      // start from the beginning

									  // skip over initial spaces & tabs
									  //
		while (line[pos] != '\0' && (line[pos] == ' ' || line[pos] == '\t')) {
			pos++;
		}

		// Keep going until end of line
		//
		while (line[pos] != '\0') {

			// is it a comment?? skip rest of line.
			//
			if (line[pos] == '/' && line[pos + 1] == '/') break;

			// is it a string literal? 
			//
			if (line[pos] == '.' && line[pos + 1] == '"') {

				pos += 2;  // skip over the ."
				len = 0;   // track length of literal

						   // look for matching quote or end of line
						   //
				while (line[pos + len] != '\0' && line[pos + len] != '"') {
					len++;
				}

				// make new string with characters from
				// line[pos] to line[pos+len-1]
				string literal(line, pos, len);  // copy from pos for len chars

												 // Add to token list
												 //
				tkBuffer.push_back(Token(STRING, 0, literal));

				// Different update if end reached or " found
				//
				if (line[pos + len] == '\0') {
					pos = pos + len;
				}
				else {
					pos = pos + len + 1;
				}

			}
			else {  // otherwise "normal" token

				len = 0;  // track length of token

						  // line[pos] should be an non-white space character
						  // look for end of line or space or tab
						  //
				while (line[pos + len] != '\0' && line[pos + len] != ' ' && line[pos + len] != '\t') {
					len++;
				}

				string literal(line, pos, len);   // copy form pos for len chars
				pos = pos + len;

				// Try to convert to a number
				//
				n = strtol(literal.c_str(), &endPtr, 10);

				if (*endPtr == '\0') {
					tkBuffer.push_back(Token(INTEGER, n, literal));
				}
				else {
					tkBuffer.push_back(Token(UNKNOWN, 0, literal));
				}
			}

			// skip over trailing spaces & tabs
			//
			while (line[pos] != '\0' && (line[pos] == ' ' || line[pos] == '\t')) {
				pos++;
			}

		}
	}
}



// Return next token from tkBuffer.
// Call fillBuffer() if needed.
// Checks for end-of-file and throws exception 
//
Token Sally::nextToken() {
	Token tk;
	bool more = true;

	while (more && tkBuffer.empty()) {
		more = fillBuffer();
	}

	if (!more && tkBuffer.empty()) {
		throw EOProgram("End of Program");
	}

	tk = tkBuffer.front();
	tkBuffer.pop_front();
	return tk;
}


// The main interpreter loop of the Sally Forth interpreter.
// It gets a token and either push the token onto the parameter
// stack or looks for it in the symbol table.
//
//
void Sally::mainLoop() {

	Token tk;
	map<string, SymTabEntry>::iterator it;

	try {
		while (1) {
			tk = nextToken();

			if (tk.m_kind == INTEGER || tk.m_kind == STRING) {

				// if INTEGER or STRING just push onto stack
				params.push(tk);

			}
			else {
				it = symtab.find(tk.m_text);

				if (it == symtab.end()) {   // not in symtab

					params.push(tk);

				}
				else if (it->second.m_kind == KEYWORD) {

					// invoke the function for this operation
					//
					it->second.m_dothis(this);

				}
				else if (it->second.m_kind == VARIABLE) {

					// variables are pushed as tokens
					//
					tk.m_kind = VARIABLE;
					params.push(tk);

				}
				else {

					// default action
					//
					params.push(tk);

				}
			}
		}

	}
	catch (EOProgram& e) {

		cerr << "End of Program\n";
		if (params.size() == 0) {
			cerr << "Parameter stack empty.\n";
		}
		else {
			cerr << "Parameter stack has " << params.size() << " token(s).\n";
		}

	}
	catch (out_of_range& e) {

		cerr << "Parameter stack underflow??\n";

	}
	catch (...) {

		cerr << "Unexpected exception caught\n";

	}
}

// -------------------------------------------------------


void Sally::doPlus(Sally *Sptr) {
	Token p1, p2;

	if (Sptr->params.size() < 2) {
		throw out_of_range("Need two parameters for +.");
	}
	p1 = Sptr->params.top();
	Sptr->params.pop();
	p2 = Sptr->params.top();
	Sptr->params.pop();
	int answer = p2.m_value + p1.m_value;
	Sptr->params.push(Token(INTEGER, answer, ""));
}


void Sally::doMinus(Sally *Sptr) {
	Token p1, p2;

	if (Sptr->params.size() < 2) {
		throw out_of_range("Need two parameters for -.");
	}
	p1 = Sptr->params.top();
	Sptr->params.pop();
	p2 = Sptr->params.top();
	Sptr->params.pop();
	int answer = p2.m_value - p1.m_value;
	Sptr->params.push(Token(INTEGER, answer, ""));
}


void Sally::doTimes(Sally *Sptr) {
	Token p1, p2;

	if (Sptr->params.size() < 2) {
		throw out_of_range("Need two parameters for *.");
	}
	p1 = Sptr->params.top();
	Sptr->params.pop();
	p2 = Sptr->params.top();
	Sptr->params.pop();
	int answer = p2.m_value * p1.m_value;
	Sptr->params.push(Token(INTEGER, answer, ""));
}


void Sally::doDivide(Sally *Sptr) {
	Token p1, p2;

	if (Sptr->params.size() < 2) {
		throw out_of_range("Need two parameters for /.");
	}
	p1 = Sptr->params.top();
	Sptr->params.pop();
	p2 = Sptr->params.top();
	Sptr->params.pop();
	int answer = p2.m_value / p1.m_value;
	Sptr->params.push(Token(INTEGER, answer, ""));
}


void Sally::doMod(Sally *Sptr) {
	Token p1, p2;

	if (Sptr->params.size() < 2) {
		throw out_of_range("Need two parameters for %.");
	}
	p1 = Sptr->params.top();
	Sptr->params.pop();
	p2 = Sptr->params.top();
	Sptr->params.pop();
	int answer = p2.m_value % p1.m_value;
	Sptr->params.push(Token(INTEGER, answer, ""));
}


void Sally::doNEG(Sally *Sptr) {
	Token p;

	if (Sptr->params.size() < 1) {
		throw out_of_range("Need one parameter for NEG.");
	}
	p = Sptr->params.top();
	Sptr->params.pop();
	Sptr->params.push(Token(INTEGER, -p.m_value, ""));
}


void Sally::doDot(Sally *Sptr) {

	Token p;
	if (Sptr->params.size() < 1) {
		throw out_of_range("Need one parameter for .");
	}

	p = Sptr->params.top();
	Sptr->params.pop();

	if (p.m_kind == INTEGER) {
		cout << p.m_value;
	}
	else {
		cout << p.m_text;
	}
}


void Sally::doSP(Sally *Sptr) {
	cout << " ";
}


void Sally::doCR(Sally *Sptr) {
	cout << endl;
}

void Sally::doDUMP(Sally *Sptr) {

	// do whatever for debugging
}

//copy the top value of the stack and push it to the stack
void Sally::doDUP(Sally *Sptr) {
	if (Sptr->params.size() < 1) {
		throw out_of_range("Need one parameters for DUP.");
	}
	Token p1;
	p1 = Sptr->params.top();//p1 copies
	Sptr->params.push(p1);//p1 is pushed
}

//take the top value off the stack
void Sally::doDROP(Sally *Sptr) {
	if (Sptr->params.size() < 1) {
		throw out_of_range("Need one parameters for DROP.");
	}
	Sptr->params.pop();//taking the top value off the stack
}

//swap the top two values on the stack
void Sally::doSWAP(Sally *Sptr) {
	if (Sptr->params.size() < 2) {
		throw out_of_range("Need two parameters for SWAP.");
	}
	//remove and save the top two values of the stack
	//then print them backwards
	Token p1, p2;
	p1 = Sptr->params.top();
	Sptr->params.pop();
	p2 = Sptr->params.top();
	Sptr->params.pop();
	Sptr->params.push(p1);
	Sptr->params.push(p2);
}

//rotate the top 3 values
void Sally::doROT(Sally *Sptr) {
	if (Sptr->params.size() < 3) {
		throw out_of_range("Need three parameters for ROT.");
	}

	//remove and save the top three values
	//push the values in order of 2 1 3
	Token p1, p2, p3;
	p1 = Sptr->params.top();
	Sptr->params.pop();
	p2 = Sptr->params.top();
	Sptr->params.pop();
	p3 = Sptr->params.top();
	Sptr->params.pop();

	Sptr->params.push(p2);
	Sptr->params.push(p1);
	Sptr->params.push(p3);
}

//create a variable
void Sally::doSET(Sally *Sptr) {
	if (Sptr->params.size() < 2) {
		throw out_of_range("Need two parameters for SET.");
	}
	//remove and save the top two values
	Token p1, p2;
	p1 = Sptr->params.top();
	Sptr->params.pop();
	p2 = Sptr->params.top();
	Sptr->params.pop();

	//create an iterator to find if the variable already exist
	map<string, SymTabEntry>::iterator it;
	it = Sptr->symtab.find(p1.m_text);

	if (it == Sptr->symtab.end()) {
		Sptr->symtab[p1.m_text] = SymTabEntry(VARIABLE, p2.m_value, NULL);  //if there is no variable of the same name
																			//Create the new variable in the symTab
	}
	else {
		cout << "ERROR: " << p1.m_text << " is already a variable" << endl;
	}

}
//retrieve the value of the variable
void Sally::doAT(Sally *Sptr) {
	Token p1;
	Token p2;

	p1 = Sptr->params.top();//take the first value off the stack that has the name
	Sptr->params.pop();

	map<string, SymTabEntry>::iterator it;
	it = Sptr->symtab.find(p1.m_text);   //find that variable in the symTab

	if (it == Sptr->symtab.end()) {
		cout << "ERROR: " << p1.m_text << " is not a variable" << endl; //if the variable does not exist

	}
	else {
		p2.m_kind = INTEGER;
		p2.m_value = Sptr->symtab.find(p1.m_text)->second.m_value; //retrieve the value

		Sptr->params.push(p2);//push the value to the stack
	}
}
//store a value to the variable
void Sally::doStore(Sally *Sptr) {
	if (Sptr->params.size() < 2) {
		throw out_of_range("Need two parameters for !.");
	}
	//remove and save the top 2 values of the stack
	Token p1, p2;
	p1 = Sptr->params.top();
	Sptr->params.pop();
	p2 = Sptr->params.top();
	Sptr->params.pop();

	map<string, SymTabEntry>::iterator it;
	it = Sptr->symtab.find(p1.m_text);   //find the variable in the symTab

	if (it == Sptr->symtab.end()) {
		cout << "ERROR: " << p1.m_text << " is not a variable" << endl;//if the variable does not exist

	}
	else {
		Sptr->symtab[p1.m_text] = SymTabEntry(VARIABLE, p2.m_value, NULL);  //set the variable equal to the input value
	}
}
//comparison operator: less than
void Sally::doLess(Sally *Sptr) {
	if (Sptr->params.size() < 2) {
		throw out_of_range("Need two parameters for <.");
	}
	Token p1, p2;
	//take two values off the top of the stack and save them
	p1 = Sptr->params.top();
	Sptr->params.pop();
	p2 = Sptr->params.top();
	Sptr->params.pop();

	Token p3;
	p3.m_kind = KEYWORD;//the pushed token will be a KEYWORD for a boolean value
						//the if else checks what the the boolean value will be
	if (p2.m_value < p1.m_value) {
		p3.m_value = 1;
		p3.m_text = "True";
	}
	else {
		p3.m_value = 0;
		p3.m_text = "False";
	}
	Sptr->params.push(p3);//push the boolean value to the stack
}
//comparison operator: Greater than
void Sally::doGreat(Sally *Sptr) {
	if (Sptr->params.size() < 2) {
		throw out_of_range("Need two parameters for >.");
	}
	Token p1, p2;
	//remove the top two values and save them
	p1 = Sptr->params.top();
	Sptr->params.pop();
	p2 = Sptr->params.top();
	Sptr->params.pop();

	Token p3;
	p3.m_kind = KEYWORD;//p3 will be a boolean
						//checking if true or false
	if (p2.m_value > p1.m_value) {
		p3.m_value = 1;
		p3.m_text = "True";
	}
	else {
		p3.m_value = 0;
		p3.m_text = "False";
	}
	Sptr->params.push(p3);//push the boolean to the stack
}
//comparison operator: less than or equal to
void Sally::doLessEq(Sally *Sptr) {
	if (Sptr->params.size() < 2) {
		throw out_of_range("Need two parameters for <.");
	}
	Token p1, p2;
	//two values, save
	p1 = Sptr->params.top();
	Sptr->params.pop();
	p2 = Sptr->params.top();
	Sptr->params.pop();

	Token p3;
	p3.m_kind = KEYWORD;//boolean value

						//either true or false
	if (p2.m_value <= p1.m_value) {
		p3.m_value = 1;
		p3.m_text = "True";
	}
	else {
		p3.m_value = 0;
		p3.m_text = "False";
	}
	Sptr->params.push(p3);//push value to stack
}
//comparison operator: Greater than or equal to
void Sally::doGreatEq(Sally *Sptr) {
	if (Sptr->params.size() < 2) {
		throw out_of_range("Need two parameters for >.");
	}
	Token p1, p2;

	p1 = Sptr->params.top();
	Sptr->params.pop();
	p2 = Sptr->params.top();
	Sptr->params.pop();

	Token p3;
	p3.m_kind = KEYWORD;
	//check if true or false
	if (p2.m_value >= p1.m_value) {
		p3.m_value = 1;
		p3.m_text = "True";
	}
	else {
		p3.m_value = 0;
		p3.m_text = "False";
	}
	Sptr->params.push(p3);//push boolean value to stack
}

//comparison operator: Equal to
void Sally::doEq(Sally *Sptr) {
	if (Sptr->params.size() < 2) {
		throw out_of_range("Need two parameters for >.");
	}
	Token p1, p2;

	p1 = Sptr->params.top();
	Sptr->params.pop();
	p2 = Sptr->params.top();
	Sptr->params.pop();

	Token p3;
	p3.m_kind = KEYWORD;
	if (p2.m_value == p1.m_value) {
		p3.m_value = 1;
		p3.m_text = "True";
	}
	else {
		p3.m_value = 0;
		p3.m_text = "False";
	}
	Sptr->params.push(p3);
}
//comparison operator: Not equal to
void Sally::doNotEq(Sally *Sptr) {
	if (Sptr->params.size() < 2) {
		throw out_of_range("Need two parameters for >.");
	}
	Token p1, p2;

	p1 = Sptr->params.top();
	Sptr->params.pop();
	p2 = Sptr->params.top();
	Sptr->params.pop();

	Token p3;
	p3.m_kind = KEYWORD;
	if (p2.m_value != p1.m_value) {
		p3.m_value = 1;
		p3.m_text = "True";
	}
	else {
		p3.m_value = 0;
		p3.m_text = "False";
	}
	Sptr->params.push(p3);
}

//Logical operator
void Sally::doOr(Sally *Sptr) {
	if (Sptr->params.size() < 2) {
		throw out_of_range("Need two parameters for >.");
	}
	Token p1, p2, p3;

	p1 = Sptr->params.top();
	Sptr->params.pop();
	p2 = Sptr->params.top();
	Sptr->params.pop();
	//take two values off the stack
	p3.m_kind = KEYWORD;
	if (p1.m_value != 0 || p2.m_value != 0) {
		p3.m_text = "True";
		p3.m_value = 1;
	}											//check if either of them are true
	else {
		p3.m_text = "False";
		p3.m_value = 0;
	}
	Sptr->params.push(p3);//push the resulting boolean to the stack
}
//logical operator: AND
void Sally::doAnd(Sally *Sptr) {
	if (Sptr->params.size() < 2) {
		throw out_of_range("Need two parameters for >.");
	}
	Token p1, p2, p3;

	p1 = Sptr->params.top();
	Sptr->params.pop();
	p2 = Sptr->params.top();
	Sptr->params.pop();
	//take two off the stack
	p3.m_kind = KEYWORD;
	if (p1.m_value != 0 && p2.m_value != 0) {
		p3.m_text = "True";
		p3.m_value = 1;
	}											//check if both are true
	else {
		p3.m_text = "False";
		p3.m_value = 0;
	}
	Sptr->params.push(p3);//push the resulting boolean value to the stack
}
//logical operator: not
void Sally::doNot(Sally *Sptr) {
	if (Sptr->params.size() < 1) {
		throw out_of_range("Need two parameters for >.");
	}
	Token p1, p3;

	p1 = Sptr->params.top();
	Sptr->params.pop();

	p3.m_kind = KEYWORD;
	if (p1.m_value == 0) {
		p3.m_text = "True";
		p3.m_value = 1;
	}									//checks what boolean value it is and then sets the opposite
	else {
		p3.m_text = "False";
		p3.m_value = 0;
	}
	Sptr->params.push(p3);
}
//If then operator
//If the statement is true. IFTHEN will return
//if the statement is false. IF THEN uses a counter to retrieve the correct else statement

void Sally::doIfThen(Sally *Sptr) {
	int count = 0;
	Token p1;
	Token p2;
	p1 = Sptr->params.top();//take the top boolean value off the stack
	Sptr->params.pop();

	if (p1.m_value == 1) {
		return;			//this checks if the statement was false and then returns to operate on the next line
	}
	else
	{
		count++;		  //When count is 0, we have reached the correct ELSE statement
		while (count > 0) //the counter increments up for each IFTHEN
						  //Counter decrements for each ELSE
		{				  //This allows the program to reach the correct ELSE statement when IFTHEN is false
			p2 = Sptr->nextToken();
			if (p2.m_text == "IFTHEN")
			{
				count++;
			}
			else if (p2.m_text == "ELSE")
			{
				count--;

			}
		}
	}
}
//When we call an ELSE, We want to reach the end of the file
//We use a counter much like the IFTHEN statement
void Sally::doElse(Sally *Sptr) {
	int count = 1;
	Token p2;
	while (count > 0)//when counter is 0, we have reached the end
	{
		p2 = Sptr->nextToken();
		if (p2.m_text == "ELSE")
		{
			count++;			//increment for each ELSE
		}
		else if (p2.m_text == "ENDIF")
		{
			count--;			//decrement for each ENDIF

		}
	}
}
//ENDIF is empty to remove the last ENDIF off the stack
void Sally::doEndIf(Sally *Sptr) {
}


//DO UNTIL uses the STL functions splice() and assign()
//DO UNTIL also uses two temporary lists
//When tkBuffer runs
//We use splice to one of the temporary list into the begining of the tkbuffer(The original list)
void Sally::doDo(Sally *Sptr) {
	int count = 0;
	Token p;
	//int i = 0;
	while (count != -1)
	{
		p = Sptr->nextToken();


		Sptr->doList.push_back(p);
		if (p.m_text == "UNTIL") {	//this uses nextToken to assign the Tokens to the first temporary list
			count = -1;

		}
	}

	Sptr->doList2.assign(Sptr->doList.begin(), Sptr->doList.end());//assign then copies the first temporary list
																   //	to the second

	Sptr->tkBuffer.splice(Sptr->tkBuffer.begin(), Sptr->doList);	//then splice is used to reput the Tokens 
																	//back into tkBuffer
																	//list1 is empty after this

	Sptr->doList.assign(Sptr->doList2.begin(), Sptr->doList2.end());//then we use the second temporary list to recopy the 
																	//Tokens into the first temp list

}

//UNTIL checks if the case is true or not
//if the case is false then we reput the tokens back into the buffer
void Sally::doUntil(Sally *Sptr) {

	Token p;
	p = Sptr->params.top();

	Sptr->params.pop();

	if (p.m_value == 1) {
		return;		//statement is true and return. DOUNTIL is done
	}
	else {
		Sptr->tkBuffer.splice(Sptr->tkBuffer.begin(), Sptr->doList);//when the statement is false. Splice the first temp list into 
																	//tkBuffer
		Sptr->doList.assign(Sptr->doList2.begin(), Sptr->doList2.end());//the first temp list is empty. Use the second to reassign it
	}
}






