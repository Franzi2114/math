#include <stan/mcmc/chains.hpp>
#include <gtest/gtest.h>
#include <boost/random/additive_combine.hpp>
#include <set>
#include <exception>
#include <utility>

TEST(McmcChains, validate_dim_idxs) {
  using stan::mcmc::validate_dims_idxs;
  std::vector<size_t> dims(3,2);
  std::vector<size_t> idxs(3,0);
  EXPECT_NO_THROW(validate_dims_idxs(dims,idxs));
  idxs[0] = 4;
  EXPECT_THROW(validate_dims_idxs(dims,idxs), std::out_of_range);
  idxs[0] = 0;
  idxs[2] = 5;
  EXPECT_THROW(validate_dims_idxs(dims,idxs), std::out_of_range);
  
  std::vector<size_t> idxs4(4,0);
  EXPECT_THROW(validate_dims_idxs(dims,idxs4), std::invalid_argument);
  
}

void test_permutation(size_t N) {
  int seed = 187049587;
  boost::random::ecuyer1988 rng(seed);
  std::vector<size_t> pi;
  stan::mcmc::permutation(pi,N,rng);

  EXPECT_EQ(N,pi.size());
  for (size_t i = 0; i < N; ++i)
    EXPECT_TRUE(pi[i] < N);
  size_t match_count = 0;
  for (size_t i = 0; i < N; ++i)
    for (size_t j = 0; j < N; ++j)
      if (pi[j] == i) ++match_count;
  EXPECT_EQ(N,match_count);
  
}

TEST(McmcChains,permutation) {
  test_permutation(0);
  test_permutation(1);
  test_permutation(2);
  test_permutation(3);
  test_permutation(15);
  test_permutation(1024);
  test_permutation(1023);
  test_permutation(1025);
}


void test_permute(size_t N) {
  int seed = 187049587;
  boost::random::ecuyer1988 rng(seed);
  std::vector<size_t> pi;
  stan::mcmc::permutation(pi,N,rng);

  std::vector<double> x(N);
  for (size_t n = 0; n < N; ++n)
    x[n] = 1.0 + n / 2.0;
  std::vector<double> x_pi;
  
  stan::mcmc::permute(pi,x,x_pi);
  EXPECT_EQ(N,x_pi.size());
  if (N < 1) return;

  double sum = 0.0;
  double sum_pi = 0.0;
  for (size_t n = 0; n < N; ++n) {
    sum += x[n];
    sum_pi += x_pi[n];
  }
  EXPECT_GT(sum,0.0);
  EXPECT_EQ(sum,sum_pi);
}

TEST(McmcChains, permute) {
  test_permute(0);
  test_permute(1);  
  test_permute(2);  
  test_permute(3);  
  test_permute(4);  
  test_permute(5);  
  test_permute(2055);
  test_permute(2056);
  test_permute(2057);
}


TEST(McmcChains,get_offset) {
  using stan::mcmc::get_offset;
  using std::vector;
  vector<size_t> idxs(3);
  idxs[0] = 0;
  idxs[1] = 0;
  idxs[2] = 0;
  vector<size_t> dims(3);
  dims[0] = 2;
  dims[1] = 3;
  dims[2] = 4;
  size_t offset = 0;
  for (size_t c = 0; c < 4; ++c) {
    for (size_t b = 0; b < 3; ++b) {
      for (size_t a = 0; a < 2; ++a) {
        idxs[0] = a;
        idxs[1] = b;
        idxs[2] = c;
        EXPECT_EQ(offset,get_offset(dims,idxs));
        ++offset;
      }
    }
  }
}


