#ifndef SIRE_CONTACT_PAIR_MANAGER_HPP_
#define SIRE_CONTACT_PAIR_MANAGER_HPP_
#include <unordered_set>

#include <sire_lib_export.h>

#include <aris/core/object.hpp>

#include "sire/core/constants.hpp"
#include "sire/core/sorted_pair.hpp"
namespace sire::core {
enum class ContactPairState {
  START,
  CONTACTING,
  END,
};
enum class ContactPairType { SPLIT, STICK };
struct ContactPairValue {
  ContactPairState state;
  ContactPairType type;
  double init_penetration_depth_;
  bool is_depth_smaller_than_init_depth_;
  ContactPairValue(double init_penetration_depth = 0,
                   bool is_depth_smaller_than_init_depth = false)
      : state(ContactPairState::START),
        type(ContactPairType::SPLIT),
        init_penetration_depth_(init_penetration_depth),
        is_depth_smaller_than_init_depth_(is_depth_smaller_than_init_depth) {}
};

// ��Ҫ������
// 1. ������¼��Ҫ��Сʱ�䲽���������ֻ��Ҫ����partId
// 2.
// ������¼��ײ����ײ��ʼ�������ȥ��һ��ʼ�Ĺ�����µĻ��ֲ������ͻ�����������⣩
//    ע�⣺������������������֮ǰ������Ӧ�ö���������¼����Ϊ�ᵼ�²�������
//          ��� penetration - init_penetration < 0ʱ���͵����Ӵ��������ھͺ���
// 3.
// ������¼��ײ�㵼�µĹ������������ⲻ����ײ��prtû�б�inspect���Ϳ��Դӱ���
//    ɾ����Ӧ�ļ�¼
//
// ��һ��������������ײ�ܵ��ϴ�����Part
// ����������
// ��dt�ڣ���һ���ܴ�ĳ�� > impact_threshold�����������������
// 1. �����⵽δ�ڱ����м�¼���µ���ײ��������Դ�����ж�Ϊ��ײ����
// 2.
// ���û��⵽��Ҫ����������µ���ײ��������ж������������µĳ����Ӧ�ò������
//   ��������С������
// ���ϣ���⵽�µ���ײ��û���ڱ����ļ�¼�� && ����һ��dt�ڣ���� > threshold
//
// ɾ��������
// ��¼�е�prt����dt�ڣ�impactС�� impact_threshold2
//
// �����������¼��ײ��ĳ�ʼ����
// ����������
// ��⵽�µ���ײ��û���ڼ�¼��
// ��¼��ײpair�ͳ�ʼ�Ĵ��֮���⵽��������ײ��ʱ��penetration����Ҫ���������ʼ����
//
// ɾ��������
// ��ⲻ������ײ���Ϳ���ɾ��
//
// ������Ҫ��ֱ����ÿһ�ּ��Ľ���ͺ���
// ������������¼�����˽ϴ�������ײ��
// ����������
// ��ײ�������prt��inspection set�� and ��ײ��û�б�����
// ɾ��������
// ��ⲻ����ײ || ��ײ���prt������inspection set��
class SIRE_API ContactPairManager {
 public:
  // auto getContactingSet() const
  //     -> const std::unordered_set<core::SortedPair<sire::PartId>>&;
  auto insert(const sire::PartId ground, const sire::PartId id_B,
              double init_penetration_depth) -> void;
  auto insert(const core::SortedPair<sire::PartId>& pair,
              double init_penetration_depth) -> void;
  auto insert(const core::SortedPair<sire::PartId>& pair,
              const ContactPairValue& value) -> void;
  auto containsContactPair(const core::SortedPair<sire::PartId>& pair) const
      -> bool;
  auto hasImpactedPrt(sire::PartId prt_id) const -> bool;
  auto isImpactedSetEmpty() const -> bool;
  auto impactedContactSet() -> std::unordered_set<SortedPair<sire::PartId>>&;
  auto contactPairMap()
      -> std::unordered_map<SortedPair<sire::PartId>, ContactPairValue>&;
  auto impactedPrtSet() -> std::unordered_set<sire::PartId>&;
  auto getValue(const core::SortedPair<sire::PartId>& pair) const
      -> const ContactPairValue&;
  auto getValue(const core::SortedPair<sire::PartId>& pair)
      -> ContactPairValue& {
    return const_cast<ContactPairValue&>(
        static_cast<const ContactPairManager*>(this)->getValue(pair));
  }
  auto setValue(const core::SortedPair<sire::PartId>& pair,
                const ContactPairValue& value) -> void;
  // auto setState(const core::SortedPair<sire::PartId>& pair, ContactPairState
  // state)
  //     -> void;
  // auto setType(const core::SortedPair<sire::PartId>& pair, ContactPairType
  // type)
  //     -> void;
  auto clear() -> void;
  auto init() -> void;
  ContactPairManager();
  ~ContactPairManager();
  ARIS_DECLARE_BIG_FOUR(ContactPairManager);

 private:
  struct Imp;
  aris::core::ImpPtr<Imp> imp_;
};
}  // namespace sire::core
#endif