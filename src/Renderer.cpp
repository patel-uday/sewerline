#include "Renderer.hpp"
#include "Maze.hpp"
#include "Player.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#define NOMINMAX
#include <windows.h>
#include <wincodec.h>
#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstring>
#include "Assets.hpp"
#include <fstream>
#include <sstream>

#ifndef GL_VERTEX_SHADER
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_TEXTURE0 0x84C0
#define GL_FRAMEBUFFER 0x8D40
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_RENDERBUFFER 0x8D41
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_DEPTH_COMPONENT24 0x81A6
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_RGBA8 0x8058
#define GL_RGB8 0x8051
#endif

namespace {
using P=void(*)();
#define GLPROC(ret,name,args) static ret (APIENTRY *p##name) args=nullptr
GLPROC(GLuint,glCreateShader,(GLenum));GLPROC(void,glShaderSource,(GLuint,GLsizei,const GLchar*const*,const GLint*));GLPROC(void,glCompileShader,(GLuint));GLPROC(void,glGetShaderiv,(GLuint,GLenum,GLint*));GLPROC(void,glGetShaderInfoLog,(GLuint,GLsizei,GLsizei*,GLchar*));GLPROC(GLuint,glCreateProgram,(void));GLPROC(void,glAttachShader,(GLuint,GLuint));GLPROC(void,glLinkProgram,(GLuint));GLPROC(void,glGetProgramiv,(GLuint,GLenum,GLint*));GLPROC(void,glGetProgramInfoLog,(GLuint,GLsizei,GLsizei*,GLchar*));GLPROC(void,glDeleteShader,(GLuint));GLPROC(void,glDeleteProgram,(GLuint));GLPROC(void,glUseProgram,(GLuint));GLPROC(void,glGenVertexArrays,(GLsizei,GLuint*));GLPROC(void,glBindVertexArray,(GLuint));GLPROC(void,glDeleteVertexArrays,(GLsizei,const GLuint*));GLPROC(void,glGenBuffers,(GLsizei,GLuint*));GLPROC(void,glBindBuffer,(GLenum,GLuint));GLPROC(void,glBufferData,(GLenum,ptrdiff_t,const void*,GLenum));GLPROC(void,glDeleteBuffers,(GLsizei,const GLuint*));GLPROC(void,glEnableVertexAttribArray,(GLuint));GLPROC(void,glVertexAttribPointer,(GLuint,GLint,GLenum,GLboolean,GLsizei,const void*));GLPROC(GLint,glGetUniformLocation,(GLuint,const GLchar*));GLPROC(void,glUniformMatrix4fv,(GLint,GLsizei,GLboolean,const GLfloat*));GLPROC(void,glUniform4f,(GLint,GLfloat,GLfloat,GLfloat,GLfloat));GLPROC(void,glUniform3f,(GLint,GLfloat,GLfloat,GLfloat));GLPROC(void,glUniform1i,(GLint,GLint));GLPROC(void,glActiveTexture,(GLenum));GLPROC(void,glGenFramebuffers,(GLsizei,GLuint*));GLPROC(void,glBindFramebuffer,(GLenum,GLuint));GLPROC(void,glFramebufferTexture2D,(GLenum,GLenum,GLenum,GLuint,GLint));GLPROC(void,glDeleteFramebuffers,(GLsizei,const GLuint*));GLPROC(void,glGenRenderbuffers,(GLsizei,GLuint*));GLPROC(void,glBindRenderbuffer,(GLenum,GLuint));GLPROC(void,glRenderbufferStorage,(GLenum,GLenum,GLsizei,GLsizei));GLPROC(void,glFramebufferRenderbuffer,(GLenum,GLenum,GLenum,GLuint));GLPROC(GLenum,glCheckFramebufferStatus,(GLenum));GLPROC(void,glDeleteRenderbuffers,(GLsizei,const GLuint*));GLPROC(void,glGenerateMipmap,(GLenum));
#undef GLPROC

uint32_t px(uint8_t r,uint8_t g,uint8_t b,uint8_t a=255){return uint32_t(r)|(uint32_t(g)<<8)|(uint32_t(b)<<16)|(uint32_t(a)<<24);}
std::array<uint8_t,7> glyph(char c){
  // Compact 5x7 rows. Unlisted punctuation deliberately renders as a small block.
  switch(c){
#define G(ch,a,b,c,d,e,f,g) case ch:return {a,b,c,d,e,f,g}
 G('A',14,17,17,31,17,17,17);G('B',30,17,17,30,17,17,30);G('C',14,17,16,16,16,17,14);G('D',30,17,17,17,17,17,30);G('E',31,16,16,30,16,16,31);G('F',31,16,16,30,16,16,16);G('G',14,17,16,23,17,17,15);G('H',17,17,17,31,17,17,17);G('I',31,4,4,4,4,4,31);G('J',7,2,2,2,18,18,12);G('K',17,18,20,24,20,18,17);G('L',16,16,16,16,16,16,31);G('M',17,27,21,21,17,17,17);G('N',17,25,21,19,17,17,17);G('O',14,17,17,17,17,17,14);G('P',30,17,17,30,16,16,16);G('Q',14,17,17,17,21,18,13);G('R',30,17,17,30,20,18,17);G('S',15,16,16,14,1,1,30);G('T',31,4,4,4,4,4,4);G('U',17,17,17,17,17,17,14);G('V',17,17,17,17,17,10,4);G('W',17,17,17,21,21,21,10);G('X',17,17,10,4,10,17,17);G('Y',17,17,10,4,4,4,4);G('Z',31,1,2,4,8,16,31);
 G('0',14,17,19,21,25,17,14);G('1',4,12,4,4,4,4,14);G('2',14,17,1,2,4,8,31);G('3',30,1,1,14,1,1,30);G('4',2,6,10,18,31,2,2);G('5',31,16,16,30,1,1,30);G('6',14,16,16,30,17,17,14);G('7',31,1,2,4,8,8,8);G('8',14,17,17,14,17,17,14);G('9',14,17,17,15,1,1,14);G(':',0,4,4,0,4,4,0);G('/',1,2,4,8,16,0,0);G('-',0,0,0,31,0,0,0);G('[',14,8,8,8,8,8,14);G(']',14,2,2,2,2,2,14);G('.',0,0,0,0,0,12,12);G('!',4,4,4,4,4,0,4);G(' ',0,0,0,0,0,0,0);
#undef G
  default:return {0,0,4,0,4,0,0};}
}
}

