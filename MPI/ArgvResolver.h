#pragma once
#include <stdio.h>
#include <map>
#include <memory>
using namespace std;
class ArgvResolver
{
public:
	ArgvResolver(const int argc, char* argv[]);
	ArgvResolver(){};
	string path;
	map<string, string> arguments;
	int argc;
	char** argv;
	int getArgc();
	char** getArgv();
	ArgvResolver addArgument(const string arg, const string default);
	string getArgument(const string arg);
	string printArgRow();
	ArgvResolver resolveArgs(const int argc, char* argv[]);
	ArgvResolver resolveArgs();
	~ArgvResolver();
};

