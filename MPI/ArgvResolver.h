#pragma once
#include <stdio.h>
#include <map>
#include <memory>
using namespace std;
class ArgvResolver
{
public:
	ArgvResolver();
	shared_ptr<map<int, string>> arguments;
	shared_ptr<map<string, string>> options;
	ArgvResolver addArgument(const int position, const string default) const;
	string getArgument();
	ArgvResolver* resolveArgs(char* []);
	ArgvResolver& addOption();
	void fill(int &plain_data, const size_t size) const;

	~ArgvResolver();
};

