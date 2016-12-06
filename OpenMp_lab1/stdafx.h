// stdafx.h: включаемый файл дл€ стандартных системных включаемых файлов
// или включаемых файлов дл€ конкретного проекта, которые часто используютс€, но
// не часто измен€ютс€
//

#pragma once

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
typedef std::tuple<size_t, size_t, size_t> dimensions;
typedef std::vector<int64_t> m_vector;
typedef std::vector<m_vector> matrix;
typedef std::chrono::high_resolution_clock m_clock;


// TODO: ”становите здесь ссылки на дополнительные заголовки, требующиес€ дл€ программы
