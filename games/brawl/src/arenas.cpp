// SPDX-License-Identifier: GPL-3.0-or-later
#include "battle.hpp"
#include "world_details.hpp"
namespace sw {
void buildArenaLandscape(Frame& f){
 Mesh mesh;mesh.name="reclaimed-valley-basin";constexpr int N=100;
 auto height=[](float x,float z){return -34+5*std::sin(x*.016f+z*.008f)+4*std::cos(z*.019f-x*.009f)+10*std::exp(-std::pow((z+175)/72,2.f));};
 for(int z=0;z<=N;z++)for(int x=0;x<=N;x++){float xx=(x-50)*5.f,zz=z*5.f-360;V3 normal=unit({height(xx-.5f,zz)-height(xx+.5f,zz),1,height(xx,zz-.5f)-height(xx,zz+.5f)});mesh.v.push_back({{xx,height(xx,zz),zz},normal,{x*.4f,z*.4f}});}
 for(int z=0;z<N;z++)for(int x=0;x<N;x++){unsigned a=z*(N+1)+x,b=a+N+1;mesh.ix.insert(mesh.ix.end(),{a,b,a+1,a+1,b,b+1});}
 f.batches[Terrain].mesh=std::move(mesh);
}
void stageModel(Frame& f,int stage,double time,std::uint64_t seed,bool environment){
 const auto ps=platforms(stage,time);bool garden=stage==1,bliss=stage==3;
 Material concrete{{.30f,.36f,.34f},0,.88f,.08f},metal{{.085f,.14f,.17f},0,.52f,.50f},paint{{.75f,.76f,.60f},0,.84f,.12f,21};
 V3 accent=garden||bliss?V3{.33f,.79f,.56f}:stage==2?V3{1,.55f,.24f}:V3{.27f,.78f,.91f};
 // Deck tops are the collision planes. Rear trusses support the upper catwalks.
 for(int i=0;i<4;i++){
  const auto& p=ps[i];float thick=i?.22f:.62f;
  // The tread sits on the structural deck, never coplanar with its top face.
  // Both faces formerly ended at p.y and fought in the depth buffer.
  worldBox(f,{p.x,p.y-thick-.045f,0},{p.half,thick-.045f,p.depth},concrete);
  worldBox(f,{p.x,p.y-.045f,0},{p.half-.05f,.045f,p.depth-.06f},{{.52f,.58f,.54f},0,.8f,.14f,21});
  worldLine(f,{p.x-p.half+.1f,p.y+.025f,p.depth-.04f},{p.x+p.half-.1f,p.y+.025f,p.depth-.04f},.045f,{accent,.5f,.55f,.2f});
  for(int side:{-1,1}){
   float x=p.x+side*(p.half-.6f);
   if(i){worldLine(f,{x,-1.3f,-p.depth-.28f},{x,p.y-.3f,-p.depth-.28f},.16f,metal);worldLine(f,{x-side*1.0f,p.y-1.4f,-p.depth-.28f},{x,p.y-.3f,-p.depth-.28f},.10f,metal);}
   else{worldBox(f,{x,-1.5f,0},{.7f,.45f,p.depth-.2f},metal);worldBox(f,{x,-3.2f,-1},{.6f,1.5f,1.3f},concrete);}
  }
  for(int j=0;j<6;j++)worldBox(f,{p.x-p.half+.45f+j*(2*p.half-.9f)/5,p.y-.29f,p.depth+.012f},{.18f,.13f,.015f},{{.87f,.62f,.19f},0,.75f,.1f});
 }
 if(!environment)return;
 add(f,Terrain,M4::identity(),{garden||bliss?V3{.65f,.80f,.49f}:V3{.17f,.22f,.23f},0,.95f,0,garden||bliss?20.f:0.f});
 // The fighting deck is cantilevered over a real drop. Scenery stays behind the action.
 worldBox(f,{0,-2.57f,-12},{23,.55f,7},concrete);
 for(int side:{-1,1})for(int j=0;j<3;j++)worldBox(f,{side*(7.f+j*6),-9,-12},{.65f,6,1.2f},concrete);
 worldBox(f,{0,-1.7f,-12},{23,.32f,7},garden||bliss?Material{{.65f,.8f,.48f},0,.95f,0,20}:metal);
 for(int side:{-1,1}){
  worldLine(f,{side*10.f,-1,-2},{side*17.f,-5,-12},.35f,metal);
  worldBox(f,{side*19.f,-2,-12},{3.5f,.3f,5},concrete);
 }
 if(garden||bliss){
  // Reclaimed valley terraces and planted banks frame the clear foreground combat plane.
  for(int i=0;i<9;i++){
   float x=(i-4)*8.f;float y=-8+std::sin(i*1.5f)*2;
   add(f,Primitive(Rock0+i%6),translate({x,y,-29.f-i%2*6})*scale({11,7,9}),{{.65f,.85f,.47f},0,.97f,0,20});
   if(i%2==0)meadowTree(f,{x,-2.f,-22.f-i%3*4},1.25f,i);
  }
  for(int i=0;i<14;i++){
   float x=(i-6.5f)*3.1f;
   add(f,Fern,translate({x,-1.35f,-8.5f-i%3*1.7f})*ry(i*.7f)*scale({1.3f,1.3f,1.3f}),{{.67f,.86f,.49f},0,.96f,0,20});
  }
  for(int side:{-1,1}){
   add(f,WindPump,translate({side*25.f,-6,-35})*scale({1.8f,1.8f,1.8f}),paint);
   for(int j=0;j<3;j++){M4 m=translate({side*(13.f+j*2.2f),-1.35f,-8.f-j*1.5f})*ry(side*.4f+j)*rz(j*.16f);crt(f,m);}
  }
  if(bliss){
   add(f,SalvageGantry,translate({0,-1.4f,-19})*scale({1.15f,1.4f,1}),paint);serverRack(f,translate({-15,-1.35f,-11}));serverRack(f,translate({15,-1.35f,-11}));
   authoredProp(f,AppliancePile,translate({-8,-1.35f,-14})*ry(.3f));
   authoredProp(f,Conveyor,translate({4,-1.35f,-13})*ry(.4f)*scale({.9f,.9f,.9f}));
   authoredProp(f,Compactor,translate({12,-1.35f,-16}));
   authoredProp(f,SalvageBin,translate({-20,-1.35f,-15}));
   authoredProp(f,CableReel,translate({19,-1.35f,-11})*ry(.7f));
  }
  else { // Old irrigation pipes and a restored glasshouse establish a botanical utility garden.
   for(int side:{-1,1})authoredProp(f,Greenhouse,translate({side*6.f,-1.35f,-13})*scale({1.1f,1.1f,1.1f}));
   authoredProp(f,WaterTank,translate({17,-1.35f,-15}));
   authoredProp(f,PumpSkid,translate({-16,-1.35f,-15}));
   add(f,CableCoil,translate({-15,-1.4f,-7}),{{.11f,.13f,.08f},0,.92f,0});
  }
  distantCity(f,{-20,-39,-200},22,12);
 }else if(stage==0){
  // A freight terminal: dock cranes, service racks, conduit and an open city-facing apron.
  for(int side:{-1,1}){
   // Each crane has an actual quay and foundations, not feet hanging beyond the deck.
   worldBox(f,{side*20.f,-2.6f,-24},{10,.9f,3},concrete);
   for(int leg:{-1,1})worldBox(f,{side*20.f+leg*8.1f,-14,-24},{1.3f,11.6f,1.7f},concrete);
   add(f,SalvageGantry,translate({side*20.f,-1.5f,-24})*scale({.9f,1.7f,1.4f}),paint);
   for(int j=0;j<4;j++){worldBox(f,{side*(14.f+j*.7f),.2f+j*.4f,-11.f-j*3},{3.3f,1.6f,1.5f},{{.13f+j*.035f,.29f,.31f},0,.7f,.25f,21});}
   serverRack(f,translate({side*11.f,-1.35f,-8}));
   worldLine(f,{side*13.f,-1.0f,-7},{side*13.f,-1.0f,-24},.26f,paint);
  }
  distantCity(f,{0,-22,-82},24,6);
 }else{
  // Rooftop plant: cooling fans, water tanks and parapets belong to the tower below.
  worldBox(f,{0,-24,-13},{24,19,8},{{.07f,.12f,.16f},0,.65f,.35f,14});
  for(int side:{-1,1}){
   for(int j=0;j<2;j++)authoredProp(f,VentBank,translate({side*(13.f+j*5),-1.35f,-11.f-j*3}));
   authoredProp(f,WaterTank,translate({side*20.f,-1.35f,-17}));
   worldLine(f,{side*22.f,-1,-6},{side*22.f,9,-6},.13f,metal);
   add(f,Sphere,translate({side*22.f,9,-6})*scale({.13f,.13f,.13f}),{accent,1.2f,.4f,0});
  }
  distantCity(f,{0,-35,-85},26,7);
 }
 (void)seed;
}
}
