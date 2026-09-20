#include "livery.hpp"
#include <cairo-svg.h>
#include <filesystem>
#include <fstream>
#include <iostream>
int main(int argc,char** argv){
 if(argc!=2)return 2;std::filesystem::path out=argv[1];std::filesystem::create_directories(out);
 for(int svg=0;svg<2;svg++){
  auto path=out/(svg?"racer-liveries-v2.svg":"racer-liveries-v2.png");
  auto* s=svg?cairo_svg_surface_create(path.c_str(),2048,2048):cairo_image_surface_create(CAIRO_FORMAT_ARGB32,2048,2048);
  auto* c=cairo_create(s);arcade::paintLiveries(c);cairo_destroy(c);
  if(!svg&&cairo_surface_write_to_png(s,path.c_str()))return 1;
  cairo_surface_finish(s);if(cairo_surface_status(s))return 1;cairo_surface_destroy(s);std::cout<<path<<'\n';
 }
 for(int id=0;id<8;id++){
  std::ofstream f(out/("kart-"+std::to_string(id)+"-uv.svg"));
  f<<"<svg xmlns='http://www.w3.org/2000/svg' width='2048' height='256' viewBox='0 0 2048 256'><rect width='2048' height='256' fill='#18252a'/>";
  const char* labels[]={"NUMBER / planar nose","SPONSOR / planar panel","BUMPER / aspect 4.8:1","EMBLEM / spray or mark"};
  for(int col=0;col<4;col++)f<<"<rect x='"<<col*512+12.8<<"' y='6.4' width='486.4' height='243.2' fill='none' stroke='#73e0bf' stroke-width='3'/><text x='"<<col*512+30<<"' y='130' fill='white' font-family='sans-serif' font-size='22'>"<<labels[col]<<"</text>";
  f<<"</svg>";if(!f)return 1;
 }
}
