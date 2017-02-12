// OpenMp_lab1.cpp: определяет точку входа для консольного приложения.
//
#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"

using namespace std;
class OpenMPClass {
	const string OUTPUT_FILE = "output.txt";
	const size_t CHUNK_COUNT = 10;
	struct inputMatrix {
		inputMatrix(string name){
			filename = name;
		}
	public:
		string filename;
		shared_ptr<matrix> matrix;
		size_t m;
		size_t n;
	};

private:
	shared_ptr<inputMatrix> matrix1;
	shared_ptr<inputMatrix> matrix2;


	void resolveArgs(const int argc, char * argv[]){

		if (argc < 3) {
			writeHelp();
			exit(400);
		}

		matrix1 = make_shared<inputMatrix>(string(argv[1]));
		matrix2 = make_shared<inputMatrix>(string(argv[2]));
		if (string(argv[3]) == "-gen"){
			int m = stoi(argv[4]);
			int n = stoi(argv[5]);
			printMatrix((*matrix1).filename, genMatrix(m, n));
			printMatrix((*matrix2).filename, genMatrix(n, m));
		}

	}
	shared_ptr<matrix> genMatrix(size_t m, size_t n){
		auto result = make_shared<matrix>(m, m_vector(n));
		for (size_t i = 0; i < m; ++i) {
			for (size_t j = 0; j < n; ++j) {
				(*result)[i][j] = rand() % 1000 + 1;
			}
		}
		return result;
	}

	void readMatrix(){
		vector<shared_ptr<inputMatrix>> vectorMatrix;
		vectorMatrix.push_back(matrix1);
		vectorMatrix.push_back(matrix2);

		size_t m, n;
		for (int i = 0; i < vectorMatrix.size(); i++){

			shared_ptr<inputMatrix> mtrx = vectorMatrix[i];
			ifstream inputFile((*mtrx).filename);
			if (!inputFile) {
				throw runtime_error("Input file does not exist or not ready to read: " + (*mtrx).filename);
			}
			inputFile.exceptions(ifstream::badbit | ifstream::failbit);

			inputFile >> m >> n;
			auto result = make_shared<matrix>(m, m_vector(n));

			for (size_t i = 0; i < m; ++i) {
				for (size_t j = 0; j < n; ++j) {
					if (!inputFile.eof()) {
						inputFile >> (*result)[i][j];
					}
					else {
						stringstream ss;
						ss << "Input file " << (*mtrx).filename << " contains less than "
							<< m << "x" << n << " elements!";
						throw range_error(ss.str());
					}
				}
			}
			(*mtrx).m = m;
			(*mtrx).n = n;
			(*mtrx).matrix = result;
		}

	}

	void writeHelp(){
		stringstream ss;
		ss << "format: matrix1_filename matrix2_filename";
		throw invalid_argument(ss.str());
	}
	dimensions checkRange(const shared_ptr<matrix> &a, const shared_ptr<matrix> &b) {
		if (a->empty() || b->empty()) {
			throw invalid_argument("Can't multiply matrix with zero rows!");
		}

		const auto m = a->size();
		const auto m1 = (*a)[0].size();
		const auto n = b->size();
		const auto n1 = (*b)[0].size();

		if (n != m1) {
			stringstream ss;
			ss << "Can't multiply matrices with not arranged rows and columns number: "
				<< "a[" << m << "][" << n1 << "] and  "
				<< "b[" << m1 << "][" << n << "]";
			throw invalid_argument(ss.str());
		}

		return dimensions(m, n);
	}

	dimensions generateChunkSizes(const shared_ptr<matrix> &a, const shared_ptr<matrix> &b) {
		if (a->empty() || b->empty()) {
			throw invalid_argument("Can't multiply matrix with zero rows!");
		}

		checkRange(a, b);
		const auto m = a->size();
		const auto n = b->size();
		auto c = m < n ? m : n;
		return dimensions(c, (c / CHUNK_COUNT) + 1);
	}

