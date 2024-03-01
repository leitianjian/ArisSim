#include "./test_events.hpp"

#include <algorithm>
#include <iterator>
#include <limits>
#include <set>

#include <aris/dynamic/model.hpp>
#include <aris/dynamic/screw.hpp>

#include "sire/core/base_factory.hpp"
#include "sire/core/event_manager.hpp"
#include "sire/physics/common/penetration_as_point_pair.hpp"

namespace sire::free_fall::glb_time_step {
using namespace sire::physics;
auto calContactForce(simulator::SimulatorBase* simulator_ptr) -> void {
  physics::PhysicsEngine* engine_ptr = simulator_ptr->physicsEnginePtr();
  // physicsEngine ptr -> handleContact()
  engine_ptr->updateGeometryLocationFromModel();
  std::vector<common::PenetrationAsPointPair> pairs;
  // 碰撞检测
  engine_ptr->cptPointPairPenetration(pairs);

  std::vector<common::PointPairContactInfo> contact_info;
  // 接触求解，得到接触力
  engine_ptr->cptContactInfo(pairs, contact_info);
  // 重置上一时刻关节和forcePool设置的力
  // TODO(ltj): 关节的控制力怎么进来，控制要怎么写
  engine_ptr->resetPartContactForce();
  // 根据接触信息将力设置回model的forcePool
  engine_ptr->cptGlbForceByContactInfo(contact_info);
}
static std::fstream file;
auto initLog() -> void {
  auto file_name =
      aris::core::defaultLogDirectory() /
      ("state-log--" +
       aris::core::logFileTimeFormat(std::chrono::system_clock::now()) + "--");
  file.open(file_name.string() + ".txt", std::ios::out | std::ios::trunc);
}
auto logCurrentState(double time, double realtime_rate,
                     simulator::SimulatorBase* base) -> void {
  std::vector<std::array<double, 6>> parts_pe;
  std::vector<std::array<double, 6>> parts_vs;
  for (int i = 0; i < base->model()->partPool().size(); ++i) {
    std::array<double, 6> buffer_pe{0}, buffer_vs{0};
    base->model()->partPool().at(i).getPe(buffer_pe.data());
    base->model()->partPool().at(i).getVs(buffer_vs.data());
    parts_pe.push_back(buffer_pe);
    parts_vs.push_back(buffer_vs);
  }

  file << time << " " << realtime_rate << " " << nlohmann::json(parts_pe).dump()
       << " " << nlohmann::json(parts_vs).dump() << std::endl;
}
auto InitEvent::init() -> void {}
auto InitHandler::init(simulator::SimulatorBase* simulator) -> void {
  simulator_ptr = simulator;
}
auto InitHandler::handle(core::EventBase* e) -> bool {
  InitEvent* event_ptr = dynamic_cast<InitEvent*>(e);
  initLog();
  logCurrentState(0, 1, simulator_ptr);
  calContactForce(simulator_ptr);

  std::unique_ptr<core::EventBase> step_event =
      simulator_ptr->createEventById(1);
  step_event->eventProp().addProp("dt", simulator_ptr->deltaT());
  simulator_ptr->eventManager().addEvent(std::move(step_event));

  return true;
}
auto StepEvent::init() -> void {}
auto StepHandler::init(simulator::SimulatorBase* simulator) -> void {
  simulator_ptr = simulator;
}
auto StepHandler::handle(core::EventBase* e) -> bool {
  // 积分到当前event记录的时间
  double dt = e->eventProp().getPropValue("dt");
  simulator_ptr->integratorPoolPtr()->at(0).step(dt);
  simulator_ptr->timer().updateSimTime(dt);
  // if (dt == 0.0000001) std::cout << "dt=" << dt << " ";
  logCurrentState(simulator_ptr->timer().simTime(),
                  simulator_ptr->timer().realtimeRate(), simulator_ptr);
  calContactForce(simulator_ptr);
  std::unique_ptr<core::EventBase> step_event =
      simulator_ptr->createEventById(1);
  step_event->eventProp().addProp("dt", simulator_ptr->deltaT());
  // if (!manager_ptr->impactedPrtSet().empty()) {
  //   std::cout
  //       << "shrinked dt "
  //       << manager_ptr->contactPairMap().at({0, 1}).init_penetration_depth_
  //       << std::endl;
  // }
  simulator_ptr->eventManager().addEvent(std::move(step_event));

  return true;
}
#ifdef SIRE_THREE_BALL_FREE_FALLING_USING_GLOBAL_STEP
ARIS_REGISTRATION {
  core::EventBaseFactory::instance().clear();
  core::HandlerBaseFactory::instance().clear();
  core::EventRegister<InitEvent>::registration("initial", 0);
  core::EventRegister<StepEvent>::registration("step", 1);
  core::HandlerRegister<InitHandler>::registration("initial", 0);
  core::HandlerRegister<StepHandler>::registration("step", 1);
}
#endif
}  // namespace sire::free_fall::glb_time_step

