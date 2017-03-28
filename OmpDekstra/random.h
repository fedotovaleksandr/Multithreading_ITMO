#include "stdafx.h"


template <typename T>
class random
{
	std::default_random_engine gen;
	std::uniform_real_distribution<double> dist;
public:
	random(double min, double max) : gen(std::chrono::system_clock::now().time_since_epoch().count()),
		dist(min, max)
	{
	}

	T next()
	{
		return dist(gen);
	}
};