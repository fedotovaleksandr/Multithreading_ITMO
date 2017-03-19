#pragma once
#include "stdafx.h"
#include "SpecVector.h"
class SpecMatrix
{
public:
	typedef std::vector<std::shared_ptr<SpecVector>> data_t;
	const std::shared_ptr<data_t> data;
	const int rows, columns;
public:
	typedef std::shared_ptr<SpecMatrix> matrix_t;
	explicit SpecMatrix(const int rows, const int columns);
	static matrix_t generate_matrix(const int rows, const int columns);
	std::pair<matrix_t, SpecVector::vector_t> split() const;
	std::shared_ptr<std::vector<long double>> get_plain_data() const;
	void fill(std::shared_ptr<std::vector<long double>> &plain_data, const int size) const;

	int get_rows() const
	{
		return rows;
	}

	int get_columns() const
	{
		return columns;
	}

	friend std::istream &operator >>(std::istream &input, SpecMatrix &o)
	{
		for (int i = 0; i < o.rows; ++i)
		{
			for (int j = 0; j < o.columns; ++j)
			{
				if (!input.eof())
				{
					input >> o[i][j];
				}
				else
				{
					std::stringstream ss;
					ss << "Input stream contains less than "
						<< o.rows << "x" << o.columns << " elements!";
					throw std::range_error(ss.str());
				}
			}
		}

		return input;
	}

	friend std::ostream &operator <<(std::ostream &output, const SpecMatrix &o)
	{
		for (int i = 0; i < o.rows; ++i)
		{
			for (int j = 0; j < o.columns; ++j)
			{
				output << o[i][j] << " ";
			}
		}

		return output;
	}

	SpecVector &operator[](const int x) const
	{
		return *(*data).at(x);
	}
	~SpecMatrix();
};

