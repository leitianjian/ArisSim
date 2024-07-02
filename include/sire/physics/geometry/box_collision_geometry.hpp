#ifndef SIRE_BOX_COLLISION_GEOMETRY_HPP_
#define SIRE_BOX_COLLISION_GEOMETRY_HPP_

#include <atomic>
#include <string>
#include <utility>

#include <sire_lib_export.h>

#include <hpp/fcl/collision_object.h>

#include <aris/core/object.hpp>
#include <aris/dynamic/model_basic.hpp>
#include <aris/dynamic/model_coordinate.hpp>

#include "sire/core/geometry/box_geometry.hpp"
#include "sire/core/geometry/box_shape.hpp"
#include "sire/core/sire_decl_def_macro.hpp"
#include "sire/ext/json.hpp"
#include "sire/physics/geometry/collidable.hpp"
#include "sire/physics/geometry/collidable_geometry.hpp"

namespace sire::physics {
namespace geometry {
/* unique geometry id for every added collision geometry */
using json = nlohmann::json;
using namespace std;
using namespace hpp;
using GeometryId = sire::geometry::GeometryId;
class BoxCollisionGeometry : public CollidableGeometry {
 public:
  sire::geometry::BoxShape boxShape;
  auto init() -> void override;
  explicit BoxCollisionGeometry(double x = 0.1, double y = 0.1, double z = 0.1,
                                const double* prt_pm = nullptr);
  virtual ~BoxCollisionGeometry();
  SIRE_DECLARE_MOVE_CTOR(BoxCollisionGeometry)
  // ���ڲ�ʹ�õ�to_json from_json������
  SIRE_DECLARE_JSON_INTER_OVERRIDE_TWO

  // nlohammn::json j = o;��ʱ����Զ����õ�to_json from_json������
  SIRE_DECLARE_JSON_FRIEND_TWO(BoxCollisionGeometry)
};
}  // namespace geometry
}  // namespace sire::physics
#endif