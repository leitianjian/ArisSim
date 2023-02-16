#ifndef SIRE_CAM_BACKEND_HPP_
#define SIRE_CAM_BACKEND_HPP_

#include <map>
#include <string>

#include <hpp/fcl/broadphase/broadphase_callbacks.h>
#include <hpp/fcl/broadphase/broadphase_collision_manager.h>
#include <hpp/fcl/broadphase/default_broadphase_callbacks.h>
#include <hpp/fcl/collision.h>
#include <hpp/fcl/collision_data.h>
#include <hpp/fcl/collision_object.h>

#include <aris/core/expression_calculator.hpp>

#include "sire/collision/collided_objects_callback.hpp"
#include "sire/collision/collision_engine.hpp"
#include "sire/collision/geometry/collision_geometry.hpp"
#include "sire/core/constants.hpp"
#include "sire/core/module_base.hpp"

namespace sire::cam_backend {
using namespace std;
using namespace hpp;

// ���߹�����װ��ʽ
// EX_WOBJ_HAND_TOOL: ������װ��ʽ��ĩ�˹��ߣ��̶�����
// HAND_WOBJ_EX_TOOL: ĩ�ˣ��ֳ֣��������̶�����
enum class WobjToolInstallMethod {
  EX_WOBJ_HAND_TOOL,
  HAND_WOBJ_EX_TOOL,
};

// ֮����CAM_Backendȡ��CS��Ȼ��ͳһmodel
class CamBackend : public core::SireModuleBase {
 public:
  // @param[in] cpt_option  compute collision map option with two case.
  // AxisA6, tilt angle.
  //
  // @param[in] resolution  number of slot to divide range of angle can turn
  //
  // @param[in] pSize  number of tool path points
  //
  // @param[in] points  tool path point cartesian coordinate value
  //  with size 3 * pSize
  //
  // @param[in] tool_axis_angles  angles rotate about tool axis
  //
  // @param[in] side_tilt_angles  Additional tilt angle in the plane
  // perpendicular to the tool motion
  //
  // @param[in] forward_tilt_angles  Additional tilt angle in the direction of
  // tool motion
  //
  // @param[in] normal  number of tool path points
  // @param[in] tangent  number of tool path points
  auto cptCollisionMap(WobjToolInstallMethod install_method, int option,
                       sire::Size resolution, sire::Size pSzie, double* points,
                       double* tool_axis_angles, double* side_tilt_angles,
                       double* forward_tilt_angles, double* normal,
                       double* tangent) -> void;

  // @param[in] cpt_option  compute collision map option with two case.
  // AxisA6, tilt angle.
  //
  // @param[in] resolution  number of slot to divide range of angle can turn
  //
  // @param[in] pSize  number of tool path points
  //
  // @param[in] points_pm  tool path points position matrix
  //  with size 16 * pSize
  //
  // @param[in] side_tilt_angles  Additional tilt angle in the plane
  // perpendicular to the tool motion
  //
  // @param[in] forward_tilt_angles  Additional tilt angle in the direction of
  // tool motion
  auto cptCollisionMap(WobjToolInstallMethod install_method, int cpt_option,
                       sire::Size resolution, sire::Size pSzie,
                       double* points_pm, double* tool_axis_angles,
                       double* side_tilt_angles, double* forward_tilt_angles)
      -> void;
  auto getCollisionEngine() -> collision::CollisionEngine&;
  auto resetCollisionEngine(collision::CollisionEngine* engine) -> void;
  auto getCollisionMapResult() -> const vector<bool>&;
  auto getCollidedObjectsResult()
      -> const vector<set<std::pair<collision::geometry::GeometryId,
                                    collision::geometry::GeometryId>>>&;
  // initial CAM backend by two config file
  auto init(string model_xml_path = ".", string collision_xml_path = ".")
      -> void;
  // initial CAM backend by control server default
  auto init() -> void;
  CamBackend();
  ~CamBackend();

 private:
  // @param[in] ee_pe  end effector pose with [position, EULER321] in double[6]
  auto cptCollisionByEEPose(double* ee_pe,
                            collision::CollidedObjectsCallback& callback)
      -> void;
  // @param[in] install_method  wobj/tool install method
  //
  // @param[in] cpt_option  compute option of which collision map to compute
  // 0 => AxisA6
  // 1 => side_tilt_angle
  // reference spurtCAM �������Ż�����ѡ�
  //
  // @param[in] angle  (radians) The angle of axisA6/side/forward.
  //
  // @param[in] tool_path_point_pm  tool path point position matrix 4 * 4
  // using normal vector of processing plane and direction of tool motion
  // to construct cartesian coordinate.
  //
  // @param[out] target_ee_pose
  auto cptEEPose(WobjToolInstallMethod install_method, int cpt_option,
                 double angle, double* tool_path_point_pm,
                 double tool_axis_angle, double side_tilt_angle,
                 double forward_tilt_angle, double* target_ee_pe) -> void;

  struct Imp;
  unique_ptr<Imp> imp_;
};

auto mapAngleToSymRange(double angle, double range) -> double;
auto vectorCross(double* in_1, double* in_2, double* out) -> void;
auto vectorNormalize(sire::Size n, double* in) -> void;
auto xyz2pm(double* x, double* y, double* z, double* out) -> void;

auto tiltAngle2pm(double side_tilt_angle, double forward_tilt_angle,
                  double* out) -> void;

}  // namespace sire::cam_backend
#endif