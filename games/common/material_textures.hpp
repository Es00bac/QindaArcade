// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "assets.hpp"
#include <algorithm>
#include <vector>
#include <stdexcept>
namespace sw {
inline void replaceMaterialTexture(GLuint& target,const char* name){
 auto file=arcade::assets()/"textures"/name;auto* s=cairo_image_surface_create_from_png(file.c_str());
 if(cairo_surface_status(s)!=CAIRO_STATUS_SUCCESS){cairo_surface_destroy(s);throw std::runtime_error("Cannot load material: "+file.string());}
 int w=cairo_image_surface_get_width(s),h=cairo_image_surface_get_height(s),stride=cairo_image_surface_get_stride(s);
 auto* source=cairo_image_surface_get_data(s);std::vector<unsigned char> rgba(size_t(w)*h*4);
 for(int y=0;y<h;y++)for(int x=0;x<w;x++){auto* a=source+y*stride+x*4;auto* b=rgba.data()+(size_t(y)*w+x)*4;int alpha=a[3];b[0]=alpha?std::min(255,a[2]*255/alpha):0;b[1]=alpha?std::min(255,a[1]*255/alpha):0;b[2]=alpha?std::min(255,a[0]*255/alpha):0;b[3]=alpha;}
 if(target)glDeleteTextures(1,&target);
 glGenTextures(1,&target);glBindTexture(GL_TEXTURE_2D,target);
 glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,rgba.data());glGenerateMipmap(GL_TEXTURE_2D);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
 // Optional anisotropy is queried, not assumed; long grazing track surfaces benefit.
 GLint extensions=0;glGetIntegerv(GL_NUM_EXTENSIONS,&extensions);bool anisotropic=false;
 for(int i=0;i<extensions;i++){const char* e=reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS,i));if(e&&std::string(e)=="GL_EXT_texture_filter_anisotropic")anisotropic=true;}
 if(anisotropic){GLfloat limit=1;glGetFloatv(0x84FF,&limit);glTexParameterf(GL_TEXTURE_2D,0x84FE,std::min(8.f,limit));}
 cairo_surface_destroy(s);
}
}
