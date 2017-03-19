#pragma once
#include "stdafx.h"
class SpecVector
{
public:
	typedef std::vector<long double> data_t;
	const std::shared_ptr<data_t> data;
	const size_t size;
public:
	typedef std::shared_ptr<SpecVector> vector_t;
	explicit SpecVector(const int size);
	static vector_t generateVector(const int size);
	std::shared_ptr<std::vector<long double>> getData() const;
	void fill(std::shared_ptr<std::vector<long double>> &plain_data, const int size) const;

	friend std::istream &operator >>(std::istream &input, SpecVector &o)
	{
		for (size_t i = 0; i < o.get_size(); ++i)
		{
			if (!input.eof())
			{
				input >> o[i];
			}
			else
			{
				std::stringstream ss;
				ss << "Input stream contains less than " << o.get_size() << " elements!";
				throw std::range_error(ss.str());
			}
		}

		return input;
	}

	friend std::ostream &operator <<(std::ostream &output, SpecVector &o)
	{
		for (size_t i = 0; i < o.get_size(); ++i)
		{
			output << o[i];
			if (i != o.get_size() - 1)
			{
				output << " ";
			}
		}

		return output;
	}

	friend std::ostream &operator <<(std::ofstream &output, SpecVector &o)
	{
		if (!output)
		{
			throw std::runtime_error("Output stream is not ready to write!");
		}

		output.exceptions(std::ifstream::badbit | std::ifstream::failbit);
		output << o.get_size() << std::endl;

		for (size_t i = 0; i < o.get_size(); ++i)
		{
			output << o[i] << std::endl;
		}

		return output;
	}

	int get_size() const
	{
		return size;
	}

	long double &operator[](const size_t x) const
	{
		return (*data).at(x);
	}
	
	~SpecVector();
};

