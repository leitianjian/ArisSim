#ifndef SIRE_BOX_GEOMETRY_HPP_
#define SIRE_BOX_GEOMETRY_HPP_

#include <atomic>
#include <string>
#include <utility>

#include <aris/core/object.hpp>
#include <aris/dynamic/model_basic.hpp>
#include <aris/dynamic/model_coordinate.hpp>

#include "sire/core/geometry/box_shape.hpp"
#include "sire/core/geometry/geometry_on_part.hpp"
#include "sire/core/sire_decl_def_macro.hpp"
#include "sire/ext/json.hpp"

namespace sire::geometry {
using namespace std;
using json = nlohmann::json;
class BoxGeometry : public GeometryOnPart {
 public:
  BoxShape boxShape;
  explicit BoxGeometry(double x = 0.1, double y = 0.1, double z = 0.1,
                       const double* prt_pm = nullptr);
  virtual ~BoxGeometry();
  ARIS_DECLARE_BIG_FOUR(BoxGeometry)

  // ���ڲ�ʹ�õ�to_json from_json������
  SIRE_DECLARE_JSON_INTER_OVERRIDE_TWO

  // nlohammn::json j = o;��ʱ����Զ����õ�to_json from_json������
  SIRE_DECLARE_JSON_FRIEND_TWO(BoxGeometry)
};
}  // namespace sire::geometry
#endif