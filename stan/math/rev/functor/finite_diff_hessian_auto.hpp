#ifndef STAN_MATH_REV_FUNCTOR_HESSIAN_HPP
#define STAN_MATH_REV_FUNCTOR_HESSIAN_HPP

#include <stan/math/rev/meta.hpp>
#include <stan/math/rev/core.hpp>
#include <stan/math/rev/fun/value_of.hpp>
#include <stan/math/prim/fun/Eigen.hpp>
#include <stan/math/rev/functor.hpp>
#include <stan/math/prim/fun/finite_diff_stepsize.hpp>
#include <stdexcept>

namespace stan {
namespace math {
namespace internal {
/**
 * Calculate the value and the Hessian of the specified function at
 * the specified argument using first-order finite difference of gradients,
 * automatically setting the stepsize between the function evaluations
 * along a dimension.
 *
 * Instead of returning the full symmetric Hessian, we return the
 * lower-triangular only as a column-major compressed sparse matrix.
 *
 * <p>The functor must implement
 *
 * <code>
 * var
 * operator()(const
 * Eigen::Matrix<var, Eigen::Dynamic, 1>&)
 * </code>
 *
 * <p>For details of the algorithm, see
 * "Central difference approximation", under "Second-order derivatives based on
 * gradient", in: https://v8doc.sas.com/sashtml/ormp/chap5/sect28.htm
 *
 * <p>Step size for dimension `i` is set automatically using
 * `stan::math::finite_diff_stepsize(x(i))`.
 *
 * 2n gradient calls are needed for the algorithm.
 *
 * @tparam F Type of function
 * @param[in] f Function
 * @param[in] x Argument to function
 * @param[out] fx Function applied to argument
 * @param[out] grad_fx Gradient of function at argument
 * @param[out] hess_fx Hessian of function at argument, as a lower-triangular
 *                      compressed sparse matrix
 */
template <typename F>
void finite_diff_hessian_auto(const F& f, const Eigen::VectorXd& x, double& fx,
                              Eigen::VectorXd& grad_fx,
                              Eigen::SparseMatrix<double>& hess_fx) {
  int d = x.size();
  if (d == 0) {
    fx = value_of(f(x));
    return;
  }

  gradient(f, x, fx, grad_fx);

  Eigen::VectorXd x_temp(x);
  hess_fx.resize(d, d);
  hess_fx.reserve(Eigen::VectorXi::LinSpaced(d, 1, d).reverse());

  std::vector<Eigen::VectorXd> g_plus(d);
  std::vector<Eigen::VectorXd> g_minus(d);
  std::vector<double> epsilons(d);
  double tmp;

  // compute the gradient at x+eps_i*e_i
  // such that eps_i is the step size and e_i is the unit vector
  // in the i-th direction
  for (size_t i = 0; i < d; ++i) {
    Eigen::VectorXd x_temp(x);
    epsilons[i] = finite_diff_stepsize(x(i));
    x_temp(i) += epsilons[i];
    gradient(f, x_temp, tmp, g_plus[i]);
  }

  // similarly, compute the gradient at x-eps_i*e_i
  for (size_t i = 0; i < d; ++i) {
    Eigen::VectorXd x_temp(x);
    x_temp(i) -= epsilons[i];
    gradient(f, x_temp, tmp, g_minus[i]);
  }
  // approximate the hessian as a finite difference of gradients
  for (int i = 0; i < d; ++i) {
    for (int j = i; j < d; ++j) {
      hess_fx.insert(j, i) = (g_plus[j](i) - g_minus[j](i)) / (4 * epsilons[j])
                      + (g_plus[i](j) - g_minus[i](j)) / (4 * epsilons[i]);
    }
  }
  hess_fx.makeCompressed();
}

/**
 * Calculate the value and the Hessian of the specified function at
 * the specified argument using first-order finite difference of gradients,
 * automatically setting the stepsize between the function evaluations
 * along a dimension.
 *
 * Overload for returning the Hessian as a symmetric dense matrix.
 *
 * @tparam F Type of function
 * @param[in] f Function
 * @param[in] x Argument to function
 * @param[out] fx Function applied to argument
 * @param[out] grad_fx Gradient of function at argument
 * @param[out] hess_fx Hessian of function at argument, as a symmetric matrix
 */
template <typename F>
void finite_diff_hessian_auto(const F& f, const Eigen::VectorXd& x, double& fx,
                              Eigen::VectorXd& grad_fx,
                              Eigen::MatrixXd& hess_fx) {
  Eigen::SparseMatrix<double> hess_sparse;
  finite_diff_hessian_auto(f, x, fx, grad_fx, hess_sparse);

  hess_fx = Eigen::MatrixXd(hess_sparse).selfadjointView<Eigen::Lower>();
}

}  // namespace internal
}  // namespace math
}  // namespace stan
#endif
