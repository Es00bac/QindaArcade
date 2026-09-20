// SPDX-License-Identifier: GPL-3.0-or-later
#include "scene.hpp"
namespace sw {
namespace {
const Material carbon{{.021f,.031f,.048f},0,.30f,.65f};
const Material steel{{.21f,.29f,.36f},0,.24f,.84f};
const Material ceramic{{.81f,.86f,.87f},0,.24f,.18f};
const Material copper{{.76f,.32f,.12f},0,.27f,.75f};
const Material rubber{{.009f,.014f,.021f},0,.75f,.03f,15};
const Material onyx{{.008f,.013f,.025f},0,.14f,.15f};
const Material cream{{.96f,.90f,.74f},0,.52f,.02f};
const Material iris{{.003f,.01f,.018f},0,.06f,.40f};
Material paint(V3 c){return {c,0,.26f,.42f};}
Material glow(V3 c,float e=2){return {c,e,.23f,.2f};}
void ell(Frame& f,M4 m,V3 p,V3 s,Material c){add(f,Sphere,m*translate(p)*scale(s),c);}
void plate(Frame& f,M4 m,V3 p,V3 s,Material c){add(f,Box,m*translate(p)*scale(s),c);}
void tube(Frame& f,M4 m,V3 a,V3 b,float r,Material c){add(f,Cylinder,m*segment(a,b,r),c);}
void ring(Frame& f,M4 m,V3 p,V3 s,Material c){add(f,Torus,m*translate(p)*scale(s),c);}
void badge(Frame& f,M4 m,float size,Material c){
 ring(f,m*rx(pi/2),{0,0,0},{size,.35f,size},c);
 tube(f,m,{size*.45f,-size*.55f,.02f},{size*.95f,-size*1.04f,.02f},size*.11f,c);
}
}
void animal(Frame& f,M4 m,int id,double time,float steering){
 float t=float(std::fmod(time,4096.0));
 const V3 accent=racerColors[id];Material coat{{.029f,.039f,.047f},0,.61f,.015f};
 if(id==1)coat={{1.f,.62f,.12f},0,.46f,.02f};
 if(id==2||id==6)coat={{id==2?.84f:.61f,.19f,.074f},0,.60f,.02f};
 if(id==3)coat={{.29f,.33f,.39f},0,.62f,.03f};
 if(id==4)coat={{.83f,.79f,.90f},0,.55f,.02f};
 if(id==7)coat={{1.f,.56f,.70f},0,.44f,.03f};
 const Material belly=id==1?Material{{1.f,.82f,.35f},0,.5f,.01f}:cream;
 ell(f,m,{0,.30f,0},{.52f,.59f,.42f},coat);
 ell(f,m,{0,.22f,.34f},{.40f,.40f,.17f},belly);
 // Tail silhouettes make the species readable from rear chase views.
 if(id==2||id==3||id==6){
  for(int k=0;k<6;k++){
   float a=k/5.f;V3 p{.25f+std::sin(a*2.1f)*.44f,.13f+a*.36f,-.30f-a*1.0f};
   Material color=coat;if(id==3)color=k%2?onyx:coat;if(id==6)color=k%2?cream:coat;if(id==2&&k>3)color=cream;
   ell(f,m*ry(.09f*std::sin(t*1.5f)),p,{.22f*(1-a*.35f),.23f*(1-a*.3f),.22f},color);
  }
 }
 if(id==5){for(int k=0;k<7;k++)ell(f,m,{.3f+float(k)*.045f,.10f+std::pow(k/6.f,2.f)*.7f,-.38f-k*.12f},{.09f,.12f,.12f},coat);}
 // Racing suit and seated feet. Metallic buckles and piping remain non-emissive.
 for(int side:{-1,1}){
  plate(f,m,{side*.30f,.30f,.42f},{.087f,.30f,.06f},carbon);
  plate(f,m,{side*.30f,.57f,.41f},{.099f,.063f,.061f},copper);
  ell(f,m,{side*.25f,-.22f,.34f},{.23f,.13f,.34f},id<2?copper:carbon);
  plate(f,m,{side*.25f,-.23f,.53f},{.18f,.057f,.095f},steel);
  V3 shoulder{side*.48f,.50f,.01f};
  V3 elbow{side*.57f,.25f,.39f};
  V3 hand{side*.34f+steering*.18f,.30f+side*steering*.12f,.72f};
  tube(f,m,shoulder,elbow,.12f,coat);ell(f,m,elbow,{.15f,.15f,.15f},coat);
  tube(f,m,elbow,hand,.105f,carbon);ell(f,m,hand,{.14f,.12f,.15f},carbon);
 }
 plate(f,m,{0,.24f,.46f},{.027f,.23f,.018f},steel);
 badge(f,m*translate({-.25f,.43f,.492f}),.055f,glow(accent,.9f));
 float lean=.025f*std::sin(t*2.1f+id);
 M4 head=m*translate({0,1.08f+.012f*std::sin(t*1.8f+id),0})*ry(steering*.45f+.035f*std::sin(t*.4f+id))*rz(lean);
 ell(f,head,{0,0,0},{.69f,.70f,.60f},coat);
 if(id==0){
  for(int side:{-1,1})ell(f,head,{side*.225f,-.02f,.46f},{.35f,.44f,.20f},cream);
  ell(f,head,{0,-.27f,.48f},{.42f,.24f,.18f},cream);
 }else if(id==2||id==6){
  for(int side:{-1,1}){
   ell(f,head*rz(side*.25f),{side*.28f,-.16f,.45f},{.35f,.25f,.21f},cream);
   if(id==6)ell(f,head,{side*.30f,.105f,.53f},{.20f,.245f,.096f},Material{{.32f,.08f,.034f},0,.6f,0});
  }
 }else if(id==3){
  for(int side:{-1,1})ell(f,head*rz(side*-.16f),{side*.26f,.015f,.486f},{.33f,.245f,.15f},onyx);
  ell(f,head,{0,-.24f,.52f},{.26f,.20f,.15f},cream);
 }else if(id==4){
  for(int side:{-1,1})ell(f,head,{side*.19f,-.20f,.53f},{.22f,.20f,.11f},cream);
 }else if(id==7){
  for(int side:{-1,1})ell(f,head,{side*.43f,-.13f,.49f},{.16f,.10f,.05f},Material{{1.f,.26f,.49f},0,.5f,0});
 }
 // Species-specific ears and gills are actual articulated geometry.
 for(int side:{-1,1}){
  if(id==2||id==5){
   M4 e=head*translate({side*.43f,.42f,-.05f})*rz(side*-.18f+std::sin(t*1.8f+id)*.025f);
   add(f,Ear,e*scale({.72f,.51f,.86f}),coat);
   add(f,Ear,e*translate({0,.09f,.075f})*scale({.43f,.36f,.62f}),id==2?cream:paint({.26f,.12f,.28f}));
   tube(f,e,{-.16f,.27f,.18f},{-.055f,.66f,.08f},.017f,glow(accent,1));
  }else if(id==3||id==6){
   ell(f,head,{side*.51f,.49f,-.035f},{.23f,.27f,.13f},cream);
   ell(f,head,{side*.51f,.49f,.071f},{.145f,.185f,.04f},coat);
  }else if(id==4){
   M4 e=head*translate({side*.28f,.46f,-.08f})*rz(-side*.17f+.045f*std::sin(t*1.7f+side));
   ell(f,e,{0,.44f,0},{.22f,.74f,.17f},coat);
   ell(f,e,{0,.48f,.13f},{.115f,.49f,.047f},paint({.70f,.31f,.53f}));
   tube(f,e,{0,.12f,.17f},{0,.73f,.13f},.017f,glow(accent,1));
  }else if(id==7){
   for(int k=0;k<3;k++){
    float a=(k-1)*.48f+.05f*std::sin(t*1.4f+k);
    V3 base{side*.54f,.10f,0},tip{side*(.89f+.08f*std::cos(a)),.12f+std::sin(a)*.68f,.08f};
    tube(f,head,base,tip,.065f,paint({.9f,.25f,.47f}));
    ell(f,head,tip,{.15f,.11f,.095f},paint({1.f,.32f,.56f}));
    ell(f,head,tip+V3{side*.06f,0,.07f},{.04f,.037f,.025f},glow(accent,1.3f));
   }
  }
 }
 float phase=std::fmod(t+id*.68f,5.8f),blink=phase<.21f?std::pow(std::sin(phase*pi/.21f),2.f):0;
 if(id!=1){
  for(int side:{-1,1}){
   ell(f,head,{side*.25f,.045f,.665f},{.16f,.199f,.073f},iris);
   ell(f,head,{side*.25f-.043f,.112f,.731f},{.035f,.046f,.018f},cream);
   ell(f,head,{side*.25f+.024f,-.003f,.735f},{.014f,.018f,.012f},glow(accent,.45f));
   if(blink>.002f)ell(f,head,{side*.25f,.236f-blink*.19f,.747f},{.17f,.209f*blink,.022f},id==0?cream:coat);
  }
 }else{
  for(int side:{-1,1}){
   plate(f,head,{side*.265f,.085f,.553f},{.275f,.225f,.087f},copper);
   plate(f,head,{side*.265f,.09f,.631f},{.232f,.178f,.04f},Material{{.013f,.028f,.064f},0,.055f,.72f});
   plate(f,head*rz(-.18f),{side*.265f,.16f,.677f},{.15f,.012f,.011f},paint({.34f,.59f,.81f}));
  }
  plate(f,head,{0,.13f,.62f},{.09f,.043f,.04f},onyx);
 }
 if(id==0){
  M4 lens=head*translate({.255f,.065f,.753f})*rx(pi/2);
  ring(f,lens,{0,0,0},{.245f,.45f,.245f},steel);
  ring(f,lens,{0,-.036f,0},{.209f,.18f,.209f},glow({.04f,.92f,.62f},1.8f));
  plate(f,head,{.52f,.09f,.65f},{.12f,.055f,.04f},carbon);
 }
 if(id<2){
  ell(f,head,{0,-.25f,.67f},id==0?V3{.21f,.13f,.24f}:V3{.34f,.125f,.29f},paint({1.f,.43f,.055f}));
  ell(f,head,{0,-.314f,.72f},id==0?V3{.19f,.033f,.18f}:V3{.32f,.027f,.24f},paint({.54f,.17f,.035f}));
  ell(f,head,{0,-.35f,.68f},id==0?V3{.16f,.055f,.19f}:V3{.27f,.052f,.23f},paint({1.f,.65f,.14f}));
 }else if(id==7){
  tube(f,head,{-.12f,-.26f,.581f},{0,-.285f,.59f},.014f,paint({.45f,.09f,.18f}));
  tube(f,head,{0,-.285f,.59f},{.12f,-.26f,.581f},.014f,paint({.45f,.09f,.18f}));
 }else{
  ell(f,head,{0,-.21f,.59f},{.22f,.145f,id==2?.24f:.13f},id==5?coat:cream);
  ell(f,head,{0,-.17f,id==2?.80f:.714f},{.10f,.072f,.07f},id==4?paint({.75f,.29f,.40f}):iris);
  tube(f,head,{0,-.22f,.70f},{0,-.29f,.675f},.011f,onyx);
 }
 // Shared cyberpunk headset language, individual color-coded ear lights.
 for(int side:{-1,1}){
  ell(f,head,{side*.65f,-.03f,-.09f},{.10f,.27f,.28f},carbon);
  M4 cup=head*translate({side*.742f,-.025f,-.09f})*rz(pi/2);
  ring(f,cup,{0,0,0},{.185f,.36f,.185f},steel);
  ring(f,cup,{0,-side*.018f,0},{.14f,.15f,.14f},glow(accent,.85f));
 }
 tube(f,head,{-.73f,.13f,-.13f},{-.80f,.63f,-.15f},.018f,steel);
 ell(f,head,{-.80f,.63f,-.15f},{.037f,.045f,.037f},glow(accent,1.6f));
 if(id==0){
  for(int side:{-1,1})add(f,Ribbon,m*translate({side*.15f,.68f,-.37f})*ry(-pi/2)*scale({.6f,.5f,.55f}),Material{{.77f,.21f,.055f},0,.64f,.03f,11});
 }
}
void kart(Frame& f,M4 m,int id,double time,float wheel,float steer,float drift,float boost){
 const V3 accent=racerColors[id];
 M4 chassis=m*translate({0,float(.018f*std::sin(time*6.5+id)),0})*rz(-drift*.16f);
 // Eight authored bodies, each with its own material/UV assignments and construction.
 add(f,Primitive(Kart0+id),chassis,{{1,1,1},0,.5f,.1f});
 const float driverScale=sw::driverScale(id);
 const float seatLift=driverScale<.8f?.16f:0.f;
 const float driverY=.90f+seatLift,driverZ=driverScale<.8f?.025f:-.10f;
 const float wheelY=driverY+.30f*driverScale,wheelZ=driverZ+.72f*driverScale;
 Material upholstery{{.10f,.115f,.12f},0,.84f,0,27};
 plate(f,chassis,{0,driverY+.22f,-.42f},{.43f,.36f,.12f},upholstery);
 plate(f,chassis,{0,driverY-.10f,driverZ},{.43f,.085f,.38f},upholstery);
 if(seatLift>0)plate(f,chassis,{0,.91f,driverZ},{.44f,.10f,.37f},paint(accent*.5f));
 tube(f,chassis,{0,.78f,.63f},{0,wheelY,wheelZ},.036f,steel);
 M4 steering=chassis*translate({0,wheelY,wheelZ})*rx(.72f)*ry(steer*.3f);
 ring(f,steering,{0,0,0},{driverScale*.37f,.65f,driverScale*.37f},rubber);
 for(int spoke=0;spoke<3;spoke++){
  float a=spoke*2*pi/3;
  tube(f,steering,{0,0,0},{std::cos(a)*driverScale*.31f,0,std::sin(a)*driverScale*.31f},.018f,steel);
 }
 // Equal radius/contact points; individual hubs, spoke patterns and brake hardware.
 const int spokes[]={6,3,5,4,6,3,4,5};
 for(int axle=0;axle<2;axle++)for(int side:{-1,1}){
  float z=axle?1.03f:-1.03f;
  M4 carrier=m*translate({side*1.17f,.49f,z})*ry(axle?steer*.33f:0);
  M4 w=carrier*rx(wheel);
  add(f,Cylinder,w*rz(pi/2)*scale({.48f,.245f,.48f}),rubber);
  Material rim=id==1?copper:id==5?carbon:id==6?paint({.63f,.62f,.46f}):steel;
  for(float x:{-.247f,.247f}){
   M4 hub=w*translate({x,0,0})*rz(pi/2);
   ring(f,hub,{0,0,0},{.35f,.45f,.35f},rim);
   ring(f,hub,{0,0,0},{.31f,.4f,.31f},paint(accent*.65f));
   if(id==1||id==5){
    add(f,Cylinder,hub*scale({.29f,.019f,.29f}),id==5?carbon:(side==1&&axle==1?ceramic:copper));
    for(int vent=0;vent<3;vent++){float a=vent*2*pi/3;ell(f,w,{x+side*.02f,std::cos(a)*.21f,std::sin(a)*.21f},{.013f,.043f,.043f},onyx);}
   }else for(int spoke=0;spoke<spokes[id];spoke++){
    float a=spoke*2*pi/spokes[id];
    tube(f,w,{x,std::cos(a)*.10f,std::sin(a)*.10f},{x,std::cos(a+.13f)*.31f,std::sin(a+.13f)*.31f},id==4?.021f:.034f,rim);
   }
   add(f,Cylinder,hub*scale({.082f,.032f,.082f}),id==3?paint({.87f,.43f,.10f}):steel);
  }
  tube(f,m,{side*.70f,.58f,z},{side*1.03f,.49f,z},.044f,steel);
  tube(f,m,{side*.66f,.82f,z-.10f},{side*1.03f,.54f,z},.026f,copper);
  plate(f,carrier,{side*.19f,.1f,-.25f},{.035f,.12f,.07f},paint(accent));
 }
 animal(f,chassis*translate({0,driverY,driverZ})*scale({driverScale,driverScale,driverScale}),id,time,steer);
 if(boost>.02f)for(int side:{-1,1})for(int j=0;j<5;j++){
  float z=-2.0f-j*.55f;
  tube(f,m,{side*.85f,.22f,z},{side*.85f,.22f,z-.32f},.025f*(1-j*.13f),glow(accent,2.4f*clamp(boost)));
 }
}
void arch(Frame& f,M4 m,int sector,double time){
 V3 c=sector==0?V3{.14f,.93f,.69f}:sector==1?V3{1,.30f,.51f}:sector==2?V3{.45f,.31f,1}:V3{.20f,.67f,1};
 for(int side:{-1,1}){
  plate(f,m,{side*7.65f,2.15f,0},{.26f,2.4f,.47f},carbon);
  tube(f,m,{side*7.60f,4.2f,0},{side*5.5f,6.5f,0},.23f,steel);
  tube(f,m,{side*7.62f,.25f,-.49f},{side*7.62f,4.3f,-.49f},.063f,glow(c,2));
  for(int j=0;j<6;j++)plate(f,m,{side*7.96f,.7f+j*.57f,-.07f},{.067f,.14f,.32f},paint(c*.5f));
 }
 plate(f,m,{0,6.5f,0},{5.7f,.24f,.45f},carbon);
 tube(f,m,{-5.6f,6.70f,-.48f},{5.6f,6.70f,-.48f},.064f,glow(c,2.2f));
 badge(f,m*translate({0,6.46f,.49f}),.35f,glow(c,1.1f));
 for(int k=0;k<3;k++)ell(f,m,{(k-1)*1.5f,5.85f,0},{.16f,.16f,.16f},glow(c,1.5f+.18f*std::sin(time*.5+k)));
}
}
