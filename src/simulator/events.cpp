#include "sire/simulator/events.hpp"

#include <algorithm>
#include <iterator>
#include <limits>
#include <set>

#include <aris/core/log.hpp>
#include <aris/dynamic/model.hpp>
#include <aris/dynamic/screw.hpp>

#include "sire/core/event_manager.hpp"
#include "sire/ext/json.hpp"
#include "sire/physics/common/penetration_as_point_pair.hpp"

namespace sire::simulator {
using namespace sire::physics;
auto process_impact_threshold(aris::dynamic::Model* m,
                              double impact_threshold_insert,
                              double impact_threshold_remove)
    -> std::pair<double, double> {
  double min_mass = std::numeric_limits<double>::infinity();
  for (auto& part : m->partPool()) {
    min_mass = min_mass < part.prtIv()[0] ? min_mass : part.prtIv()[0];
  }
  min_mass = min_mass <= 0 ? 1.0 : min_mass;
  double max_allow_velocity_change_dt_insert = 5000.0;
  double impact_threshold_1 = min_mass * max_allow_velocity_change_dt_insert;
  impact_threshold_insert = impact_threshold_insert < impact_threshold_1
                                ? impact_threshold_insert
                                : impact_threshold_1;
  double max_allow_velocity_change_dt_remove = 10.0;
  double impact_threshold_2 = min_mass * max_allow_velocity_change_dt_remove;
  impact_threshold_remove = impact_threshold_remove < impact_threshold_2
                                ? impact_threshold_remove
                                : impact_threshold_2;
  return std::make_pair(impact_threshold_insert, impact_threshold_remove);
}
// ��Ҫ�������˽Ӵ��������������µĴ�impact
auto cpt_prt_contact_impact(simulator::SimulationLoop* s)
    -> std::vector<double> {
  SIRE_DEMAND(s != nullptr);
  aris::dynamic::Model* m = s->model();

  std::vector<std::array<double, 6>> part_fnet(m->partPool().size(), {0.0});

  for (sire::Size i = 0; i < m->partPool().size(); ++i) {
    auto& contact_force = m->forcePool().at(i + m->motionPool().size());
    double fsI[6], fsJ[6];
    contact_force.cptGlbFs(fsI, fsJ);
    if (&contact_force.makI()->fatherPart() != &m->ground())
      aris::dynamic::s_va(
          6, fsI, part_fnet[contact_force.makI()->fatherPart().id()].data());
    if (&contact_force.makJ()->fatherPart() != &m->ground())
      aris::dynamic::s_va(
          6, fsJ, part_fnet[contact_force.makJ()->fatherPart().id()].data());
  }
  double dt = s->deltaT();
  std::vector<double> result(m->partPool().size());
  for (sire::PartId i = 0; i < m->partPool().size(); ++i) {
    // ����ground
    if (i != m->ground().id()) {
      double part_impact = dt * aris::dynamic::s_norm(3, part_fnet[i].data()) /
                           m->partPool().at(i).prtIv()[0];
      result[i] = part_impact;
      // aris::dynamic::s_nv(6, dt, part_fnet[i].data());
    } else {
      result[i] = 0;
    }
  }
  return result;
}
auto process_penetration_depth_and_maintain_impact_set(
    simulator::SimulationLoop* simulator_ptr) -> void {
  physics::PhysicsEngine* engine_ptr = simulator_ptr->physicsEnginePtr();
  core::ContactPairManager* manager_ptr = simulator_ptr->contactPairManager();

  // physicsEngine ptr -> handleContact()
  engine_ptr->updateGeometryLocationFromModel();
  std::vector<common::PenetrationAsPointPair> pairs;
  // ��ײ���
  engine_ptr->cptPointPairPenetration(pairs);

  using ContactPairMap = std::unordered_map<core::SortedPair<sire::PartId>,
                                            core::ContactPairValue>;
  ContactPairMap& contact_pair_map = manager_ptr->contactPairMap();
  // if (pairs.size() != 0) {
  //   for (auto& pair : pairs) {
  //     if (pair.id_B == 1) {
  //       std::cout << "pair_depth=" << pair.depth << " ";
  //     }
  //   }
  // }
  // 1. �޸ı��
  // ������ײ��Ϣ�����ײ��ļ�¼���±������ײ���¼����ײ��Ϣ depth-init_depth
  // Map���еģ�vector��û�У���ɾ��
  for (ContactPairMap::iterator it = contact_pair_map.begin();
       it != contact_pair_map.end();) {
    if (auto search = std::find_if(pairs.begin(), pairs.end(),
                                   [it](common::PenetrationAsPointPair& pair) {
                                     return it->first ==
                                            core::SortedPair<sire::PartId>(
                                                pair.id_A, pair.id_B);
                                   });
        search == pairs.end()) {
      contact_pair_map.erase(it++);
    } else {
      ++it;
    }
  }
  // Vector���еģ�Map��û�У��Ͳ��룬�Ȳ��޸��¼����Ĵ������һ��huge_impact_prt,
  // ���޸Ĵ�����л��֡���¼û�м�ȥ������¼�����index
  std::vector<common::PenetrationAsPointPair*> new_contacts_ptr;
  for (auto& pair : pairs) {
    if (auto search = contact_pair_map.find({pair.id_A, pair.id_B});
        search == contact_pair_map.end()) {
      contact_pair_map.insert({{pair.id_A, pair.id_B}, {pair.depth, false}});
      new_contacts_ptr.push_back(&pair);
    } else {
      auto& contact_pair_value = contact_pair_map[{pair.id_A, pair.id_B}];
      if (pair.depth < 0) {
        // ���¼�¼�ĳ�ʼ����
        contact_pair_value.init_penetration_depth_ = pair.depth;
        // ��¼Ϊ�µ���ײ��
        // new_contacts_ptr.push_back(&pair);
      }
      pair.depth -= contact_pair_value.init_penetration_depth_;
      // if (contact_pair_value.is_depth_smaller_than_init_depth_) {
      //   // ����initial depth
      //   // ����ֿ�ʼ�Ӵ�����Ҫ��ʼ����
      //   if (pair.depth > 0) {
      //     contact_pair_value.is_depth_smaller_than_init_depth_ = false;
      //     new_contacts_ptr.push_back(&pair);
      //   } else {
      //     // ��depthһֱ����Ϊ�㣬�����¼�¼��initial depth
      //     contact_pair_value.init_penetration_depth_ += pair.depth;
      //     pair.depth = 0;
      //   }
      // } else {
      //   // �޸��Ѿ��ڱ���еĵ�Ĵ���Ѿ����������vector�еļ�¼��
      //   if (pair.depth < 0) {
      //     contact_pair_value.is_depth_smaller_than_init_depth_ = true;
      //     contact_pair_value.init_penetration_depth_ += pair.depth;
      //   }
      // }
    }
  }
  // if (contact_pair_map.size() != 0) {
  //   double vs[6], as[6];
  //   simulator_ptr->model()->partPool().at(1).getVs(vs);
  //   simulator_ptr->model()->partPool().at(1).getAs(as);
  //   std::cout << "init_depth="
  //             << contact_pair_map.at({0, 1}).init_penetration_depth_ << " "
  //             << vs[2] << " " << as[2] << " ";
  // }
  // if (pairs.size() != 0)
  //   std::cout << " pair_depth2=" << pairs.at(0).depth << " ";

  std::vector<common::PointPairContactInfo> contact_info;
  // �Ӵ���⣬�õ��Ӵ���
  engine_ptr->cptContactInfo(pairs, contact_info);
  // ������һʱ�̹ؽں�forcePool���õ���
  // TODO(ltj): �ؽڵĿ�������ô����������Ҫ��ôд
  engine_ptr->resetPartContactForce();
  // ���ݽӴ���Ϣ�������û�model��forcePool
  engine_ptr->cptGlbForceByContactInfo(contact_info);
  // ����ÿ���˼���ײ���ͼ��ٶȺ�dt�����ݣ��ж��Ƿ���ײ����ײ��Ҫ��С����
  double impact_threshold_insert =
      simulator_ptr->getGlobalVariablePool().getPropValueOrDefault(
          "impact_threshold_insert", 5);
  double impact_threshold_remove =
      simulator_ptr->getGlobalVariablePool().getPropValueOrDefault(
          "impact_threshold_remove", 0.2);
  std::tie(impact_threshold_insert, impact_threshold_remove) =
      process_impact_threshold(simulator_ptr->model(), impact_threshold_insert,
                               impact_threshold_remove);
  std::vector<double> prt_impact_result = cpt_prt_contact_impact(simulator_ptr);
  // if (contact_pair_map.size() != 0)
  //   std::cout << "prt_impact=" << prt_impact_result[1] << " ";
  std::set<sire::PartId> impact_prts_insert, impact_prts_remove;
  for (sire::Size i = 0; i < prt_impact_result.size(); ++i) {
    if (prt_impact_result[i] > impact_threshold_insert)
      impact_prts_insert.insert(i);
    if (prt_impact_result[i] < impact_threshold_remove)
      impact_prts_remove.insert(i);
  }
  impact_prts_remove.erase(simulator_ptr->model()->ground().id());
  // 2. �޸ı�һ������µ�prt��
  using ImpactedPrtSet = std::unordered_set<sire::PartId>;
  ImpactedPrtSet& impacted_prt_set = manager_ptr->impactedPrtSet();
  for (sire::PartId impact_prt : impact_prts_insert) {
    impacted_prt_set.insert(impact_prt);
    // if (!simulator_ptr->contactPairManager()->hasImpactedPrt(impact_prt)) {
    //   // 1. �������ĸ˼�û�б���¼
    //   //
    //   ��Ҫ�����ײ�б��Ƿ����µ���ײ��Ҫ���룬���û�оͲ���Ҫ��¼�˼����޸ı�1��
    //   // �޸ı�һ
    //   if (auto search = std::find_if(
    //           contact_info.begin(), contact_info.end(),
    //           [&impact_prt](common::PointPairContactInfo& contact) {
    //             return contact.partId_A() == impact_prt ||
    //                    contact.partId_B() == impact_prt;
    //           });
    //       search != contact_info.end()) {
    //     impacted_prt_set.insert(impact_prt);
    //   }
    // }
  }
  // 3. �޸ı�һ���Ƴ���¼��
  for (ImpactedPrtSet::iterator it = impacted_prt_set.begin();
       it != impacted_prt_set.end();) {
    if (auto search = impact_prts_remove.find(*it);
        search != impact_prts_remove.end()) {
      impacted_prt_set.erase(it++);
    } else {
      ++it;
    }
  }
  // if (impacted_prt_set.empty()) {
  //   for (auto& contact_pair : contact_pair_map)
  //     contact_pair.second.is_depth_smaller_than_init_depth_ = true;
  // }
  if (new_contacts_ptr.size() != 0) {
    // ��ȥ init_depth���ټ���F
    for (auto new_contact : new_contacts_ptr) {
      new_contact->depth -=
          contact_pair_map[{new_contact->id_A, new_contact->id_B}]
              .init_penetration_depth_;
    }
    contact_info.clear();
    // �Ӵ���⣬�õ��Ӵ���
    engine_ptr->cptContactInfo(pairs, contact_info);
    // ������һʱ�̹ؽں�forcePool���õ���
    // TODO(ltj): �ؽڵĿ�������ô����������Ҫ��ôд
    engine_ptr->resetPartContactForce();
    // ���ݽӴ���Ϣ�������û�model��forcePool
    engine_ptr->cptGlbForceByContactInfo(contact_info);
  }
  // if (pairs.size() != 0) {
  //   double vs[6];
  //   simulator_ptr->model()->partPool().at(1).getVs(vs);
  //   std::cout << pairs.at(0).depth << " " << impacted_prt_set.size() << " "
  //             << vs[2] << " " << std::endl;
  // }
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
                     simulator::SimulationLoop* base) -> void {
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

auto InitTrigger::trigger(simulator::SimulationLoop*) -> void {
  // manager->addEvent();
}
auto InitEvent::init() -> void {}
auto InitHandler::init(simulator::SimulationLoop* simulator) -> void {
  simulator_ptr = simulator;
}
auto InitHandler::handle(core::EventBase* e) -> bool {
  InitEvent* event_ptr = dynamic_cast<InitEvent*>(e);
  core::ContactPairManager* manager_ptr = simulator_ptr->contactPairManager();
  process_penetration_depth_and_maintain_impact_set(simulator_ptr);
  // ֮��Ϳ�����������

  std::unique_ptr<core::EventBase> step_event =
      simulator_ptr->createEventById(1);
  step_event->eventProp().addProp(
      "dt", manager_ptr->impactedPrtSet().empty()
                ? simulator_ptr->deltaT()
                : simulator_ptr->getGlobalVariablePool().getPropValueOrDefault(
                      "shrink_dt", 1e-7));
  simulator_ptr->eventManager().addEvent(std::move(step_event));

  return true;
  // Add Step Trigger to trigger list in Event Manager
  // EventManager ptr
}
auto StepEvent::init() -> void {}
auto StepHandler::init(simulator::SimulationLoop* simulator) -> void {
  simulator_ptr = simulator;
}
static double cumulate_time = 0;
auto StepHandler::handle(core::EventBase* e) -> bool {
  // ���ֵ���ǰevent��¼��ʱ��
  double dt = e->eventProp().getPropValue("dt");
  simulator_ptr->integratorPoolPtr()->at(0).step(dt);
  simulator_ptr->timer().updateSimTime(dt);
  StepEvent* event_ptr = dynamic_cast<StepEvent*>(e);
  core::ContactPairManager* manager_ptr = simulator_ptr->contactPairManager();
  process_penetration_depth_and_maintain_impact_set(simulator_ptr);
  std::unique_ptr<core::EventBase> step_event =
      simulator_ptr->createEventById(1);
  step_event->eventProp().addProp(
      "dt", manager_ptr->impactedPrtSet().empty()
                ? simulator_ptr->deltaT()
                : simulator_ptr->getGlobalVariablePool().getPropValueOrDefault(
                      "shrink_dt", 1e-7));
  // if (!manager_ptr->impactedPrtSet().empty()) {
  //   std::cout
  //       << "shrinked dt "
  //       << simulator_ptr->getGlobalVariablePool().getPropValueOrDefault(
  //              "shrink_dt", 1e-7)
  //       << " "
  //       << manager_ptr->contactPairMap().at({0, 1}).init_penetration_depth_
  //       << std::endl;
  // }
  simulator_ptr->eventManager().addEvent(std::move(step_event));

  return true;
  // Add Step Trigger to trigger list in Event Manager
  // EventManager ptr
}
auto InitEvent1::init() -> void {}
auto InitHandler1::init(simulator::SimulationLoop* simulator) -> void {
  simulator_ptr = simulator;
}
auto InitHandler1::handle(core::EventBase* e) -> bool {
  InitEvent1* event_ptr = dynamic_cast<InitEvent1*>(e);
  core::ContactPairManager* manager_ptr = simulator_ptr->contactPairManager();
  simulator_ptr->timer().reset();
  // initLog();
  // logCurrentState(0, 1, simulator_ptr);
  process_penetration_depth_and_maintain_impact_set(simulator_ptr);
  // ֮��Ϳ�����������

  std::unique_ptr<core::EventBase> step_event =
      simulator_ptr->createEventById(1);
  step_event->eventProp().addProp(
      "dt", manager_ptr->impactedPrtSet().empty()
                ? simulator_ptr->deltaT()
                : simulator_ptr->getGlobalVariablePool().getPropValueOrDefault(
                      "shrink_dt", 1e-5));
  simulator_ptr->eventManager().addEvent(std::move(step_event));

  return true;
  // Add Step Trigger to trigger list in Event Manager
  // EventManager ptr
}
auto StepEvent1::init() -> void {}
auto StepHandler1::init(simulator::SimulationLoop* simulator) -> void {
  simulator_ptr = simulator;
}
auto StepHandler1::handle(core::EventBase* e) -> bool {
  // ���ֵ���ǰevent��¼��ʱ��
  double dt = e->eventProp().getPropValue("dt");
  simulator_ptr->integratorPoolPtr()->at(0).step(dt);
  // std::cout << "dt=" << dt << " ";
  simulator_ptr->timer().updateSimTime(dt);
  // logCurrentState(simulator_ptr->timer().simTime(),
  //                 simulator_ptr->timer().realtimeRate(), simulator_ptr);
  // if (dt == 0.0000001) std::cout << "dt=" << dt << " ";
  StepEvent* event_ptr = dynamic_cast<StepEvent*>(e);
  core::ContactPairManager* manager_ptr = simulator_ptr->contactPairManager();
  if (dt == simulator_ptr->deltaT()) {
    manager_ptr->contactPairMap().clear();
  }
  process_penetration_depth_and_maintain_impact_set(simulator_ptr);
  // std::cout << std::endl;

  std::unique_ptr<core::EventBase> step_event =
      simulator_ptr->createEventById(1);
  step_event->eventProp().addProp(
      "dt", manager_ptr->impactedPrtSet().empty()
                ? simulator_ptr->deltaT()
                : simulator_ptr->getGlobalVariablePool().getPropValueOrDefault(
                      "shrink_dt", 1e-5));
  // if (!manager_ptr->impactedPrtSet().empty()) {
  //   std::cout
  //       << "shrinked dt "
  //       << simulator_ptr->getGlobalVariablePool().getPropValueOrDefault(
  //              "shrink_dt", 1e-7)
  //       << " "
  //       << manager_ptr->contactPairMap().at({0, 1}).init_penetration_depth_
  //       << std::endl;
  // }
  simulator_ptr->eventManager().addEvent(std::move(step_event));

  return true;
  // Add Step Trigger to trigger list in Event Manager
  // EventManager ptr
}
ARIS_REGISTRATION {
  // core::EventRegister<InitEvent>::registration("initial", 0);
  // core::EventRegister<StepEvent>::registration("step", 1);
  // core::HandlerRegister<InitHandler>::registration("initial", 0);
  // core::HandlerRegister<StepHandler>::registration("step", 1);
  core::EventRegister<InitEvent1>::registration("initial1", 0);
  core::EventRegister<StepEvent1>::registration("step1", 1);
  core::HandlerRegister<InitHandler1>::registration("initial1", 0);
  core::HandlerRegister<StepHandler1>::registration("step1", 1);
}
}  // namespace sire::simulator