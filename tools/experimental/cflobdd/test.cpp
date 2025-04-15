#include "aterm_cflobdd.h"

#include "mcrl2/atermpp/aterm_io.h"

using namespace atermpp;

std::string to_string(std::vector<bool> vec)
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
  test_cflobdd(x.negate());

  const aterm_cflobdd& y = aterm_cflobdd(e, read_list_from_string("[0,1,1,0]"));
  test_cflobdd(y);
  test_cflobdd(y.negate());

  const aterm_cflobdd& z = aterm_cflobdd(v, read_list_from_string("[0,0]"));
  test_cflobdd(z);
  test_cflobdd(z.negate());

  return 0;
}