	size_t timeCalcFunction(
		void(*multiplier)(const shared_ptr<matrix>,
		const shared_ptr<matrix>,
		const shared_ptr<matrix>,
		const size_t,
		const size_t,
		const size_t,
		const size_t),
		const shared_ptr<matrix> a,
		const shared_ptr<matrix> b,
		const size_t chunk_size,
		stringstream *sResult) {
		const dimensions dimensions = checkRange(a, b);
		const auto m = get<0>(dimensions);
		const auto n = get<1>(dimensions);
		const auto c = m < n ? m : n;
		const auto result = make_shared<matrix>(m, m_vector(n));

		auto before = m_clock::now();
		multiplier(a, b, result, m, n, c, chunk_size);
		auto after = m_clock::now();

		auto time = chrono::duration_cast<chrono::nanoseconds>(after - before).count();
		auto nanoSecondsTotal = time;
		const auto seconds = time / 1000000000;
		time %= 1000000000;
		const auto milliseconds = time / 1000000;
		time %= 1000000;
		const auto microseconds = time / 1000;
		time %= 1000;
		const auto nanoseconds = time;

		/*(*sResult) << "time taken for matrices "
			<< "a[" << m << "][" << c << "] "
			<< "b[" << c << "][" << n << "] : "
			<< seconds << "s "
			<< milliseconds << "ms "
			<< microseconds << "mcs "
			<< nanoseconds << "ns" << endl;*/

		return nanoSecondsTotal;
	}
	void static multiplierNoMP(const shared_ptr<matrix> a,
		const shared_ptr<matrix> b,
		const shared_ptr<matrix> result,
		const size_t m,
		const size_t n,
		const size_t c,
		const size_t chunk_size) {
			{
				for (size_t r = 0; r < m * n * c; ++r) {
					const auto i = r / n / c;
					const auto j = r / c % n;
					const auto k = r % c;

					(*result)[i][j] += (*a)[i][k] * (*b)[k][j];
				}
			}
	}
	void static multiplierStaticMP(
		const shared_ptr<matrix> a,
		const shared_ptr<matrix> b,
		const shared_ptr<matrix> result,
		const size_t m,
		const size_t n,
		const size_t c,
		const size_t chunk_size) {
#pragma omp parallel
		{
#pragma omp for ordered schedule(static,chunk_size)
			for (auto r = 0; r < m * n * c; ++r) {
				const auto i = r / n / c;
				const auto j = r / c % n;
				const auto k = r % c;
				(*result)[i][j] += (*a)[i][k] * (*b)[k][j];
			}
		}
	}

	void static multiplierDynamicMP(
		const shared_ptr<matrix> a,
		const shared_ptr<matrix> b,
		const shared_ptr<matrix> result,
		const size_t m,
		const size_t n,
		const size_t c,
		const size_t chunk_size) {
#pragma omp parallel
		{
#pragma omp for ordered schedule(dynamic,chunk_size)
			for (auto r = 0; r < m * n * c; ++r) {
				const auto i = r / n / c;
				const auto j = r / c % n;
				const auto k = r % c;
				(*result)[i][j] += (*a)[i][k] * (*b)[k][j];
			}
		}
	}

