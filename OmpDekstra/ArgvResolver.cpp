#include "stdafx.h"
#include "ArgvResolver.h"

ArgvResolver::ArgvResolver(const int argc, char* argv[])
{
	this->argc = argc;
	this->argv = argv;
	this->arguments = std::map< string, string>();
}
ArgvResolver ArgvResolver::addArgument(const string arg, const string default)
{
	if (arguments.find(arg) == arguments.end()){
		arguments[arg] = default;
	}
	return *this;
}

int ArgvResolver::getArgc(){
	return argc;
};
char** ArgvResolver::getArgv(){
	return argv;
};

ArgvResolver ArgvResolver::resolveArgs(const int argc, char* argv[])
{

	path = argv[0];
	string previus = "";
	for (int index = 1; index < argc; index++)	{
		string current = argv[index];
		string::size_type loc = current.find("-", 0);
		if (loc != string::npos) {
			previus = current;
		}
		else {
			if (!previus.empty()){
				arguments[string(previus)] = string(current);
				previus.clear();
			}
		}

	}
	return *this;
}

string ArgvResolver::getArgument(const string arg)
{
	if ((arguments).find(arg) == (arguments).end()) {
		throw std::runtime_error("Argument not find: " + arg);
	}
	else {
		return (arguments)[arg];
	}
}

string ArgvResolver::printArgRow(){
	std::stringstream answer;
	for (auto const &item : (arguments)) {
		string key = item.first;
		string value = item.second;
		answer << key << "=" << value << std::endl;

	}
	return answer.str();
}


ArgvResolver::~ArgvResolver()
{
}