TEST(McmcChains,increment_indexes) {
  using std::vector;
  using stan::mcmc::increment_indexes;
  vector<size_t> idxs(3);
  idxs[0] = 0;
  idxs[1] = 0;
  idxs[2] = 0;
  vector<size_t> dims(3);
  dims[0] = 2;
  dims[1] = 3;
  dims[2] = 4;
  for (size_t c = 0; c < 4; ++c) {
    for (size_t b = 0; b < 3; ++b) {
      for (size_t a = 0; a < 2; ++a) {
        EXPECT_FLOAT_EQ(a,idxs[0]);
        EXPECT_FLOAT_EQ(b,idxs[1]);
        EXPECT_FLOAT_EQ(c,idxs[2]);
        if (a != 1 || b != 2 || c != 3)
          increment_indexes(dims,idxs);
      }
    }
  }

  increment_indexes(dims,idxs);
  EXPECT_FLOAT_EQ(0.0,idxs[0]);
  EXPECT_FLOAT_EQ(0.0,idxs[1]);
  EXPECT_FLOAT_EQ(0.0,idxs[2]);
  
  vector<size_t> dims4(4,5);
  vector<size_t> idxs4(4,0); 
  EXPECT_THROW(increment_indexes(dims4,idxs),
               std::invalid_argument);
  EXPECT_THROW(increment_indexes(dims,idxs4),
               std::invalid_argument);
  EXPECT_NO_THROW(increment_indexes(dims4,idxs4));

  idxs4[3] = 12; // now out of range
  EXPECT_THROW(increment_indexes(dims4,idxs4),
               std::out_of_range);
}

TEST(McmcChains,ctor_and_immutable_getters) {
  using std::vector;
  using std::string;
  using stan::mcmc::chains;

  size_t K = 4;

  // b(), a(2), d(3,4,5), c(6,7)

  vector<string> names;
  names.push_back("b");
  names.push_back("a");
  names.push_back("d");
  names.push_back("c");

  vector<size_t> b_dims;

  vector<size_t> a_dims;
  a_dims.push_back(2);

  vector<size_t> d_dims;
  d_dims.push_back(3);
  d_dims.push_back(4);
  d_dims.push_back(5);

  vector<size_t> c_dims;
  c_dims.push_back(6);
  c_dims.push_back(7);

  vector<vector<size_t> > dimss;
  dimss.push_back(b_dims);
  dimss.push_back(a_dims);
  dimss.push_back(d_dims);
  dimss.push_back(c_dims);

  chains<> c(K,names,dimss);

  EXPECT_EQ(4U, c.num_chains());

  EXPECT_EQ(size_t(1 + 2 + 3*4*5 + 6*7), c.num_params());

  EXPECT_EQ(4U, c.num_param_names());

  EXPECT_EQ(4U, c.param_names().size());
  EXPECT_EQ("b", c.param_names()[0]);
  EXPECT_EQ("a", c.param_names()[1]);
  EXPECT_EQ("d", c.param_names()[2]);
  EXPECT_EQ("c", c.param_names()[3]);
  EXPECT_EQ("b", c.param_name(0));
  EXPECT_EQ("a", c.param_name(1));
  EXPECT_EQ("d", c.param_name(2));
  EXPECT_EQ("c", c.param_name(3));
  EXPECT_THROW(c.param_name(5), std::out_of_range);

  EXPECT_EQ(0U, c.param_start(0));
  EXPECT_EQ(1U, c.param_start(1));
  EXPECT_EQ(3U, c.param_start(2));
  EXPECT_EQ(63U, c.param_start(3));
  EXPECT_EQ(0U, c.param_starts()[0]);
  EXPECT_EQ(1U, c.param_starts()[1]);
  EXPECT_EQ(3U, c.param_starts()[2]);
  EXPECT_EQ(63U, c.param_starts()[3]);
  EXPECT_THROW(c.param_start(5), std::out_of_range);

  EXPECT_EQ(1U, c.param_size(0));
  EXPECT_EQ(1U, c.param_sizes()[0]);
  EXPECT_EQ(2U, c.param_size(1));
  EXPECT_EQ(2U, c.param_sizes()[1]);
  EXPECT_EQ(60U, c.param_size(2));
  EXPECT_EQ(60U, c.param_sizes()[2]);
  EXPECT_EQ(42U, c.param_size(3));
  EXPECT_EQ(42U, c.param_sizes()[3]);
  EXPECT_EQ(4U, c.param_sizes().size());
  EXPECT_THROW(c.param_size(5), std::out_of_range);

  EXPECT_EQ(0U, c.param_name_to_index("b"));
  EXPECT_EQ(1U, c.param_name_to_index("a"));
  EXPECT_EQ(2U, c.param_name_to_index("d"));
  EXPECT_EQ(3U, c.param_name_to_index("c"));

  EXPECT_EQ(4U, c.param_dimss().size());

  EXPECT_EQ(0U, c.param_dimss()[0].size());
  EXPECT_EQ(0U, c.param_dims(0).size());
  EXPECT_EQ(1U, c.param_dimss()[1].size());
  EXPECT_EQ(1U, c.param_dims(1).size());
  EXPECT_EQ(3U, c.param_dimss()[2].size());
  EXPECT_EQ(3U, c.param_dims(2).size());
  EXPECT_EQ(2U, c.param_dimss()[3].size());
  EXPECT_EQ(2U, c.param_dims(3).size());
  EXPECT_THROW(c.param_dims(5), std::out_of_range);

  EXPECT_EQ(2U, c.param_dimss()[1][0]);
  EXPECT_EQ(2U, c.param_dims(1)[0]);

  EXPECT_EQ(4U, c.param_dimss()[2][1]);
  EXPECT_EQ(4U, c.param_dims(2)[1]);

  EXPECT_THROW(c.param_dims(5), std::out_of_range);

  size_t pos = 0;
  std::vector<size_t> idxs(0);
  EXPECT_EQ(pos, c.get_total_param_index(0,idxs));
  ++pos;

  idxs.resize(1);
  idxs[0] = 0;
  EXPECT_EQ(pos, c.get_total_param_index(1,idxs));
  ++pos;
  idxs[0] = 1;
  EXPECT_EQ(pos, c.get_total_param_index(1,idxs));
  ++pos;

  idxs.resize(3);
  for (size_t i2 = 0; i2 < 5; ++i2) {
    idxs[2] = i2;
    for (size_t i1 = 0; i1 < 4; ++i1) {
      idxs[1] = i1;
      for (size_t i0 = 0; i0 < 3; ++i0) {
        idxs[0] = i0;
        EXPECT_EQ(pos, c.get_total_param_index(2,idxs));
        ++pos;
      }
    }
  }

  idxs.resize(2);
  for (size_t i1 = 0; i1 < 7; ++i1) {
    idxs[1] = i1;
    for (size_t i0 = 0; i0 < 6; ++i0) {
      idxs[0] = i0;
      EXPECT_EQ(pos, c.get_total_param_index(3,idxs));
      ++pos;
    }
  }
  
  EXPECT_THROW(c.get_total_param_index(5,idxs), std::out_of_range);

}

