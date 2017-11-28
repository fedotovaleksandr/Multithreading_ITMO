#pragma comment (lib, "msmpi.lib")
#include "stdafx.h"
#include "Jacoby.h"
#include "mpi.h"
Jacoby::Jacoby(shared_ptr<ArgvResolver> argvResolver)
{
	this->processArgvResolver(argvResolver);
}

void Jacoby::processArgvResolver(shared_ptr<ArgvResolver> argvResolver){
	argvResolver->addArgument("-gen", "1");
	argvResolver->addArgument("-v", "0");
	this->verbose = argvResolver->getArgument("-v") == "1";
	this->useGen = argvResolver->getArgument("-gen") == "1";
	this->processes = atoi(argvResolver->getArgument("-proc").c_str());
	this->matrixRows = atoi(argvResolver->getArgument("-rows").c_str());
	this->matrixColumns = this->matrixRows + 1;
	this->maxIterations = atoi(argvResolver->getArgument("-iter").c_str());
	this->precision = stod(argvResolver->getArgument("-pre").c_str());
	if (this->processes > this->matrixRows) {
		this->processes = this->matrixRows;
	}
	if (!this->useGen){
		this->outputFile = argvResolver->getArgument("-out");
		this->inputFileMatrix = argvResolver->getArgument("-in").c_str();
	}
	argvResolver->addArgument("-out", this->getTitle() + "out.txt");
	this->outputFile = argvResolver->getArgument("-out");

};

string Jacoby::getTitle(){
	std::stringstream ss;
	ss << "matrix_" << to_string(this->matrixRows) << "x" << to_string(this->matrixRows) << "__proccessor_" << this->processes;
	return ss.str();
};
void Jacoby::log(string vec)
{
	if (this->verbose)
	{
		std::cout << vec << std::endl;
	}
}
int Jacoby::run(){
	MPI_Comm_size(MPI_COMM_WORLD, &this->processes);
	MPI_Comm_rank(MPI_COMM_WORLD, &this->processId);
	if (processId >= this->processes) {
		log("process  hide id:" + to_string(processId));
		return EXIT_SUCCESS;
	}

	if (this->processId == this->ROOT_ID)
	{
		if (this->useGen)
		{
			log("Generating matrix and approximation");
			this->coeffMatrix = SpecMatrix::generate_matrix(this->matrixRows, this->matrixRows);
			this->rightHandSide = SpecVector::generateVector(this->matrixRows);
			this->approximation = SpecVector::generateVector(this->matrixRows);
		}
		else
		{
			log("Reading matrix and approximation");
			this->readMatrix();
			this->approximation = std::make_shared<SpecVector>(this->matrixRows);
		}
		this->calcDataForEachProcess();
		this->sendMetaData();
		this->sendInitialData();
	}
	else
	{
		this->receiveMetaData();
		this->calcDataForEachProcess();
		this->receiveInitialData();
	}
	return this->process();
}
void Jacoby::receiveMetaData()
{
	log("Receiving metadata procId:" + this->processId);

	long size = 0;
	auto precision = 0.0;
	auto max_iterations = static_cast<long>(this->maxIterations);
	//Broadcasts a message from the process with rank "root" to all other processes of the communicator
	MPI_Bcast(&size, 1, MPI_LONG, this->ROOT_ID, MPI_COMM_WORLD);
	MPI_Bcast(&precision, 1, MPI_DOUBLE, this->ROOT_ID, MPI_COMM_WORLD);
	MPI_Bcast(&max_iterations, 1, MPI_LONG, this->ROOT_ID, MPI_COMM_WORLD);

	this->matrixRows = static_cast<size_t>(size);
	this->matrixColumns = this->matrixRows + 1;
	this->precision = precision;
	this->maxIterations = max_iterations;
}
void Jacoby::receiveInitialData()
{
	log("Receiving initial data" + this->processId);

	auto rows = (*this->rowsNumberDistribution)[this->processId];
	auto cells = (*this->cellsNumberDistribution)[this->processId];

	this->coeffMatrix = std::make_shared<SpecMatrix>(rows, this->getSize());
	this->rightHandSide = std::make_shared<SpecVector>(rows);
	this->approximation = std::make_shared<SpecVector>(this->matrixRows);

	auto plain_matrix_data = std::make_shared<std::vector<long double>>(cells);
	auto plain_right_hand_data = std::make_shared<std::vector<long double>>(rows);
	//Scatters a buffer in parts to all processes in a communicator
	MPI_Scatterv(
		//buffer
		nullptr,
		//sizes for aech process
		this->cellsNumberDistribution->data(),
		//position in buffer for each process
		this->cellsPositionsDistribution->data(), MPI_DOUBLE,
		plain_matrix_data->data(), cells, MPI_DOUBLE,
		this->ROOT_ID, MPI_COMM_WORLD);
	MPI_Scatterv(nullptr,
		this->rowsNumberDistribution->data(),
		this->rowsPositionsDistribution->data(), MPI_DOUBLE,
		plain_right_hand_data->data(), rows, MPI_DOUBLE,
		this->ROOT_ID, MPI_COMM_WORLD);

	this->coeffMatrix->fill(plain_matrix_data, cells);
	this->rightHandSide->fill(plain_right_hand_data, rows);
}

