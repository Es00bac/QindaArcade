#pragma once
#include <cairo.h>
#include <algorithm>
#include <string>
namespace arcade {
struct HudCanvas {
 cairo_t* c;
 HudCanvas(cairo_t* context,int w,int h):c(context){double s=std::min(w/1440.,h/810.);cairo_save(c);cairo_translate(c,(w-1440*s)*.5,(h-810*s)*.5);cairo_scale(c,s,s);}
 ~HudCanvas(){cairo_restore(c);}
 void color(double r,double g,double b,double a=1){cairo_set_source_rgba(c,r,g,b,a);}
 void box(double x,double y,double w,double h){cairo_rectangle(c,x,y,w,h);cairo_fill(c);}
 void panel(double x,double y,double w,double h){color(.018,.037,.047,.90);box(x,y,w,h);}
 void text(double x,double y,double size,const std::string& value,bool bold=false){cairo_select_font_face(c,"DejaVu Sans",CAIRO_FONT_SLANT_NORMAL,bold?CAIRO_FONT_WEIGHT_BOLD:CAIRO_FONT_WEIGHT_NORMAL);cairo_set_font_size(c,size);cairo_move_to(c,x,y);cairo_show_text(c,value.c_str());}
 void dot(double x,double y,double radius){cairo_arc(c,x,y,radius,0,6.283185307);cairo_fill(c);}
};
}