TEST(McmcChains,warmup_get_set) {
  using std::vector;
  using std::string;
  using stan::mcmc::chains;

  chains<> c(2,
             vector<string>(1,"a"), 
             vector<vector<size_t> >(1,vector<size_t>(0)));

  EXPECT_EQ(0U, c.warmup());
  c.set_warmup(1000U);
  EXPECT_EQ(1000U,c.warmup());
}




TEST(McmcChains,add) {
  using std::vector;
  using std::string;
  using stan::mcmc::chains;

  size_t K = 4; // num chains

  vector<string> names;
  names.push_back("b");
  names.push_back("a");
  names.push_back("c");

  vector<size_t> b_dims;

  vector<size_t> a_dims;
  a_dims.push_back(2);
  a_dims.push_back(3);

  vector<size_t> c_dims;
  c_dims.push_back(4);

  vector<vector<size_t> > dimss;
  dimss.push_back(b_dims);
  dimss.push_back(a_dims);
  dimss.push_back(c_dims);
  chains<> c(K,names,dimss);

  size_t N = 1 + 2*3 + 4;

  vector<double> theta(N);
  for (size_t n = 0; n < N; ++n)
    theta[n] = n;
  

  EXPECT_EQ(0U,c.num_samples());
  EXPECT_EQ(0U,c.num_samples(0));
  EXPECT_EQ(0U,c.num_samples(1));
  EXPECT_EQ(0U,c.num_samples(2));
  EXPECT_EQ(0U,c.num_samples(3));

  c.add(0,theta);
  
  EXPECT_EQ(1U,c.num_samples());
  EXPECT_EQ(1U,c.num_samples(0));
  EXPECT_EQ(0U,c.num_samples(1));

  for (size_t n = 0; n < N; ++n)
    theta[n] *= 2.0;

  c.add(0,theta);

  EXPECT_EQ(2U,c.num_samples());
  EXPECT_EQ(2U,c.num_samples(0));
  EXPECT_EQ(0U,c.num_samples(1));

  c.add(1,theta);

  EXPECT_EQ(3U,c.num_samples());
  EXPECT_EQ(2U,c.num_samples(0));
  EXPECT_EQ(1U,c.num_samples(1));

  EXPECT_THROW(c.num_samples(5), std::out_of_range);

  c.add(0,theta);
  c.add(1,theta);
  c.add(1,theta);
  c.add(1,theta);
  c.add(2,theta);

  // 0=3, 1=4, 2=1, 3=0

  c.set_warmup(1);
  EXPECT_EQ(3U, c.num_warmup_samples());
  EXPECT_EQ(1U, c.num_warmup_samples(0));
  EXPECT_EQ(1U, c.num_warmup_samples(1));
  EXPECT_EQ(1U, c.num_warmup_samples(2));
  EXPECT_EQ(0U, c.num_warmup_samples(3));
  EXPECT_THROW(c.num_warmup_samples(5), std::out_of_range);

  EXPECT_EQ(5U, c.num_kept_samples());
  EXPECT_EQ(2U, c.num_kept_samples(0));
  EXPECT_EQ(3U, c.num_kept_samples(1));
  EXPECT_EQ(0U, c.num_kept_samples(2));
  EXPECT_EQ(0U, c.num_kept_samples(3));
  EXPECT_THROW(c.num_kept_samples(5), std::out_of_range);

  c.set_warmup(0);
  EXPECT_EQ(0U, c.num_warmup_samples());
  EXPECT_EQ(0U, c.num_warmup_samples(0));
  EXPECT_EQ(0U, c.num_warmup_samples(1));
  EXPECT_EQ(0U, c.num_warmup_samples(2));
  EXPECT_EQ(0U, c.num_warmup_samples(3));

  EXPECT_EQ(8U, c.num_kept_samples());
  EXPECT_EQ(3U, c.num_kept_samples(0));
  EXPECT_EQ(4U, c.num_kept_samples(1));
  EXPECT_EQ(1U, c.num_kept_samples(2));
  EXPECT_EQ(0U, c.num_kept_samples(3));
}

