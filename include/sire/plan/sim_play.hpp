#ifndef SIRE_SIM_PLAY_COMMAND_H_
#define SIRE_SIM_PLAY_COMMAND_H_

#include <aris.hpp>

namespace sire::plan {
class SimPlay
    : public aris::core::CloneObject<SimPlay, aris::plan::Plan> {
 public:
  auto virtual prepareNrt() -> void override;
  virtual ~SimPlay() = default;
  explicit SimPlay(
      const std::string& name = "SimReset");
  ARIS_DECLARE_BIG_FOUR(SimPlay);

 private:
  struct Imp;
  aris::core::ImpPtr<Imp> imp_;
};
}  // namespace sire::plan
#endif