void Jacoby::sendInitialData(){
	log("Sending initial data" + this->processId);

	auto rows = (*this->rowsNumberDistribution)[this->processId];
	auto cells = (*this->cellsNumberDistribution)[this->processId];

	auto plain_matrix_data = std::make_shared<std::vector<long double>>(cells);
	auto plain_right_hand_data = std::make_shared<std::vector<long double>>(rows);
	//Scatters a buffer in parts to all processes in a communicator
	MPI_Scatterv(
		//buffer
		this->coeffMatrix->get_plain_data()->data(),
		//sizes for aech process
		this->cellsNumberDistribution->data(),
		//position in buffer for each process
		this->cellsPositionsDistribution->data(),
		MPI_DOUBLE,
		plain_matrix_data->data(), static_cast<int>(cells), MPI_DOUBLE,
		this->ROOT_ID, MPI_COMM_WORLD);
	MPI_Scatterv(this->rightHandSide->getData()->data(),
		this->rowsNumberDistribution->data(),
		this->rowsPositionsDistribution->data(),
		MPI_DOUBLE,
		plain_right_hand_data->data(), static_cast<int>(rows), MPI_DOUBLE,
		this->ROOT_ID, MPI_COMM_WORLD);
}
void Jacoby::sendMetaData(){
	log("Sending metadata" + this->processId);
	//
	auto size = static_cast<long>(this->getSize());
	auto precision = this->precision;
	auto max_iterations = static_cast<long>(this->maxIterations);
	//Broadcasts a message from the process with rank "root" to all other processes of the communicator
	MPI_Bcast(&size, 1, MPI_LONG, this->ROOT_ID, MPI_COMM_WORLD);
	MPI_Bcast(&precision, 1, MPI_DOUBLE, this->ROOT_ID, MPI_COMM_WORLD);
	MPI_Bcast(&max_iterations, 1, MPI_LONG, this->ROOT_ID, MPI_COMM_WORLD);
}
int Jacoby::getSize(){
	return this->matrixRows;
}
void Jacoby::calcDataForEachProcess()
{
	auto div = std::div(static_cast<int>(this->getSize()), this->processes);

	this->rowsPerProcess = (div.rem ? div.quot + 1 : div.quot);
	this->cellsPerProcess = this->getSize() * this->rowsPerProcess;
	this->rowsNumberDistribution = std::make_shared<std::vector<int>>(this->processes, 0);
	this->rowsPositionsDistribution = std::make_shared<std::vector<int>>(this->processes, 0);
	this->cellsNumberDistribution = std::make_shared<std::vector<int>>(this->processes, 0);
	this->cellsPositionsDistribution = std::make_shared<std::vector<int>>(this->processes, 0);

	auto row = 0;
	for (auto i = 0; i < this->processes; ++i)
	{
		auto left = this->getSize() - row;
		auto current = static_cast<int>(std::min(left, this->rowsPerProcess));
		(*this->rowsNumberDistribution)[i] = current;
		(*this->rowsPositionsDistribution)[i] = row;
		(*this->cellsNumberDistribution)[i] = current * static_cast<int>(this->getSize());
		(*this->cellsPositionsDistribution)[i] = row * static_cast<int>(this->getSize());
		row += current;
	}

	this->printCalcData();
}

void Jacoby::printCalcData()
{
	if (this->verbose)
	{
		std::stringstream ss;
		ss << "Rows to process: " << (*this->rowsNumberDistribution)[this->processId] << ", "
			<< "rows position: " << (*this->rowsPositionsDistribution)[this->processId] << ", "
			<< "cells to process: " << (*this->cellsNumberDistribution)[this->processId] << ", "
			<< "cells position: " << (*this->cellsPositionsDistribution)[this->processId];
		auto msg = ss.str();
		log(msg);
	}
}

