#pragma once
#include "hud_canvas.hpp"
namespace sw {
inline void paintGameHud(cairo_t* c,const Frame& f,int w,int h){
 arcade::HudCanvas ui(c,w,h);ui.panel(28,26,300,73);ui.color(.99,.73,.28);ui.box(28,26,4,73);
 ui.text(45,50,11,"QINDA  /  PRISM KART",true);ui.color(.93,.96,.93);ui.text(45,80,16,f.courseName,true);
 ui.panel(1230,26,180,111);ui.color(.99,.77,.31);ui.text(1248,97,64,std::to_string(f.position),true);ui.color(.70,.82,.83);ui.text(1326,63,19,"/ 8",true);ui.text(1326,94,12,"POSITION");
 ui.panel(1026,26,190,73);ui.color(.93,.96,.93);ui.text(1046,57,22,"LAP "+std::to_string(f.lap)+" / 3",true);
 int seconds=std::max(0,int(f.raceTime));std::string timer=std::to_string(seconds/60)+":"+(seconds%60<10?"0":"")+std::to_string(seconds%60);ui.color(.58,.74,.76);ui.text(1046,83,14,timer+"  RACE TIME");
 ui.panel(28,720,360,66);V3 co=racerColors[f.focus];ui.color(co.x,co.y,co.z);ui.box(28,720,4,66);ui.text(46,744,13,(f.ui.exhibition?"CPU":"P"+std::to_string(f.ui.player+1))+std::string("  ")+racerNames[f.focus],true);
 ui.color(.94,.97,.94);ui.text(46,775,29,std::to_string(int(f.speed)),true);ui.color(.60,.75,.77);ui.text(106,775,12,"KM/H");
 for(int i=0;i<3;i++){bool charged=f.driftCharge>(i==0?.45f:i==1?.9f:1.5f);ui.color(charged?(i==2?.98:.24):.12,charged?(i==2?.71:.81):.22,charged?(i==2?.25:.91):.24);ui.box(220+i*45,760,36,7);}
 ui.color(.63,.77,.79);ui.text(220,749,10,f.boost>0?"TURBO ACTIVE":"DRIFT CHARGE",true);
 ui.panel(498,734,444,52);ui.color(f.itemLabel.empty()?.51:.34,f.itemLabel.empty()?.67:.94,f.itemLabel.empty()?.69:.75);ui.text(520,767,16,f.itemLabel.empty()?"PICK UP AN ITEM BOX":f.itemLabel,true);
 if(f.itemReady){ui.color(.69,.81,.81);ui.text(839,767,11,f.ui.playstation?"SQUARE":"X / J",true);}
 ui.panel(1226,597,184,189);ui.color(.61,.76,.77);ui.text(1243,622,11,"CIRCUIT",true);
 if(!f.map.empty()){
  double mx=1318,my=704,size=76;cairo_set_line_width(c,4);ui.color(.33,.48,.50);
  for(std::size_t i=0;i<f.map.size();i++){auto p=f.map[i];if(i)cairo_line_to(c,mx+p.x*size,my+p.y*size);else cairo_move_to(c,mx+p.x*size,my+p.y*size);}cairo_stroke(c);
  for(int i=7;i>=0;i--){V3 color=racerColors[i];ui.color(color.x,color.y,color.z);ui.dot(mx+f.mapX[i]*size,my+f.mapY[i]*size,i==f.focus?5:3);}
 }
 if(f.countdown>0){ui.panel(659,155,122,130);ui.color(.99,.79,.36);ui.text(687,254,89,std::to_string(f.countdown),true);}
 if(f.driftCharge>.45f){ui.color(.44,.95,.86);ui.text(545,703,16,"RELEASE DRIFT FOR TURBO",true);}
}
}
