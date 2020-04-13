#ifndef STAN_MATH_REV_FUNCTOR_INTEGRATE_ODE_ADAMS_HPP
#define STAN_MATH_REV_FUNCTOR_INTEGRATE_ODE_ADAMS_HPP

#include <stan/math/rev/meta.hpp>
#include <stan/math/rev/functor/cvodes_integrator.hpp>
#include <ostream>
#include <vector>

namespace stan {
namespace math {

template <typename F, typename T_initial, typename T_param, typename T_t0,
          typename T_ts>
std::vector<std::vector<return_type_t<T_initial, T_param, T_t0, T_ts>>>
integrate_ode_adams(const F& f, const std::vector<T_initial>& y0,
                    const T_t0& t0, const std::vector<T_ts>& ts,
                    const std::vector<T_param>& theta,
                    const std::vector<double>& x, const std::vector<int>& x_int,
                    std::ostream* msgs = nullptr,
                    double relative_tolerance = 1e-10,
                    double absolute_tolerance = 1e-10,
                    long int max_num_steps = 1e8) {  // NOLINT(runtime/int)
  stan::math::cvodes_integrator<CV_ADAMS, F, T_initial, T_t0, T_ts,
                                std::vector<T_param>, std::vector<double>,
                                std::vector<int>>
      integrator(f, y0, t0, ts, theta, x, x_int, msgs, relative_tolerance,
                 absolute_tolerance, max_num_steps);
  return integrator.integrate();
}

}  // namespace math
}  // namespace stan
#endif
