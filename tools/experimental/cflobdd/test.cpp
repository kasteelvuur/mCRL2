#include "aterm_cflobdd_io.h"
#include <chrono>

using namespace atermpp;

std::string to_string(const std::vector<bool>& vec)
{
  std::string res = "[";
  bool first = true;
  for (const bool& val : vec) {
    if (!first) res.push_back(',');
    res.push_back(val ? '1' : '0');
    first = false;
  }
  res.push_back(']');
  return res;
}

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

void test_proto_cflobdd(const aterm_proto_cflobdd& c)
{
  std::cout << "Proto-CFLOBDD: " << c << "\n";

  const size_t& level = c.level();
  std::cout << "Level: " << level << "\n";

  const size_t& out_degree = c.out_degree();
  std::cout << "Out degree: " << out_degree << "\n";

  const size_t& is_reduced = c.is_reduced();
  std::cout << "Is reduced: " << is_reduced << "\n";

  const size_t& letter_count = std::pow(2, level);
  const size_t& configuration_count = std::pow(2, letter_count);
  std::vector<bool> sigma (letter_count);
  for (size_t i = 0; i < configuration_count; i++)
  {
    for (size_t j = 0; j < letter_count; j++)
    {
      sigma[j] = i & (((size_t) 1) << (letter_count - j - 1));
    }
    const size_t& eval = c.evaluate(sigma);
    std::cout << to_string(sigma) << " evaluates to " << eval << "\n";
  }

  std::cout << "\n";
}

void test_cflobdd(const aterm_cflobdd& c)
{
  std::cout << "CFLOBDD: " << c << "\n";

  const size_t& is_reduced = c.is_reduced();
  std::cout << "Is reduced: " << is_reduced << "\n";

  const size_t& level = down_cast<aterm_proto_cflobdd>(c[0]).level();
  const size_t& letter_count = std::pow(2, level);
  const size_t& configuration_count = std::pow(2, letter_count);
  std::vector<bool> sigma (letter_count);
  for (size_t i = 0; i < configuration_count; i++)
  {
    for (size_t j = 0; j < letter_count; j++)
    {
      sigma[j] = i & (((size_t) 1) << (letter_count - j - 1));
    }
    const size_t& eval = c.evaluate(sigma);
    std::cout << to_string(sigma) << " evaluates to " << eval << "\n";
  }

  std::cout << "\n";
}

