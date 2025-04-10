#include "aterm_cflobdd.h"

#include "mcrl2/atermpp/aterm_io.h"

using namespace atermpp;

void test_proto_cflobdd(const aterm_proto_cflobdd& c)
{
  std::cout << "Proto-CFLOBDD: " << c << "\n";

  const size_t& level = c.level();
  std::cout << "Level: " << level << "\n";

  const size_t& out_degree = c.out_degree();
  std::cout << "Out degree: " << out_degree << "\n";

  const size_t& is_reduced = c.is_reduced();
  std::cout << "Is reduced: " << is_reduced << "\n\n";
}

void test_cflobdd(const aterm_cflobdd& c)
{
  std::cout << "CFLOBDD: " << c << "\n\n";
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

  const aterm_cflobdd& y = aterm_cflobdd(e, read_list_from_string("[0,1,1,0]"));
  test_cflobdd(y);

  return 0;
}
