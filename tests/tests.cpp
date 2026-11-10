#include "Maze.hpp"
#include "Player.hpp"
#include <cassert>
#include <iostream>
#include <set>
int main(){
  std::set<std::string> layouts;
  for(uint32_t seed=1;seed<=150;seed++){
    Maze m(15,15,seed); assert(m.connected());assert(!m.path(0,m.exitIndex()).empty());assert(m.supplies().size()>=3&&m.supplies().size()<=5);
    for(int s:m.supplies()){assert(s!=0&&s!=m.exitIndex());assert(!m.path(0,s).empty());}
    assert(m.enemySpawns().size()>=14&&m.enemySpawns().size()<=20);
    auto flow=m.flowField(m.exitIndex());assert(flow[0]>0&&flow[m.exitIndex()]==0);
    int cursor=0,guard=0;while(cursor!=m.exitIndex()&&guard++<225){int next=m.flowNext(cursor,flow);assert(next!=cursor);assert(flow[next]<flow[cursor]);cursor=next;}assert(cursor==m.exitIndex());
    std::string sig;for(int y=0;y<m.height();y++)for(int x=0;x<m.width();x++)sig+=char(m.at(x,y).links);layouts.insert(sig);
  }
  assert(layouts.size()>145);
  Ammo a;for(int i=0;i<6;i++)assert(a.fire());assert(!a.fire());assert(a.reload()==6);assert(a.loaded==6&&a.reserve==6);for(int i=0;i<6;i++)a.fire();a.reload();for(int i=0;i<6;i++)a.fire();assert(a.total()==0&&!a.fire());
  Maze m(15,15,42);assert(m.walkable(m.worldCenter(0)));assert(!m.walkable({-4,-4}));
  std::cout<<"SEWERLINE logic tests passed (150 generated mazes).\n";
}
