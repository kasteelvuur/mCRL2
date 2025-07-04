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
    const aterm_cflobdd& proposition_letter = aterm_cflobdd(level, i);
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

std::pair<aterm_cflobdd, aterm_cflobdd> construct_reachability(const size_t& n)
{
  // Variables
  std::vector<std::string> variables;
  const size_t& next_power_two = std::pow(2, std::ceil(std::log2(n)));
  for (size_t i = 1; i <= next_power_two; i++)
  {
    if (i <= n)
    {
      variables.push_back("p" + std::to_string(i));
    }
    else
    {
      variables.push_back("");
    }
  }
  for (size_t i = 1; i <= n; i++)
  {
    variables.push_back("q" + std::to_string(i));
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
  const aterm_cflobdd& initial = read_cflobdd_from_string(initial_formula_stream.str(), variables);

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
  const aterm_cflobdd& transition_relation = read_cflobdd_from_string(transition_formula_stream.str(), variables);

  return {initial, transition_relation};
}

void add_peg_solitaire_transition(
  aterm_cflobdd& transition_formula,
  const std::vector<aterm_cflobdd>& variables,
  const size_t& n,
  const size_t& i,
  const size_t& i_1,
  const size_t& i_2
) {
  aterm_cflobdd transition = variables[2 * i + 1]
    && variables[2 * i_1 + 1]
    && !variables[2 * i_2 + 1]
    && !variables[2 * i]
    && !variables[2 * i_1]
    && variables[2 * i_2];
  for (size_t j = 0; j < n; j++)
  {
    if (j != i && j != i_1 && j != i_2)
    {
      transition = transition && variables[2 * j].iff(variables[2 * j + 1]);
    }
  }
  transition_formula = transition_formula || transition;
}

std::tuple<
  std::vector<size_t>,
  std::pair<std::vector<size_t>, aterm_cflobdd>,
  aterm_cflobdd,
  aterm_cflobdd
> peg_solitaire_simplified() {
  const size_t& n = 33;
  const size_t& middle_index = n / 2;
  const size_t& level = std::ceil(std::log2(n)) + 1;

  // Variables
  std::vector<size_t> variables_sub_indices;
  std::vector<aterm_cflobdd> variables;
  std::pair<std::vector<size_t>, aterm_cflobdd> substitution = {{}, aterm_cflobdd(level, true)};
  for (size_t i = 0; i < n; i++)
  {
    const aterm_cflobdd& variable_main = aterm_cflobdd(level, 2 * i);
    const aterm_cflobdd& variable_sub = aterm_cflobdd(level, 2 * i + 1);
    variables.push_back(variable_main);
    variables.push_back(variable_sub);
    variables_sub_indices.push_back(2 * i + 1);
    substitution.first.push_back(2 * i);
    substitution.second = substitution.second && variable_main.iff(variable_sub);
  }

  // Initial states only contains the state where everything except middle is filled
  aterm_cflobdd initial_formula = !variables[2 * middle_index];
  for (size_t i = 0; i < n; i++)
  {
    if (i != middle_index) initial_formula = initial_formula && variables[2 * i];
  }

  // Transition relation - common transitions
  aterm_cflobdd transition_formula = aterm_cflobdd(level, false);
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
      add_peg_solitaire_transition(transition_formula, variables, n, i, i + 1, i + 2);
    }

    // Move left
    if (mid || right || ((top || bot) && i % 3 == 2))
    {
      add_peg_solitaire_transition(transition_formula, variables, n, i, i - 1, i - 2);
    }

    // Move up
    if (mid || bot || ((left || right) && i >= 20))
    {
      const size_t& i_1 = i >= 30 ? i - 3 : i <= 10 || i >= 27 ? i - 5 : i - 7;
      const size_t& i_2 = i <= 10 || i >= 30 ? i - 8 : i <= 17 || i >= 27 ? i - 12 : i - 14;
      add_peg_solitaire_transition(transition_formula, variables, n, i, i_1, i_2);
    }

    // Move down
    if (mid || top || ((left || right) && i <= 12))
    {
      const size_t& i_1 = i <= 2 ? i + 3 : i <= 5 || i >= 22 ? i + 5 : i + 7;
      const size_t& i_2 = i <= 2 || i >= 22 ? i + 8 : i <= 5 || i >= 15 ? i + 12 : i + 14;
      add_peg_solitaire_transition(transition_formula, variables, n, i, i_1, i_2);
    }
  }

  // Transition relation - special ready transition (end)
  aterm_cflobdd ready_transition = variables[2 * middle_index + 1];
  for (size_t i = 0; i < n; i++)
  {
    if (i != middle_index) ready_transition = ready_transition && !variables[2 * i + 1];
    ready_transition = ready_transition && !variables[2 * i];
  }
  transition_formula = transition_formula || ready_transition;

  return {variables_sub_indices, substitution, initial_formula, transition_formula};
}

int main()
{
  aterm_cflobdd reach_p = aterm_cflobdd(0, false);
  auto [variables_q, substitution, reach_new, transition_relation] = peg_solitaire_simplified();
  const auto& [vertex_count, edge_count] = transition_relation.count_vertices_and_edges();
  std::cout << "Vertex count: " << vertex_count << "\t|\t" << "Edge count: " << edge_count << "\n";

  do
  {
    const auto& [vertex_count, edge_count] = reach_new.count_vertices_and_edges();
    std::cout << "Vertex count: " << vertex_count << "\t|\t" << "Edge count: " << edge_count << "\n";
    const std::chrono::steady_clock::time_point& start = std::chrono::high_resolution_clock::now();

    // Update reach_p and reach_q for this iteration, constructing reach_q from reach_p
    reach_p = reach_new;
    const aterm_cflobdd& reach_q = reach_p.substitute(substitution.first, substitution.second);

    // Calculate the new reachability iteration
    reach_new = reach_p || (reach_q && transition_relation).exists(variables_q);

    const std::chrono::steady_clock::time_point& stop = std::chrono::high_resolution_clock::now();
    std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Step duration: " << duration.count() << " microseconds\n";
  } while (reach_p != reach_new);

  return 0;
}
