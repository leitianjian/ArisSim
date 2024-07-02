#ifndef SIRE_SPHERE_GEOMETRY_HPP_
#define SIRE_SPHERE_GEOMETRY_HPP_

#include <atomic>
#include <string>
#include <utility>

#include <sire_lib_export.h>

#include <aris/core/object.hpp>
#include <aris/dynamic/model_basic.hpp>
#include <aris/dynamic/model_coordinate.hpp>

#include "sire/core/geometry/geometry_on_part.hpp"
#include "sire/core/geometry/sphere_shape.hpp"
#include "sire/core/sire_decl_def_macro.hpp"
#include "sire/ext/json.hpp"

namespace sire::geometry {
using namespace std;
using json = nlohmann::json;
class SphereGeometry : public GeometryOnPart {
 public:
  SphereShape sphereShape;
  explicit SphereGeometry(double radius = 0.1, const double* prt_pm = nullptr);
  virtual ~SphereGeometry();
  ARIS_DECLARE_BIG_FOUR(SphereGeometry)

  // ���ڲ�ʹ�õ�to_json from_json������
  SIRE_DECLARE_JSON_INTER_OVERRIDE_TWO

  // nlohammn::json j = o;��ʱ����Զ����õ�to_json from_json������
  SIRE_DECLARE_JSON_FRIEND_TWO(SphereGeometry)
};
}  // namespace sire::geometry
#endif