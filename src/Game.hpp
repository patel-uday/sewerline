#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "Audio.hpp"
#include "Maze.hpp"
#include "Player.hpp"
#include "Renderer.hpp"
union SDL_Event;
struct SDL_Window;

class Game {
public:
  bool init(std::string& error); void shutdown(); bool event(const SDL_Event& e); bool tick();
private:
  enum class Screen{Menu,Controls,Playing,Paused,Dead,Won};
  enum class WeaponAnim{Idle,Fire,Melee,Call,Reload};
  struct Enemy{Vec2 pos{};int cell{};float hp{60},cooldown{},hurt{},phase{},stuck{};enum State{Idle,Chase,Attack,Dead}state{Idle};};
  struct Supply{int cell{},state{};float timer{},boxHeight{},boxVy{},phase{};};
  SDL_Window* window_{};Renderer renderer_;Audio audio_;std::unique_ptr<Maze>maze_;Player player_;std::vector<Enemy>enemies_;std::vector<Supply>supplies_;
  Screen screen_{Screen::Menu};WeaponAnim weapon_{WeaponAnim::Idle};float weaponTime_{},fireCooldown_{},meleeCooldown_{},runTime_{},subtitleTime_{},stepTime_{},creatureSoundCooldown_{},directorTimer_{},threat_{};std::string subtitle_;uint32_t seed_{};int kills_{},shots_{},flowTarget_{-1};uint64_t lastTicks_{};bool keys_[512]{};Vec2 playerVelocity_{};std::vector<int>flowField_;
  void newGame(uint32_t seed=0);void update(float dt);void render();void updateDirector(float dt);void updateEnemies(float dt);void updateSupplies(float dt);void movePlayer(float dt);void attack();void melee();bool clearShot(Vec2 a,Vec2 b)const;void setSubtitle(std::string s,float t);void menuInput(int key);float nearestSupplyDistance(int* index=nullptr)const;
};
