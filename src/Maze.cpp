#include "Maze.hpp"
#include <algorithm>
#include <chrono>
#include <queue>
#include <random>
#include <stdexcept>

namespace { constexpr int DX[4]={0,1,0,-1}, DY[4]={-1,0,1,0}; constexpr Dir DS[4]={North,East,South,West}, OP[4]={South,West,North,East}; }

Maze::Maze(int width,int height,uint32_t seed):w_(width),h_(height){
  if(w_<5||h_<5) throw std::invalid_argument("maze must be at least 5x5"); generate(seed);
}
void Maze::generate(uint32_t seed){
  seed_=seed?seed:static_cast<uint32_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
  std::mt19937 rng(seed_); cells_.assign(w_*h_,{}); supplies_.clear(); enemies_.clear();
  std::vector<int> stack{0}; std::vector<bool> seen(cells_.size()); seen[0]=true;
  while(!stack.empty()){
    int cur=stack.back(),x=cur%w_,y=cur/w_; std::vector<int> dirs;
    for(int d=0;d<4;d++){int nx=x+DX[d],ny=y+DY[d];if(inside(nx,ny)&&!seen[ny*w_+nx])dirs.push_back(d);}
    if(dirs.empty()){stack.pop_back();continue;}
    int d=dirs[rng()%dirs.size()],n=(y+DY[d])*w_+x+DX[d]; cells_[cur].links|=DS[d];cells_[n].links|=OP[d];seen[n]=true;stack.push_back(n);
  }
  // Sparse loops make pursuit and navigation less brittle without erasing maze character.
  int loops=std::max(3,(w_*h_)/28);
  for(int i=0;i<loops;i++)for(int tries=0;tries<30;tries++){
    int x=rng()%w_,y=rng()%h_,d=rng()%4,nx=x+DX[d],ny=y+DY[d]; if(!inside(nx,ny)||cells_[y*w_+x].links&DS[d])continue;
    cells_[y*w_+x].links|=DS[d];cells_[ny*w_+nx].links|=OP[d];break;
  }
  distances(); exit_=int(std::max_element(cells_.begin(),cells_.end(),[](auto&a,auto&b){return a.distance<b.distance;})-cells_.begin()); cells_[exit_].exit=true;
  std::vector<int> candidates;for(int i=0;i<(int)cells_.size();i++)if(cells_[i].distance>4&&i!=exit_)candidates.push_back(i);
  std::shuffle(candidates.begin(),candidates.end(),rng);
  int supplyCount=std::clamp((w_*h_)/55,3,5);
  for(int i:candidates){bool far=true;for(int s:supplies_){auto a=worldCenter(i),b=worldCenter(s);if(length(a-b)<Spacing*4)far=false;}if(far){supplies_.push_back(i);cells_[i].supply=true;if((int)supplies_.size()==supplyCount)break;}}
  for(int i:candidates){if(cells_[i].supply||i==exit_||cells_[i].distance<5)continue;enemies_.push_back(i);if((int)enemies_.size()>=std::clamp(w_*h_/13,14,20))break;}
  if(!connected()||supplies_.size()<3) throw std::runtime_error("maze validation failed");
}
void Maze::distances(){for(auto&c:cells_)c.distance=-1;std::queue<int>q;q.push(0);cells_[0].distance=0;while(!q.empty()){int i=q.front();q.pop();int x=i%w_,y=i/w_;for(int d=0;d<4;d++)if(cells_[i].links&DS[d]){int n=(y+DY[d])*w_+x+DX[d];if(cells_[n].distance<0){cells_[n].distance=cells_[i].distance+1;q.push(n);}}}}
bool Maze::connected()const{return std::all_of(cells_.begin(),cells_.end(),[](auto&c){return c.distance>=0;})&&cells_[exit_].distance>0;}
bool Maze::canMove(int x,int y,Dir d)const{return inside(x,y)&&(at(x,y).links&d);}
std::vector<int> Maze::path(int from,int to)const{std::vector<int>prev(cells_.size(),-1);std::queue<int>q;q.push(from);prev[from]=from;while(!q.empty()){int i=q.front();q.pop();if(i==to)break;int x=i%w_,y=i/w_;for(int d=0;d<4;d++)if(cells_[i].links&DS[d]){int n=(y+DY[d])*w_+x+DX[d];if(prev[n]<0){prev[n]=i;q.push(n);}}}std::vector<int>r;if(prev[to]<0)return r;for(int i=to;;i=prev[i]){r.push_back(i);if(i==from)break;}std::reverse(r.begin(),r.end());return r;}
std::vector<int> Maze::flowField(int target)const{
  std::vector<int> field(cells_.size(),-1);
  if(target<0||target>=(int)cells_.size())return field;
  std::queue<int>q;q.push(target);field[target]=0;
  while(!q.empty()){
    int i=q.front();q.pop();int x=i%w_,y=i/w_;
    for(int d=0;d<4;d++)if(cells_[i].links&DS[d]){
      int n=(y+DY[d])*w_+x+DX[d];
      if(field[n]<0){field[n]=field[i]+1;q.push(n);}
    }
  }
  return field;
}
int Maze::flowNext(int from,const std::vector<int>&field)const{
  if(from<0||from>=(int)cells_.size()||field.size()!=cells_.size()||field[from]<=0)return from;
  int best=from,x=from%w_,y=from/w_;
  for(int d=0;d<4;d++)if(cells_[from].links&DS[d]){
    int n=(y+DY[d])*w_+x+DX[d];
    if(field[n]>=0&&field[n]<field[best])best=n;
  }
  return best;
}
Vec2 Maze::worldCenter(int index)const{return {(index%w_)*Spacing,(index/w_)*Spacing};}
int Maze::nearestCell(Vec2 p)const{int x=std::clamp(int(std::round(p.x/Spacing)),0,w_-1),y=std::clamp(int(std::round(p.y/Spacing)),0,h_-1);return y*w_+x;}
bool Maze::walkable(Vec2 p,float radius)const{
  int i=nearestCell(p),x=i%w_,y=i/w_;Vec2 c=worldCenter(i),d=p-c;float chamber=2.45f-radius,corridor=1.42f-radius;
  if(std::abs(d.x)<=chamber&&std::abs(d.y)<=chamber)return true;
  if(std::abs(d.x)<=corridor){if(d.y<0&&canMove(x,y,North)&&d.y>=-Spacing*.55f)return true;if(d.y>0&&canMove(x,y,South)&&d.y<=Spacing*.55f)return true;}
  if(std::abs(d.y)<=corridor){if(d.x<0&&canMove(x,y,West)&&d.x>=-Spacing*.55f)return true;if(d.x>0&&canMove(x,y,East)&&d.x<=Spacing*.55f)return true;}
  return false;
}
