#include <iostream>
#include <fstream>
using namespace std;

#include "Sally.h"

int main() {
	string fname;


	fname = "test.sally";
	ifstream ifile(fname.c_str());

	Sally S(ifile);

	S.mainLoop();

	ifile.close();
	system("pause");
	return 0;
}