// MPI.cpp: определяет точку входа для консольного приложения.
//
#pragma comment (lib, "msmpi.lib")
#include "stdafx.h"
#include "Math.h"

class SinusParalel {
	int ROOT_ID = 0;
	int processes;
	int processId;
	double edge;
	int precision;
	int elems_per_process;
	shared_ptr<vector<int>> numbersCalculate;
	shared_ptr<vector<long double>> recovData;
public:
	SinusParalel(double edge, int precision)
	{
		this->edge = 3.14159265358979323846 * edge / 180;
		this->precision = precision;
	};


	double run(){
		MPI_Comm_size(MPI_COMM_WORLD, &this->processes);
		MPI_Comm_rank(MPI_COMM_WORLD, &this->processId);

		MPI_Bcast(&edge, 1, MPI_LONG, this->ROOT_ID, MPI_COMM_WORLD);
		MPI_Bcast(&precision, 1, MPI_DOUBLE, this->ROOT_ID, MPI_COMM_WORLD);
		auto before = std::chrono::high_resolution_clock::now();
		
		auto div = std::div(static_cast<int>(this->precision), this->processes);
		this->elems_per_process = (div.rem ? div.quot + 1 : div.quot);
		if (this->ROOT_ID == this->processId) {
			std::cout << "Precision:" << precision << std::endl;
			std::cout << "Edge:" << edge << std::endl;
			std::cout << "Processes:" << processes << std::endl;
			std::cout << "elems_per_process:" << elems_per_process << std::endl;
		}

		
		/*this->numbersCalculate = std::make_shared<std::vector<int>>(this->processes, 0);
		auto currentNumber = 0;
		for (auto i = 0; i < this->processes; ++i, currentNumber += this->elems_per_process)
		{
			(*this->numbersCalculate)[i] = currentNumber;

		}*/

		this->recovData = std::make_shared<std::vector<long double>>(this->processes, 0);
		auto startPositionCurrent = this->elems_per_process* this->processId;
		std::cout << "Start position:" << startPositionCurrent << std::endl;
		for (int n = startPositionCurrent; n < (startPositionCurrent + elems_per_process) && n < precision; n++) {

			(*recovData)[this->processId] += (powl(-1, n) * powl(this->edge, 2 * n + 1)) / factorial((2 * n + 1));
		}

		auto supData = std::make_shared<std::vector<long double>>(this->processes, 0);
		MPI_Reduce(recovData->data(),
			supData->data(),
			this->processes,
			MPI_LONG_DOUBLE,
			MPI_SUM,
			this->ROOT_ID,
			MPI_COMM_WORLD);

		if (this->ROOT_ID == this->processId) {
			long double result = 0;
			for (auto i = 0; i < this->processes; ++i)
			{
				result += (*supData)[i];

			}
			std::cout << "Sinus: " << result << std::endl;
			auto after = std::chrono::high_resolution_clock::now();
			auto time = std::chrono::duration_cast<std::chrono::milliseconds>(after - before).count();

			std::cout << "Time taken:" << ": " << time << "ms" << std::endl;
		}
		return 1;


	}

	unsigned long long factorial(unsigned long long f)
	{
		if (f == 0)
			return 1;
		return(f * factorial(f - 1));
	}
};

int main(int argc, char * argv[]) {
	
	
	
	MPI_Init(&argc, &argv);
	try {

		auto resolver = make_shared<ArgvResolver>();
		resolver->resolveArgs(argc, argv);
		resolver->addArgument("-edge", "1");
		resolver->addArgument("-pre", "0");
		SinusParalel* sinus = new SinusParalel(stoi(resolver->getArgument("-edge")), stoi(resolver->getArgument("-pre")));
		sinus->run();
	}
	catch (std::exception const &e) {
		std::cerr << "Error occurred: " << e.what() << std::endl;
	}
	MPI_Finalize();

	
	return 0;
}