TEST(McmcChains,get_samples) {
  using std::vector;
  using std::string;
  using stan::mcmc::chains;

  size_t K = 3; // num chains

  vector<string> names; // dims
  names.push_back("b"); // ()
  names.push_back("a"); // (2,3)
  names.push_back("c"); // (4)

  vector<size_t> b_dims;

  vector<size_t> a_dims;
  a_dims.push_back(2);
  a_dims.push_back(3);

  vector<size_t> c_dims;
  c_dims.push_back(4);

  vector<vector<size_t> > dimss;
  dimss.push_back(b_dims);
  dimss.push_back(a_dims);
  dimss.push_back(c_dims);

  chains<> c(K,names,dimss);

  size_t N = 1 + 2*3 + 4;
  vector<double> theta(N);
  for (size_t n = 0; n < N; ++n)
    theta[n] = n;

  c.add(0,theta); // (1,0,0)
  c.add(1,theta); // (1,1,0)
  c.add(2,theta); // (1,1,1)

  for (size_t n = 0; n < N; ++n)
    theta[n] = n * n;
  
  c.add(0,theta); // (2,1,1)
  c.add(1,theta); // (2,2,1)
  c.add(2,theta); // (2,2,2)

  vector<double> rho;
  c.get_samples(0,rho);
  EXPECT_EQ(6U, rho.size());
  EXPECT_THROW(c.get_samples(112,rho), std::out_of_range);

  c.add(1,theta); // (2,3,2)
  c.add(2,theta); // (2,3,3)
  c.add(2,theta); // (2,3,4)
  c.get_samples(0,rho);
  EXPECT_EQ(9U, rho.size());

  for (size_t warmup = 0; warmup < 10; warmup += 2) {
    c.get_samples(0,0,rho);
    EXPECT_EQ(2U,rho.size());
    c.get_samples(1,0,rho);
    EXPECT_EQ(3U,rho.size());
    c.get_samples(2,0,rho);
    EXPECT_EQ(4U,rho.size());
    EXPECT_THROW(c.get_samples(112,0,rho), std::out_of_range);
    EXPECT_THROW(c.get_samples(0,59,rho), std::out_of_range);

    c.get_samples(0,0,rho);
    EXPECT_EQ(2U,rho.size());
    c.get_samples(1,0,rho);
    EXPECT_EQ(3U,rho.size());
    c.get_samples(2,0,rho);
    EXPECT_EQ(4U,rho.size());
    EXPECT_THROW(c.get_samples(112,0,rho), std::out_of_range);
    EXPECT_THROW(c.get_samples(0,59,rho), std::out_of_range);
  }

  c.set_warmup(0);
  for (size_t k = 0; k < K; ++k) {
    for (size_t n = 0; n < N; ++n) { 
      rho.resize(10);
      c.get_warmup_samples(k,n,rho);
      EXPECT_EQ(0U,rho.size());
    }
  }
  EXPECT_THROW(c.get_warmup_samples(40,0,rho), std::out_of_range);
  EXPECT_THROW(c.get_warmup_samples(0,100,rho), std::out_of_range);

  rho.resize(10);
  c.get_warmup_samples(0,rho);
  EXPECT_EQ(0U,rho.size());
  EXPECT_THROW(c.get_warmup_samples(100,rho), std::out_of_range);

  c.set_warmup(1);
  rho.resize(20);
  c.get_warmup_samples(0,rho);
  EXPECT_EQ(3U, rho.size());

  c.set_warmup(2);
  rho.resize(20);
  c.get_warmup_samples(0,rho);
  EXPECT_EQ(6U, rho.size());

  c.set_warmup(100);
  rho.resize(20);
  c.get_warmup_samples(0,rho);
  EXPECT_EQ(9U, rho.size());

  c.set_warmup(3);
  rho.resize(20);
  c.get_warmup_samples(0,0,rho);
  EXPECT_EQ(2U, rho.size());
  c.get_warmup_samples(1,0,rho);
  EXPECT_EQ(3U, rho.size());
  c.get_warmup_samples(2,0,rho);
  EXPECT_EQ(3U, rho.size());
  
  
  c.set_warmup(20);
  for (size_t n = 0; n < N; ++n) {
    for (size_t k = 0; k < K; ++k) {
      rho.resize(27);
      c.get_kept_samples_permuted(n,rho);
      EXPECT_EQ(0U,rho.size());
    }
  }
  EXPECT_THROW(c.get_kept_samples_permuted(92,rho), std::out_of_range);

  c.set_warmup(2);
  rho.resize(15);
  c.get_kept_samples(0,5,rho);
  EXPECT_EQ(0U, rho.size());
  c.get_kept_samples(1,5,rho);
  EXPECT_EQ(1U, rho.size());
  c.get_kept_samples(2,5,rho);
  EXPECT_EQ(2U, rho.size());
  EXPECT_THROW(c.get_kept_samples(27,0,rho), std::out_of_range);
  EXPECT_THROW(c.get_kept_samples(0,1012,rho), std::out_of_range);
}