	void static multiplierGuidedMP(
		const shared_ptr<matrix> a,
		const shared_ptr<matrix> b,
		const shared_ptr<matrix> result,
		const size_t m,
		const size_t n,
		const size_t c,
		const size_t chunk_size) {
#pragma omp parallel
		{
#pragma omp for ordered schedule(guided,chunk_size)
			for (auto r = 0; r < m * n * c; ++r) {
				const auto i = r / n / c;
				const auto j = r / c % n;
				const auto k = r % c;
				(*result)[i][j] += (*a)[i][k] * (*b)[k][j];
			}
		}
	}
	void printMatrix(const string &file_path, const shared_ptr<matrix> &result) {
		ofstream out_file(file_path, ofstream::trunc);
		out_file.exceptions(ifstream::badbit | ifstream::failbit);
		const auto m = result->size();
		const auto n = (m == 0) ? 0 : (*result)[0].size();
		out_file << m << " " << n << endl;
		for (size_t i = 0; i < m; ++i) {
			for (size_t j = 0; j < n; ++j) {
				out_file << (*result)[i][j] << " ";
			}
			out_file << endl;
		}
	}
	void printStr(const string &file_path, string &str) {
		ofstream out_file(file_path, ofstream::trunc);
		out_file.exceptions(ifstream::badbit | ifstream::failbit);
		out_file << str << endl;
	}


public:
	OpenMPClass(const int argc, char * argv[]){
		resolveArgs(argc, argv);
	}
	vector<csvResult> process(){

		readMatrix();
		size_t result;
		stringstream sResult;

		const dimensions chunkSize = generateChunkSizes((*matrix1).matrix, (*matrix2).matrix);
		const auto size = get<0>(chunkSize);
		const auto step = get<1>(chunkSize);
		vector<csvResult> resultCsv;
		for (size_t chunks = 1; chunks < size; chunks += step){

			result = timeCalcFunction(OpenMPClass::multiplierNoMP, (*matrix1).matrix, (*matrix2).matrix, chunks, &sResult);
			dimensions noOpenMP = dimensions(chunks, result);
			result = timeCalcFunction(OpenMPClass::multiplierStaticMP, (*matrix1).matrix, (*matrix2).matrix, chunks, &sResult);
			dimensions staticMP = dimensions(chunks, result);
			result = timeCalcFunction(OpenMPClass::multiplierDynamicMP, (*matrix1).matrix, (*matrix2).matrix, chunks, &sResult);
			dimensions dynamicMP = dimensions(chunks, result);
			result = timeCalcFunction(OpenMPClass::multiplierGuidedMP, (*matrix1).matrix, (*matrix2).matrix, chunks, &sResult);
			dimensions guidedMP = dimensions(chunks, result);
			resultCsv.push_back(csvResult(noOpenMP, staticMP, dynamicMP, guidedMP));
		}
		return resultCsv;

	}

};
int main(int argc, char * argv[])
{
	vector<inputParam> strvector;
	string path = "C:\\Users\\aleksandr\\Documents\\Visual Studio 2013\\Projects\\Multithreading\\OpenMp\\";
	strvector.push_back(inputParam("C:\\Users\\aleksandr\\Documents\\Visual Studio 2013\\Projects\\Multithreading\\OpenMp\\matrix1.txt", "C:\\Users\\aleksandr\\Documents\\Visual Studio 2013\\Projects\\Multithreading\\OpenMp\\matrix2.txt", "-gen", "100", "100"));
	strvector.push_back(inputParam("C:\\Users\\aleksandr\\Documents\\Visual Studio 2013\\Projects\\Multithreading\\OpenMp\\matrix1.txt", "C:\\Users\\aleksandr\\Documents\\Visual Studio 2013\\Projects\\Multithreading\\OpenMp\\matrix2.txt", "-gen", "2", "200"));
	strvector.push_back(inputParam("C:\\Users\\aleksandr\\Documents\\Visual Studio 2013\\Projects\\Multithreading\\OpenMp\\matrix1.txt", "C:\\Users\\aleksandr\\Documents\\Visual Studio 2013\\Projects\\Multithreading\\OpenMp\\matrix2.txt", "-gen", "200", "51"));

	csvfile csv("C:\\Users\\aleksandr\\Documents\\Visual Studio 2013\\Projects\\Multithreading\\OpenMp\\labResult.csv");

	for each (auto cArgv in strvector)
	{
		char* argv2[] = { "", get<0>(cArgv), get<1>(cArgv), get<2>(cArgv), get<3>(cArgv), get<4>(cArgv) };
		OpenMPClass* op = new OpenMPClass(5, argv2);
		vector<csvResult> csvResult = (*op).process();

		// Hearer
		csv << "chunks" << "noMP" << "static" << "dynamic" << "guided" << "Size:" << get<3>(cArgv) << get<4>(cArgv) << endrow;
		// Data
		for each (auto csvR in csvResult)
		{
			size_t chunkS = get<0>(get<0>(csvR));
			csv << chunkS << get<1>(get<0>(csvR))
				<< get<1>(get<1>(csvR))
				<< get<1>(get<2>(csvR))
				<< get<1>(get<3>(csvR))
				<< endrow;
			
		}
		std::cout << "Size:" << get<3>(cArgv) << "x" << get<4>(cArgv) << '\n' ;
		delete(op);
		csv << endrow;
	}



	system("pause");
}