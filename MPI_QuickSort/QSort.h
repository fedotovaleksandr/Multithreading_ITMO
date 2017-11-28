#pragma once
#include "stdafx.h"
template <typename T>
class random
{
	std::default_random_engine gen;
	std::uniform_real_distribution<double> dist;
public:
	random(double min, double max) : gen(std::chrono::system_clock::now().time_since_epoch().count()),
		dist(min, max)
	{
	}

	T next()
	{
		return static_cast<T>(dist(gen));
	}
};
class QSort : public IMpiTest
{
	typedef int64_t type_t;
private:	
	static const int ROOT_ID = 0;
	std::string input_file = "";
	std::string output_file = "";
	std::vector<type_t> data;
	std::vector<type_t> sorted_data;
	random<type_t> rnd { std::numeric_limits<type_t>::min(), std::numeric_limits<type_t>::max() };
	bool use_gen_input = false;
	bool verbose = false;
	size_t size = 0;
	int process_id = 0;
	int total_processes = 0;
	int last_start = 0;
	size_t elems_per_process = 0;
	std::vector<int> elems_distribution;
	std::vector<int> positions_distribution;
	std::vector<int> class_starts;
	std::vector<int> class_lengths;
	std::vector<type_t> pivot_buffer;
	size_t pivot_buffer_size;
	double start_time, end_time;

	static std::string get_help();
	static void fillVectorRandomly(std::vector<type_t> &v, const size_t size, random<type_t> &rnd);
	static void checkArgumentsAavailable(const int total, const int current, const int required);
	static size_t parseSize_t(const char *value, const char *parse_error, const char *overflow_error);

	template <typename H, typename... T>
	void log(std::stringstream &ss, H &p, T ... t) const;
	template <typename H, typename... T>
	void log(H &p, T ... t) const;

	bool isRoot() const;
	void scatterData();
	void localSort();
	void collectRegularSamples();
	void gatherSamples();
	void mergeSamples();
	void broadcastSamples();
	void partitionLocalData();
	void readVector();
	void checkArguments() const;
	void printAnswer() const;
	void printProcessId() const;
	void log(std::stringstream &ss) const;
	void calculateDataDistribution();
	void gatherAndMultimergeData();
	void collectData();
	void multimerge(std::vector<std::vector<type_t>> &starts, std::vector<type_t> &result) const;
public:
	QSort();
	void init();
	int run();
	int process();
	void processArgvResolver(shared_ptr<ArgvResolver> argvResolver);
	string getTitle();
	~QSort();
};
template <typename T>
struct mm_data
{
	int start_index;
	int index;
	T start_value;

	explicit mm_data(int st = 0, int id = 0, T stv = 0) : start_index(st), index(id), start_value(stv)
	{
	}

	bool operator<(const mm_data &o) const
	{
		return this->start_value > o.start_value;
	}
};
