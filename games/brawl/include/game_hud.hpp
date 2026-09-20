#pragma once
#include "hud_canvas.hpp"
namespace sw {
inline void paintGameHud(cairo_t* c,const Frame& f,int w,int h){
 arcade::HudCanvas ui(c,w,h);ui.panel(28,26,312,73);ui.color(.30,.92,.78);ui.box(28,26,4,73);ui.text(45,50,11,"QINDA  /  MEGA BRAWL",true);ui.color(.92,.96,.93);ui.text(45,80,16,f.courseName,true);
 ui.panel(1241,26,169,73);int seconds=std::min(90,f.secondsLeft);ui.color(.93,.96,.93);ui.text(1263,77,35,std::to_string(seconds/60)+":"+(seconds%60<10?"0":"")+std::to_string(seconds%60),true);
 double card=std::min(236.,1296./f.active),gap=12,left=(1440-(card+gap)*f.active+gap)*.5;
 for(int i=0;i<f.active;i++){
  double x=left+i*(card+gap),y=713;V3 color=racerColors[f.roster[i]];
  ui.panel(x,y,card,74);ui.color(color.x,color.y,color.z,f.stocks[i]>0?1:.35);ui.box(x,y,card,3);
  ui.text(x+12,y+25,13,(!f.ui.exhibition&&i<f.ui.players?"P"+std::to_string(i+1):"CPU")+std::string("  ")+racerNames[f.roster[i]],true);
  float damage=clamp(f.damage[i]/160);ui.color(1,.95-.58*damage,.89-.67*damage);ui.text(x+12,y+59,31,f.stocks[i]>0?std::to_string(int(f.damage[i]))+"%":"OUT",true);
  for(int stock=0;stock<3;stock++){ui.color(color.x,color.y,color.z,stock<f.stocks[i]?1:.18);ui.dot(x+card-49+stock*14,y+48,4);}
  ui.color(color.x*.5,color.y*.6,color.z*.6);ui.box(x+12,y+66,(card-24)*f.shield[i],2);
  if(!f.specials[i].empty()){ui.color(color.x,color.y,color.z);ui.text(x+6,y-12,12,f.specials[i],true);}
 }
 if(f.countdown>0){ui.panel(659,138,122,124);ui.color(.49,.96,.84);ui.text(685,235,86,std::to_string(f.countdown),true);}
}
}
