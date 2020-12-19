#ifndef STAN_MATH_PRIM_ERR_IS_LDLT_FACTOR_HPP
#define STAN_MATH_PRIM_ERR_IS_LDLT_FACTOR_HPP

#include <stan/math/prim/meta.hpp>
#include <stan/math/prim/fun/Eigen.hpp>
#include <stan/math/prim/fun/LDLT_factor.hpp>

namespace stan {
namespace math {

/**
 * Return <code>true</code> if the specified LDLT factor is invalid.
 * An <code>LDLT_factor</code> is invalid if it was constructed from
 * a matrix that is not positive definite.  The check is that the
 * <code>.success()</code> method returns <code>true</code>.
 * @tparam T Type of scalar, requires class access to <code>.success()</code>
 * @tparam R number of rows or Eigen::Dynamic
 * @tparam C number of columns or Eigen::Dynamic
 * @param A The LDLT factor to check for validity
 * @return <code>true</code> if the LDLT factor is valid
 */
template <typename T, bool alloc_in_arena>
inline bool is_ldlt_factor(const LDLT_factor<T, alloc_in_arena>& A) {
  return A.ldlt().info() == Eigen::Success &&
    A.ldlt().isPositive() &&
    (A.ldlt().vectorD().array() > 0).all();
}

}  // namespace math
}  // namespace stan
#endif