void Jacoby::readMatrix()
{
	std::ifstream input_file(this->inputFileMatrix);
	int m, n;

	if (!input_file)
	{
		throw std::runtime_error("Input file does not exist or not ready to read: " + this->inputFileMatrix);
	}

	input_file.exceptions(std::ifstream::badbit | std::ifstream::failbit);
	input_file >> m >> n;

	auto full_matrix = std::make_shared<SpecMatrix>(m, n);

	input_file >> *full_matrix;

	input_file.close();

	auto pair = full_matrix->split();

	this->coeffMatrix = pair.first;
	this->rightHandSide = pair.second;
	this->matrixRows = (*this->coeffMatrix).get_rows();
	this->matrixColumns = this->matrixRows + 1;
}

std::pair<bool, int> Jacoby::applyJacobi()
{
	auto x_old = std::make_shared<SpecVector>(this->getSize());
	auto x_new = std::make_shared<SpecVector>(this->getSize());

	auto rows = (*this->rowsNumberDistribution)[this->processId];
	//Gathers data from all tasks and deliver the combined data to all tasks
	MPI_Allgatherv(
		//buffer
		this->rightHandSide->getData()->data(),
		//number of elements in send buffer
		rows,
		MPI_DOUBLE,
		//containing the number of elements that are to be received from each process
		x_new->getData()->data(),
		// containing the number of elements that are to be received from each process
		this->rowsNumberDistribution->data(),
		//Entry i specifies the displacement (relative to recvbuf ) at which to place the incoming data from process i
		this->rowsPositionsDistribution->data(),
		MPI_DOUBLE,
		MPI_COMM_WORLD);

	auto iteration = 0;

	do
	{
		iteration++;
		x_old.swap(x_new);

		for (auto i = 0; i < rows; ++i)
		{
			auto g = i + this->processId * rows;
			(*approximation)[i] = (*this->rightHandSide)[i];
			for (auto j = 0; j < g; ++j)
				(*approximation)[i] -= (*coeffMatrix)[i][j] * (*x_old)[j];
			for (auto j = g + 1; j < this->getSize(); ++j)
				(*approximation)[i] -= (*coeffMatrix)[i][j] * (*x_old)[j];
			(*approximation)[i] /= (*coeffMatrix)[i][g];
		}
		//Gathers data from all tasks and deliver the combined data to all tasks
		MPI_Allgatherv(
			//buffer
			approximation->getData()->data(),
			//number of elements in send buffer
			rows,
			MPI_DOUBLE,
			//containing the number of elements that are to be received from each process
			x_new->getData()->data(),
			// containing the number of elements that are to be received from each process
			this->rowsNumberDistribution->data(),
			//Entry i specifies the displacement (relative to recvbuf ) at which to place the incoming data from process i

			this->rowsPositionsDistribution->data(),
			MPI_DOUBLE, MPI_COMM_WORLD);

	} while (iteration < this->maxIterations && this->sqrtDeviation(x_new, x_old) >= this->precision);

	return std::make_pair(this->sqrtDeviation(x_new, x_old) < this->precision, iteration);
}

double Jacoby::sqrtDeviation(SpecVector::vector_t &new_appr, SpecVector::vector_t &old_appr){
	auto size = new_appr->get_size();
	auto sum = 0.0;

	for (auto i = 0; i < size; ++i)
	{
		sum += ((*new_appr)[i] - (*old_appr)[i]) * ((*new_appr)[i] - (*old_appr)[i]);
	}
	return sqrt(sum);
}

int Jacoby::process(){
	log("Processing" + this->processId);

	auto converged = this->applyJacobi();
	auto rows = (*this->rowsNumberDistribution)[this->processId];
	auto plain_approximation_data = std::make_shared<std::vector<long double>>(this->matrixRows);

	std::stringstream ss;
	ss << "Local approximation: " << *approximation;
	log(ss.str());

	auto x_new = std::make_shared<SpecVector>(this->getSize());

	MPI_Gatherv(this->approximation->getData()->data(), rows, MPI_DOUBLE,
		plain_approximation_data->data(), this->rowsNumberDistribution->data(),
		this->rowsPositionsDistribution->data(), MPI_DOUBLE,
		this->ROOT_ID, MPI_COMM_WORLD);

	if (this->processId == ROOT_ID)
	{
		this->approximation->fill(plain_approximation_data, this->getSize());

		std::stringstream ss2;
		ss2 << "Global approximation: " << *approximation << ", "
			<< "Converged: " << (converged.first ? "true" : "false") << ", iterations: " << converged.second;
		log(ss2.str());
		this->printAnswer();
	}
}

void Jacoby::printAnswer(){
	std::ofstream out_file(this->outputFile, std::ofstream::trunc | std::ofstream::out);

	if (!out_file)
	{
		throw std::runtime_error("Output file does not exist or not ready to read: " + this->outputFile);
	}

	out_file.exceptions(std::ofstream::badbit | std::ofstream::failbit);

	out_file << *this->approximation;

	out_file.close();
}

Jacoby::~Jacoby()
{
}
