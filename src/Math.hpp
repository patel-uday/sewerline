#pragma once
#include <cmath>
#include <algorithm>

struct Vec2 { float x{}, y{}; };
struct Vec3 { float x{}, y{}, z{}; };
inline Vec2 operator+(Vec2 a, Vec2 b){ return {a.x+b.x,a.y+b.y}; }
inline Vec2 operator-(Vec2 a, Vec2 b){ return {a.x-b.x,a.y-b.y}; }
inline Vec2 operator*(Vec2 a, float s){ return {a.x*s,a.y*s}; }
inline float dot(Vec2 a, Vec2 b){ return a.x*b.x+a.y*b.y; }
inline float length(Vec2 a){ return std::sqrt(dot(a,a)); }
inline Vec2 normalized(Vec2 a){ float l=length(a); return l>.0001f?a*(1.f/l):Vec2{}; }
inline Vec3 operator+(Vec3 a, Vec3 b){ return {a.x+b.x,a.y+b.y,a.z+b.z}; }
inline Vec3 operator-(Vec3 a, Vec3 b){ return {a.x-b.x,a.y-b.y,a.z-b.z}; }
inline Vec3 operator*(Vec3 a,float s){ return {a.x*s,a.y*s,a.z*s}; }
inline float dot(Vec3 a,Vec3 b){return a.x*b.x+a.y*b.y+a.z*b.z;}
inline Vec3 cross(Vec3 a,Vec3 b){return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};}
inline Vec3 normalized(Vec3 a){float l=std::sqrt(dot(a,a));return l>.0001f?a*(1.f/l):Vec3{};}

struct Mat4 { float m[16]{}; };
inline Mat4 identity(){Mat4 r{};r.m[0]=r.m[5]=r.m[10]=r.m[15]=1;return r;}
inline Mat4 mul(const Mat4&a,const Mat4&b){Mat4 r{};for(int c=0;c<4;c++)for(int row=0;row<4;row++)for(int k=0;k<4;k++)r.m[c*4+row]+=a.m[k*4+row]*b.m[c*4+k];return r;}
inline Mat4 perspective(float fov,float aspect,float n,float f){Mat4 r{};float q=1/std::tan(fov*.5f);r.m[0]=q/aspect;r.m[5]=q;r.m[10]=(f+n)/(n-f);r.m[11]=-1;r.m[14]=(2*f*n)/(n-f);return r;}
inline Mat4 lookAt(Vec3 eye,Vec3 at,Vec3 up){Vec3 f=normalized(at-eye),s=normalized(cross(f,up)),u=cross(s,f);Mat4 r=identity();r.m[0]=s.x;r.m[4]=s.y;r.m[8]=s.z;r.m[1]=u.x;r.m[5]=u.y;r.m[9]=u.z;r.m[2]=-f.x;r.m[6]=-f.y;r.m[10]=-f.z;r.m[12]=-dot(s,eye);r.m[13]=-dot(u,eye);r.m[14]=dot(f,eye);return r;}
