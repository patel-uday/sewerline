#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Math.hpp"

enum Dir : uint8_t { North=1, East=2, South=4, West=8 };
struct Cell { uint8_t links{}; int distance{-1}; bool supply{}; bool exit{}; };

class Maze {
public:
  Maze(int width=15,int height=15,uint32_t seed=0);
  void generate(uint32_t seed);
  bool connected() const;
  bool canMove(int x,int y,Dir d) const;
  const Cell& at(int x,int y) const { return cells_[y*w_+x]; }
  Cell& at(int x,int y){ return cells_[y*w_+x]; }
  int width()const{return w_;} int height()const{return h_;}
  uint32_t seed()const{return seed_;}
  int exitIndex()const{return exit_;}
  std::pair<int,int> exitCell()const{return {exit_%w_,exit_/w_};}
  const std::vector<int>& supplies()const{return supplies_;}
  const std::vector<int>& enemySpawns()const{return enemies_;}
  std::vector<int> path(int from,int to)const;
  std::vector<int> flowField(int target)const;
  int flowNext(int from,const std::vector<int>& field)const;
  Vec2 worldCenter(int index)const;
  int nearestCell(Vec2 p)const;
  bool walkable(Vec2 p,float radius=.28f)const;
  static constexpr float Spacing=7.0f;
private:
  int w_,h_,exit_{}; uint32_t seed_{}; std::vector<Cell> cells_;
  std::vector<int> supplies_,enemies_;
  bool inside(int x,int y)const{return x>=0&&y>=0&&x<w_&&y<h_;}
  void distances();
};
