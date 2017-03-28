// MPI_QuickSort.cpp: определяет точку входа для консольного приложения.
//
#pragma comment (lib, "msmpi.lib")
#include "stdafx.h"
#include "QSort.h"
#include <mpi.h>
int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);
	try {

		auto resolver = make_shared<ArgvResolver>();
		resolver->resolveArgs(argc, argv);
		auto tester = make_shared<Tester>(resolver, make_shared<QSort>());
		tester->runTest();
	}
	catch (std::exception const &e) {
		std::cerr << "Error occurred: " << e.what() << std::endl;
	}
	MPI_Finalize();

	return 0;
}

