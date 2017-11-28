// OmpDekstra.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "omp_tester.h"

int main(int argc, char * argv[])
{
	try
	{
		auto resolver = make_shared<ArgvResolver>();
		resolver->resolveArgs(argc, argv);
		auto tester = make_shared<Tester>(resolver, make_shared<omp_tester>());
		tester->runTest();		
	}
	catch (std::exception const &e)
	{
		std::cerr << "Error occurred: " << e.what() << std::endl;
		return 1;
	}
	return 0;
}