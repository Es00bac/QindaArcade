// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "ui_state.hpp"
#include <cairo.h>
#include <algorithm>
#include <cmath>
namespace arcade {
inline void paintUi(cairo_t* c,int width,int height,const Ui& ui){
 const double scale=std::min(width/1440.,height/810.);
 const double ox=(width-1440*scale)*.5,oy=(height-810*scale)*.5;
 cairo_save(c);cairo_translate(c,ox,oy);cairo_scale(c,scale,scale);
 auto fill=[&](double r,double g,double b,double a=1){cairo_set_source_rgba(c,r,g,b,a);};
 auto rect=[&](double x,double y,double w,double h){cairo_rectangle(c,x,y,w,h);cairo_fill(c);};
 auto text=[&](double x,double y,double size,const std::string& label,bool bold=false){cairo_select_font_face(c,"DejaVu Sans",CAIRO_FONT_SLANT_NORMAL,bold?CAIRO_FONT_WEIGHT_BOLD:CAIRO_FONT_WEIGHT_NORMAL);cairo_set_font_size(c,size);cairo_move_to(c,x,y);cairo_show_text(c,label.c_str());};
 auto rule=[&](double x,double y,double w){fill(.72,.83,.84,.18);rect(x,y,w,1);};
 const double ar=ui.racing?.99:.30,ag=ui.racing?.72:.92,ab=ui.racing?.27:.78;
 if(ui.page==Page::Playing){
  double hintY=ui.racing?684:655;fill(.025,.045,.055,.70);rect(29,hintY,220,25);fill(.81,.88,.90);
  text(41,hintY+17,10,ui.exhibition?"AI EXHIBITION  /  ESC Back":"P"+std::to_string(ui.player+1)+"   /   "+(ui.playstation?"OPTIONS":"ESC / START")+"  Pause",true);
  if(!ui.notice.empty()){fill(1,.75,.28);text(640,701,12,ui.notice,true);}
  cairo_restore(c);return;
 }
 // An opaque navigation surface and a soft vignette keep menu type readable over live 3D.
 auto* gradient=cairo_pattern_create_linear(0,0,1440,0);
 cairo_pattern_add_color_stop_rgba(gradient,0,.012,.025,.035,.99);
 cairo_pattern_add_color_stop_rgba(gradient,.37,.017,.032,.043,.96);
 cairo_pattern_add_color_stop_rgba(gradient,.68,.017,.032,.043,.35);
 cairo_pattern_add_color_stop_rgba(gradient,1,.017,.032,.043,.07);
 cairo_set_source(c,gradient);rect(-ox/scale,-oy/scale,width/scale,height/scale);cairo_pattern_destroy(gradient);
 fill(ar,ag,ab);rect(54,48,29,5);fill(.89,.94,.94);text(96,57,14,"Q I N D A   /   A R C A D E",true);
 fill(.69,.79,.79);text(1116,57,11,ui.racing?"VOLUME 02  /  PRISM KART":"VOLUME 01  /  MEGA BRAWL",true);
 fill(ar,ag,ab);text(54,113,11,ui.subtitle,true);
 fill(.96,.97,.94);text(50,181,50,ui.title,true);
 fill(.64,.75,.76);text(54,215,13,ui.racing?"Eight racers. One extraordinary world.":"Small heroes. Unfinished business.");
 double y=268;
 for(std::size_t i=0;i<ui.rows.size();i++){
  bool selected=int(i)==ui.selected;
  fill(selected?.09:.03,selected?.16:.065,selected?.17:.085,selected?.96:.75);rect(54,y,443,49);
  if(selected){fill(ar,ag,ab);rect(54,y,4,49);fill(.96,.98,.94);}else fill(.67,.77,.79);
  text(74,y+30,17,ui.rows[i].label,selected);
  if(!ui.rows[i].value.empty()){
   cairo_text_extents_t ext;cairo_set_font_size(c,13);cairo_text_extents(c,ui.rows[i].value.c_str(),&ext);
   fill(selected?ar:.67,selected?ag:.77,selected?ab:.79);text(477-ext.width,y+30,13,ui.rows[i].value,true);
  }
  y+=57;
 }
 if(!ui.standings.empty()){
  fill(.018,.04,.05,.94);rect(734,253,652,443);
  fill(ar,ag,ab);text(764,288,12,ui.standingsTitle,true);rule(764,308,592);
  for(std::size_t i=0;i<ui.standings.size();i++){
   double sy=339+i*43;fill(i==0?ar:.65,i==0?ag:.78,i==0?ab:.79);text(764,sy,16,std::to_string(i+1),true);
   fill(.91,.95,.93);text(802,sy,17,ui.standings[i].label,true);
   cairo_text_extents_t ext;cairo_set_font_size(c,14);cairo_text_extents(c,ui.standings[i].value.c_str(),&ext);
   fill(.65,.81,.80);text(1355-ext.width,sy,14,ui.standings[i].value);rule(764,sy+14,592);
  }
  fill(ar,ag,ab);text(734,731,17,ui.description,true);
 }else if(!ui.profileKart.empty()){
  fill(.018,.04,.05,.94);rect(552,590,834,141);fill(ar,ag,ab);rect(552,590,4,141);
  fill(.93,.95,.88);text(575,620,22,ui.profileKart,true);
  fill(ar,ag,ab);text(575,646,11,ui.profileRole,true);
  fill(.66,.79,.79);text(575,671,12,ui.profileStory);
  fill(.54,.71,.71);text(575,708,11,"BACKED BY  /  "+ui.profileSponsor,true);
 }else if(!ui.description.empty()){
  fill(.018,.04,.05,.84);rect(838,618,548,112);fill(ar,ag,ab);rect(838,618,4,112);
  fill(.88,.94,.92);text(862,653,19,ui.description,true);
  fill(.63,.76,.77);text(862,682,12,ui.racing?"Follow the route. Find your line. Make it count.":"Readable edges. Room to recover. A world worth fighting for.");
  fill(.46,.64,.65);text(862,707,10,"LIVE GAME WORLD  /  ORIGINAL QINDA CHARACTERS",true);
 }
 if(!ui.mapping.empty()){
  fill(.017,.038,.048,.98);rect(553,277,823,220);fill(ar,ag,ab);text(585,321,15,"CONTROLLER SETUP",true);
  fill(.92,.96,.94);text(585,380,23,ui.mapping,true);fill(.65,.77,.78);text(585,432,13,"Release each control before the next step. Esc cancels.");
 }
 if(!ui.notice.empty()){fill(1,.77,.32);text(54,708,12,ui.notice,true);}
 rule(54,743,1332);
 fill(.76,.85,.84);text(54,775,12,ui.playstation?"D-PAD  Navigate    CROSS  Select    CIRCLE  Back":"ARROWS / D-PAD  Navigate    ENTER / A  Select    ESC / B  Back",true);
 fill(.53,.69,.70);std::string device=ui.device;if(device.size()>47)device=device.substr(0,44)+"...";text(982,775,11,device);
 cairo_restore(c);
}
}
