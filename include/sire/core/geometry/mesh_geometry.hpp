#ifndef SIRE_MESH_GEOMETRY_HPP_
#define SIRE_MESH_GEOMETRY_HPP_

#include <string>
#include <utility>

#include <aris/core/object.hpp>
#include <aris/dynamic/model_basic.hpp>
#include <aris/dynamic/model_coordinate.hpp>

#include "sire/core/geometry/geometry_on_part.hpp"
#include "sire/core/geometry/mesh_shape.hpp"
#include "sire/core/sire_decl_def_macro.hpp"
#include "sire/ext/json.hpp"

namespace sire::geometry {
using namespace std;
class MeshGeometry : public GeometryOnPart {
 public:
  MeshShape meshShape;
  explicit MeshGeometry(string resource_path = "");
  virtual ~MeshGeometry();
  ARIS_DECLARE_BIG_FOUR(MeshGeometry)

  // ���ڲ�ʹ�õ�to_json from_json������
  SIRE_DECLARE_JSON_INTER_OVERRIDE_TWO

  // nlohammn::json j = o;��ʱ����Զ����õ�to_json from_json������
  SIRE_DECLARE_JSON_FRIEND_TWO(MeshGeometry)
};
}  // namespace sire::geometry
#endif