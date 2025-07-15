// Author(s): Richard Farla
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file lps2bdd.cpp
/// \brief This tool transforms an .lps file into a binary decision diagram.

#include <csignal>
#include <memory>
#include "bdd_io.h"
#include "mcrl2/utilities/input_tool.h"
#include "mcrl2/lps/io.h"

using namespace mcrl2;
using utilities::tools::input_tool;

class lps2bdd_tool: public input_tool
{
  typedef input_tool super;

  public:
    lps2bdd_tool()
      : super("lps2bdd",
              "Richard Farla",
              "generates a BDD from an LPS",
              "Transforms the LPS in INFILE to a BDD. "
              "If INFILE is not present or '-', stdin is used."
             )
    {}

    bool run() override
    {
      // Load the LPS specification
      lps::specification lpsspec;
      lps::load_lps(lpsspec, input_filename());

      // Create all variables for reachability, maintaining a queue for the initial state only
      oxidd::bdd_manager mgr(std::pow(2, 27), std::pow(2, 28), 6);
      std::unordered_set<std::string> variable_names;
      std::unordered_map<std::string, oxidd::bdd_function> variables;
      std::queue<oxidd::bdd_function> variable_queue;
      oxidd::bdd_function variables_q;
      std::vector<std::tuple<oxidd::bdd_function, oxidd::bdd_function>> substitution_list = {};
      variables["true"] = mgr.t();
      variables["false"] = mgr.f();
      for (const data::variable& parameter : lpsspec.process().process_parameters())
      {
        const std::string& name = pp(parameter.name());
        variable_names.insert(name);

        // TODO: Support non-boolean parameters
        const oxidd::bdd_function& variable_p = mgr.new_var();
        const oxidd::bdd_function& variable_q = mgr.new_var();
        variables[name] = variable_p;
        variables[name + "_sub"] = variable_q;
        variable_queue.push(variable_p);
        variables_q = variables_q.is_invalid() ? variable_q : (variables_q & variable_q);
        substitution_list.push_back({variable_p, variable_q});
      }
      const oxidd::bdd_substitution& substitution = oxidd::bdd_substitution(substitution_list.begin(), substitution_list.end());

      // Initial state
      oxidd::bdd_function initial_state;
      for (const data::data_expression& expression : lpsspec.initial_process().expressions())
      {
        assert(!variable_queue.empty());
        oxidd::bdd_function variable = variable_queue.front();
        if (expression[0].function().name() == "false") variable = ~variable;
        initial_state = initial_state.is_invalid() ? variable : (initial_state & variable);
        variable_queue.pop();
      }
      assert(variable_queue.empty());

      // Transition relation
      oxidd::bdd_function transition_relation;
      const std::chrono::steady_clock::time_point& start = std::chrono::high_resolution_clock::now();
      for (const lps::action_summand& action : lpsspec.process().action_summands())
      {
        oxidd::bdd_function transition = oxidd::read_bdd_from_string(pp(action.condition()), variables).substitute(substitution);
        std::unordered_set<std::string> unchanged_variable_names = variable_names;
        for (const data::assignment& assignment : action.assignments())
        {
          // TODO: Support non-boolean parameters
          const std::string& name = pp(assignment.lhs());
          unchanged_variable_names.erase(name);
          oxidd::bdd_function variable = variables.at(name);
          if (pp(assignment.rhs()) == "false") variable = ~variable;
          transition &= variable;
        }
        for (const std::string& name : unchanged_variable_names)
        {
          transition &= variables.at(name).equiv(variables.at(name + "_sub"));
        }
        transition_relation = transition_relation.is_invalid() ? transition : (transition_relation | transition);
      }
      const std::chrono::steady_clock::time_point& stop = std::chrono::high_resolution_clock::now();
      const std::chrono::microseconds& duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
      std::cout << "Transition relation construction duration: " << duration.count() << " microseconds\n";
      std::cout << "Transition relation node count: " << transition_relation.node_count() << "\n";

      // Interatively compute the the reachability function
      oxidd::bdd_function reach_new = initial_state;
      oxidd::bdd_function reach_p;
      do
      {
        std::cout << "Node count: " << reach_new.node_count() << "\n";
        const std::chrono::steady_clock::time_point& start = std::chrono::high_resolution_clock::now();

        // Update reach_p and reach_q for this iteration, constructing reach_q from reach_p
        reach_p = reach_new;
        const oxidd::bdd_function& reach_q = reach_p.substitute(substitution);

        // Compute the new reachability iteration
        reach_new = reach_p | (reach_q & transition_relation).exists(variables_q);

        const std::chrono::steady_clock::time_point& stop = std::chrono::high_resolution_clock::now();
        const std::chrono::microseconds& duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << "Step duration: " << duration.count() << " microseconds\n";
      } while (reach_p != reach_new);
      std::cout << "Final node count: " << reach_new.node_count() << "\n";

      return true;
    }
};

int main(int argc, char** argv)
{
  return lps2bdd_tool().execute(argc, argv);
}
