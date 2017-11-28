#include "stdafx.h"
#include <omp.h>
#include <chrono>
#include <numeric>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include "omp_tester.h"


const std::string omp_tester::DEFAULT_OUTPUT_FILE_NAME = "output.txt";
const std::string omp_tester::USE_GENERATED_GRAPH_ARG = "-gen";
const std::string omp_tester::OUTPUT_FILE_ARG = "-out";
const std::string omp_tester::VERBOSE_ARG = "-v";
const std::string omp_tester::THREADS_NUMBER_ARG = "-threads";
const std::string omp_tester::START_NODE_ARG = "-start_node";
const std::string omp_tester::INPUT_ARG = "-in";
const omp_tester::type_t omp_tester::NO_PATH_VALUE = -1;

void omp_tester::log(std::stringstream &ss) const
{
	if (verbose)
	{
		std::cout << ss.str() << std::endl;
	}
}

template <typename H, typename... T>
void omp_tester::log(std::stringstream &ss, H &p, T ... t) const
{
	if (verbose)
	{
		ss << p;
		log(ss, t...);
	}
}

template <typename H, typename... T>
void omp_tester::log(H &p, T ... t) const
{
	if (verbose)
	{
		std::stringstream ss;
		ss << p;
		log(ss, t...);
	}
}


std::unique_ptr<std::vector<omp_tester::type_t>> omp_tester::dijkstra_run()
{
	std::vector<bool> visited(nodes, false);
	auto dist = std::make_unique<std::vector<type_t>>(nodes, std::numeric_limits<type_t>::max());
	auto node = start_node;
	(*dist)[start_node] = 0;
	auto before = m_clock::now();
	for (auto i = 0; i < nodes; ++i)
	{
		visited[node] = true;

		#pragma omp parallel for schedule(static)
		for (auto to = 0; to < nodes; ++to)
		{
			if ((*data)[node][to] != NO_PATH_VALUE)
			{
				if ((*dist)[node] + (*data)[node][to] < (*dist)[to])
				{
					(*dist)[to] = (*dist)[node] + (*data)[node][to];
				}
			}
		}

		node = get_next_node(visited, *dist);

		if (node == -1){
			break;
		}
	}

	auto after = m_clock::now();
	auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(after - before).count();

	log( "Time taken: " + format_time(time));

	return dist;
}

std::string omp_tester::format_time(int64_t nanos)
{
	const auto seconds = nanos / 1000000000;
	nanos %= 1000000000;
	const auto milliseconds = nanos / 1000000;
	nanos %= 1000000;
	const auto microseconds = nanos / 1000;
	nanos %= 1000;
	const auto nanoseconds = nanos;

	std::stringstream ss;
	ss << std::setw(3) << seconds << "s " <<
		std::setw(3) << milliseconds << "ms " <<
		std::setw(3) << microseconds << "mcs " <<
		std::setw(3) << nanoseconds << "ns";

	return ss.str();
}


size_t omp_tester::get_next_node(std::vector<bool> &visited, std::vector<type_t> &dist) const
{
	type_t min;

	auto nextNode = -1;
	min = std::numeric_limits<type_t>::max();
	#pragma omp parallel 
	{
		type_t nextNodePrivate, minPrivate;
		#pragma omp for schedule(static)
		for (auto i = 0; i < nodes; ++i)
		{
			if (!visited[i] && dist[i] < minPrivate)
			{
				minPrivate = dist[i];
				nextNodePrivate = i;
			}
		}

		#pragma omp critical 
		{
			if (minPrivate < min) {
				min = minPrivate;
				nextNode = nextNodePrivate;
			}
		};
	}
	

	return nextNode;
}

void omp_tester::generate_matrix(const size_t &n)
{
	log("Generating matrix ", n, "x", n);
	data = std::make_unique<matrix<type_t>>(n, n);
	data->generate_matrix(-1, 100);
}

void omp_tester::read_matrix(const std::string &file_path)
{
	log("Reading matrix from file ", file_path);

	std::ifstream input_file(file_path);
	size_t n;

	if (!input_file)
	{
		throw std::runtime_error("Input file does not exist or not ready to read: " + file_path);
	}

	input_file.exceptions(std::ifstream::badbit | std::ifstream::failbit);
	input_file >> n;
	data = std::make_unique<matrix<type_t>>(n, n);
	input_file >> *data;
	nodes = n;
}

template <typename T>
std::ostream &operator <<(std::ostream &output, const std::vector<T> &o)
{
	auto indx_width = std::to_string(o.size() - 1).length();
	auto elem_width = std::to_string(*std::max_element(o.cbegin(), o.cend())).length();

	for (auto i = 0; i < o.size(); ++i)
	{
		output << std::setw(indx_width) << i << " " << std::setw(elem_width) << o[i] << std::endl;
	}

	return output;
}