TEST(McmcChains, get_kept_samples_permuted) {
  using std::vector;
  using std::string;
  using stan::mcmc::chains;

  size_t K = 3; // num chains

  vector<string> names; // dims
  names.push_back("b"); // ()
  names.push_back("a"); // ()

  vector<vector<size_t> > dimss(2,vector<size_t>(0));

  chains<> c(K,names,dimss);

  std::set<double> expected;
  std::set<double> found;

  for (size_t k = 0; k < K; ++k) {
    for (size_t n = 0; n < 20U + k; ++n) {
      double val = (k + 1) * 100 + n; // all distinct
      c.add(k,vector<double>(2,val));
      if (n >= 10U)
        expected.insert(val);
    }
  }
  c.set_warmup(10U);

  vector<double> samples0;
  vector<double> samples1;

  EXPECT_EQ(33U, c.num_kept_samples()); // 3 * 10 + (0 + 1 + 2)
  
  c.get_kept_samples_permuted(0,samples0);
  c.get_kept_samples_permuted(1,samples1);

  
  EXPECT_EQ(samples0.size(), samples1.size());
  for (size_t m = 0; m < samples0.size(); ++m)
    EXPECT_FLOAT_EQ(samples0[m], samples1[m]);

  for (size_t m = 0; m < samples0.size(); ++m)
    found.insert(samples0[m]);
  EXPECT_EQ(expected,found);
}