/// \brief Test the CFLOBDD for the conjunction of biconditions.
///   The CFLOBDD encodes \bigwedge_{i=0}^{n-1} p_i <=> q_i
///   for the order [p_0, ..., p_{n-1}, q_0, ..., q_{n-1}].
/// \param n The amount of proposition letters labelled p and q
void test_conjunction_of_biconditions(const size_t& n = 2)
{
  // Calculate the required CFLOBDD level
  const size_t& level = std::ceil(std::log2(n)) + 1;
  assert(std::pow(2, level - 1) < 2 * n && 2 * n <= std::pow(2, level));

  // Create 2*n CFLOBDDs for singular proposition letters
  std::vector<aterm_cflobdd> proposition_letters;
  for (size_t i = 0; i < 2 * n; i++)
  {
    const aterm_cflobdd& proposition_letter = construct_cflobdd(level, i);
    proposition_letters.push_back(proposition_letter);
    // test_cflobdd(proposition_letter);
  }
  assert(proposition_letters.size() == 2 * n);

  // Create paired CFLOBDDs p_i <=> q_i given the order [p_0, ..., p_{n-1}, q_0, ..., q_{n-1}]
  std::vector<aterm_cflobdd> paired_proposition_letters;
  for (size_t i = 0; i < n; i++)
  {
    const aterm_cflobdd& paired_proposition_letter = proposition_letters[i].iff(proposition_letters[i + n]);
    paired_proposition_letters.push_back(paired_proposition_letter);
    // test_cflobdd(paired_proposition_letter);
  }
  assert(paired_proposition_letters.size() == n);

  // Take the conjunction of all paired CFLOBDDs
  aterm_cflobdd conjuction = paired_proposition_letters[0];
  for (size_t i = 1; i < n; i++)
  {
    conjuction = conjuction && paired_proposition_letters[i];
    // test_cflobdd(conjuction);
  }

  // Ensure that the CFLOBDD evaluates correctly
  const size_t& configuration_count = std::pow(2, 2 * n);
  const size_t& correctness_interval = std::pow(2, n) + 1;
  std::vector<bool> sigma (std::pow(2, level));
  for (size_t i = 0; i < configuration_count; i++)
  {
    for (size_t j = 0; j < 2 * n; j++)
    {
      sigma[j] = i & (((size_t) 1) << (2 * n - j - 1));
    }
    const size_t& eval = conjuction.evaluate(sigma);
    std::cout << to_string(sigma) << " evaluates to " << eval << "\n";
    const size_t& expected = (size_t) !((bool) (i % correctness_interval));
    assert(eval == expected);
  }
  std::cout << "\n";
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

  // Fill variable list to some power of 2
  // const size_t& size = variables.size();
  // const size_t& next_power = std::pow(2, std::ceil(std::log2(size)));
  // for (size_t i = size; i < next_power; i++)
  // {
  //   variables.push_back("");
  // }

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

std::pair<aterm_cflobdd, std::vector<aterm_cflobdd>> construct_reachability(const size_t& n)
{
  // Initial states and variables
  std::ostringstream initial_formula_stream;
  std::vector<std::string> variables;
  for (size_t i = 0; i < n; i++)
  {
    const std::string& variable = "p" + std::to_string(i);
    if (i > 0)
    {
      initial_formula_stream << " && ";
    }
    initial_formula_stream << "!" << variable;
    variables.push_back(variable);
  }
  const aterm_cflobdd& initial = read_cflobdd_from_string(initial_formula_stream.str(), variables);

  // Transition relation
  std::vector<aterm_cflobdd> transition_relations;
  const size_t& state_count = std::pow(2, n);
  for (size_t i = 0; i < state_count; i++)
  {
    std::ostringstream transition_formula_stream;

    // Get the binary representation of the current number in booleans
    std::vector<bool> binary_rep (n);
    for (size_t j = 0; j < n; j++)
    {
      binary_rep[j] = i & (((size_t) 1) << (n - j - 1));
    }

    // Add a transition to all states with one 0 flipped to 1
    bool first = true;
    for (size_t j = 0; j < n; j++)
    {
      if (!binary_rep[j])
      {
        if (!first)
        {
          transition_formula_stream << " || ";
        }
        first = false;

        for (size_t k = 0; k < n; k++)
        {
          const std::string& variable = "p" + std::to_string(k);
          if (k > 0)
          {
            transition_formula_stream << " && ";
          }
          if (k != j && !binary_rep[k])
          {
            transition_formula_stream << "!";
          }
          transition_formula_stream << variable;
        }
      }
    }

    // Special case: self loop for state with all 1
    if (i == state_count - 1)
    {
      for (size_t j = 0; j < n; j++)
      {
        const std::string& variable = "p" + std::to_string(j);
        if (j > 0)
        {
          transition_formula_stream << " && ";
        }
        transition_formula_stream << variable;
      }
    }

    // Calculate this transition relation
    const aterm_cflobdd& transition_relation = read_cflobdd_from_string(transition_formula_stream.str(), variables);
    transition_relations.push_back(transition_relation);
  }

  return {initial, transition_relations};
}

int main()
{
  const size_t& n = 2; // NOTE: must currently be power of two due to eval construction
  const auto& [initial, transition_relations] = construct_reachability(n);
  aterm_cflobdd reach_old = initial;
  aterm_cflobdd reach_new = initial;
  do
  {
    reach_old = reach_new;

    const size_t& state_count = std::pow(2, n);
    for (size_t i = 0; i < state_count; i++)
    {
      // Get the binary representation of the current number in booleans
      std::vector<bool> sigma (n);
      for (size_t j = 0; j < n; j++)
      {
        sigma[j] = i & (((size_t) 1) << (n - j - 1));
      }

      // If this state is reachable, then add all states directly reachable from here
      std:: cout << to_string(sigma) << "\t" << reach_old.evaluate(sigma) << "\n";
      if (reach_old.evaluate(sigma))
      {
        reach_new = reach_new || transition_relations[i];
      }
    }
  } while (reach_old != reach_new);
  std::cout << reach_new << "\n";
  const auto& [vertex_count, edge_count] = reach_new.count_vertices_and_edges();
  std::cout << "Vertex count: " << vertex_count << "\t|\t" << "Edge count: " << edge_count << "\n";
  
  // std::cout << formula << "\n";
  // std::cout << to_string(variables) << "\n";
  // std::cout << "Start CFLOBDD construction\n";
  // auto start = std::chrono::high_resolution_clock::now();
  // const aterm_cflobdd cflobdd = read_cflobdd_from_string(formula, variables);
  // auto stop = std::chrono::high_resolution_clock::now();
  // std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
  // std::cout << "Time taken by CFLOBDD construction: " << duration.count() << " microseconds\n";
  // const auto& [vertex_count, edge_count] = cflobdd.count_vertices_and_edges();
  // std::cout << "Vertex count: " << vertex_count << "\t|\t" << "Edge count: " << edge_count << "\n";

  return 0;
}
