// Author(s): Richard Farla
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file lps2cflobdd.cpp
/// \brief This tool transforms an .lps file into a context-free-language ordered binary decision diagram.

#include <csignal>
#include <memory>
#include "aterm_cflobdd_io.h"
#include "mcrl2/utilities/input_tool.h"
#include "mcrl2/lps/io.h"

using namespace mcrl2;
using namespace atermpp;
using mcrl2::utilities::tools::input_tool;

class lps2bdd_tool: public input_tool
{
  typedef input_tool super;

  private:
    const aterm_proto_cflobdd& i_1 = aterm_proto_cflobdd(1);
    const aterm_proto_cflobdd& p_1 = aterm_proto_cflobdd(1, 0);
    const aterm_proto_cflobdd& q_1 = aterm_proto_cflobdd(1, 1);

    aterm_proto_cflobdd substitute(const aterm_proto_cflobdd& reach_p)
    {
      const size_t& level = reach_p.level();
      if (!level) throw std::runtime_error("Cannot call on level 0");

      if (level == 1)
      {
        // Switch p and q at this level
        if (reach_p == p_1) return q_1;
        else if (reach_p == i_1) return i_1;
        else throw std::runtime_error("Unexpected proto-CFLOBDD on level 1");
      }
      else
      {
        // Recurse further down
        const aterm_list& cvs_p = down_cast<aterm_list>(reach_p[1]);
        aterm_list cvs_q;
        for (reverse_term_list_iterator i = cvs_p.rbegin(); i != cvs_p.rend(); ++i)
        {
          const aterm_pair& cv = down_cast<aterm_pair>(*i);
          cvs_q.push_front(aterm_pair(
            substitute(down_cast<aterm_proto_cflobdd>(cv[0])),
            down_cast<aterm_list>(cv[1])
          ));
        }
        return aterm_proto_cflobdd(
          substitute(down_cast<aterm_proto_cflobdd>(reach_p[0])),
          cvs_q
        );
      }
    }

    aterm_cflobdd substitute(const aterm_cflobdd& reach_p)
    {
      const aterm_proto_cflobdd& c = down_cast<aterm_proto_cflobdd>(reach_p[0]);
      const aterm_list& vs = down_cast<aterm_list>(reach_p[1]);
      return aterm_cflobdd(substitute(c), vs);
    }

  public:
    lps2bdd_tool()
      : super("lps2bdd",
              "Richard Farla",
              "generates a CFLOBDD from an LPS",
              "Transforms the LPS in INFILE to a CFLOBDD. "
              "If INFILE is not present or '-', stdin is used."
             )
    {}

    bool run() override
    {
      // Load the LPS specification
      lps::specification lpsspec;
      lps::load_lps(lpsspec, input_filename());

      // Create all variables for reachability, maintaining a queue for the initial state only
      std::unordered_set<std::string> variable_names;
      std::unordered_map<std::string, aterm_cflobdd> variables;
      std::queue<aterm_cflobdd> variable_queue;
      std::vector<size_t> variables_q;
      const size_t& level = std::ceil(std::log2(lpsspec.process().process_parameters().size())) + 1;
      variables["true"] = aterm_cflobdd(level, true);
      variables["false"] = aterm_cflobdd(level, false);
      size_t index = 0;
      for (const data::variable& parameter : lpsspec.process().process_parameters())
      {
        const std::string& name = pp(parameter.name());
        variable_names.insert(name);

        // TODO: Support non-boolean parameters
        const aterm_cflobdd& variable_p = aterm_cflobdd(level, index++);
        const aterm_cflobdd& variable_q = aterm_cflobdd(level, index++);
        variables[name] = variable_p;
        variables[name + "_sub"] = variable_q;
        variable_queue.push(variable_p);
        variables_q.push_back(index - 1);
      }

      // Initial state
      aterm_cflobdd initial_state = aterm_cflobdd(level, true);
      for (const data::data_expression& expression : lpsspec.initial_process().expressions())
      {
        assert(!variable_queue.empty());
        aterm_cflobdd variable = variable_queue.front();
        if (expression[0].function().name() == "false") variable = !variable;
        initial_state = initial_state && variable;
        variable_queue.pop();
      }
      assert(variable_queue.empty());

      // Transition relation
      aterm_cflobdd transition_relation = aterm_cflobdd(level, false);
      const auto& start = std::chrono::high_resolution_clock::now();
      for (const lps::action_summand& action : lpsspec.process().action_summands())
      {
        aterm_cflobdd transition = substitute(read_cflobdd_from_string(pp(action.condition()), variables));
        std::unordered_set<std::string> unchanged_variable_names = variable_names;
        for (const data::assignment& assignment : action.assignments())
        {
          // TODO: Support non-boolean parameters
          const std::string& name = pp(assignment.lhs());
          unchanged_variable_names.erase(name);
          aterm_cflobdd variable = variables.at(name);
          if (pp(assignment.rhs()) == "false") variable = !variable;
          transition = transition && variable;
        }
        for (const std::string& name : unchanged_variable_names)
        {
          transition = transition && variables.at(name).iff(variables.at(name + "_sub"));
        }
        transition_relation = transition_relation || transition;
      }
      const auto& stop = std::chrono::high_resolution_clock::now();
      const std::chrono::microseconds& duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
      std::cout << "Transition relation construction duration: " << duration.count() << " microseconds\n";
      const auto& [vertex_count, edge_count] = transition_relation.count_vertices_and_edges();
      std::cout << "Transition relation - Vertex count: " << vertex_count << "\t|\t" << "Edge count: " << edge_count << "\n";

      // Interatively compute the the reachability function
      aterm_cflobdd reach_new = initial_state;
      aterm_cflobdd reach_p = initial_state;
      do
      {
        const auto& [vertex_count, edge_count] = reach_new.count_vertices_and_edges();
        std::cout << "Vertex count: " << vertex_count << "\t|\t" << "Edge count: " << edge_count << "\n";
        const auto& start = std::chrono::high_resolution_clock::now();

        // Update reach_p and reach_q for this iteration, constructing reach_q from reach_p
        reach_p = reach_new;
        const aterm_cflobdd& reach_q = substitute(reach_p);

        // Compute the new reachability iteration
        reach_new = reach_p || (reach_q && transition_relation).exists(variables_q);

        const auto& stop = std::chrono::high_resolution_clock::now();
        const std::chrono::microseconds& duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        std::cout << "Step duration: " << duration.count() << " microseconds\n";
      } while (reach_p != reach_new);

      return true;
    }
};

int main(int argc, char** argv)
{
  return lps2bdd_tool().execute(argc, argv);
}
