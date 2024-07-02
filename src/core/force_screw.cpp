#include "sire/core/force_screw.hpp"

#include <aris/dynamic/pose.hpp>
#include <aris/dynamic/screw.hpp>

#include "sire/core/constants.hpp"

namespace sire::core::screw {
using namespace aris::dynamic;
auto default_f() noexcept -> const double* {
  static const double value[3]{0, 0, 0};
  return value;
}
auto default_pe() noexcept -> const double* {
  static const double value[6]{0, 0, 0, 0, 0, 0};
  return value;
}
auto default_fs() noexcept -> const double* {
  static const double value[6]{0, 0, 0, 0, 0, 0};
  return value;
}
// auto default_pm() noexcept -> const double* {
//   static const double value[16]{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0,
//   1}; return value;
// }
auto default_out() noexcept -> double* {
  static thread_local double value[36]{0};
  return value;
}
auto s_fpm2fs(const double* f, const double* pm,
                       double* fs_out) -> void {
  f = f ? f : default_f();
  pm = pm ? pm : sire::default_pm;
  fs_out = fs_out ? fs_out : default_out();

  s_pm_dot_v3(pm, f, fs_out);
  s_c3(pm + 3, 4, fs_out, 1, fs_out + 3, 1);
}
}  // namespace sire::core::screw