#include "stdafx.h"
#include "ArgvResolver.h"

ArgvResolver::ArgvResolver()
{
	arguments = make_shared <map< int, string>> ();
}
ArgvResolver ArgvResolver::addArgument(const int position, const string default) const
{
	(*arguments)[position] = default;
	return *this;
}

ArgvResolver::~ArgvResolver()
{
}
