#include "sire/plan/display3d_init_cmd.hpp"

#include <tuple>

#include "sire/core/constants.hpp"
#include "sire/core/geometry/geometry_base.hpp"
#include "sire/core/sire_log.hpp"
#include "sire/ext/json.hpp"
#include "sire/server/interface.hpp"

namespace sire::plan {
auto Display3dInit::prepareNrt() -> void {
  option() |= NOT_RUN_EXECUTE_FUNCTION | NOT_RUN_COLLECT_FUNCTION;
  // option() |= NOT_PRINT_CMD_INFO;
  for (auto& m : motorOptions()) m = aris::plan::Plan::NOT_CHECK_ENABLE;
  // get control server config of geometry in part pool
  nlohmann::json geo_pool;
  sire::Size geometry_size = 0;
  // ȡ�� Part�����ÿһ��geometry
  nlohmann::json geometry_pm;
  for (sire::Size i = 0; i < model()->partPool().size(); ++i) {
    nlohmann::json json;
    aris::dynamic::Part& part = model()->partPool().at(i);
    std::array<double, 16> buffer;
    for (sire::Size j = 0; j < part.geometryPool().size(); ++j) {
      dynamic_cast<geometry::GeometryBase&>(part.geometryPool().at(j))
          .to_json(json);
      geo_pool.push_back(json);
      aris::dynamic::s_vc(
          16,
          const_cast<double*>(
              *dynamic_cast<geometry::GeometryBase&>(part.geometryPool().at(j))
                   .pm()),
          buffer.data());
      geometry_pm.push_back(buffer);
      ++geometry_size;
    }
  }
  // ����part��س�ʼ������Ϣ
  // ground Ĭ��ʹ��wobj0��Ϊ0������ϵ���±�Ϊ1
  aris::dynamic::Marker& ground = model()->partPool().at(0).markerPool().at(1);
  nlohmann::json part_init_config;
  part_init_config.push_back(
      std::array<double, sire::kPosQuatSize>({0, 0, 0, 0, 0, 0, 1}));
  for (sire::Size i = 1; i < model()->partPool().size(); ++i) {
    nlohmann::json json;
    aris::dynamic::Part& part = model()->partPool().at(i);
    std::array<double, sire::kPosQuatSize> part_pq_buffer;
    part.markerPool().at(0).getPq(part_pq_buffer.data());
    // part.markerPool().at(0).getPq(ground, part_pq_buffer.data());
    part_init_config.push_back(part_pq_buffer);
  }

  std::vector<std::pair<std::string, std::any>> out_param;
  SIRE_LOG << geo_pool << "yes" << std::endl;
  out_param.push_back(
      std::make_pair<std::string, std::any>("geometry_pool", geo_pool));
  out_param.push_back(
      std::make_pair<std::string, std::any>("geometry_pm", geometry_pm));
  out_param.push_back(
      std::make_pair<std::string, std::any>("part_pq_init", part_init_config));
  test();
  std::cout << "test()" << std::endl;
  ret() = out_param;
}

auto Display3dInit::collectNrt() -> void {}

auto Display3dInit::test() -> void {
  auto& cs = *controlServer();
  std::cout << cs.running() << std::endl;
}

Display3dInit::Display3dInit(const std::string& name) {
  aris::core::fromXmlString(command(), "<Command name=\"display3d_init\"/>");
}

ARIS_REGISTRATION {
  aris::core::class_<Display3dInit>("Display3dInit")
      .inherit<aris::plan::Plan>();
}
}  // namespace sire::plan