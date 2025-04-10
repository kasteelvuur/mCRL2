#include "aterm_cflobdd.h"

#include "mcrl2/atermpp/aterm_io.h"

using namespace atermpp;

void test_proto_cflobdd(const aterm_proto_cflobdd& c)
{
  std::cout << "Proto-CFLOBDD: " << c << "\n";

  const size_t& level = c.level();
  std::cout << "Level: " << level << "\n";

  const size_t& out_degree = c.out_degree();
  std::cout << "Out degree: " << out_degree << "\n\n";
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

  const aterm_cflobdd& x = aterm_cflobdd(d, read_list_from_string("[1]"));
  test_cflobdd(x);

  const aterm_cflobdd& y = aterm_cflobdd(e, read_list_from_string("[0,1,1,0]"));
  test_cflobdd(y);

  return 0;
}