void omp_tester::print_answer(const std::string &file_path, const std::vector<type_t> &result)
{
	std::ofstream out_file(file_path, std::ofstream::trunc);

	if (!out_file)
	{
		throw std::runtime_error("Output file is not ready to write: " + file_path);
	}

	out_file.exceptions(std::ifstream::badbit | std::ifstream::failbit);
	out_file << result;
}

void omp_tester::check_arguments_available(const int total, const int current, const int required)
{
	if (current + required >= total)
	{
		throw std::invalid_argument("Not enough arguments to resolve argument! " + get_help());
	}
}

std::string omp_tester::getTitle(){
	std::stringstream ss;
	ss << "dekstra_" << to_string(this->nodes)  << "_proccessor_" << this->threads;
	return ss.str();
}

std::string omp_tester::get_help()
{
	std::stringstream ss;
	ss << "Usage: Lab04 "
		<< "[" << OUTPUT_FILE_ARG << " output_path] "
		<< "[" << USE_GENERATED_GRAPH_ARG << " size] "
		<< "[" << THREADS_NUMBER_ARG << " threads_number] "
		<< "input_file start_node";
	return ss.str();
}

size_t omp_tester::parse_size_t(const char *value, const char *parse_error, const char *overflow_error)
{
	try
	{
		return std::stoull(value);
	}
	catch (std::invalid_argument const &e)
	{
		throw std::invalid_argument(parse_error + std::string(e.what()) + ": " + value);
	}
	catch (std::out_of_range const &e)
	{
		throw std::invalid_argument(overflow_error + std::string(e.what()) + ": " + value);
	}
}

int omp_tester::parse_int(const char *value, const char *parse_error, const char *overflow_error)
{
	try
	{
		return std::stoi(value);
	}
	catch (std::invalid_argument const &e)
	{
		throw std::invalid_argument(parse_error + std::string(e.what()) + ": " + value);
	}
	catch (std::out_of_range const &e)
	{
		throw std::invalid_argument(overflow_error + std::string(e.what()) + ": " + value);
	}
}
void omp_tester::processArgvResolver(shared_ptr<ArgvResolver> argvResolver)
{
	argvResolver->addArgument(OUTPUT_FILE_ARG, DEFAULT_OUTPUT_FILE_NAME);
	argvResolver->addArgument(VERBOSE_ARG, "1");
	argvResolver->addArgument(USE_GENERATED_GRAPH_ARG, "");
	argvResolver->addArgument(START_NODE_ARG, "");
	output_file = argvResolver->getArgument(OUTPUT_FILE_ARG);

	if (argvResolver->getArgument(USE_GENERATED_GRAPH_ARG) != ""){
		argvResolver->addArgument(INPUT_ARG, "");
		nodes = parse_size_t(argvResolver->getArgument(USE_GENERATED_GRAPH_ARG).c_str(),
			"Non-integer parameter passed as matrices dimension!",
			"Too large value passed as matrices dimension!");
		use_gen_input = true;
	}

	

	threads = parse_size_t(argvResolver->getArgument(THREADS_NUMBER_ARG).c_str(),
		"Non-integer parameter passed as threads number!",
		"Too large value passed as threads number!");
	omp_set_dynamic(0);
	omp_set_num_threads(threads);

	verbose = argvResolver->getArgument(VERBOSE_ARG) == "1";

	input_file = argvResolver->getArgument(INPUT_ARG).c_str();

	if (argvResolver->getArgument(START_NODE_ARG) != ""){
		start_node = parse_size_t(argvResolver->getArgument(START_NODE_ARG).c_str(),
			"Non-integer parameter passed as matrices dimension!",
			"Too large value passed as matrices dimension!");
		start_node_specified = true;
	}

	check_arguments();

}
int omp_tester::run(){
	process();
	return 1;
}
omp_tester::omp_tester(const int argc, const char *const argv[])
{

}

void omp_tester::check_arguments() const
{
	if (input_file.empty() && !use_gen_input)
	{
		throw std::invalid_argument("Input file is required!");
	}
	if (!nodes && use_gen_input)
	{
		throw std::invalid_argument("Nodes number must be positive!");
	}
	if (!start_node_specified)
	{
		throw std::invalid_argument("Start node is required!");
	}
}

void omp_tester::process()
{
	log("Loading graph");

	if (use_gen_input)
	{
		generate_matrix(nodes);
	}
	else if (!use_gen_input)
	{
		read_matrix(input_file);
	}

	log("Threads: ", threads);
	log("Nodes: ", nodes);
	log("Start node: ", start_node);
	log("Graph: ");
	log(*data);

	if (start_node >= nodes)
	{
		throw std::invalid_argument("Start node number must be less or equal than nodes number!");
	}


	std::unique_ptr<std::vector<type_t>> result;

	result = dijkstra_run();





	log("Distances:");
	log(*result);

	print_answer(output_file, *result);
}