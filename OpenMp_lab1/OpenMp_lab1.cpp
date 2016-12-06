// OpenMp_lab1.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"

using namespace std;
class OpenMPClass {
	const string OUTPUT_FILE = "output.txt";

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

		if (argc < 3 || argc > 3) {
			writeHelp();
			exit(400);
		}

		matrix1 = make_shared<inputMatrix>(string(argv[1]));
		matrix2 = make_shared<inputMatrix>(string(argv[2]));
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
		const auto n1 = (*a)[0].size();
		const auto m1 = b->size();
		const auto n = (*b)[0].size();

		if (n1 != m1) {
			stringstream ss;
			ss << "Can't multiply matrices with not arranged rows and columns number: "
				<< "a[" << m << "][" << n1 << "] and  "
				<< "b[" << m1 << "][" << n << "]";
			throw invalid_argument(ss.str());
		}

		return dimensions(m, n, n1);
	}

	shared_ptr<matrix> timeCalcFunction(
		void(*multiplier)(const shared_ptr<matrix>,
		const shared_ptr<matrix>,
		const shared_ptr<matrix>,
		const size_t,
		const size_t,
		const size_t),
		const shared_ptr<matrix> a,
		const shared_ptr<matrix> b) {
		const dimensions dimensions = checkRange(a, b);
		const auto m = get<0>(dimensions);
		const auto n = get<1>(dimensions);
		const auto c = get<2>(dimensions);
		const auto result = make_shared<matrix>(m, m_vector(n));

		auto before = m_clock::now();
		multiplier(a, b, result, m, n, c);
		auto after = m_clock::now();

		auto time = chrono::duration_cast<chrono::nanoseconds>(after - before).count();
		const auto seconds = time / 1000000000;
		time %= 1000000000;
		const auto milliseconds = time / 1000000;
		time %= 1000000;
		const auto microseconds = time / 1000;
		time %= 1000;
		const auto nanoseconds = time;

		cout << "time taken for matrices "
			<< "a[" << m << "][" << c << "] "
			<< "b[" << c << "][" << n << "] : "
			<< seconds << "s "
			<< milliseconds << "ms "
			<< microseconds << "mcs "
			<< nanoseconds << "ns" << endl;

		return result;
	}
	void static multiplierNoMP(const shared_ptr<matrix> a,
		const shared_ptr<matrix> b,
		const shared_ptr<matrix> result,
		const size_t m,
		const size_t n,
		const size_t c) {			
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
		const std::shared_ptr<matrix> a,
		const std::shared_ptr<matrix> b,
		const std::shared_ptr<matrix> result,
		const size_t m,
		const size_t n,
		const size_t c) {
		#pragma omp parallel
		{
			#pragma omp for ordered schedule(static)
			for (auto r = 0; r < m * n * c; ++r) {
				const auto i = r / n / c;
				const auto j = r / c % n;
				const auto k = r % c;
				(*result)[i][j] += (*a)[i][k] * (*b)[k][j];
			}
		}
	}

	void static multiplierDynamicMP(
		const std::shared_ptr<matrix> a,
		const std::shared_ptr<matrix> b,
		const std::shared_ptr<matrix> result,
		const size_t m,
		const size_t n,
		const size_t c) {
		#pragma omp parallel
		{
			#pragma omp for ordered schedule(dynamic)
				for (auto r = 0; r < m * n * c; ++r) {
				const auto i = r / n / c;
				const auto j = r / c % n;
				const auto k = r % c;
				(*result)[i][j] += (*a)[i][k] * (*b)[k][j];
			}
		}
	}

	void static multiplierGuidedMP(
		const std::shared_ptr<matrix> a,
		const std::shared_ptr<matrix> b,
		const std::shared_ptr<matrix> result,
		const size_t m,
		const size_t n,
		const size_t c) {
		#pragma omp parallel
		{
			#pragma omp for ordered schedule(guided)
			for (auto r = 0; r < m * n * c; ++r) {
				const auto i = r / n / c;
				const auto j = r / c % n;
				const auto k = r % c;
				(*result)[i][j] += (*a)[i][k] * (*b)[k][j];
			}
		}
	}
	void printMatrix(const std::string &file_path, const std::shared_ptr<matrix> &result) {
		std::ofstream out_file(file_path, std::ofstream::trunc);
		out_file.exceptions(std::ifstream::badbit | std::ifstream::failbit);
		const auto m = result->size();
		const auto n = (m == 0) ? 0 : (*result)[0].size();
		for (size_t i = 0; i < m; ++i) {
			for (size_t j = 0; j < n; ++j) {
				out_file << (*result)[i][j] << " ";
			}
			out_file << std::endl;
		}
	}

public:
	OpenMPClass(const int argc, char * argv[]){
		resolveArgs(argc, argv);
	}
	void process(){
		readMatrix();
		std::shared_ptr<matrix> result;
		std::cout << "No openmp" << std::endl;
		result = timeCalcFunction(OpenMPClass::multiplierNoMP, (*matrix1).matrix, (*matrix2).matrix);
		printMatrix("nomp_" + OUTPUT_FILE, result);
		std::cout << "Static openmp" << std::endl;
		result = timeCalcFunction(OpenMPClass::multiplierStaticMP, (*matrix1).matrix, (*matrix2).matrix);
		printMatrix("static_" + OUTPUT_FILE, result);
		std::cout << "Dynamic openmp" << std::endl;
		result = timeCalcFunction(OpenMPClass::multiplierDynamicMP, (*matrix1).matrix, (*matrix2).matrix);
		printMatrix("dyn_" + OUTPUT_FILE, result);
		std::cout << "Guided openmp" << std::endl;
		result = timeCalcFunction(OpenMPClass::multiplierGuidedMP, (*matrix1).matrix, (*matrix2).matrix);
		printMatrix("guided_" + OUTPUT_FILE, result);
	}

};
int main(int argc, char * argv[])
{
	OpenMPClass* op = new OpenMPClass(argc, argv);
	(*op).process();
	system("pause");
}