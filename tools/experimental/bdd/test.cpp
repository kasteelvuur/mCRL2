#include "bdd_io.h"
#include <iostream>
#include <chrono>

using namespace oxidd;

std::string to_string(const std::vector<std::string>& vec)
{
  std::string res = "[";
  bool first = true;
  for (const std::string& val : vec) {
    if (!first) res.push_back(',');
    res.append(val);
    first = false;
  }
  res.push_back(']');
  return res;
}

std::pair<std::string, std::vector<std::string>> construct_pq(const size_t& n)
{
  std::ostringstream formula_stream;
  std::vector<std::string> variables;

  for (size_t i = 1; i <= n; i++)
  {
    if (i > 1)
    {
      formula_stream << " && ";
    }
    formula_stream << '(' << 'p' << i << " <=> " << 'q' << i << ')';
    variables.push_back("p" + std::to_string(i));
  }
  for (size_t i = n; i > 0; i--)
  {
    variables.push_back("q" + std::to_string(i));
  }

  return {formula_stream.str(), variables};
}

int main()
{
  const auto& [formula, variables] = construct_pq(2);
  std::cout << formula << "\n";
  std::cout << to_string(variables) << "\n";
  std::cout << "Start BDD construction\n";
  auto start = std::chrono::high_resolution_clock::now();
  const bdd_function bdd = read_bdd_from_string(formula, variables);
  auto stop = std::chrono::high_resolution_clock::now();
  std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  std::cout << "Time taken by BDD construction: " << duration.count() << " milliseconds\n";
  std::cout << "Node count: " << bdd.node_count() << "\n"; // 3 * 2^n - 1 for pq

  return 0;
}
