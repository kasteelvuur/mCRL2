#include "bdd_io.h"
#include <iostream>
#include <chrono>
#include <thread>

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

int main()
{
  bdd_manager mgr(65536, 1024, 1);
  bdd_function x = mgr.new_var();
  bdd_function y = mgr.new_var();
  bdd_function z = mgr.new_var();
  std::cout << "Is satisfiable: " << (x & y | z).satisfiable() << "\n";

  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "Start input construction\n";
  std::ostringstream bdd_string_stream;
  std::vector<std::string> bdd_variables;
  const size_t n = 1;
  for (size_t i = 1; i <= n; i++)
  {
    if (i > 1)
    {
      bdd_string_stream << " && ";
    }
    bdd_string_stream << '(' << 'p' << i << " <=> " << 'q' << i << ')';
    bdd_variables.push_back("p" + std::to_string(i));
  }
  for (size_t i = n; i > 0; i--)
  {
    bdd_variables.push_back("q" + std::to_string(i));
  }
  const std::string bdd_string = bdd_string_stream.str();
  std::cout << bdd_string << "\n";
  std::cout << to_string(bdd_variables) << "\n";
  std::cout << "Start BDD construction\n";
  auto start = std::chrono::high_resolution_clock::now();
  const bdd_function bdd = read_bdd_from_string(bdd_string, bdd_variables);
  auto stop = std::chrono::high_resolution_clock::now();
  std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  std::cout << "Time taken by BDD construction: " << duration.count() << " milliseconds\n";
  std::cout << "Is satisfiable: " << bdd.satisfiable() << "\n";
  std::this_thread::sleep_for(std::chrono::seconds(1));

  return 0;
}
