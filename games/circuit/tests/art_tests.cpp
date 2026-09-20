// SPDX-License-Identifier: GPL-3.0-or-later
#include "race.hpp"
#include "racer_profiles.hpp"
#include <iostream>
#include <set>
using namespace sw;
static void check(bool b,const char* message){if(!b)throw std::runtime_error(message);}
int main(){try{
 Frame frame;initialize(frame);std::set<std::string> names;std::set<size_t> silhouettes;
 for(int k=Kart0;k<Count;k++){
  const auto& mesh=frame.batches[k].mesh;check(!mesh.v.empty()&&mesh.ix.size()%3==0,"Authored mesh must contain triangles");
  check(names.insert(mesh.name).second,"Every asset has a unique name");
  for(const auto& v:mesh.v){
   check(v.tint.w==1,"QMS2 face material survives loading");
   check(std::isfinite(v.p.x+v.p.y+v.p.z+v.uv.x+v.uv.y),"Position and UV must be finite");
   check(v.finish.x>=0&&v.finish.x<=1&&v.finish.y>=0&&v.finish.y<=1,"PBR parameters must be in range");
   check(length(v.n)>.98f&&length(v.n)<1.02f,"Authored normals must be normalized");
   if(v.finish.z>15.5f&&v.finish.z<17.f){check(v.uv.x>=0&&v.uv.x<=1&&v.uv.y>=0&&v.uv.y<=1,"Livery islands remain in the padded atlas");}
  }
 }
 for(int id=0;id<8;id++){
  for(auto& b:frame.batches)b.instances.clear();kart(frame,M4::identity(),id,0,0,0,0,0);
  check(frame.batches[Kart0+id].instances.size()==1,"Racer uses its own authored chassis");
  silhouettes.insert(frame.batches[Kart0+id].mesh.ix.size());
  std::set<std::pair<int,int>> contacts;
  for(const auto& wheel:frame.batches[Cylinder].instances)if(wheel.surface.z==15){
   auto center=point(wheel.model,{0,0,0});check(std::abs(std::abs(center.x)-1.17f)<.001f&&std::abs(std::abs(center.z)-1.03f)<.001f&&std::abs(center.y-.49f)<.001f,"All eight karts keep identical tire contact geometry");
   check(std::abs(length(direction(wheel.model,{1,0,0}))-.48f)<.001f,"Equal tire radius across the roster");contacts.insert({center.x>0?1:-1,center.z>0?1:-1});
  }
  check(contacts.size()==4,"Four distinct wheels per racer");
  check(std::string(arcade::profiles[id].kart).size()>3&&std::string(arcade::profiles[id].story).size()>40,"Every racer has a machine and origin");
 }
 check(silhouettes.size()==8,"Eight different body meshes, not recolors");
 for(int id:{1,4,7})check(driverScale(id)<driverScale(0)*.81f,"Ducke, Mochi and Axi are smaller drivers");
 for(int course=0;course<4;course++){
  Race race(41,course);buildTrackMeshes(frame,race.track);SceneOptions options;options.camera=Camera::Overview;
  compose(frame,race.sample(),race.track,options);int types=0,instances=0;
  for(int k=SalvageBin;k<Count;k++)if(!frame.batches[k].instances.empty()){types++;instances+=frame.batches[k].instances.size();}
  check(types>=5&&instances>=35,"Each track uses a varied authored scenery kit");
  std::cout<<race.track.name()<<": "<<types<<" new prop types / "<<instances<<" placed instances\n";
 }
 std::cout<<"PASS: 28 authored meshes, PBR/UV data, eight distinct bodies, equal wheel geometry, small drivers, four varied track kits\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