bool Renderer::loadGL(std::string& error){
#define LOAD(name) p##name=reinterpret_cast<decltype(p##name)>(SDL_GL_GetProcAddress(#name));if(!p##name){error="Missing OpenGL function: " #name;return false;}
 LOAD(glCreateShader);LOAD(glShaderSource);LOAD(glCompileShader);LOAD(glGetShaderiv);LOAD(glGetShaderInfoLog);LOAD(glCreateProgram);LOAD(glAttachShader);LOAD(glLinkProgram);LOAD(glGetProgramiv);LOAD(glGetProgramInfoLog);LOAD(glDeleteShader);LOAD(glDeleteProgram);LOAD(glUseProgram);LOAD(glGenVertexArrays);LOAD(glBindVertexArray);LOAD(glDeleteVertexArrays);LOAD(glGenBuffers);LOAD(glBindBuffer);LOAD(glBufferData);LOAD(glDeleteBuffers);LOAD(glEnableVertexAttribArray);LOAD(glVertexAttribPointer);LOAD(glGetUniformLocation);LOAD(glUniformMatrix4fv);LOAD(glUniform4f);LOAD(glUniform3f);LOAD(glUniform1i);LOAD(glActiveTexture);LOAD(glGenFramebuffers);LOAD(glBindFramebuffer);LOAD(glFramebufferTexture2D);LOAD(glDeleteFramebuffers);LOAD(glGenRenderbuffers);LOAD(glBindRenderbuffer);LOAD(glRenderbufferStorage);LOAD(glFramebufferRenderbuffer);LOAD(glCheckFramebufferStatus);LOAD(glDeleteRenderbuffers);LOAD(glGenerateMipmap);
#undef LOAD
 return true;
}
bool Renderer::init(SDL_Window* window,std::string& error){window_=window;SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,3);SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);context_=SDL_GL_CreateContext(window);if(!context_){error=SDL_GetError();return false;}if(!loadGL(error)||!makeProgram(error))return false;SDL_GL_SetSwapInterval(1);pglGenVertexArrays(1,&vao_);pglBindVertexArray(vao_);pglGenBuffers(1,&vbo_);pglBindBuffer(GL_ARRAY_BUFFER,vbo_);for(int i=0;i<3;i++){pglEnableVertexAttribArray(i);pglVertexAttribPointer(i,i==2?2:3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(uintptr_t)(i==0?0:i==1?12:24));}if(!makeTextures(error))return false;makeFramebuffer();return true;}
bool Renderer::makeProgram(std::string& error){
 const char*vs="#version 330 core\nlayout(location=0)in vec3 aPos;layout(location=1)in vec3 aNormal;layout(location=2)in vec2 aUV;uniform mat4 uMVP;uniform mat4 uModel;out vec2 vUV;out vec3 vNormal;out float vDepth;void main(){vec4 w=uModel*vec4(aPos,1);gl_Position=uMVP*w;vUV=aUV;vNormal=mat3(uModel)*aNormal;vDepth=gl_Position.w;}";
 const char*fs="#version 330 core\nin vec2 vUV;in vec3 vNormal;in float vDepth;out vec4 FragColor;uniform sampler2D uTex;uniform vec4 uTint;uniform vec3 uFog;uniform int uMode;void main(){vec4 t=texture(uTex,vUV);if(t.a<.1)discard;float l=uMode==0?(.38+.52*max(0.,dot(normalize(vNormal),normalize(vec3(.3,.8,.2))))):1.;vec4 c=t*uTint*vec4(vec3(l),1);float f=clamp((vDepth-8.)/36.,0.,.9);FragColor=vec4(mix(c.rgb,uFog,f),c.a);}";
 auto compile=[&](GLenum type,const char*s){GLuint sh=pglCreateShader(type);pglShaderSource(sh,1,&s,nullptr);pglCompileShader(sh);GLint ok;pglGetShaderiv(sh,GL_COMPILE_STATUS,&ok);if(!ok){char log[1024];pglGetShaderInfoLog(sh,1024,nullptr,log);error=log;}return sh;};GLuint v=compile(GL_VERTEX_SHADER,vs),f=compile(GL_FRAGMENT_SHADER,fs);if(!error.empty())return false;program_=pglCreateProgram();pglAttachShader(program_,v);pglAttachShader(program_,f);pglLinkProgram(program_);GLint ok;pglGetProgramiv(program_,GL_LINK_STATUS,&ok);pglDeleteShader(v);pglDeleteShader(f);if(!ok){char log[1024];pglGetProgramInfoLog(program_,1024,nullptr,log);error=log;return false;}pglUseProgram(program_);uMvp_=pglGetUniformLocation(program_,"uMVP");uModel_=pglGetUniformLocation(program_,"uModel");uTint_=pglGetUniformLocation(program_,"uTint");uFog_=pglGetUniformLocation(program_,"uFog");uMode_=pglGetUniformLocation(program_,"uMode");pglUniform1i(pglGetUniformLocation(program_,"uTex"),0);return true;}
