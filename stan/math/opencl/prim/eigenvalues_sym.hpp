#ifndef STAN_MATH_OPENCL_PRIM_EIGENVALUES_SYM_HPP
#define STAN_MATH_OPENCL_PRIM_EIGENVALUES_SYM_HPP
#ifdef STAN_OPENCL

#include <stan/math/opencl/err/check_symmetric.hpp>
#include <stan/math/opencl/symmetrc_eigensolver_cl.hpp>

namespace stan {
namespace math {

matrix_cl<double>
eigenvalues_sym(const matrix_cl<double>& m) {
  check_nonzero_size("eigenvalues_sym", "m", m);
  check_symmetric("eigenvalues_sym", "m", m);

  matrix_cl<double> eigenvalues, eigenvectors;
  symmetric_eigensolver<true>(m,eigenvalues, eigenvectors);
  return eigenvalues;
}

}  // namespace math
}  // namespace stan
#endif
#endif
