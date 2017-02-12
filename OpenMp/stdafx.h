// stdafx.h: включаемый файл дл€ стандартных системных включаемых файлов
// или включаемых файлов дл€ конкретного проекта, которые часто используютс€, но
// не часто измен€ютс€
//

#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <tuple>
#include <chrono>
#include <random>
#include <fstream>
#include <sstream>
#include <iostream>
#include <tchar.h>
#include <memory>
#include <vector>
#include "csvfile.h"
#include <windows.h>
typedef std::tuple<size_t, size_t> dimensions;
typedef std::tuple<char*, char*, char*, char*, char*> inputParam;
typedef std::tuple<dimensions, dimensions, dimensions, dimensions> csvResult;
typedef std::vector<int64_t> m_vector;
typedef std::vector<m_vector> matrix;
typedef std::chrono::high_resolution_clock m_clock;


// TODO: ”становите здесь ссылки на дополнительные заголовки, требующиес€ дл€ программы