unsigned Renderer::loadPngTexture(const std::string& relativePath,bool repeat,std::string& error){
 auto asset=embeddedAsset(relativePath);if(!asset){error="Missing embedded asset: "+relativePath;return 0;}IWICImagingFactory*factory=nullptr;IWICStream*stream=nullptr;IWICBitmapDecoder*decoder=nullptr;IWICBitmapFrameDecode*frame=nullptr;IWICFormatConverter*converter=nullptr;CoInitializeEx(nullptr,COINIT_MULTITHREADED);HRESULT hr=CoCreateInstance(CLSID_WICImagingFactory,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&factory));if(SUCCEEDED(hr))hr=factory->CreateStream(&stream);if(SUCCEEDED(hr))hr=stream->InitializeFromMemory((BYTE*)asset.data,(DWORD)asset.size);if(SUCCEEDED(hr))hr=factory->CreateDecoderFromStream(stream,nullptr,WICDecodeMetadataCacheOnLoad,&decoder);if(SUCCEEDED(hr))hr=decoder->GetFrame(0,&frame);if(SUCCEEDED(hr))hr=factory->CreateFormatConverter(&converter);if(SUCCEEDED(hr))hr=converter->Initialize(frame,GUID_WICPixelFormat32bppRGBA,WICBitmapDitherTypeNone,nullptr,0,WICBitmapPaletteTypeCustom);UINT w=0,h=0;if(SUCCEEDED(hr))hr=converter->GetSize(&w,&h);std::vector<uint8_t>pixels;if(SUCCEEDED(hr)){pixels.resize(size_t(w)*h*4);hr=converter->CopyPixels(nullptr,w*4,(UINT)pixels.size(),pixels.data());}if(converter)converter->Release();if(frame)frame->Release();if(decoder)decoder->Release();if(stream)stream->Release();if(factory)factory->Release();if(FAILED(hr)){error="Could not load PNG asset: "+relativePath;return 0;}for(UINT y=0;y<h/2;y++)for(UINT x=0;x<w*4;x++)std::swap(pixels[size_t(y)*w*4+x],pixels[size_t(h-1-y)*w*4+x]);GLuint tex=0;glGenTextures(1,&tex);glBindTexture(GL_TEXTURE_2D,tex);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,(GLsizei)w,(GLsizei)h,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,repeat?GL_REPEAT:GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,repeat?GL_REPEAT:GL_CLAMP_TO_EDGE);return tex;
}
bool Renderer::makeTextures(std::string& error){
 worldTex_=loadPngTexture("assets/textures/sewer_brick.png",true,error);if(!worldTex_)return false;leechTex_=loadPngTexture("assets/sprites/leech_sheet.png",false,error);if(!leechTex_)return false;weaponTex_=loadPngTexture("assets/sprites/pistol_sheet.png",false,error);if(!weaponTex_)return false;fistsIdleTex_=loadPngTexture("assets/sprites/fists_idle.png",false,error);if(!fistsIdleTex_)return false;fistPunchTex_=loadPngTexture("assets/sprites/fist_punch.png",false,error);if(!fistPunchTex_)return false;phoneTex_=loadPngTexture("assets/sprites/smartphone_call.png",false,error);if(!phoneTex_)return false;
 std::vector<uint32_t> s(64*64,0);auto set=[&](int tile,int x,int y,uint32_t c){int ox=(tile%4)*16,oy=(tile/4)*16;if(x>=0&&y>=0&&x<16&&y<16)s[(oy+y)*64+ox+x]=c;};
 for(int t=0;t<5;t++)for(int y=3;y<14;y++)for(int x=2;x<14;x++){float dx=(x-8)/6.f,dy=(y-9)/5.f;if(dx*dx+dy*dy<1){uint8_t q=uint8_t(55+(x+y+t*5)%18);set(t,x,y,px(q,72-(t%2)*12,36));if(y>6&&y<11&&x>5&&x<11)set(t,x,y,px(24,13,11));if((x==6||x==10)&&y==8)set(t,x,y,px(205,190,130));}}
 for(int y=5;y<13;y++)for(int x=3;x<13;x++)set(5,x,y,px(126,75,25));for(int x=4;x<12;x++){set(5,x,6,px(210,149,42));set(5,x,10,px(54,36,19));}
 // A stark octagonal supply sign: pale rim, black center, amber ammunition bars.
 for(int y=1;y<15;y++)for(int x=1;x<15;x++){
  bool outer=(x>=3&&x<=12&&y>=1&&y<=14)||(x>=1&&x<=14&&y>=3&&y<=12);
  bool inner=(x>=4&&x<=11&&y>=2&&y<=13)||(x>=2&&x<=13&&y>=4&&y<=11);
  if(outer)set(6,x,y,px(224,218,194));
  if(inner)set(6,x,y,px(4,5,4));
 }
 for(int x:{5,8,11})for(int y=5;y<=10;y++)set(6,x,y,px(216,146,43));
 for(int x:{5,8,11})set(6,x,4,px(255,219,105));
 for(int y=2;y<15;y++)for(int x=6;x<10;x++)set(7,x,y,px(206,179,83));
 glGenTextures(1,&spriteTex_);glBindTexture(GL_TEXTURE_2D,spriteTex_);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,64,64,0,GL_RGBA,GL_UNSIGNED_BYTE,s.data());glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
 uint32_t white=0xffffffff;glGenTextures(1,&uiTex_);glBindTexture(GL_TEXTURE_2D,uiTex_);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,&white);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);return true;
}
void Renderer::makeFramebuffer(){glGenTextures(1,&colorTex_);glBindTexture(GL_TEXTURE_2D,colorTex_);glTexImage2D(GL_TEXTURE_2D,0,GL_RGB8,width(),height(),0,GL_RGB,GL_UNSIGNED_BYTE,nullptr);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);pglGenRenderbuffers(1,&depth_);pglBindRenderbuffer(GL_RENDERBUFFER,depth_);pglRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,width(),height());pglGenFramebuffers(1,&fbo_);pglBindFramebuffer(GL_FRAMEBUFFER,fbo_);pglFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,colorTex_,0);pglFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depth_);pglBindFramebuffer(GL_FRAMEBUFFER,0);}
void Renderer::shutdown(){if(!context_)return;if(fbo_)pglDeleteFramebuffers(1,&fbo_);if(depth_)pglDeleteRenderbuffers(1,&depth_);if(worldTex_)glDeleteTextures(1,&worldTex_);if(spriteTex_)glDeleteTextures(1,&spriteTex_);if(leechTex_)glDeleteTextures(1,&leechTex_);if(weaponTex_)glDeleteTextures(1,&weaponTex_);if(fistsIdleTex_)glDeleteTextures(1,&fistsIdleTex_);if(fistPunchTex_)glDeleteTextures(1,&fistPunchTex_);if(phoneTex_)glDeleteTextures(1,&phoneTex_);if(uiTex_)glDeleteTextures(1,&uiTex_);if(colorTex_)glDeleteTextures(1,&colorTex_);if(vbo_)pglDeleteBuffers(1,&vbo_);if(vao_)pglDeleteVertexArrays(1,&vao_);if(program_)pglDeleteProgram(program_);SDL_GL_DestroyContext((SDL_GLContext)context_);context_=nullptr;}
void Renderer::upload(const std::vector<Vertex>&v){pglBindBuffer(GL_ARRAY_BUFFER,vbo_);pglBufferData(GL_ARRAY_BUFFER,ptrdiff_t(v.size()*sizeof(Vertex)),v.data(),GL_DYNAMIC_DRAW);}
void Renderer::pushTunnel(Vec2 a,Vec2 b){Vec2 f=normalized(b-a),r={-f.y,f.x};const Vec2 ring[8]={{-1.45f,.05f},{-1.45f,2.35f},{-1.f,3.1f},{0,3.45f},{1,3.1f},{1.45f,2.35f},{1.45f,.05f},{0,-.08f}};float len=length(b-a);for(int k=0;k<8;k++){int n=(k+1)%8;Vec3 p[4]={{a.x+r.x*ring[k].x,ring[k].y,a.y+r.y*ring[k].x},{b.x+r.x*ring[k].x,ring[k].y,b.y+r.y*ring[k].x},{b.x+r.x*ring[n].x,ring[n].y,b.y+r.y*ring[n].x},{a.x+r.x*ring[n].x,ring[n].y,a.y+r.y*ring[n].x}};Vec3 norm=normalized(cross(p[1]-p[0],p[3]-p[0]));float v0=k/8.f*2.5f,v1=(k+1)/8.f*2.5f;Vertex q[4]={{p[0].x,p[0].y,p[0].z,norm.x,norm.y,norm.z,0,v0},{p[1].x,p[1].y,p[1].z,norm.x,norm.y,norm.z,len/2.f,v0},{p[2].x,p[2].y,p[2].z,norm.x,norm.y,norm.z,len/2.f,v1},{p[3].x,p[3].y,p[3].z,norm.x,norm.y,norm.z,0,v1}};for(int i:{0,1,2,0,2,3})mesh_.push_back(q[i]);}}
void Renderer::buildLevel(const Maze& m){
 mesh_.clear();
 auto quad=[&](Vec3 a,Vec3 b,Vec3 c,Vec3 d,Vec3 n,float uScale=2.f,float vScale=2.f){Vertex q[4]={{a.x,a.y,a.z,n.x,n.y,n.z,0,0},{b.x,b.y,b.z,n.x,n.y,n.z,uScale,0},{c.x,c.y,c.z,n.x,n.y,n.z,uScale,vScale},{d.x,d.y,d.z,n.x,n.y,n.z,0,vScale}};for(int k:{0,1,2,0,2,3})mesh_.push_back(q[k]);};
 constexpr float r=2.48f,roof=3.45f;
 for(int y=0;y<m.height();y++)for(int x=0;x<m.width();x++){
  int i=y*m.width()+x;Vec2 c=m.worldCenter(i);
  if(m.canMove(x,y,East))pushTunnel({c.x+r,c.y},{c.x+Maze::Spacing-r,c.y});
  if(m.canMove(x,y,South))pushTunnel({c.x,c.y+r},{c.x,c.y+Maze::Spacing-r});
  quad({c.x-r,0,c.y-r},{c.x+r,0,c.y-r},{c.x+r,0,c.y+r},{c.x-r,0,c.y+r},{0,1,0},2.4f,2.4f);
  quad({c.x-r,roof,c.y+r},{c.x+r,roof,c.y+r},{c.x+r,roof,c.y-r},{c.x-r,roof,c.y-r},{0,-1,0},2.4f,2.4f);
  auto point=[&](int dir,float lateral,float height){if(dir==0)return Vec3{c.x+lateral,height,c.y-r};if(dir==1)return Vec3{c.x+r,height,c.y+lateral};if(dir==2)return Vec3{c.x-lateral,height,c.y+r};return Vec3{c.x-r,height,c.y-lateral};};
  auto normal=[&](int dir){return dir==0?Vec3{0,0,1}:dir==1?Vec3{-1,0,0}:dir==2?Vec3{0,0,-1}:Vec3{1,0,0};};
  auto wallPanel=[&](int dir,float s0,float y0,float s1,float y1){quad(point(dir,s0,y0),point(dir,s1,y1),point(dir,s1,roof),point(dir,s0,roof),normal(dir),std::max(.3f,(s1-s0)/2.f),1.8f);};
  for(int dir=0;dir<4;dir++){
   Dir d=dir==0?North:dir==1?East:dir==2?South:West;
   if(!m.canMove(x,y,d)){quad(point(dir,-r,0),point(dir,r,0),point(dir,r,roof),point(dir,-r,roof),normal(dir),2.4f,1.8f);continue;}
   // An octagonal portal ring seals every tube into its junction. The six panels
   // exactly follow the tunnel cross-section, eliminating the former black voids.
   wallPanel(dir,-r,0,-1.45f,0);wallPanel(dir,1.45f,0,r,0);
   wallPanel(dir,-1.45f,2.35f,-1.f,3.1f);wallPanel(dir,-1.f,3.1f,0,roof);
   wallPanel(dir,0,roof,1.f,3.1f);wallPanel(dir,1.f,3.1f,1.45f,2.35f);
  }
 }
 worldVerts_=mesh_.size();upload(mesh_);
}
void Renderer::begin3D(const Player&p,float){pglBindFramebuffer(GL_FRAMEBUFFER,fbo_);glViewport(0,0,width(),height());glEnable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glDisable(GL_BLEND);glClearColor(.025f,.038f,.027f,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);cameraPos_=p.pos;cameraRight_={std::cos(p.yaw),std::sin(p.yaw)};Vec3 eye{p.pos.x,1.65f+std::sin(p.bob)*.035f,p.pos.y};Vec3 dir{std::sin(p.yaw)*std::cos(p.pitch),std::sin(p.pitch),-std::cos(p.yaw)*std::cos(p.pitch)};viewProj_=mul(perspective(1.22f,float(width())/height(),.05f,52.f),lookAt(eye,eye+dir,{0,1,0}));pglUseProgram(program_);pglUniformMatrix4fv(uMvp_,1,GL_FALSE,viewProj_.m);Mat4 id=identity();pglUniformMatrix4fv(uModel_,1,GL_FALSE,id.m);pglUniform3f(uFog_,.025f,.04f,.028f);pglUniform1i(uMode_,0);pglUniform4f(uTint_,1,1,1,1);pglActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,worldTex_);upload(mesh_);glDrawArrays(GL_TRIANGLES,0,(GLsizei)worldVerts_);}
void Renderer::drawQuad3D(Vec3 p,float w,float h,int tile,uint32_t tint,unsigned texture,int columns,int rows){float du=1.f/columns,dv=1.f/rows,u=(tile%columns)*du;int row=tile/columns;float v=texture==leechTex_?1.f-(row+1)*dv:row*dv;Vec3 right{cameraRight_.x,0,cameraRight_.y};Vec3 a=p-right*w*.5f,b=p+right*w*.5f;Vertex q[6]={{a.x,p.y,a.z,0,0,1,u,v},{b.x,p.y,b.z,0,0,1,u+du,v},{b.x,p.y+h,b.z,0,0,1,u+du,v+dv},{a.x,p.y,a.z,0,0,1,u,v},{b.x,p.y+h,b.z,0,0,1,u+du,v+dv},{a.x,p.y+h,a.z,0,0,1,u,v+dv}};std::vector<Vertex>vv(q,q+6);upload(vv);pglUniform1i(uMode_,1);pglUniform4f(uTint_,(tint&255)/255.f,((tint>>8)&255)/255.f,((tint>>16)&255)/255.f,((tint>>24)&255)/255.f);glBindTexture(GL_TEXTURE_2D,texture?texture:spriteTex_);glDisable(GL_CULL_FACE);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDrawArrays(GL_TRIANGLES,0,6);glDisable(GL_BLEND);}
void Renderer::drawCeilingOctagon(Vec2 p){
 constexpr float pi=3.14159265f,y=3.39f,outer=1.18f,inner=.84f;std::vector<Vertex>ring,center;
 auto point=[&](float radius,int i){float a=pi/8.f+i*pi/4.f;return Vec3{p.x+std::cos(a)*radius,y,p.y+std::sin(a)*radius};};
 auto add=[&](std::vector<Vertex>&v,Vec3 a,Vec3 b,Vec3 c){for(Vec3 q:{a,b,c})v.push_back({q.x,q.y,q.z,0,-1,0,.5f,.5f});};
 for(int i=0;i<8;i++){int n=(i+1)%8;Vec3 o0=point(outer,i),o1=point(outer,n),i0=point(inner,i),i1=point(inner,n);add(ring,o0,o1,i1);add(ring,o0,i1,i0);add(center,{p.x,y-.006f,p.y},i1,i0);}
 auto draw=[&](const std::vector<Vertex>&v,uint32_t c){upload(v);pglUniform1i(uMode_,1);pglUniform4f(uTint_,(c&255)/255.f,((c>>8)&255)/255.f,((c>>16)&255)/255.f,1);glBindTexture(GL_TEXTURE_2D,uiTex_);glDisable(GL_CULL_FACE);glDrawArrays(GL_TRIANGLES,0,(GLsizei)v.size());};
 draw(ring,0xffc2b271);draw(center,0xff030403);
}
void Renderer::drawWorld(const Maze&m,const std::vector<EnemyView>&es,const std::vector<SupplyView>&ss,const Player&,float muzzle){for(auto&s:ss){drawCeilingOctagon(s.pos);drawQuad3D({s.pos.x,2.02f,s.pos.y},.9f,1.15f,6,0xffffffff);if(s.state>=2&&s.state<4)drawQuad3D({s.pos.x+.4f,s.boxHeight,s.pos.y+.2f},.75f,.55f,5,0xffffffff);}auto [ex,ey]=m.exitCell();Vec2 ep=m.worldCenter(ey*m.width()+ex);drawQuad3D({ep.x,0,ep.y},1.8f,3.5f,7,0xffffffff);for(auto&e:es){int tile=e.dead?7:e.state==2?4+int(e.phase*5)%2:e.state==1?2+int(e.phase*5)%2:int(e.phase*3)%2;drawQuad3D({e.pos.x,0,e.pos.y},2.3f,e.dead?.95f:1.8f,tile,e.dead?0xbbffffff:0xffffffff,leechTex_,4,2);}if(muzzle>0){/* weapon flash supplies the deliberately brief screen-space light */}}
void Renderer::beginUI(){pglBindFramebuffer(GL_FRAMEBUFFER,0);int ww,wh;SDL_GetWindowSizeInPixels(window_,&ww,&wh);glViewport(0,0,ww,wh);glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glClear(GL_COLOR_BUFFER_BIT);pglUseProgram(program_);Mat4 id=identity();pglUniformMatrix4fv(uMvp_,1,GL_FALSE,id.m);pglUniformMatrix4fv(uModel_,1,GL_FALSE,id.m);pglUniform1i(uMode_,1);pglUniform3f(uFog_,0,0,0);glBindTexture(GL_TEXTURE_2D,colorTex_);pglUniform4f(uTint_,1,1,1,1);Vertex q[6]={{-1,-1,0,0,0,1,0,0},{1,-1,0,0,0,1,1,0},{1,1,0,0,0,1,1,1},{-1,-1,0,0,0,1,0,0},{1,1,0,0,0,1,1,1},{-1,1,0,0,0,1,0,1}};std::vector<Vertex>v(q,q+6);upload(v);glDrawArrays(GL_TRIANGLES,0,6);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);}
void Renderer::rect(float x,float y,float w,float h,uint32_t c){float l=x/width()*2-1,r=(x+w)/width()*2-1,t=1-y/height()*2,b=1-(y+h)/height()*2;Vertex q[6]={{l,b,0,0,0,1,0,0},{r,b,0,0,0,1,0,0},{r,t,0,0,0,1,0,0},{l,b,0,0,0,1,0,0},{r,t,0,0,0,1,0,0},{l,t,0,0,0,1,0,0}};std::vector<Vertex>v(q,q+6);upload(v);glBindTexture(GL_TEXTURE_2D,uiTex_);pglUniform4f(uTint_,(c&255)/255.f,((c>>8)&255)/255.f,((c>>16)&255)/255.f,((c>>24)&255)/255.f);glDrawArrays(GL_TRIANGLES,0,6);}
void Renderer::text(float x,float y,float scale,const std::string&s,uint32_t c,bool centered){if(centered)x-=s.size()*6*scale*.5f;for(char raw:s){char ch=char(std::toupper((unsigned char)raw));auto rows=glyph(ch);for(int yy=0;yy<7;yy++)for(int xx=0;xx<5;xx++)if(rows[yy]&(1<<(4-xx)))rect(x+xx*scale,y+yy*scale,scale,scale,c);x+=6*scale;}}
void Renderer::sprite2D(float x,float y,float w,float h,unsigned texture,int tile,int columns,int rows,uint32_t c,bool flipX,float rotation,float cropBottom){float du=1.f/columns,dv=1.f/rows,u=(tile%columns)*du,v=1.f-(tile/columns+1)*dv,u0=flipX?u+du:u,u1=flipX?u:u+du,v0=v+dv*std::clamp(cropBottom,0.f,.85f),v1=v+dv,cs=std::cos(rotation),sn=std::sin(rotation),pivotX=x+w*.78f,pivotY=y+h*.82f;auto pt=[&](float sx,float sy,float tu,float tv){float dx=sx-pivotX,dy=sy-pivotY,rx=pivotX+dx*cs-dy*sn,ry=pivotY+dx*sn+dy*cs;return Vertex{rx/width()*2-1,1-ry/height()*2,0,0,0,1,tu,tv};};Vertex bl=pt(x,y+h,u0,v0),br=pt(x+w,y+h,u1,v0),tr=pt(x+w,y,u1,v1),tl=pt(x,y,u0,v1);Vertex q[6]={bl,br,tr,bl,tr,tl};std::vector<Vertex>vv(q,q+6);upload(vv);glBindTexture(GL_TEXTURE_2D,texture);pglUniform4f(uTint_,(c&255)/255.f,((c>>8)&255)/255.f,((c>>16)&255)/255.f,((c>>24)&255)/255.f);glDrawArrays(GL_TRIANGLES,0,6);}
void Renderer::weapon(int animation,float t,float bob,bool unarmed){float x=138+std::sin(bob)*3,y=53+std::abs(std::cos(bob))*2;if(animation==3){float lift=std::sin(std::min(1.f,t/.3f)*1.5708f);x=133;y=72-8*lift;sprite2D(x,y,160,205,phoneTex_,0,1,1);return;}if(unarmed&&animation!=1){if(animation==2){float p=std::min(1.f,t/.34f),jab=std::sin(p*3.14159f),w=213+20*jab,h=320+30*jab;x=106-10*jab;y=15-12*jab;sprite2D(x,y,w,h,fistPunchTex_,0,1,1);}else sprite2D(78,-70,270,405,fistsIdleTex_,0,1,1);return;}int frame=animation==1?1:animation==4?3:0;bool flipReload=animation==4;if(animation==1)y-=std::sin(std::min(1.f,t*8)*3.14159f)*13;else if(animation==4)y+=8*std::sin(std::min(1.f,t*2)*3.14159f);sprite2D(x,y,150,200,weaponTex_,frame,4,1,0xffffffff,flipReload);}
void Renderer::present(){glDisable(GL_BLEND);SDL_GL_SwapWindow(window_);}
