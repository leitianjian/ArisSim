#include "sire/simulator/simulation_loop.hpp"

#include <aris/core/object.hpp>
#include <aris/core/reflection.hpp>
#include <aris/dynamic/model.hpp>
#include <aris/server/control_server.hpp>

#include "sire/core/base_factory.hpp"
#include "sire/core/constants.hpp"
#include "sire/core/module_base.hpp"
#include "sire/core/prop_map.hpp"
#include "sire/core/sire_assert.hpp"
#include "sire/middleware/sire_middleware.hpp"
#include "sire/sensor/sensor.hpp"
#include "sire/simulator/simulator_modules.hpp"

namespace sire::simulator {
using core::TriggerBase, core::EventBase, core::HandlerBase;
using std::map;
struct SimulationLoop::Imp {
  middleware::SireMiddleware* middleware_ptr_;
  physics::PhysicsEngine* physics_engine_ptr_;
  simulator::SimulatorModules* simulator_modules_ptr_;
  IntegratorPool* integrator_pool_ptr_;
  SensorPool* sensor_pool_ptr_;

  // ��������ȫ�ֱ�����ʹ��xml���ã���trigger event handle�п���ʹ��
  core::PropMap global_variable_pool_;
  // Event���
  // BaseFactory<TriggerBase>* trigger_factory_;
  core::EventBaseFactory* event_factory_;
  core::HandlerBaseFactory* handler_factory_;
  map<sire::Size, std::string> trigger_pool_;
  map<sire::Size, std::string> event_pool_;
  map<sire::Size, std::string> handler_pool_;
  map<sire::Size, std::unique_ptr<TriggerBase> (*)()> trigger_creator_;
  map<sire::Size, std::unique_ptr<EventBase> (*)()> event_creator_;
  map<sire::Size, std::unique_ptr<HandlerBase> (*)()> handler_creator_;
  std::unique_ptr<core::EventManager> event_manager_;

  core::ContactPairManager contact_pair_manager_;

  core::Timer timer_;

  // all time represent in seconds;
  double dt_;
  std::chrono::system_clock::time_point current_time_;
  std::chrono::system_clock::time_point start_time_;
  std::int64_t sim_count_;

  std::atomic_bool is_simulation_running_{false};
  std::thread simulation_thread;

  std::atomic_bool is_data_fetch_running_{false};
  std::thread data_fetch_thread;
  std::atomic_bool can_get_data_{true};
  // std::unique_ptr<aris::dynamic::Model> prev_model_{
  //     std::make_unique<aris::dynamic::Model>()};

  // �򶴣���ȡ���� //
  std::atomic_bool if_get_data_{false}, if_get_data_ready_{false};
  const std::function<void(aris::server::ControlServer&, SimulationLoop&,
                           std::any&)>* get_data_func_{nullptr};
  std::any* get_data_{nullptr};

