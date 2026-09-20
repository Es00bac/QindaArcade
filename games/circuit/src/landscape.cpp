// SPDX-License-Identifier: GPL-3.0-or-later
#include "race.hpp"
#include "world_details.hpp"
namespace sw {
namespace {
float baseHeight(float x,float z){return 7*std::sin(x*.011f+z*.006f)+6*std::cos(z*.018f-x*.004f)+2.2f*std::sin(x*.04f)*std::cos(z*.033f);}
float terrainAt(const std::vector<TrackPose>& road,int kind,float x,float z){
 if(kind==0)return -30;
 float dist=1e20f,height=0;
 for(const auto& pose:road){const auto p=pose.position;float dx=x-p.x,dz=z-p.z,d=dx*dx+dz*dz;if(d<dist){dist=d;const auto r=pose.right;height=p.y+(dx*r.x+dz*r.z)*r.y/(r.x*r.x+r.z*r.z);}}
 float edge=std::sqrt(dist);float shoulder=kind==3?18.f:kind==1?20.f:8.f;
 float blend=smooth(clamp((edge-shoulder)/(kind==2?22.f:30.f)));
 float rolling=baseHeight(x,z)+(kind==2?12.f:3.f);
 float mountains=std::max(0.f,(std::sqrt(x*x+z*z)-220)*.18f);
 return (height-.92f)*(1-blend)+(rolling+mountains)*blend;
}
std::vector<TrackPose> roadPoints(const Track& tr){std::vector<TrackPose> points;for(int i=0;i<720;i++)points.push_back(tr.at(tr.length*i/720));return points;}
}
float groundHeight(const Track& track,float x,float z){return terrainAt(roadPoints(track),track.kind,x,z);}
void buildLandscape(Frame& f,const Track& tr){
 auto road=roadPoints(tr);Mesh mesh;mesh.name="authored-valley-terrain";
 constexpr int N=180;const float step=tr.kind==0?18.f:4.8f,half=N*step*.5f;
 std::vector<float> heights((N+1)*(N+1));
 for(int z=0;z<=N;z++)for(int x=0;x<=N;x++)heights[z*(N+1)+x]=terrainAt(road,tr.kind,x*step-half,z*step-half);
 for(int z=0;z<=N;z++)for(int x=0;x<=N;x++){
  auto height=[&](int a,int b){return heights[std::clamp(b,0,N)*(N+1)+std::clamp(a,0,N)];};
  V3 normal=unit({height(x-1,z)-height(x+1,z),2*step,height(x,z-1)-height(x,z+1)});
  float soil=0;
  if(tr.kind==3){
   float nearest=1e20f;int segment=0;
   for(int j=0;j<int(road.size());j++){float dx=x*step-half-road[j].position.x,dz=z*step-half-road[j].position.z,d=dx*dx+dz*dz;if(d<nearest){nearest=d;segment=j;}}
   float edge=std::sqrt(nearest),patch=.5f+.5f*std::sin(segment*.19f+x*.83f+z*.36f);
   soil=smooth(clamp((edge-8)/4))*smooth(clamp((29-edge)/9))*(.30f+.70f*patch);
  }
  mesh.v.push_back({{x*step-half,height(x,z),z*step-half},normal,{soil,0}});
 }
 for(int z=0;z<N;z++)for(int x=0;x<N;x++){unsigned a=z*(N+1)+x,b=a+N+1;mesh.ix.insert(mesh.ix.end(),{a,b,a+1,a+1,b,b+1});}
 f.batches[Terrain].mesh=std::move(mesh);
}
void landscape(Frame& f,const Track& track,double time){
 const bool bliss=track.kind==3,garden=track.kind==1,ice=track.kind==2;
 const float viewDistance=f.eye.y>120?850.f:330.f;
 const Material steel{{.16f,.23f,.24f},0,.64f,.4f},rust{{.65f,.49f,.21f},0,.72f,.3f,21};
 auto road=roadPoints(track);auto ground=[&](float x,float z){return terrainAt(road,track.kind,x,z);};
 add(f,Terrain,M4::identity(),{ice?V3{.82f,.87f,.88f}:garden||bliss?V3{.9f,1.0f,.77f}:V3{.11f,.15f,.17f},0,.95f,0,garden||bliss?20.f:ice?25.f:0.f});
 add(f,RoadSurface,M4::identity(),{ice?V3{.62f,.74f,.80f}:V3{.73f,.78f,.78f},0,.91f,.08f,23});
 add(f,RoadShell,M4::identity(),{ice?V3{.18f,.25f,.30f}:V3{.25f,.23f,.17f},0,.8f,.2f});
 // Road furniture follows the route: chevrons announce bends, guardrails frame the line.
 for(int i=0;i<int(track.length/5);i++){
  double s=i*5;auto p=track.at(s),q=track.at(s+5);if(length(p.position-f.eye)>205)continue;
  for(int side:{-1,1}){
   V3 a=p.position+p.right*(side*7.25f),b=q.position+q.right*(side*7.25f);
   if(track.kind==0||ice||i%9<5){worldLine(f,a+p.up*.55f,b+q.up*.55f,.085f,steel);worldLine(f,a,a+p.up*.82f,.075f,steel);}
   if(i%8==0){auto m=p.matrix(side*8.3f,.15f);add(f,Box,m*scale({.18f,.8f,.17f}),{{.75f,.63f,.21f},0,.68f,.2f});}
  }
  if(i%4==0){auto m=p.matrix(0,.045f);add(f,Box,m*scale({.075f,.009f,1.25f}),{{.73f,.71f,.48f},0,.85f,0});}
  if(track.kind==0&&i%5==0){float floor=ground(p.position.x,p.position.z);worldLine(f,{p.position.x,floor,p.position.z},p.position-V3{0,1.0f,0},1.1f,steel);worldBox(f,p.position-V3{0,1.4f,0},{9,.55f,1.0f},steel);}
 }
 // Start/finish, pit gantry and sector markers share the actual race coordinates.
 for(int row=0;row<2;row++)for(int col=0;col<16;col++)add(f,Box,track.at(row*.8).matrix((col-7.5f)*.86f,.05f)*scale({.42f,.02f,.38f}),{(col+row)%2?V3{.86f,.84f,.73f}:V3{.04f,.055f,.057f},0,.85f,0});
 for(int sector=0;sector<4;sector++){
  auto p=track.at(track.length*sector/4);if(length(p.position-f.eye)>220)continue;
  if(bliss)add(f,SalvageGantry,p.matrix()*scale({1,.80f,1}),rust);
  else if(track.kind==0)arch(f,p.matrix(),sector,time);
  else{
   Material post=ice?Material{{.70f,.30f,.10f},0,.65f,.3f,28}:Material{{.43f,.30f,.17f},0,.82f,0,29};
   for(int side:{-1,1}){
    auto leg=p.matrix(side*8.8f,4);add(f,Box,leg*scale({.22f,4,.3f}),post);
    add(f,Box,p.matrix(side*8.8f,7)*scale({.4f,.32f,.4f}),{{.81f,.81f,.65f},0,.65f,.2f});
   }
   add(f,Box,p.matrix(0,8)*scale({9.1f,.16f,.24f}),post);
  }
  auto m=p.matrix(0,7.8f);add(f,Box,m*translate({0,0,.85f})*scale({4.8f,.45f,.08f}),{{.025f,.085f,.09f},0,.7f,.2f});
  for(int k=0;k<sector+1;k++)add(f,Box,m*translate({(k-sector*.5f)*.6f,0,.96f})*scale({.16f,.26f,.035f}),{{.40f,.95f,.70f},.35f,.5f,.1f});
 }
 for(const auto& pad:track.pads){auto p=track.at(pad.distance);if(length(p.position-f.eye)>160)continue;auto m=p.matrix(pad.lane,.06f);
  add(f,Box,m*scale({1.5f,.02f,2.3f}),{{.075f,.19f,.19f},0,.6f,.2f});
  for(int j=0;j<3;j++)for(int side:{-1,1})worldLine(f,point(m,{side*1.1f,.03f,float(j-1)*1.25f-.4f}),point(m,{0,.03f,float(j-1)*1.25f+.4f}),.09f,{{.28f,.86f,.73f},1.2f,.4f,.1f});
 }
 // Four authored work districts per course. Placement follows the road but is
 // grounded horizontally; large props receive foundations and never overlap lanes.
 for(int cluster=0;cluster<40;cluster++){
  Random rng(track.seed+6031+std::uint64_t(cluster)*27321);
  auto p=track.at(track.length*(cluster+.4)/40);int side=cluster%2?-1:1,zone=cluster/10;
  V3 forward=unit({p.forward.x,0,p.forward.z}),right=unit({p.right.x,0,p.right.z});
  V3 anchor=p.position+right*(side*((bliss?13.f:17.f)+cluster%3*2));
  anchor.y=track.kind==0?p.position.y:ground(anchor.x,anchor.z);
  if(length(anchor-f.eye)>viewDistance)continue;
  auto clear=[&](V3 pos,float radius){
   for(const auto& q:road){float dx=pos.x-q.position.x,dz=pos.z-q.position.z;if(dx*dx+dz*dz<(8.5f+radius)*(8.5f+radius))return false;}
   return true;
  };
  auto place=[&](Primitive type,float along,float away,float yaw=0,float size=1.f,bool foundation=false,float radius=3.f){
   V3 pos=anchor+forward*along+right*(side*away);
   if(!clear(pos,radius*size))return;
   float y=track.kind==0?p.position.y:ground(pos.x,pos.z);
   if(foundation||track.kind==0){
    float high=y,low=y;
    if(track.kind!=0)for(int a:{-1,1})for(int b:{-1,1}){float sample=ground(pos.x+a*radius*size,pos.z+b*radius*size);high=std::max(high,sample);low=std::min(low,sample);}
    pos.y=high+.12f;
    M4 foot=basis({pos.x,(low+high)*.5f,pos.z},right,{0,1,0},forward);
    add(f,Box,foot*scale({radius*size+.25f,(high-low)*.5f+.12f,radius*size+.25f}),{{.27f,.29f,.27f},0,.92f,0});
    if(track.kind==0){
     // Roof machinery belongs on occupied buildings, not implausible fifty-meter stools.
     float height=pos.y+30;
     M4 building=basis({pos.x,-30+height*.5f-.2f,pos.z},right,{0,1,0},forward);
     add(f,Box,building*scale({radius*size+.12f,height*.5f-.2f,radius*size+.12f}),{{.23f,.29f,.31f},0,.75f,.1f,14});
     for(int edge:{-1,1})add(f,Box,basis(pos,right,{0,1,0},forward)*translate({edge*(radius*size+.1f),.23f,0})*scale({.09f,.23f,radius*size+.2f}),{{.43f,.47f,.44f},0,.8f,.1f});
    }
   }else pos.y=y;
   authoredProp(f,type,basis(pos,right,{0,1,0},forward)*ry(yaw)*scale({size,size,size}));
  };
  if(bliss){
   // Intake -> electronics windrows -> cable recovery -> meadow reclamation.
   if(zone==0){
    place(SalvageBin,-6,0,0,1,true,2.6f);place(Conveyor,3,4,.15f,1,true,3.2f);
    if(cluster%3==0)place(Compactor,3,12,0,1.5f,true,4.5f);
    place(Pallet,-5,6,.12f);place(FreightCrate,-4,12,0,1,true,2.8f);
   }else if(zone==1){
    place(AppliancePile,-4,0,.3f,1.35f);place(ScrapCar,5,5,.4f,1.15f);
    place(SalvageBin,-2,12,-.3f,1,false,2.6f);place(Pallet,9,2,.4f);
   }else if(zone==2){
    place(CableReel,-5,0,.15f,1.25f);place(CableReel,-2,6,-.55f,.86f);
    place(PumpSkid,7,3,0,1.2f,true,2.2f);place(Conveyor,2,13,-.6f);
    if(cluster%3==0)place(Compactor,-5,15,.2f);
   }else{
    place(ScrapCar,-7,4,-.6f,1.15f);place(AppliancePile,4,3,.7f,.9f);
    if(cluster%3==0){place(SolarArray,0,13,.2f,1.4f,true,3.2f);place(WaterTank,8,10,0,1.2f);}
    place(Pallet,-3,0,.2f,.9f);
   }
   int count=zone==1?12:zone==3?7:5;
   for(int j=0;j<count;j++){
    V3 pos=anchor+forward*((j-count*.5f)*2.1f)+right*(side*rng.range(0,6));pos.y=ground(pos.x,pos.z)-.06f;
    if(!clear(pos,1.9f))continue;
    M4 m=translate(pos)*ry(rng.range(-pi,pi))*rz(rng.range(-.20f,.20f))*rx(rng.range(-.12f,.16f));
    if((j+cluster)%5==0)serverRack(f,m*scale({.86f,.86f,.86f}));else crt(f,m*scale({1.06f,1.06f,1.06f}),j+cluster);
    if(j%4==0){crt(f,m*translate({.2f,1.48f,-.1f})*ry(.8f)*rz(.15f)*scale({.87f,.87f,.87f}),cluster+1);}
    for(int scrap=0;scrap<3;scrap++){
     V3 debris=pos+right*(side*(1.8f+scrap*.7f))+forward*rng.range(-1.2f,1.2f);debris.y=ground(debris.x,debris.z)+.10f;
     add(f,Box,translate(debris)*ry(rng.range(0,pi))*scale({rng.range(.3f,.55f),.04f,rng.range(.25f,.6f)}),{{.48f,.58f,.43f},0,.79f,.1f,scrap%2?22.f:28.f});
    }
   }
   if(zone==2||cluster%4==0)place(CableCoil,0,2,.4f,1.4f,false,2.2f);
  }else if(garden){
   if(zone==0){
    place(Greenhouse,0,7,0,1.25f,true,6.2f);
    place(Planter,-6,0,0,1.15f);place(Planter,6,0,0,1.15f);
    if(cluster%3==0)place(FreightCrate,2,20,.2f);
   }else if(zone==1){
    place(WaterTank,-5,0,0,1.35f,true,1.7f);place(PumpSkid,3,2,0,1.35f,true,2.3f);
    place(Planter,-3,10,0,1.4f);place(Planter,4,10,0,1.4f);
   }else if(zone==2){
    place(SolarArray,-5,3,.15f,1.3f,true,3.1f);place(SolarArray,6,4,.15f,1.3f,true,3.1f);
    if(cluster%3==0)place(WindPump,0,17,0,1.35f,true,1.5f);
   }else{
    place(PumpSkid,-5,1,0,1.3f,true,2.2f);place(WaterTank,3,0,0,1.5f,true,1.6f);
    place(Pallet,-4,8,.2f);place(TransitShelter,7,12,0,1.1f,true,4.5f);
   }
  }else if(ice){
   if(zone==0){
    place(WeatherStation,-5,0,0,1.3f,true,1.7f);place(SnowFence,5,2,0,1.4f);
    place(FreightCrate,-4,8,.1f,.8f,true,2.4f);
   }else if(zone==1){
    place(SnowFence,-5,0,0,1.4f);place(SnowFence,5,0,0,1.4f);
    place(RescueSled,3,7,.4f,1.3f);place(AlpinePine,-8,10,0,1.5f);
   }else if(zone==2){
    place(SignalMast,0,5,0,1.6f,true,1.4f);place(SolarArray,6,8,0,1.3f,true,3.1f);
    place(WeatherStation,-7,0,.3f);place(FreightCrate,3,0,0,1.1f,true,2.5f);
   }else{
    place(RescueSled,-4,0,.3f,1.1f);place(TransitShelter,5,8,0,1.0f,true,4.5f);
    place(SnowFence,-5,8,0,1.2f);
   }
   for(int tree=0;tree<3;tree++)place(AlpinePine,tree*8-8,17+tree%2*8,cluster*.42f,1.3f+.22f*(cluster%3),false,2.2f);
   if(cluster%2==0){V3 rock=anchor+right*(side*25.f);rock.y=ground(rock.x,rock.z)-1;add(f,Primitive(Rock0+cluster%6),translate(rock)*ry(cluster*.73f)*scale({8,9.f+cluster%4*3,7}),{{.60f,.68f,.71f},0,.89f,.04f,26});}
  }else{
   if(zone==0){place(TransitShelter,0,0,0,1,true,4.4f);place(SignalMast,7,6,0,1.1f,true,1.2f);}
   else if(zone==1){place(VentBank,-5,0,0,1.15f,true,2.6f);place(SolarArray,4,4,0,1.2f,true,3.2f);place(WaterTank,6,12,0,1.2f,true,1.5f);}
   else if(zone==2){place(FreightCrate,-4,0,0,1.2f,true,3.0f);place(FreightCrate,4,5,0,1.1f,true,2.7f);place(Pallet,-7,7,0,1,true,1.4f);}
   else{place(SignalMast,-4,0,0,1.5f,true,1.5f);place(VentBank,4,4,0,1.3f,true,2.6f);place(TransitShelter,7,12,0,1,true,4.4f);}
  }
  if(garden||bliss){
   if(garden||zone==3||cluster%5==0){V3 pos=anchor+right*(side*20.f);pos.y=ground(pos.x,pos.z);if(clear(pos,5))meadowTree(f,pos,1.3f+cluster%3*.18f,cluster);}
   for(int j=0;j<5;j++){
    V3 pos=anchor+forward*(j*3.f-6)+right*(side*(7.f+j%2*5));pos.y=ground(pos.x,pos.z);
    if(clear(pos,1))add(f,Fern,translate(pos)*ry(j+cluster*.31f)*scale({.8f,.8f,.8f}),{{.24f,.37f,.14f},0,.94f,0});
   }
  }
 }
 if(bliss||garden)distantCity(f,{-50,-8,-330},26,13);
 if(track.kind==0){
  distantCity(f,{0,-30,-210},32,12);distantCity(f,{0,-30,230},28,13);
  for(int block=0;block<5;block++)distantCity(f,{0,-30,-65.f+block*30},9,22);
  for(int street=-6;street<=6;street++){
   Material asphalt{{.23f,.25f,.27f},0,.94f,0};
   worldBox(f,{street*32.f,-29.97f,0},{3.4f,.025f,280},asphalt);
   worldBox(f,{0,-29.97f,street*32.f},{280,.025f,3.4f},asphalt);
  }
 }
 if(ice)for(int ridge=0;ridge<14;ridge++){
  float angle=2*pi*ridge/14;V3 pos{std::cos(angle)*330,0,std::sin(angle)*330};pos.y=ground(pos.x,pos.z)-8;
  add(f,Primitive(Rock0+ridge%6),translate(pos)*ry(angle)*scale({64,48.f+ridge%3*19,65}),{{.58f,.70f,.78f},0,.95f,0,26});
 }
 (void)time;
}
}
