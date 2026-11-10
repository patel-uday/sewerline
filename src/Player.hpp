#pragma once
#include "Math.hpp"

struct Ammo { int loaded{6},reserve{12}; static constexpr int Capacity=6; bool fire(){if(loaded<=0)return false;--loaded;return true;}int reload(){int n=std::min(Capacity-loaded,reserve);loaded+=n;reserve-=n;return n;}int total()const{return loaded+reserve;} };
struct Player { Vec2 pos{}; float yaw{},pitch{}; int health{100}; Ammo ammo{}; float bob{},hurtFlash{}; };