  // std::vector<aris::dynamic::Model*> model_pool_{2};
  aris::dynamic::Model* model_ptr_;
  Imp()
      : event_factory_(&core::EventBaseFactory::instance()),
        handler_factory_(&core::HandlerBaseFactory::instance()) {}
};

SimulationLoop::SimulationLoop() : imp_(new Imp) {}
SimulationLoop::~SimulationLoop() = default;
SIRE_DEFINE_MOVE_CTOR_CPP(SimulationLoop);

// ��ʼ���Լ��ƿص���Դ�ͻ�ȡ���������ڵ��µ���Դ
// ��Ҫcs.init()֮���ֶ����ã����ᱻ�Զ�����
auto SimulationLoop::init(middleware::SireMiddleware* middleware) -> void {
  imp_->middleware_ptr_ = middleware;
  SIRE_ASSERT(imp_->middleware_ptr_ != nullptr);
  imp_->physics_engine_ptr_ = &imp_->middleware_ptr_->physicsEngine();
  imp_->simulator_modules_ptr_ = &imp_->middleware_ptr_->simulatorModules();
  imp_->integrator_pool_ptr_ = &imp_->simulator_modules_ptr_->integratorPool();
  imp_->sensor_pool_ptr_ = &imp_->simulator_modules_ptr_->sensorPool();

  // ��ʼ��Simulator�е�Modelָ��
  // ControlServer����һ��Model����Դ����һ���������ݵ�Model��Simulator����
  imp_->model_ptr_ = &dynamic_cast<aris::dynamic::Model&>(
      aris::server::ControlServer::instance().model());

  //  aris::core::fromXmlString(*(imp_->prev_model_),
  //                            aris::core::toXmlString(*(imp_->model_ptr_)));
  //  imp_->prev_model_->init();

  //  imp_->model_pool_[0] = imp_->model_ptr_;
  //  imp_->model_pool_[1] = imp_->prev_model_.get();

  // ��ʼ��Simulator�е���Դ
  // event manager;
  // contact pair manager;
  // imp_->event_manager_.simulator_ptr_ = this;
  // imp_->event_manager_.engine_ptr_ = imp_->physics_engine_ptr_;

  imp_->timer_.init();
  // imp_->trigger_pool_ = imp_->trigger_factory_->idNamePair();
  // imp_->trigger_creator_ = imp_->trigger_factory_->map();
  imp_->event_pool_ = imp_->event_factory_->idNamePair();
  imp_->event_creator_ = imp_->event_factory_->map();
  imp_->handler_pool_ = imp_->handler_factory_->idNamePair();
  imp_->handler_creator_ = imp_->handler_factory_->map();

  // create init trigger to event manager (must at id = 0)
  // std::unique_ptr<TriggerBase> init_trigger = createTriggerById(0);
  // imp_->event_manager_.addImmediateTrigger(std::move(init_trigger));
  std::unique_ptr<EventBase> init_event = createEventById(0);
  imp_->event_manager_->addEvent(std::move(init_event));
  imp_->event_manager_->init();

  // ��ȷ����model�е���
  imp_->physics_engine_ptr_->initPartContactForce2Model();
  imp_->model_ptr_->init();

  // ��ʼ�����ȡ����
  imp_->is_data_fetch_running_.store(true);
  imp_->data_fetch_thread = std::thread([this]() {
    while (imp_->is_data_fetch_running_) {
      while (!(imp_->if_get_data_.load() && imp_->can_get_data_.load()))
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

      imp_->get_data_func_->operator()(aris::server::ControlServer::instance(),
                                       *this, *imp_->get_data_);
      imp_->if_get_data_ready_.store(true);  // ԭ�Ӳ���
      imp_->if_get_data_.store(false);
    }
  });
}
auto SimulationLoop::timer() -> core::Timer& { return imp_->timer_; }
auto SimulationLoop::step(sire::Size frame_skip, bool pause_if_fast) -> void {
  for (sire::Size i = 0; i < frame_skip; ++i) {
    // Get header event pointer
    core::EventBase* header = imp_->event_manager_->eventListHeader();
    // New handler by event id
    std::unique_ptr<core::HandlerBase> handler =
        createHandlerByEventId(header->eventId());
    handler->init(this);
    imp_->can_get_data_.store(false);
    if (handler->handle(header)) {
      imp_->can_get_data_.store(true);
      if (pause_if_fast) {
        imp_->timer_.pauseIfTooFast();
      }
      imp_->event_manager_->headerNextEvent(1);
      imp_->event_manager_->popEventListHeader();
    }
    // double p[6]{0}, v[6]{0}, a[6]{0};
    // model()->generalMotionPool().at(0).getP(p);
    // model()->generalMotionPool().at(0).getV(v);
    // model()->generalMotionPool().at(0).getA(a);
    // std::cout << "step: " << i << std::endl;
    // std::cout << "general_p: ";
    // aris::dynamic::dsp(1, 6, p);
    // aris::dynamic::dsp(1, 6, v);
    // aris::dynamic::dsp(1, 6, a);
    // std::cout << "action: "
    //           << dynamic_cast<aris::dynamic::SingleComponentForce&>(
    //                  model()->forcePool().at(0))
    //                  .fce()
    //           << ", "
    //           << dynamic_cast<aris::dynamic::SingleComponentForce&>(
    //                  model()->forcePool().at(1))
    //                  .fce()
    //           << std::endl;
  }
}
auto SimulationLoop::start() -> void {
  imp_->is_simulation_running_.store(true);
  imp_->simulation_thread = std::thread([this]() {
    while (imp_->is_simulation_running_ &&
           imp_->event_manager_->isEventListEmpty()) {
      step(1, true);
    }
  });
}
auto SimulationLoop::isRunning() -> bool {
  return imp_->is_simulation_running_.load();
}
auto SimulationLoop::pause() -> void {
  imp_->is_simulation_running_.store(false);
  imp_->simulation_thread.join();
}
auto SimulationLoop::createTriggerById(sire::Size trigger_id)
    -> std::unique_ptr<TriggerBase> {
  std::unique_ptr<TriggerBase> new_trigger =
      imp_->trigger_creator_[trigger_id]();
  new_trigger->setTriggerId(trigger_id);
  new_trigger->setTriggerType(imp_->trigger_pool_[trigger_id]);
  return new_trigger;
}
auto SimulationLoop::createEventById(sire::Size event_id)
    -> std::unique_ptr<EventBase> {
  std::unique_ptr<EventBase> new_event = imp_->event_creator_[event_id]();
  new_event->setEventId(event_id);
  new_event->setEventType(imp_->event_pool_[event_id]);
  return std::move(new_event);
}
auto SimulationLoop::createHandlerById(sire::Size handler_id)
    -> std::unique_ptr<HandlerBase> {
  std::unique_ptr<HandlerBase> new_handler =
      imp_->handler_creator_[handler_id]();
  new_handler->setHandlerId(handler_id);
  new_handler->setHandlerType(imp_->handler_pool_[handler_id]);
  return new_handler;
}
auto SimulationLoop::createEventByTriggerId(sire::Size trigger_id)
    -> std::unique_ptr<EventBase> {
  return createEventById(
      imp_->event_manager_->getEventIdByTriggerId(trigger_id));
}
auto SimulationLoop::createHandlerByEventId(sire::Size event_id)
    -> std::unique_ptr<HandlerBase> {
  return createHandlerById(
      imp_->event_manager_->getHandlerIdByEventId(event_id));
}
auto SimulationLoop::model() noexcept -> aris::dynamic::Model* {
  return imp_->model_ptr_;
}
auto SimulationLoop::contactPairManager() noexcept
    -> core::ContactPairManager* {
  return &imp_->contact_pair_manager_;
}
auto SimulationLoop::getModelState(
    const std::function<void(aris::server::ControlServer&, SimulationLoop&,
                             std::any&)>& get_func,
    std::any& get_data) -> void {
  if (!imp_->is_data_fetch_running_)
    THROW_FILE_LINE("data fetch thread not start");
  imp_->get_data_func_ = &get_func;
  imp_->get_data_ = &get_data;

  imp_->if_get_data_ready_.store(false);
  imp_->if_get_data_.store(true);

  // spin waitting for get data
  while (!imp_->if_get_data_ready_.load())
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

  imp_->if_get_data_ready_.store(false);
}
// auto SimulationLoop::getModelState(
//     const std::function<void(aris::server::ControlServer&, SimulationLoop&,
//                              std::any&)>& get_func,
//     std::any& get_data) -> void {
//   imp_->event_manager_.getModelState(get_func, get_data);
// }
auto SimulationLoop::backupModel() -> void {
  //  aris::core::fromXmlString(*(imp_->model_pool_[1]),
  //                            aris::core::toXmlString(*(imp_->model_pool_[0])));
}
auto SimulationLoop::restoreModel() -> void {
  //  aris::core::fromJsonFile(*(imp_->model_pool_[0]),
  //                           aris::core::toXmlString(*(imp_->model_pool_[1])));
}
auto SimulationLoop::integrateAs2Ps() -> void {}

auto SimulationLoop::updateSysTime() -> void {
  imp_->current_time_ = std::chrono::system_clock::now();
}
auto SimulationLoop::resetIntegratorPoolPtr(IntegratorPool* pool) -> void {
  imp_->integrator_pool_ptr_ = pool;
}
auto SimulationLoop::integratorPoolPtr() const -> const IntegratorPool* {
  return imp_->integrator_pool_ptr_;
}
auto SimulationLoop::resetSensorPoolPtr(SensorPool* pool) -> void {
  imp_->sensor_pool_ptr_ = pool;
}
auto SimulationLoop::sensorPoolPtr() const -> const SensorPool* {
  return imp_->sensor_pool_ptr_;
}
auto SimulationLoop::resetPhysicsEnginePtr(physics::PhysicsEngine* engine)
    -> void {
  imp_->physics_engine_ptr_ = engine;
}
auto SimulationLoop::physicsEnginePtr() const -> const physics::PhysicsEngine* {
  return imp_->physics_engine_ptr_;
}
auto SimulationLoop::resolveContact() -> void {
  // imp_->physics_engine_ptr_->handleContact();
}
auto SimulationLoop::collisionDetection() -> void {
  imp_->physics_engine_ptr_->hasCollision();
}
auto SimulationLoop::resetEventManager(core::EventManager* manager) -> void {
  imp_->event_manager_.reset(manager);
}
auto SimulationLoop::eventManager() const -> const core::EventManager& {
  return *imp_->event_manager_;
}
auto SimulationLoop::deltaT() -> double { return imp_->dt_; }
auto SimulationLoop::setDeltaT(double delta_t_in) -> void {
  SIRE_ASSERT(delta_t_in >= 0);
  imp_->dt_ = delta_t_in;
}
auto SimulationLoop::targetRealtimeRate() -> double {
  return imp_->timer_.targetRealtimeRate();
}
auto SimulationLoop::realtimeRate() -> double {
  return imp_->timer_.realtimeRate();
}
auto SimulationLoop::setRealtimeRate(double rate) -> void {
  imp_->timer_.setRealtimeRate(rate);
}
auto SimulationLoop::setGlobalVariablePool(core::PropMap& pool) -> void {
  imp_->global_variable_pool_ = pool;
}
auto SimulationLoop::getGlobalVariablePool() const -> const core::PropMap& {
  return imp_->global_variable_pool_;
}
auto SimulationLoop::reset() -> void {
  imp_->contact_pair_manager_.contactPairMap().clear();
}

ARIS_REGISTRATION {
  auto setGlobalVariablePool = [](SimulationLoop* p,
                                  core::PropMap map) -> void {
    p->setGlobalVariablePool(map);
  };
  auto getGlobalVariablePool = [](SimulationLoop* p) -> core::PropMap {
    return p->getGlobalVariablePool();
  };
  typedef core::EventManager& (SimulationLoop::*EventManagerFunc)();

  aris::core::class_<SimulationLoop>("SimulationLoop")
      .prop("dt", &SimulationLoop::setDeltaT, &SimulationLoop::deltaT)
      .prop("realtime_rate", &SimulationLoop::setRealtimeRate,
            &SimulationLoop::realtimeRate)
      .prop("global_variable_pool", &setGlobalVariablePool,
            &getGlobalVariablePool)
      .prop("event_manager", &SimulationLoop::resetEventManager,
            EventManagerFunc(&SimulationLoop::eventManager));
}
}  // namespace sire::simulator