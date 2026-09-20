// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "prop_meshes.hpp"
namespace sw {
inline void worldBox(Frame& f,V3 p,V3 s,Material m){add(f,Box,translate(p)*scale(s),m);}
inline void worldLine(Frame& f,V3 a,V3 b,float radius,Material m){add(f,Cylinder,segment(a,b,radius),m);}
inline void meadowTree(Frame& f,V3 p,float s,int variant){
 Material bark{{.14f,.095f,.05f},0,.9f,0};
 worldLine(f,p,p+V3{.18f*s,4.8f*s,0},.24f*s,bark);
 for(int j=0;j<4;j++){
  float a=j*2*pi/4+variant*.67f;V3 tip=p+V3{std::cos(a)*2.0f*s,(4.6f+j*.27f)*s,std::sin(a)*2*s};
  worldLine(f,p+V3{0,2.8f*s,0},tip,.10f*s,bark);
  V3 color=variant%3==0?V3{.25f,.38f,.055f}:variant%3==1?V3{.12f,.29f,.07f}:V3{.31f,.42f,.10f};
  add(f,Primitive(Rock0+j),translate(tip)*scale({2.2f*s,1.6f*s,2.1f*s}),{color,0,.92f,0,2});
 }
}
inline void distantCity(Frame& f,V3 origin,int count,float spacing){
 Random r(8413);for(int i=0;i<count;i++){
  float height=r.range(15,52),width=r.range(2.4f,5.6f);V3 p=origin+V3{(i-(count-1)*.5f)*spacing,height*.5f,r.range(-12,12)};
  worldBox(f,p,{width,height*.5f,width*.65f},{{.18f,.27f,.30f},0,.62f,.3f,14});
  worldBox(f,p+V3{0,height*.5f+1,0},{width*.60f,1,width*.48f},{{.23f,.36f,.37f},0,.6f,.35f});
  if(i%4==0)worldLine(f,p+V3{0,height*.5f,0},p+V3{0,height*.5f+9,0},.18f,{{.21f,.33f,.34f},0,.45f,.4f});
 }
}
}
