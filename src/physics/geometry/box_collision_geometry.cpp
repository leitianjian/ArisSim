#include "sire/physics/geometry/box_collision_geometry.hpp"

#include <array>
#include <memory>
#include <string>
#include <string_view>

#include <hpp/fcl/BVH/BVH_model.h>
#include <hpp/fcl/shape/geometric_shapes.h>

#include <aris/core/reflection.hpp>

#include "sire/core/geometry/shape_calculator.hpp"

namespace sire::physics::geometry {
SIRE_DEFINE_TO_JSON_HEAD(BoxCollisionGeometry) {
  GeometryOnPart::to_json(j);
  sire::geometry::ShapeToName cal;
  boxShape.Reify(&cal);
  j["shape_type"] = cal.string();
  j["length"] = boxShape.length();
  j["width"] = boxShape.width();
  j["height"] = boxShape.height();
}

auto BoxCollisionGeometry::init() -> void {
  fcl::Transform3f trans(
      (fcl::Matrix3f() << partPm()[0][0], partPm()[0][1], partPm()[0][2],
       partPm()[1][0], partPm()[1][1], partPm()[1][2], partPm()[2][0],
       partPm()[2][1], partPm()[2][2])
          .finished(),
      (fcl::Vec3f() << partPm()[0][3], partPm()[1][3], partPm()[2][3])
          .finished());
  // std::array<double, 3> temp = side();
  resetCollisionObject(new fcl::CollisionObject(
      make_shared<fcl::Box>(boxShape.side()[0], boxShape.side()[1],
                            boxShape.side()[2]),
      trans));
}
BoxCollisionGeometry::BoxCollisionGeometry(double x, double y, double z,
                                           const double* prt_pm)
    : CollidableGeometry(prt_pm), boxShape(x, y, z) {}
BoxCollisionGeometry::~BoxCollisionGeometry() = default;
SIRE_DEFINE_MOVE_CTOR_CPP(BoxCollisionGeometry)

SIRE_DEFINE_JSON_OUTER_TWO(BoxCollisionGeometry)

ARIS_REGISTRATION {
  auto setSide = [](BoxCollisionGeometry* box, aris::core::Matrix mat) -> void {
    box->boxShape.setSide(mat.data());
  };
  auto getSide = [](BoxCollisionGeometry* box) -> aris::core::Matrix {
    return aris::core::Matrix(1, 3, box->boxShape.side());
  };
  aris::core::class_<BoxCollisionGeometry>("BoxCollisionGeometry")
      .inherit<CollidableGeometry>()
      .prop("side", &setSide, &getSide);
}
}  // namespace sire::physics::geometry