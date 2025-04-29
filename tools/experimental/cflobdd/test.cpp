#include "aterm_cflobdd_io.h"

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
      sigma[j] = i & (1 << (letter_count - j - 1));
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
      sigma[j] = i & (1 << (letter_count - j - 1));
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
      sigma[j] = i & (1 << (2 * n - j - 1));
    }
    const size_t& eval = conjuction.evaluate(sigma);
    std::cout << to_string(sigma) << " evaluates to " << eval << "\n";
    const size_t& expected = !(i % correctness_interval);
    assert(eval == expected);
  }
  std::cout << "\n";
}

int main()
{
  const aterm_proto_cflobdd_i i;
  test_proto_cflobdd(i);

  const aterm_proto_cflobdd_v v;
  test_proto_cflobdd(v);

  aterm_list p;
  p.push_front(aterm_pair(v, read_list_from_string("[2,1]")));
  p.push_front(aterm_pair(v, read_list_from_string("[0,1]")));
  const aterm_proto_cflobdd& c = aterm_proto_cflobdd(v,p);
  test_proto_cflobdd(c);

  aterm_list q;
  q.push_front(aterm_pair(i, read_list_from_string("[0]")));
  const aterm_proto_cflobdd& d = aterm_proto_cflobdd(i, q);
  test_proto_cflobdd(d);

  aterm_list r;
  r.push_front(aterm_pair(c, read_list_from_string("[3,0,1]")));
  r.push_front(aterm_pair(d, read_list_from_string("[1]")));
  r.push_front(aterm_pair(c, read_list_from_string("[0,1,2]")));
  const aterm_proto_cflobdd& e = aterm_proto_cflobdd(c, r);
  test_proto_cflobdd(e);

  aterm_list s;
  s.push_front(aterm_pair(aterm_proto_cflobdd_v(), read_list_from_string("[0,1]")));
  s.push_front(aterm_pair(aterm_proto_cflobdd_v(), read_list_from_string("[0,1]")));
  const aterm_proto_cflobdd& f = aterm_proto_cflobdd(aterm_proto_cflobdd_v(), s);
  test_proto_cflobdd(f);

  aterm_list t;
  t.push_front(aterm_pair(v, read_list_from_string("[0,0]")));
  const aterm_proto_cflobdd& g = aterm_proto_cflobdd(i, t);
  test_proto_cflobdd(g);

  aterm_list u;
  u.push_front(aterm_pair(i, read_list_from_string("[1]")));
  const aterm_proto_cflobdd& h = aterm_proto_cflobdd(i, u);
  test_proto_cflobdd(h);

  aterm_list tt;
  tt.push_front(aterm_pair(g, read_list_from_string("[0]")));
  const aterm_proto_cflobdd& gg = aterm_proto_cflobdd(g, tt);
  test_proto_cflobdd(gg);

  const aterm_cflobdd& x = aterm_cflobdd(d, read_list_from_string("[1]"));
  test_cflobdd(x);
  test_cflobdd(!x);

  const aterm_cflobdd& y = aterm_cflobdd(e, read_list_from_string("[0,1,0,1]"));
  test_cflobdd(y);
  test_cflobdd(!y);

  const aterm_cflobdd& z = aterm_cflobdd(v, read_list_from_string("[0,0]"));
  test_cflobdd(z);
  test_cflobdd(!z);

  const aterm_cflobdd& g2 = aterm_cflobdd(gg, read_list_from_string("[1]"));
  test_cflobdd(g2);
  test_cflobdd(!g2);

  const aterm_cflobdd& yg2 =  y && g2;
  test_cflobdd(yg2);
  test_cflobdd(!yg2);
 
  // p_0 <=> q_0 && p_1 <=> q_1 -- with order p_0, p_1, q_0, q_1
  // i = 0
  aterm_list proto_conj_0_a_b;
  proto_conj_0_a_b.push_front(aterm_pair(i, read_list_from_string("[1]")));
  proto_conj_0_a_b.push_front(aterm_pair(i, read_list_from_string("[0]")));
  const aterm_proto_cflobdd& proto_conj_0_a = aterm_proto_cflobdd(v, proto_conj_0_a_b);
  aterm_list proto_conj_0_b;
  proto_conj_0_b.push_front(aterm_pair(proto_conj_0_a, read_list_from_string("[1,0]")));
  proto_conj_0_b.push_front(aterm_pair(proto_conj_0_a, read_list_from_string("[0,1]")));
  const aterm_proto_cflobdd& proto_conj_0 = aterm_proto_cflobdd(proto_conj_0_a, proto_conj_0_b);
  test_proto_cflobdd(proto_conj_0);
  // i = 1
  aterm_list proto_conj_1_a_b;
  proto_conj_1_a_b.push_front(aterm_pair(v, read_list_from_string("[0,1]")));
  const aterm_proto_cflobdd& proto_conj_1_a = aterm_proto_cflobdd(i, proto_conj_1_a_b);
  aterm_list proto_conj_1_b;
  proto_conj_1_b.push_front(aterm_pair(proto_conj_1_a, read_list_from_string("[1,0]")));
  proto_conj_1_b.push_front(aterm_pair(proto_conj_1_a, read_list_from_string("[0,1]")));
  const aterm_proto_cflobdd& proto_conj_1 = aterm_proto_cflobdd(proto_conj_1_a, proto_conj_1_b);
  test_proto_cflobdd(proto_conj_1);
  // combine
  const aterm_cflobdd& conj_0 = aterm_cflobdd(proto_conj_0, read_list_from_string("[1,0]"));
  test_cflobdd(conj_0);
  const aterm_cflobdd& conj_1 = aterm_cflobdd(proto_conj_1, read_list_from_string("[1,0]"));
  test_cflobdd(conj_1);
  const aterm_cflobdd& conj = conj_0 && conj_1;
  test_cflobdd(conj);

  test_cflobdd(conj.exists(0));
  test_cflobdd(conj.exists(1));
  test_cflobdd(conj.exists(2));
  test_cflobdd(conj.exists(3));
  test_cflobdd(conj.exists(1) || conj.exists(2));

  test_conjunction_of_biconditions(3);

  test_cflobdd(read_cflobdd_from_string("p || q && r || s", {"p", "q", "r", "s"}));

  return 0;
}