TEST(McmcChains, quantiles_means) {
  using std::vector;
  using std::string;
  using stan::mcmc::chains;
  unsigned int K = 2;
  chains<> c(K,
             vector<std::string>(1,"a"),
             vector<vector<size_t> >(1, vector<size_t>(0)));
  for (size_t k = 0; k < K; ++k)
    for (size_t i = 0; i < 100; ++i)
      c.add(k,std::vector<double>(1,100000)); 
  c.set_warmup(100); // discard above, keep below
  for (size_t k = 0; k < K; ++k)
    for (size_t i = 0; i <= 1000; ++i)
      c.add(k,std::vector<double>(1,i/1000.0));

  // test low, middle and high branches

  // single quantile, single chain
  EXPECT_FLOAT_EQ(0.1, c.quantile(0,0,0.1));
  EXPECT_FLOAT_EQ(0.5, c.quantile(0,0,0.5));
  EXPECT_FLOAT_EQ(0.9, c.quantile(0,0,0.9));

  EXPECT_FLOAT_EQ(0.1, c.quantile(1,0,0.1));
  EXPECT_FLOAT_EQ(0.5, c.quantile(1,0,0.5));
  EXPECT_FLOAT_EQ(0.9, c.quantile(1,0,0.9));

  EXPECT_THROW(c.quantile(2,0,0.9), std::out_of_range);
  EXPECT_THROW(c.quantile(0,2,0.9), std::out_of_range);


  // single quantile, cross chain
  EXPECT_FLOAT_EQ(0.1, c.quantile(0,0.1));
  EXPECT_FLOAT_EQ(0.5, c.quantile(0,0.5));
  EXPECT_FLOAT_EQ(0.9, c.quantile(0,0.9));
  
  EXPECT_THROW(c.quantile(2,0.9), std::out_of_range);

  // multi quantiles, single chain
  vector<double> qs;
  vector<double> probs(5);
  probs[0] = 0.025;  
  probs[1] = 0.25; 
  probs[2] = 0.5;
  probs[3] = 0.75;
  probs[4] = 0.975;
  c.quantiles(0,0, probs, qs);

  EXPECT_EQ(5U,qs.size());
  EXPECT_FLOAT_EQ(0.025,qs[0]);
  EXPECT_FLOAT_EQ(0.25,qs[1]);
  EXPECT_FLOAT_EQ(0.5,qs[2]);
  EXPECT_FLOAT_EQ(0.75,qs[3]);
  EXPECT_FLOAT_EQ(0.975,qs[4]);
  
  
  EXPECT_THROW(c.quantiles(5,0,probs,qs), std::out_of_range);
  EXPECT_THROW(c.quantiles(0,10,probs,qs), std::out_of_range);


  // multi quantiles, cross chains
  c.quantiles(0, probs, qs);

  EXPECT_EQ(5U,qs.size());
  EXPECT_FLOAT_EQ(0.025,qs[0]);
  EXPECT_FLOAT_EQ(0.25,qs[1]);
  EXPECT_FLOAT_EQ(0.5,qs[2]);
  EXPECT_FLOAT_EQ(0.75,qs[3]);
  EXPECT_FLOAT_EQ(0.975,qs[4]);
  
  
  EXPECT_THROW(c.quantiles(5,probs,qs), std::out_of_range);

  // bad prob test within and across
  probs[1] = 1.2;
  EXPECT_THROW(c.quantiles(0,probs,qs), std::invalid_argument);
  EXPECT_THROW(c.quantiles(0,0,probs,qs), std::invalid_argument);

  // central interval, single chain
  EXPECT_FLOAT_EQ(0.10,c.central_interval(0,0,0.8).first);
  EXPECT_FLOAT_EQ(0.90,c.central_interval(0,0,0.8).second);

  EXPECT_THROW(c.central_interval(2,0,0.8), std::out_of_range);
  EXPECT_THROW(c.central_interval(0,3,0.8), std::out_of_range);
  EXPECT_THROW(c.central_interval(0,0,1.2), std::invalid_argument);

  // central interval, cross chains
  EXPECT_FLOAT_EQ(0.10,c.central_interval(0,0,0.8).first);
  EXPECT_FLOAT_EQ(0.90,c.central_interval(0,0,0.8).second);

  EXPECT_THROW(c.central_interval(2,0.8), std::out_of_range);
  EXPECT_THROW(c.central_interval(0,1.2), std::invalid_argument);

  std::vector<double> samps;
  c.get_kept_samples(0,0,samps);
  EXPECT_FLOAT_EQ(stan::math::mean(samps),
                  c.mean(0,0));
  EXPECT_FLOAT_EQ(stan::math::sd(samps),
                  c.sd(0,0));

  c.get_kept_samples_permuted(0,samps);
  EXPECT_FLOAT_EQ(stan::math::mean(samps),
                  c.mean(0));
  EXPECT_FLOAT_EQ(stan::math::sd(samps),
                  c.sd(0));

  
  
}
TEST(McmcChains,read_variables) {
  //"src/test/mcmc/test_csv_files/blocker1.csv"
  //"src/test/mcmc/test_csv_files/blocker1.csv"
  std::vector<std::string> expected_names;
  std::vector<std::vector<size_t> > expected_dimss;
  expected_names.push_back("lp__");
  expected_names.push_back("treedepth__");
  expected_names.push_back("d");
  expected_names.push_back("sigmasq_delta");
  expected_names.push_back("mu");
  expected_names.push_back("delta");
  expected_names.push_back("sigma_delta");

  std::vector<size_t> dims;
  dims.push_back(1);
  expected_dimss.push_back(dims);
  expected_dimss.push_back(dims);
  expected_dimss.push_back(dims);
  expected_dimss.push_back(dims);
  dims.clear();
  dims.push_back(22);
  expected_dimss.push_back(dims);
  dims.clear();
  dims.push_back(22);
  expected_dimss.push_back(dims);
  dims.clear();
  dims.push_back(1);
  expected_dimss.push_back(dims);

  std::pair<std::vector<std::string>,
    std::vector<std::vector<size_t> > > variables;
  variables = stan::mcmc::read_variables("src/test/mcmc/test_csv_files/blocker1.csv");  

  // check names
  ASSERT_EQ(expected_names.size(), variables.first.size());
  for (size_t i = 0; i < expected_names.size(); i++) {
    EXPECT_EQ(expected_names[i], variables.first[i]);
  }
  // check dims
  ASSERT_EQ(expected_dimss.size(), variables.second.size());
  for (size_t i = 0; i < expected_dimss.size(); i++) {
    for (size_t j = 0; j < expected_dimss[i].size(); j++) {
      EXPECT_EQ(expected_dimss[i][j], variables.second[i][j]);
    }
  }
}

