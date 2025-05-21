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
    const size_t& expected = (size_t) !((bool) i % correctness_interval);
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

  // Construct the formula
  std::string formula;
  for (size_t j = n / 2; j > 0; j--)
  {
    const std::string& j_string = std::to_string(j);
    const std::string& x_string = "x" + j_string;
    const std::string& y_string = "y" + j_string;

    if (empty(formula))
    {
      formula = "!(" + x_string + " && " + y_string + ")";
    }
    else
    {
      formula = "(!(" + x_string + " && " + y_string + ") <=> " + formula + ")";
    }
  }

  // Construct the variables in order
  std::vector<std::string> variables;
  for (size_t j = 1; j <= n / 2; j++)
  {
    variables.push_back("x" + std::to_string(j));
    variables.push_back("y" + std::to_string(j));
  }

  return {formula, variables};
}

int main()
{
  const auto& [formula, variables] = construct_hadamard(1);
  std::cout << formula << "\n";
  std::cout << to_string(variables) << "\n";
  std::cout << "Start CFLOBDD construction\n";
  auto start = std::chrono::high_resolution_clock::now();
  const aterm_cflobdd cflobdd = read_cflobdd_from_string(formula, variables);
  auto stop = std::chrono::high_resolution_clock::now();
  std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  std::cout << "Time taken by CFLOBDD construction: " << duration.count() << " milliseconds\n";
  const auto& [vertex_count, edge_count] = cflobdd.count_vertices_and_edges();
  std::cout << "Vertex count: " << vertex_count << "\t|\t" << "Edge count: " << edge_count << "\n";

  return 0;
}
