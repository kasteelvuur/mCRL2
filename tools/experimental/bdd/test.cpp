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

std::pair<std::string, std::vector<std::string>> construct_hadamard(const size_t& i)
{
  // Calculate n and the size of the matrix as size x size
  assert(i >= 1);
  const size_t& n = std::pow(2, i);
  const size_t& size = std::pow(2, n / 2);

  // Construct the formula and variables
  std::ostringstream formula_stream;
  std::vector<std::string> variables;
  for (size_t j = 1; j <= n / 2; j++)
  {
    const std::string& j_string = std::to_string(j);
    const std::string& x_string = "x" + j_string;
    const std::string& y_string = "y" + j_string;

    // Formula
    if (j > 1)
    {
      formula_stream << " <=> ";
    }
    formula_stream << "!(" << x_string << " && " << y_string << ")";

    // Variables
    variables.push_back(x_string);
    variables.push_back(y_string);
  }

  return {formula_stream.str(), variables};
}

std::tuple<std::unordered_map<std::string, bdd_function>, bdd_function, bdd_function> construct_reachability(const size_t& n)
{
  // Variables
  bdd_manager mgr(std::pow(2, 31), 1024, 1);
  std::unordered_map<std::string, bdd_function> variables;
  for (size_t i = 1; i <= n; i++)
  {
    variables["p" + std::to_string(i)] = mgr.new_var();
    variables["q" + std::to_string(i)] = mgr.new_var();
  }

  // Initial states and variables
  std::ostringstream initial_formula_stream;
  for (size_t i = 1; i <= n; i++)
  {
    const std::string& variable = "p" + std::to_string(i);
    if (i > 1)
    {
      initial_formula_stream << " && ";
    }
    initial_formula_stream << "!" << variable;
  }
  const bdd_function& initial = read_bdd_from_string(initial_formula_stream.str(), variables);

  // Transition relation
  std::ostringstream transition_formula_stream;
  const size_t& state_count = std::pow(2, n);
  for (size_t i = 0; i < state_count; i++)
  {
    if (i > 0)
    {
      transition_formula_stream << " || ";
    }
    transition_formula_stream << "(";

    // Get the binary representation of the current number in booleans
    std::vector<bool> binary_rep (n);
    for (size_t j = 0; j < n; j++)
    {
      binary_rep[j] = i & (((size_t) 1) << (n - j - 1));
      if (j > 0)
      {
        transition_formula_stream << " && ";
      }
      if (!binary_rep[j])
      {
        transition_formula_stream << "!";
      }
      transition_formula_stream << "q" << std::to_string(j + 1);
    }

    // Add a transition to all states with one 0 flipped to 1
    for (size_t j = 0; j < n; j++)
    {
      if (binary_rep[j])
      {
        transition_formula_stream << " && p" << std::to_string(j + 1);
      }
    }
    bool first_j = true;
    for (size_t j = 0; j < n; j++)
    {
      if (!binary_rep[j])
      {
        if (first_j)
        {
          transition_formula_stream << " && (";
          first_j = false;
        }
        else
        {
          transition_formula_stream << " || ";
        }

        bool first_k = true;
        for (size_t k = 0; k < n; k++)
        {
          if (!binary_rep[k])
          {
            if (!first_k)
            {
              transition_formula_stream << " && ";
            }
            first_k = false;

            if (k != j)
            {
              transition_formula_stream << "!";
            }
            transition_formula_stream << "p" << std::to_string(k + 1);
          }
        }
      }
    }

    if (first_j)
    {
      transition_formula_stream << ")";
    }
    else
    {
      transition_formula_stream << "))";
    }
  }
  const bdd_function& transition_relation = read_bdd_from_string(transition_formula_stream.str(), variables);

  return {variables, initial, transition_relation};
}

int main()
{
  const size_t& n = 3;
  const auto& [variables, initial, transition_relation] = construct_reachability(n);
  bdd_function variables_q = variables.at("q1");
  for (size_t i = 2; i <= n; i++)
  {
    variables_q &= variables.at("q" + std::to_string(i));
  }
  std::vector<std::tuple<bdd_function, bdd_function>> substitution_list = {};
  for (const auto& [key, value] : variables)
  {
    if (key[0] == 'p')
    {
      substitution_list.push_back({value, variables.at("q" + key.substr(1, key.size() - 1))});
    }
  }
  bdd_function reach_p = initial;
  bdd_function reach_new = initial;

  do
  {
    std::cout << "Node count: " << reach_new.node_count() << "\n";
    auto start = std::chrono::high_resolution_clock::now();

    // Update reach_p and reach_q for this iteration, constructing reach_q from reach_p
    reach_p = reach_new;
    const bdd_function& reach_q = reach_p.substitute(bdd_substitution(substitution_list.begin(), substitution_list.end()));

    // Calculate the new reachability iteration
    reach_new = reach_p | (reach_q & transition_relation).exists(variables_q);

    auto stop = std::chrono::high_resolution_clock::now();
    std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Step duration: " << duration.count() << " microseconds\n";
  } while (reach_p != reach_new);
  // std::cout << "Node count: " << reach_new.node_count() << "\n";

  // const auto& [formula, variables] = construct_hadamard(6);
  // std::cout << formula << "\n";
  // std::cout << to_string(variables) << "\n";
  // std::cout << "Start BDD construction\n";
  // auto start = std::chrono::high_resolution_clock::now();
  // const bdd_function bdd = read_bdd_from_string(formula, variables);
  // auto stop = std::chrono::high_resolution_clock::now();
  // std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
  // std::cout << "Time taken by BDD construction: " << duration.count() << " microseconds\n";
  // std::cout << "Node count: " << bdd.node_count() << "\n"; // 3 * 2^n - 1 for pq

  return 0;
}
