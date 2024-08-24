#ifndef SIRE_PHYSICS_ENGINE_HPP_
#define SIRE_PHYSICS_ENGINE_HPP_

#include <array>
#include <unordered_map>
#include <vector>

#include <sire_lib_export.h>

#include "sire/core/material_manager.hpp"
#include "sire/integrator/integrator_base.hpp"
#include "sire/physics/collision/collision_detection.hpp"
#include "sire/physics/common/point_pair_contact_info.hpp"
#include "sire/physics/contact/contact_solver.hpp"
#include "sire/physics/physics.hpp"

namespace sire {
namespace physics {
class SIRE_API PhysicsEngine {
 public:
  // Config get set method
  auto collisionDetectionFlag() const -> bool;
  auto setCollisionDetectionFlag(bool flag) -> void;
  auto contactSolverFlag() const -> bool;
  auto setContactSolverFlag(bool flag) -> void;
  // collision_detection //
  auto resetCollisionDetection(
      collision::CollisionDetection* collision_detection_in) -> void;
  auto collisionDetection() const -> const collision::CollisionDetection&;
  auto collisionDetection() -> collision::CollisionDetection& {
    return const_cast<collision::CollisionDetection&>(
        const_cast<const PhysicsEngine*>(this)->collisionDetection());
  }

  // contact_solver //
  auto resetContactSolver(contact::ContactSolver* contact_solver_in) -> void;
  auto contactSolver() const -> const contact::ContactSolver&;
  auto contactSolver() -> contact::ContactSolver& {
    return const_cast<contact::ContactSolver&>(
        const_cast<const PhysicsEngine*>(this)->contactSolver());
  }

  // collision filter
  auto resetCollisionFilter(collision::CollisionFilter* filter) -> void;
  auto collisionFilter() -> collision::CollisionFilter&;

  // geometry pool
  auto resetGeometryPool(
      aris::core::PointerArray<geometry::CollidableGeometry,
                               aris::dynamic::Geometry>* pool) -> void;
  auto geometryPool() noexcept
      -> aris::core::PointerArray<geometry::CollidableGeometry,
                                  aris::dynamic::Geometry>&;

  auto queryGeometryPoolById(const GeometryId& id) const
      -> geometry::CollidableGeometry*;
  auto queryGeometryPoolById(const GeometryId& id)
      -> geometry::CollidableGeometry* {
    return const_cast<const PhysicsEngine*>(this)->queryGeometryPoolById(id);
  };

  // objects map
  auto dynamicObjectsMap()
      -> std::unordered_map<GeometryId, geometry::CollidableGeometry*>&;
  auto anchoredObjectsMap()
      -> std::unordered_map<GeometryId, geometry::CollidableGeometry*>&;

  // continuous collision detection
  // will insert some time value which should be processed.
  auto continuousCollisionDetection() -> void{};

  // compute contact wrench of model
  auto cptModelContactWrench() -> void{};

  // compute point pair penetration and get result
  auto cptPointPairPenetration(
      std::vector<common::PenetrationAsPointPair>& pairs) -> void;

  // compute real contact time
  auto cptContactTime(const common::PenetrationAsPointPair& penetration)
      -> double;

  // compute proximity velocity
  auto cptProximityVelocity(const common::PenetrationAsPointPair& penetration)
      -> double;
  auto cptTangentialVelocity(const common::PenetrationAsPointPair& penetration,
                             const std::array<double, 16>& T_contact,
                             std::array<double, 2>& vt) -> void;
  auto cptContactVelocityB2A(const common::PenetrationAsPointPair& penetration,
                             const std::array<double, 16>& T_contact,
                             std::array<double, 3>& v_contact) -> void;

  // engine state getter
  inline auto numGeometries() -> sire::Size { return geometryPool().size(); }
  auto numDynamicGeometries() -> sire::Size;

  // engine state control
  auto doInit() -> void;
  auto init() -> void;
  auto initByModel(aris::dynamic::Model* m) -> void;

  // this prt_pm represent the pose of geometry on part coordinate
  auto addSphereGeometry(double radius, int part_id = 0,
                         const double* prt_pm = nullptr,
                         bool is_dynamic = false) -> bool;
  auto addDynamicGeometry(geometry::CollidableGeometry& dynamic_geometry)
      -> bool;
  auto addAnchoredGeometry(geometry::CollidableGeometry& anchored_geometry)
      -> bool;

  auto removeGeometry() -> bool;
  auto clearDynamicGeometries() -> bool;
  auto clearAnchoredGeometries() -> bool;
  auto clearGeometries() -> bool;

  // Functions for the main usage
  auto updateGeometryLocationFromModel() -> void;
  auto hasCollision() -> bool;
  auto computePointPairPenetration()
      -> std::vector<common::PenetrationAsPointPair>;

  auto cptContactInfo(
      const std::vector<common::PenetrationAsPointPair>& penetration_pairs,
      std::vector<common::PointPairContactInfo>& contact_info) -> bool;
  auto cptGlbForceByContactInfo(
      const std::vector<common::PointPairContactInfo>& contact_info) -> bool;

  auto handleContact() -> void;
  // ��ÿ���˼��䱸һ��GeneralForce��Componenet���������ýӴ���
  auto initPartContactForce2Model() -> void;
  auto resetPartContactForce() -> void;
  auto setForcePoolSimulation() -> void;

  auto saveInitialModel(aris::dynamic::Model& model) -> void;
  auto resetInitialModel() -> void;

  PhysicsEngine();
  virtual ~PhysicsEngine();
  PhysicsEngine(const PhysicsEngine&) = delete;
  PhysicsEngine& operator=(const PhysicsEngine&) = delete;

 private:
  struct Imp;
  aris::core::ImpPtr<Imp> imp_;
};
}  // namespace physics
}  // namespace sire
#endif