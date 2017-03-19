#include "stdafx.h"
#include "SpecMatrix.h"
#include "SpecVector.h"

SpecMatrix::SpecMatrix(const int rows, const int columns) : data(std::make_shared<data_t>(rows, nullptr)),
rows(rows),
columns(columns)
{
	for (auto i = 0; i < rows; ++i)
	{
		(*this->data)[i] = std::make_shared<SpecVector>(columns);
	}
}

std::pair<SpecMatrix::matrix_t, SpecVector::vector_t> SpecMatrix::split() const
{
	if (this->get_columns() < 2)
	{
		throw std::range_error("Can't extract the right-hand values from matrix with less than 2 columns!");
	}

	auto a = std::make_shared<SpecMatrix>(this->get_rows(), this->get_rows());
	auto b = std::make_shared<SpecVector>(this->get_rows());

	for (int i = 0; i < this->get_rows(); ++i)
	{
		for (int j = 0; j < this->get_columns() - 1; ++j)
		{
			(*a)[i][j] = (*(*this->data)[i])[j];
		}
		(*b)[i] = (*(*this->data)[i])[this->get_columns() - 1];
	}

	return make_pair(a, b);
}


std::shared_ptr<std::vector<long double>> SpecMatrix::get_plain_data() const
{
	auto result = std::make_shared<std::vector<long double>>(this->get_rows() * this->get_columns());

	for (auto i = 0; i < (*this->data).size(); ++i)
	{
		auto row = (*this->data)[i];
		for (auto j = 0; j < row->get_size(); ++j)
		{
			(*result)[i * row->get_size() + j] = (*(*this->data)[i])[j];
		}
	}

	return result;
}


void SpecMatrix::fill(std::shared_ptr<std::vector<long double>> &plain_data, const int size) const
{
	for (auto i = 0; i < size; ++i)
	{
		(*(*this->data)[i / this->columns])[i % columns] = (*plain_data)[i];
	}
}

SpecMatrix::matrix_t SpecMatrix::generate_matrix(const int m, const int n)
{
	auto result = std::make_shared<SpecMatrix>(m, n);

	for (auto i = 0; i < m; ++i)
	{
		for (auto j = 0; j < n; ++j)
		{
			if (i == j)
			{
				(*result)[i][j] = static_cast<long double>(1);
			}
			else if (i < j)
			{
				(*result)[i][j] = static_cast<long double>(-2);
			}
		}
	}

	return result;
}

SpecMatrix::~SpecMatrix()
{
}
