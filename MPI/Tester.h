#pragma once
#include "stdafx.h"

__interface IMpiTest{
public:
	int run() = 0;
	void processArgvResolver(shared_ptr<ArgvResolver> argvResolver) = 0;
	string getTitle() = 0;
};

class Tester
{
private:
	
	shared_ptr<ArgvResolver> argvResolver;
	shared_ptr<IMpiTest> testObject;
public:
	Tester(){};
	Tester(shared_ptr<ArgvResolver> argvResolver, shared_ptr<IMpiTest> testObject);
	int runTest();
	~Tester();
};


