#include "stdafx.h"
#include "SpecVector.h"


SpecVector::SpecVector(const int size) : data(std::make_shared<data_t>(size, 0.0)), size(size)
{
}

SpecVector::vector_t SpecVector::generateVector(const int size)
{
	auto result = std::make_shared<SpecVector>(size);

	for (auto i = 0; i < size; ++i)
	{
		(*result)[i] = static_cast<long double>(i % 2 ? -1.0 / 3.0 : 1);
	}

	return result;
}

std::shared_ptr<std::vector<long double>> SpecVector::getData() const
{
	return this->data;
}

void SpecVector::fill(std::shared_ptr<std::vector<long double>> &plain_data, const int size) const
{
	for (auto i = 0; i < size; ++i)
	{
		(*this->data).at(i) = (*plain_data)[i];
	}
}

SpecVector::~SpecVector()
{
}
