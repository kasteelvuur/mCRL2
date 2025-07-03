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
  bdd_manager mgr(std::pow(2, 20), 1024, 1);
  std::unordered_map<std::string, bdd_function> variables;
  std::vector<std::tuple<bdd_function, bdd_function>> substitution_list = {};
  for (size_t i = 1; i <= n; i++)
  {
    const bdd_function& variable_p = mgr.new_var();
    const bdd_function& variable_q = mgr.new_var();
    variables["p" + std::to_string(i)] = variable_p;
    variables["q" + std::to_string(i)] = variable_q;
    substitution_list.push_back({variable_p, variable_q});
  }
  const bdd_substitution substitution(substitution_list.begin(), substitution_list.end());

  // Predefine all states as BDDs
  const size_t& state_count = std::pow(2, n);
  std::vector<bdd_function> states;
  for (size_t i = 0; i < state_count; i++)
  {
    bdd_function state_formula;
    for (size_t j = 0; j < n; j++)
    {
      bdd_function variable_formula = variables.at("p" + std::to_string(j + 1));
      if (!((bool) (i & (((size_t) 1) << (n - j - 1))))) variable_formula = ~variable_formula;
      state_formula = state_formula.is_invalid() ? variable_formula : (state_formula & variable_formula);
    }
    states.push_back(state_formula);
  }

  // Initial states only contains the first state
  const bdd_function& initial_formula = states[0];

  // Transition relation
  bdd_function transition_formula;
  for (size_t i = 0; i < state_count - 1; i++)
  {
    const bdd_function& source_state = states[i].substitute(substitution);
    bdd_function target_states;
    for (size_t j = i + 1; j < state_count; j++)
    {
      const size_t& x = i ^ j;
      if (x && !(x & (x - 1)))
      {
        target_states = target_states.is_invalid() ? states[j] : (target_states | states[j]);
      }
    }
    if (!target_states.is_invalid())
    {
      const bdd_function& transition = source_state & target_states;
      transition_formula = transition_formula.is_invalid() ? transition : (transition_formula | transition);
    }
  }

  return {variables, initial_formula, transition_formula};
}

void add_peg_solitaire_transition(
  bdd_function& transition_formula,
  const std::unordered_map<std::string, bdd_function>& variables,
  const std::string& source_prefix,
  const std::string& target_prefix,
  const size_t& i,
  const size_t& i_1,
  const size_t& i_2
) {
  const bdd_function& transition = variables.at(source_prefix + std::to_string(i))
    & variables.at(source_prefix + std::to_string(i_1))
    & ~variables.at(source_prefix + std::to_string(i_2))
    & ~variables.at(target_prefix + std::to_string(i))
    & ~variables.at(target_prefix + std::to_string(i_1))
    & variables.at(target_prefix + std::to_string(i_2));
    transition_formula = transition_formula.is_invalid() ? transition : (transition_formula | transition);
}

void peg_solitaire_simplified(
  std::unordered_map<std::string, oxidd::bdd_function>& variables,
  bdd_function& variables_sub,
  bdd_substitution& substitution,
  bdd_function& initial_formula,
  bdd_function& transition_formula
) {
  const size_t& n = 33;
  const size_t& middle_index = 16;
  const std::string& main_letter = "p";
  const std::string& sub_letter = "q";

  // Variables
  bdd_manager mgr(std::pow(2, 20), 1024, 1);
  std::vector<std::tuple<bdd_function, bdd_function>> substitution_list = {};
  for (size_t i = 0; i < n; i++)
  {
    const bdd_function& variable_main = mgr.new_var();
    const bdd_function& variable_sub = mgr.new_var();
    variables[main_letter + std::to_string(i)] = variable_main;
    variables[sub_letter + std::to_string(i)] = variable_sub;
    variables_sub = variables_sub.is_invalid() ? variable_sub : (variables_sub & variable_sub);
    substitution_list.push_back({variable_main, variable_sub});
  }
  substitution = bdd_substitution(substitution_list.begin(), substitution_list.end());

  // Initial states only contains the state where everything except middle is filled
  initial_formula = ~variables[main_letter + std::to_string(middle_index)];
  for (size_t i = 0; i < n; i++)
  {
    if (i != middle_index) initial_formula &= variables[main_letter + std::to_string(i)];
  }

  // Transition relation - common transitions
  for (size_t i = 0; i < n; i++)
  {
    // Predefine booleans for the different sections
    const bool& top = i <= 5;
    const bool& bot = i >= 27;
    const bool& mid_horiz = 6 <= i && i <= 26;
    const size_t& mid_col_idx = (i + 1) % 7;
    const bool& mid = mid_horiz && 2 <= mid_col_idx && mid_col_idx <= 4;
    const bool& left = mid_horiz && mid_col_idx <= 1;
    const bool& right = mid_horiz && mid_col_idx >= 5;

    // Move right
    if (mid || left || ((top || bot) && i % 3 == 0))
    {
      add_peg_solitaire_transition(transition_formula, variables, sub_letter, main_letter, i, i + 1, i + 2);
    }

    // Move left
    if (mid || right || ((top || bot) && i % 3 == 2))
    {
      add_peg_solitaire_transition(transition_formula, variables, sub_letter, main_letter, i, i - 1, i - 2);
    }

    // Move up
    if (mid || bot || ((left || right) && i >= 20))
    {
      const size_t& i_1 = i >= 30 ? i - 3 : i <= 10 || i >= 27 ? i - 5 : i - 7;
      const size_t& i_2 = i <= 10 || i >= 30 ? i - 8 : i <= 17 || i >= 27 ? i - 12 : i - 14;
      add_peg_solitaire_transition(transition_formula, variables, sub_letter, main_letter, i, i_1, i_2);
    }

    // Move down
    if (mid || top || ((left || right) && i <= 12))
    {
      const size_t& i_1 = i <= 2 ? i + 3 : i <= 5 || i >= 22 ? i + 5 : i + 7;
      const size_t& i_2 = i <= 2 || i >= 22 ? i + 8 : i <= 5 || i >= 15 ? i + 12 : i + 14;
      add_peg_solitaire_transition(transition_formula, variables, sub_letter, main_letter, i, i_1, i_2);
    }
  }

  // Transition relation - special ready transition (end)
  bdd_function ready_transition = variables[sub_letter + std::to_string(middle_index)];
  for (size_t i = 0; i < n; i++)
  {
    if (i != middle_index) ready_transition &= ~variables[sub_letter + std::to_string(i)];
    ready_transition &= ~variables[main_letter + std::to_string(i)];
  }
  transition_formula |= ready_transition;
}

int main()
{
  std::unordered_map<std::string, oxidd::bdd_function> variables;
  bdd_function variables_q, initial, transition_relation;
  bdd_substitution substitution;
  peg_solitaire_simplified(variables, variables_q, substitution, initial, transition_relation);

  std::cout << "Transition relation node count: " << transition_relation.node_count() << "\n";
  bdd_function reach_p = initial;
  bdd_function reach_new = initial;

  do
  {
    std::cout << "Node count: " << reach_new.node_count() << "\n";
    auto start = std::chrono::high_resolution_clock::now();

    // Update reach_p and reach_q for this iteration, constructing reach_q from reach_p
    reach_p = reach_new;
    const bdd_function& reach_q = reach_p.substitute(substitution);

    // Calculate the new reachability iteration
    reach_new = reach_p | (reach_q & transition_relation).exists(variables_q);

    auto stop = std::chrono::high_resolution_clock::now();
    std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Step duration: " << duration.count() << " microseconds\n";
  } while (reach_p != reach_new);

  return 0;
}