namespace sire::free_fall::without_penetration_adjustment {
using namespace sire::physics;
auto calContactForce(simulator::SimulatorBase* simulator_ptr) -> bool {
  physics::PhysicsEngine* engine_ptr = simulator_ptr->physicsEnginePtr();
  // physicsEngine ptr -> handleContact()
  engine_ptr->updateGeometryLocationFromModel();
  std::vector<common::PenetrationAsPointPair> pairs;
  // 碰撞检测
  engine_ptr->cptPointPairPenetration(pairs);

  std::vector<common::PointPairContactInfo> contact_info;
  // 接触求解，得到接触力
  engine_ptr->cptContactInfo(pairs, contact_info);
  // 重置上一时刻关节和forcePool设置的力
  // TODO(ltj): 关节的控制力怎么进来，控制要怎么写
  engine_ptr->resetPartContactForce();
  // 根据接触信息将力设置回model的forcePool
  engine_ptr->cptGlbForceByContactInfo(contact_info);
  return !pairs.empty();
}
static std::fstream file;
auto initLog() -> void {
  auto file_name =
      aris::core::defaultLogDirectory() /
      ("state-log--" +
       aris::core::logFileTimeFormat(std::chrono::system_clock::now()) + "--");
  file.open(file_name.string() + ".txt", std::ios::out | std::ios::trunc);
}
auto logCurrentState(double time, double realtime_rate,
                     simulator::SimulatorBase* base) -> void {
  std::vector<std::array<double, 6>> parts_pe;
  std::vector<std::array<double, 6>> parts_vs;
  for (int i = 0; i < base->model()->partPool().size(); ++i) {
    std::array<double, 6> buffer_pe{0}, buffer_vs{0};
    base->model()->partPool().at(i).getPe(buffer_pe.data());
    base->model()->partPool().at(i).getVs(buffer_vs.data());
    parts_pe.push_back(buffer_pe);
    parts_vs.push_back(buffer_vs);
  }

  file << time << " " << realtime_rate << " " << nlohmann::json(parts_pe).dump()
       << " " << nlohmann::json(parts_vs).dump() << std::endl;
}
auto InitEvent::init() -> void {}
auto InitHandler::init(simulator::SimulatorBase* simulator) -> void {
  simulator_ptr = simulator;
}
auto InitHandler::handle(core::EventBase* e) -> bool {
  InitEvent* event_ptr = dynamic_cast<InitEvent*>(e);
  initLog();
  logCurrentState(0, 1, simulator_ptr);
  bool using_small_step = calContactForce(simulator_ptr);

  std::unique_ptr<core::EventBase> step_event =
      simulator_ptr->createEventById(1);
  step_event->eventProp().addProp(
      "dt", using_small_step
                ? simulator_ptr->getGlobalVariablePool().getPropValueOrDefault(
                      "shrink_dt", 1e-5)
                : simulator_ptr->deltaT());
  simulator_ptr->eventManager().addEvent(std::move(step_event));

  return true;
}
auto StepEvent::init() -> void {}
auto StepHandler::init(simulator::SimulatorBase* simulator) -> void {
  simulator_ptr = simulator;
}
auto StepHandler::handle(core::EventBase* e) -> bool {
  // 积分到当前event记录的时间
  double dt = e->eventProp().getPropValue("dt");
  simulator_ptr->integratorPoolPtr()->at(0).step(dt);
  simulator_ptr->timer().updateSimTime(dt);
  // if (dt == 0.0000001) std::cout << "dt=" << dt << " ";
  logCurrentState(simulator_ptr->timer().simTime(),
                  simulator_ptr->timer().realtimeRate(), simulator_ptr);
  bool using_small_step = calContactForce(simulator_ptr);
  std::unique_ptr<core::EventBase> step_event =
      simulator_ptr->createEventById(1);
  step_event->eventProp().addProp(
      "dt", using_small_step
                ? simulator_ptr->getGlobalVariablePool().getPropValueOrDefault(
                      "shrink_dt", 1e-5)
                : simulator_ptr->deltaT());
  // if (!manager_ptr->impactedPrtSet().empty()) {
  //   std::cout
  //       << "shrinked dt "
  //       << manager_ptr->contactPairMap().at({0, 1}).init_penetration_depth_
  //       << std::endl;
  // }
  simulator_ptr->eventManager().addEvent(std::move(step_event));

  return true;
}
#ifdef SIRE_THREE_BALL_FREE_FALLING_WITHOUT_PA
ARIS_REGISTRATION {
  core::EventBaseFactory::instance().clear();
  core::HandlerBaseFactory::instance().clear();
  core::EventRegister<InitEvent>::registration("initial", 0);
  core::EventRegister<StepEvent>::registration("step", 1);
  core::HandlerRegister<InitHandler>::registration("initial", 0);
  core::HandlerRegister<StepHandler>::registration("step", 1);
}
#endif
}  // namespace sire::free_fall::without_penetration_adjustment