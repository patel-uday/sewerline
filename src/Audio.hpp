#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
struct SDL_AudioStream;
class Audio {
public:
  bool init();void shutdown();void play(int sound,float volume=1.f);void ambience(float dt);
  enum Sound{Shot,Reload,Melee,Hit,Leech,PlayerHurt,Ring,Radio,Drop,Pickup,Victory,Death,Step,SoundCount};
private:
  uint32_t device_{};std::array<SDL_AudioStream*,4> streams_{};uint32_t rate_{44100};
  std::array<std::vector<float>,SoundCount> clips_{};
  std::vector<float> alternateStep_{};bool alternateStepNext_{};
  bool loadClip(std::vector<float>& target,const std::string& relativePath);
  std::vector<float> synth(int sound);
};
