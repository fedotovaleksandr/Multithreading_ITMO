#pragma once
#include "SpecMatrix.h"
#include "SpecVector.h"

class Jacoby : public IMpiTest
{
	int ROOT_ID = 0;
	int processes;
	int processId;
	string outputFile;
	int	matrixRows;
	int	matrixColumns;
	int	maxIterations;
	double	precision;
	string	inputFileMatrix;
	bool	verbose;
	bool	useGen;
	SpecMatrix::matrix_t coeffMatrix;
	SpecVector::vector_t rightHandSide;
	SpecVector::vector_t approximation;
	SpecVector::vector_t answer;
	//each process data
	int rowsPerProcess;
	int cellsPerProcess;
	shared_ptr<vector<int>> rowsNumberDistribution;
	shared_ptr<vector<int>> rowsPositionsDistribution;
	//calc how cells for each process to send
	shared_ptr<vector<int>> cellsNumberDistribution;
	shared_ptr<vector<int>> cellsPositionsDistribution;

public:
	Jacoby(shared_ptr<ArgvResolver> argvResolver);
	Jacoby(){};
	int run();
	int process();
	void log(string vec);
	void processArgvResolver(shared_ptr<ArgvResolver> argvResolver);
	string getTitle();
	void readMatrix();
	void calcDataForEachProcess();
	void printCalcData();
	void sendMetaData();
	void sendInitialData();
	void receiveMetaData();
	void receiveInitialData();
	int getSize();
	void printAnswer();
	std::pair<bool, int> applyJacobi();
	double sqrtDeviation(SpecVector::vector_t &new_appr, SpecVector::vector_t &old_appr);
	~Jacoby();
};

