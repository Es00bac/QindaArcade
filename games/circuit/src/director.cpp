// SPDX-License-Identifier: GPL-3.0-or-later
#include "race.hpp"
namespace sw {
namespace {
V3 prism(float h){return {.52f+.43f*std::cos(h*2*pi),.53f+.43f*std::cos((h-.333f)*2*pi),.54f+.43f*std::cos((h+.333f)*2*pi)};}
void environment(Frame& f,const Track& track,double time){landscape(f,track,time);}

}
void compose(Frame& f,const RaceState& race,const Track& track,const SceneOptions& opt){
 for(auto& batch:f.batches)batch.instances.clear();
 f.time=race.time;f.motionTime=std::fmod(race.time,8192.0);f.voyage=track.seed%997;f.courseName=track.name();f.fade=1;
 f.winner=race.winner;f.order=race.order;f.chapter=track.kind;
 if(opt.gallery){
  int id=opt.galleryId;f.eye={4.9f,3.3f,6.9f};f.target={0,1.17f,0};f.up={0,1,0};f.fov=.59f;f.chapter=1;
  if(opt.galleryMenu){V3 shift=unit(cross(f.target-f.eye,f.up))*-1.3f;f.eye=f.eye+shift;f.target=f.target+shift;}
  f.pengu={0,1,0};f.focus=id;f.position=id+1;f.lap=1;f.speed=0;f.boost=0;f.countdown=0;
  add(f,Cylinder,translate({0,-.4f,0})*scale({3.25f,.40f,3.25f}),{{.027f,.041f,.063f},0,.35f,.72f});
  add(f,Torus,translate({0,-.025f,0})*scale({3.19f,.25f,3.19f}),{racerColors[id],.35f,.5f,.2f});
  add(f,Box,translate({0,-.82f,0})*scale({70,.4f,70}),{{.19f,.23f,.23f},0,.82f,.1f});
  kart(f,ry(opt.galleryAngle),id,race.time,0,.15f,0,0);
  return;
 }
 int focus=opt.focus>=0?opt.focus:0;
 // Director changes subjects only with a smooth interpolation between their live positions.
 float chapter=std::fmod(float(race.time),192.f)/24.f;int shot=int(chapter)%8;
 float blend=smooth(clamp((chapter-shot-.75f)/.25f));
 int next=opt.focus>=0?focus:(shot+1)%8;
 if(opt.focus<0)focus=shot;
 if(opt.camera!=Camera::Director&&opt.focus<0){focus=0;next=0;blend=0;}
 const auto& car=race.cars[focus];const auto& other=race.cars[next];
 auto p=track.at(car.distance),q=track.at(other.distance);
 V3 hero=p.position+p.right*car.lane;
 V3 following=q.position+q.right*other.lane;
 V3 center=mix(hero,following,blend);
 V3 forward=unit(mix(p.forward,q.forward,blend));
 V3 right=unit(mix(p.right,q.right,blend));
 V3 up=unit(mix(p.up,q.up,blend));
 f.focus=blend>.5f?next:focus;f.lap=std::clamp(int(std::max(0.,race.cars[f.focus].distance)/track.length)+1,1,3);
 f.position=race.cars[f.focus].rank;f.speed=race.cars[f.focus].speed*3.6f;f.boost=race.cars[f.focus].boost;
 f.driftCharge=race.cars[f.focus].driftCharge;f.raceTime=std::max(0.,race.roundTime-4);
 f.pengu=center;f.ducke=following;
 f.countdown=race.roundTime<4?std::max(1,4-int(race.roundTime)):0;
 f.chapter=track.kind;f.local=0;
 float theta=.50f;
 if(opt.camera==Camera::Director)theta=.60f+1.86f*(.5f-.5f*std::cos(float(race.time)*.052f));
 if(opt.camera==Camera::Chase)theta=pi;
 if(opt.camera==Camera::Front)theta=.49f;
 if(opt.camera==Camera::Orbit)theta=float(race.time)*.09f+.5f;
 float range=12.2f,vertical=5.8f;
 if(opt.camera==Camera::Director){range=12.7f+4.f*std::pow(std::sin(float(race.time)*.034f),4);vertical=5.5f+3.7f*std::pow(std::sin(float(race.time)*.034f),4);}
 if(opt.reduced){theta=.57f;range=18;vertical=9;}
 f.target=center+up*1.02f-forward*.35f;
 f.eye=center+forward*(std::cos(theta)*range)+right*(std::sin(theta)*range)+up*vertical;
 f.up=unit(mix(V3{0,1,0},up,.20f));f.fov=.83f;
 if(opt.camera==Camera::Chase){f.eye=center-forward*12.6f+up*4.7f;f.target=center+forward*9.f+up*1.7f;f.fov=.94f;}
 if(opt.camera==Camera::Overview){
  float a=float(race.time)*.011f;f.eye={float(std::sin(a)*325),218,float(std::cos(a)*325)};f.target={0,-2,0};f.up={0,1,0};f.fov=.94f;
 }
 // Fade a race reset through black. No visible grid teleport or new-course pop.
 if(race.finishedAt>=0){float age=float(race.time-race.finishedAt);f.fade=1-ease(8.6f,10.8f,age);}
 else if(race.round>0&&race.roundTime<1.5)f.fade=ease(0,1.3f,float(race.roundTime));
 environment(f,track,race.time);
 for(int box=0;box<24;++box)if(race.crates[box]<=0){auto pose=track.at(track.length*(box/3+.35)/8);auto m=pose.matrix((box%3-1)*4.1f,1.25f+.2f*std::sin(race.time*2+box));V3 color=prism(box*.19f);
  if(length(point(m,{0,0,0})-f.eye)>180)continue;
  add(f,Box,m*ry(race.time*.8+box)*scale({.54f,.54f,.54f}),{{.045f,.09f,.16f},0,.16f,.72f});
  for(int side:{-1,1})add(f,Torus,m*ry(race.time*.8+box)*translate({0,side*.54f,0})*scale({.48f,.15f,.48f}),{color,1.7f,.22f,.2f});
  add(f,Monolith,m*scale({.15f,.17f,.15f}),{color,2.1f,.3f,.3f});
 }
 for(int i=0;i<RacerCount;++i){const auto& car=race.cars[i];auto pose=track.at(car.distance);auto m=pose.matrix(car.lane,.8f);V3 co=racerColors[i];
  if(car.item!=Item::None){auto icon=m*translate({0,3.7f,0})*ry(race.time*2);V3 hue=int(car.item)==1?V3{1,.6f,.13f}:int(car.item)==2?V3{.2f,.7f,1}:int(car.item)==3?V3{.85f,.2f,1}:V3{.2f,1,.65f};add(f,Monolith,icon*scale({.27f,.25f,.27f}),{hue,1.3f,.24f,.5f});add(f,Torus,icon*scale({.5f,.2f,.5f}),{hue,1.5f,.2f,.2f});}
  if(car.shield>0){add(f,Torus,m*translate({0,-.63f,0})*scale({1.7f,.10f,2.1f}),{{.2f,.65f,1},.6f,.2f,.2f});}
  if(car.pulse>0){float radius=2+(1-car.pulse/.85f)*8;add(f,Torus,m*scale({radius,.24f,radius}),{co,car.pulse*2,.2f,.2f});}
  if(car.magnet>0){for(int k=0;k<3;++k)add(f,Torus,m*translate({0,.2f,-1.8f-k*.8f})*rx(pi/2)*scale({.65f+k*.22f,.12f,.65f+k*.22f}),{{.23f,1,.65f},.85f,.2f,.2f});}
 }
 f.itemReady=race.cars[focus].item!=Item::None;f.itemLabel=itemName(race.cars[focus].item);if(f.itemLabel.empty()){const auto& c=race.cars[focus];f.itemLabel=c.shield>0?"AEGIS ACTIVE":c.magnet>0?"MAGNET ACTIVE":c.boost>0?"TURBO":"";}

 for(int i=0;i<RacerCount;i++){
  const auto& c=race.cars[i];auto road=track.at(c.distance);
  M4 m=road.matrix(c.lane,.014f)*ry(c.heading-c.drift);
  float wheel=float(std::fmod(c.distance/.48,2*pi));
  kart(f,m,i,race.time,wheel,c.steer,c.drift,c.boost);
  V3 pos=point(m,{0,0,0});f.mapX[i]=pos.x/190;f.mapY[i]=pos.z/190;
  // Gentle near-road drift sparks tied to wheel contact, never random screen flashes.
  if(std::abs(c.drift)>.07f&&c.speed>15){
   int side=c.drift>0?-1:1;
   for(int j=0;j<4;j++){
    float age=std::fmod(float(race.time)*2.6f+j*.25f,1.f);
    V3 spark=point(m,{side*1.22f,.08f+std::sin(age*pi)*.24f,-1.2f-age*1.8f});
    add(f,Sphere,translate(spark)*scale({.035f,.035f,.09f}),{{1,.50f,.12f},(1-age)*2,.3f,0});
   }
  }
 }
}
}
