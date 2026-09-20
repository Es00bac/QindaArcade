// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "racer_profiles.hpp"
#include <cairo.h>
#include <algorithm>
#include <cmath>
#include <string>
namespace arcade {
// Four independently mapped 512x256 islands per driver. Native editable graphics,
// not a flattened texture stretched over a whole car. Used by runtime and exporter.
inline void paintLiveries(cairo_t* c){
 const double colors[8][3]={{.08,.48,.38},{.84,.52,.08},{.73,.08,.12},{.12,.35,.56},{.5,.36,.71},{.75,.10,.43},{.36,.46,.16},{.80,.35,.43}};
 auto color=[&](double r,double g,double b){cairo_set_source_rgb(c,r,g,b);};
 auto rect=[&](double x,double y,double w,double h){cairo_rectangle(c,x,y,w,h);cairo_fill(c);};
 auto label=[&](const std::string& s,double y,double size,double width=454){
  cairo_select_font_face(c,"DejaVu Sans",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_BOLD);cairo_set_font_size(c,size);
  cairo_text_extents_t e;cairo_text_extents(c,s.c_str(),&e);if(e.width>width){cairo_set_font_size(c,size*width/e.width);cairo_text_extents(c,s.c_str(),&e);}
  cairo_move_to(c,256-e.width*.5-e.x_bearing,y);cairo_show_text(c,s.c_str());
 };
 for(int id=0;id<8;id++)for(int col=0;col<4;col++){
  cairo_save(c);cairo_translate(c,col*512,id*256);cairo_rectangle(c,0,0,512,256);cairo_clip(c);
  const auto& p=profiles[id];auto co=colors[id];
  color(.84,.84,.73);rect(0,0,512,256);
  color(co[0],co[1],co[2]);rect(9,9,494,238);
  color(.055,.068,.073);rect(18,18,476,220);
  if(col==0){
   color(.89,.89,.79);label("0"+std::to_string(id+1),195,187);
   color(co[0],co[1],co[2]);rect(27,29,45,198);rect(440,29,45,198);
  }else if(col==1){
   color(.90,.89,.79);label(p.sponsor,117,53);color(co[0]*.4+.6,co[1]*.4+.6,co[2]*.4+.6);label(p.sponsorLine,175,31);
   color(.87,.86,.74);rect(91,199,330,4);label("INDEPENDENT RACING / 0"+std::to_string(id+1),222,14);
  }else if(col==2){
   color(.88,.85,.69);rect(18,18,476,220);color(.065,.072,.075);
   // Sticker meshes are 4.8:1; compensate the UV island's 2:1 aspect here.
   cairo_save(c);cairo_scale(c,1,2.25);label(p.sticker,73,37,450);cairo_restore(c);
   color(co[0],co[1],co[2]);rect(30,32,452,12);rect(30,212,452,12);
  }else{
   cairo_set_operator(c,CAIRO_OPERATOR_CLEAR);cairo_paint(c);cairo_set_operator(c,CAIRO_OPERATOR_OVER);
   color(.9,.88,.75);cairo_set_line_width(c,14);
   if(id==0){rect(230,47,52,163);rect(168,105,176,48);}
   if(id==1){cairo_arc(c,256,139,76,3.14159,6.28318);cairo_stroke(c);cairo_move_to(c,256,139);cairo_line_to(c,301,80);cairo_stroke(c);label("PSI",207,30);}
   if(id==2){cairo_move_to(c,176,54);cairo_line_to(c,342,204);cairo_move_to(c,332,42);cairo_line_to(c,173,213);cairo_stroke(c);}
   if(id==3){cairo_rectangle(c,175,62,161,133);cairo_stroke(c);cairo_move_to(c,175,101);cairo_line_to(c,336,101);cairo_move_to(c,254,62);cairo_line_to(c,254,195);cairo_stroke(c);}
   if(id==4){cairo_arc(c,256,128,75,0,6.28318);cairo_stroke(c);for(int j=0;j<8;j++){double a=j*6.28318/8;cairo_move_to(c,256+cos(a)*62,128+sin(a)*62);cairo_line_to(c,256+cos(a)*89,128+sin(a)*89);cairo_stroke(c);}}
   if(id==5){cairo_move_to(c,166,55);cairo_line_to(c,256,205);cairo_line_to(c,346,55);cairo_stroke(c);}
   if(id==6){for(int j=0;j<5;j++){double a=j*6.28318/5;cairo_arc(c,256+cos(a)*53,128+sin(a)*53,30,0,6.28318);cairo_fill(c);}color(co[0],co[1],co[2]);cairo_arc(c,256,128,24,0,6.28318);cairo_fill(c);}
   if(id==7){for(int j=0;j<3;j++){cairo_move_to(c,138,81+j*49);cairo_curve_to(c,199,21+j*49,300,141+j*49,374,81+j*49);cairo_stroke(c);}}
   // Deterministic small spray overspray; Hex's branding remains immaculate.
   if(id!=5)for(int k=0;k<130;k++){double x=std::fmod(k*137.7+id*79,456)+28,y=std::fmod(k*61.13+id*53,208)+24;
    cairo_set_source_rgba(c,.91,.88,.73,.09);cairo_arc(c,x,y,1.1+(k%3),0,6.28318);cairo_fill(c);}
  }
  // Nicks belong at the decal edge, not random dirt obscuring the sponsor.
  if(id!=5&&col!=3)for(int j=0;j<18;j++){double x=31+std::fmod(j*91.7+id*29,451),y=j%2?22:229;color(.62,.62,.53);rect(x,y,3+j%7,3+j%5);}
  cairo_restore(c);
 }
}
}
