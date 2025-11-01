// CMakeProject1.cpp : Defines the entry point for the application.
//

#include <iostream>
#include "src/BaseRunner.h"
#include "SFML-Base.h"

int main()
{
	cout << "Hello CMake." << endl;

	BaseRunner runner;
	runner.run();


	return 0;
}
