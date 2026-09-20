// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "assets.hpp"
namespace sw {
inline void loadPropMeshes(Frame& f){
 const char* names[]={"crt-body","crt-glass","server-body","server-face","cable-coil","salvage-gantry","fern","wind-pump",
  "kart-0","kart-1","kart-2","kart-3","kart-4","kart-5","kart-6","kart-7",
  "salvage-bin","cable-reel","appliance-pile","compactor","conveyor","scrap-car","pallet","solar-array","greenhouse","water-tank","pump-skid","planter","vent-bank","transit-shelter","freight-crate","signal-mast","snow-fence","weather-station","rescue-sled","alpine-pine"};
 static_assert(sizeof(names)/sizeof(*names)==Count-CrtBody);
 for(int i=0;i<Count-CrtBody;i++){
  auto path=arcade::assets()/"models"/(std::string(names[i])+".qmesh");std::ifstream in(path);
  if(!in)throw std::runtime_error("Missing game asset: "+path.string());
  std::string magic;std::size_t count;in>>magic>>count;
  if(!in||(magic!="QMS1"&&magic!="QMS2")||count>250000||count%3)throw std::runtime_error("Invalid mesh: "+path.string());
  auto& mesh=f.batches[CrtBody+i].mesh;mesh.name=names[i];mesh.v.reserve(count);mesh.ix.reserve(count);
  for(std::size_t j=0;j<count;j++){
   Vertex v;in>>v.p.x>>v.p.y>>v.p.z>>v.n.x>>v.n.y>>v.n.z>>v.uv.x>>v.uv.y;
   if(magic=="QMS2"){in>>v.tint.x>>v.tint.y>>v.tint.z>>v.finish.x>>v.finish.y>>v.finish.z;v.tint.w=1;}
   if(!in||!std::isfinite(v.p.x+v.p.y+v.p.z+v.n.x+v.n.y+v.n.z+v.uv.x+v.uv.y+v.tint.x+v.tint.y+v.tint.z+v.finish.x+v.finish.y+v.finish.z))throw std::runtime_error("Corrupt mesh: "+path.string());
   mesh.v.push_back(v);mesh.ix.push_back(j);
  }
 }
}
inline void crt(Frame& f,M4 m,int variant=0){const V3 cases[]={{.68f,.67f,.56f},{.36f,.44f,.43f},{.22f,.24f,.25f},{.67f,.57f,.41f}};add(f,CrtBody,m,{cases[variant%4],0,.74f,0,33});add(f,CrtGlass,m,{{.035f,.075f,.071f},0,.22f,.28f});}
inline void serverRack(Frame& f,M4 m){add(f,ServerBody,m,{{.39f,.46f,.44f},0,.64f,.35f,28});add(f,ServerFace,m,{{.57f,.68f,.54f},0,.8f,.25f,22});}
inline void authoredProp(Frame& f,Primitive type,M4 m,V3 tint={1,1,1}){
 if(type==CableCoil){add(f,type,m,{{.065f,.069f,.061f},0,.91f,0,27});return;}
 if(type==WindPump){add(f,type,m,{{.57f,.64f,.58f},0,.65f,.3f,28});return;}
 add(f,type,m,{tint,0,.65f,.1f});
}
}
