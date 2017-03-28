#include "Tester.h"
#include "stdafx.h"

Tester::Tester(shared_ptr<ArgvResolver> argvResolver, shared_ptr<IMpiTest> testObject)
{
	(*this).argvResolver = argvResolver;
	(*this).testObject = testObject;
}

int Tester::runTest(){
	auto before = std::chrono::high_resolution_clock::now();
	(*testObject).processArgvResolver(argvResolver);
	(*testObject).run();
	auto after = std::chrono::high_resolution_clock::now();
	auto time = std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();

	std::cout << "Time taken:" << (*testObject).getTitle() << ": " << time << "ms" << std::endl;



	return 1;
}

Tester::~Tester()
{
}
