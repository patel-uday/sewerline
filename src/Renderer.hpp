#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Math.hpp"

struct SDL_Window;
class Maze;
struct Player;
struct EnemyView { Vec2 pos; int state{}; float phase{}; bool dead{}; };
struct SupplyView { Vec2 pos; int state{}; float boxHeight{}; float phase{}; };

class Renderer {
public:
  bool init(SDL_Window* window,std::string& error);
  void shutdown();
  void buildLevel(const Maze& maze);
  void begin3D(const Player& player,float time);
  void drawWorld(const Maze& maze,const std::vector<EnemyView>& enemies,const std::vector<SupplyView>& supplies,const Player& player,float muzzle);
  void beginUI();
  void rect(float x,float y,float w,float h,uint32_t rgba);
  void text(float x,float y,float scale,const std::string& s,uint32_t rgba,bool centered=false);
  void weapon(int animation,float t,float bob,bool unarmed);
  void present();
  int width()const{return 426;} int height()const{return 240;}
private:
  struct Vertex{float x,y,z,nx,ny,nz,u,v;};
  SDL_Window* window_{}; void* context_{};
  unsigned program_{},vao_{},vbo_{},worldTex_{},spriteTex_{},leechTex_{},weaponTex_{},fistsIdleTex_{},fistPunchTex_{},phoneTex_{},uiTex_{},fbo_{},colorTex_{},depth_{};
  int uMvp_{},uModel_{},uTint_{},uFog_{},uMode_{}; size_t worldVerts_{};
  Mat4 viewProj_{}; Vec2 cameraPos_{}; Vec2 cameraRight_{1.f,0.f}; std::vector<Vertex> mesh_;
  bool loadGL(std::string& error); bool makeProgram(std::string& error); bool makeTextures(std::string& error);void makeFramebuffer();
  unsigned loadPngTexture(const std::string& relativePath,bool repeat,std::string& error);
  void upload(const std::vector<Vertex>& v);void drawQuad3D(Vec3 p,float w,float h,int tile,uint32_t tint,unsigned texture=0,int columns=4,int rows=4);void drawCeilingOctagon(Vec2 p);void pushTunnel(Vec2 a,Vec2 b);
  void sprite2D(float x,float y,float w,float h,unsigned texture,int tile,int columns,int rows,uint32_t tint=0xffffffff,bool flipX=false,float rotation=0.f,float cropBottom=0.f);
};
