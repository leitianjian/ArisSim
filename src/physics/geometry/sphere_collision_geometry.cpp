#include "sire/physics/geometry/sphere_collision_geometry.hpp"

#include <array>
#include <memory>
#include <string>
#include <string_view>

#include <hpp/fcl/BVH/BVH_model.h>
#include <hpp/fcl/shape/geometric_shapes.h>

#include <aris/core/reflection.hpp>

#include "sire/core/geometry/shape_calculator.hpp"

namespace sire::physics::geometry {
SIRE_DEFINE_TO_JSON_HEAD(SphereCollisionGeometry) {
  GeometryOnPart::to_json(j);
  sire::geometry::ShapeToName cal;
  sphereShape.Reify(&cal);
  j["shape_type"] = cal.string();
  j["radius"] = sphereShape.radius();
}

auto SphereCollisionGeometry::init() -> void {
  fcl::Transform3f trans(
      (fcl::Matrix3f() << partPm()[0][0], partPm()[0][1], partPm()[0][2],
       partPm()[1][0], partPm()[1][1], partPm()[1][2], partPm()[2][0],
       partPm()[2][1], partPm()[2][2])
          .finished(),
      (fcl::Vec3f() << partPm()[0][3], partPm()[1][3], partPm()[2][3])
          .finished());
  resetCollisionObject(new fcl::CollisionObject(
      make_shared<fcl::Sphere>(sphereShape.radius()), trans));
}
SphereCollisionGeometry::SphereCollisionGeometry(double radius, int part_id,
                                                 const double* prt_pm,
                                                 bool is_dynamic)
    : CollidableGeometry(prt_pm, part_id, is_dynamic), sphereShape(radius) {}
SphereCollisionGeometry::~SphereCollisionGeometry() = default;

// 借助类内部的from_json to_json定义，
// 使用宏定义完成用于json类型转换的from_json to_json的方法定义
SIRE_DEFINE_JSON_OUTER_TWO(SphereCollisionGeometry)

ARIS_REGISTRATION {
  auto getSphereRadius = [](SphereCollisionGeometry* geo) {
    return geo->sphereShape.radius();
  };
  auto setSphereRadius = [](SphereCollisionGeometry* geo,
                            double radius) {
    geo->sphereShape.setRadius(radius);
  };
  aris::core::class_<SphereCollisionGeometry>("SphereCollisionGeometry")
      .inherit<CollidableGeometry>()
      .prop("radius", &setSphereRadius, &getSphereRadius);
}
}  // namespace sire::physics::geometry