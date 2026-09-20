// SPDX-License-Identifier: GPL-3.0-or-later
#include "renderer.hpp"
#include "shader_strings.hpp"
#include "cairo_abi.hpp"
#include "ui_painter.hpp"
#include "game_hud.hpp"
#include "assets.hpp"
#include "material_textures.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstring>
#include <chrono>
#include <cstdlib>
namespace sw {
namespace {
GLuint program(const char* vert,const char* frag){
 auto shader=[](GLenum kind,const char* source){GLuint s=glCreateShader(kind);glShaderSource(s,1,&source,nullptr);glCompileShader(s);GLint ok=0;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);if(!ok){GLint l=0;glGetShaderiv(s,GL_INFO_LOG_LENGTH,&l);std::string log(std::max(1,l),' ');glGetShaderInfoLog(s,l,nullptr,log.data());glDeleteShader(s);throw std::runtime_error("GLSL compile: "+log);}return s;};
 GLuint vs=shader(GL_VERTEX_SHADER,vert),fs=shader(GL_FRAGMENT_SHADER,frag),p=glCreateProgram();glAttachShader(p,vs);glAttachShader(p,fs);glLinkProgram(p);glDeleteShader(vs);glDeleteShader(fs);GLint ok=0;glGetProgramiv(p,GL_LINK_STATUS,&ok);if(!ok){GLint l;glGetProgramiv(p,GL_INFO_LOG_LENGTH,&l);std::string log(std::max(1,l),' ');glGetProgramInfoLog(p,l,nullptr,log.data());glDeleteProgram(p);throw std::runtime_error("GLSL link: "+log);}return p;
}
void uniform(GLuint p,const char* n,int v){glUniform1i(glGetUniformLocation(p,n),v);}
void uniform(GLuint p,const char* n,float v){glUniform1f(glGetUniformLocation(p,n),v);}
void uniform(GLuint p,const char* n,V3 v){glUniform3f(glGetUniformLocation(p,n),v.x,v.y,v.z);}
void uniform(GLuint p,const char* n,M4 m){glUniformMatrix4fv(glGetUniformLocation(p,n),1,GL_FALSE,m.data());}
void tex(GLuint t,int unit){glActiveTexture(GL_TEXTURE0+unit);glBindTexture(GL_TEXTURE_2D,t);}
GLuint texture(int w,int h,GLint format,GLenum input=GL_RGBA,GLenum type=GL_FLOAT,const void* data=nullptr){GLuint t;glGenTextures(1,&t);glBindTexture(GL_TEXTURE_2D,t);glTexImage2D(GL_TEXTURE_2D,0,format,w,h,0,input,type,data);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);return t;}
void checkFbo(){if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)throw std::runtime_error("Incomplete OpenGL framebuffer");}
}
Renderer::Renderer(const Frame& f,int w,int h,RenderOptions options):opt_(options){
 {
  auto file=arcade::assets()/"textures/reclaimed-atlas.png";auto* surface=cairo_image_surface_create_from_png(file.c_str());
  if(cairo_surface_status(surface)!=CAIRO_STATUS_SUCCESS){auto message=std::string(cairo_status_to_string(cairo_surface_status(surface)));cairo_surface_destroy(surface);throw std::runtime_error("Environment texture: "+message);}
  int tw=cairo_image_surface_get_width(surface),th=cairo_image_surface_get_height(surface),stride=cairo_image_surface_get_stride(surface);auto* bytes=cairo_image_surface_get_data(surface);std::vector<unsigned char> rgba(size_t(tw)*th*4);
  for(int y=0;y<th;y++)for(int x=0;x<tw;x++){auto* s=bytes+y*stride+x*4;auto* d=&rgba[(size_t(y)*tw+x)*4];d[0]=s[2];d[1]=s[1];d[2]=s[0];d[3]=255;}
  for(int tile=0;tile<4;tile++){int w=tw/2,h=th/2;std::vector<unsigned char> part(size_t(w)*h*4);for(int y=0;y<h;y++)std::copy_n(rgba.data()+(size_t(y+(tile/2)*h)*tw+(tile%2)*w)*4,w*4,part.data()+size_t(y)*w*4);environment_[tile]=texture(w,h,GL_RGBA8,GL_RGBA,GL_UNSIGNED_BYTE,part.data());glGenerateMipmap(GL_TEXTURE_2D);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);}cairo_surface_destroy(surface);
 }
 replaceMaterialTexture(environment_[0],"meadow-ground-v2.png");
 replaceMaterialTexture(environment_[1],"enamel-wear-v2.png");
 replaceMaterialTexture(wood_,"timber-v2.png");
 mesh_=program(shaders::mesh_vert,shaders::mesh_frag);depth_=program(shaders::mesh_vert,shaders::shadow_frag);sky_=program(shaders::full_vert,shaders::sky_frag);blur_=program(shaders::full_vert,shaders::blur_frag);post_=program(shaders::full_vert,shaders::post_frag);
 glGenVertexArrays(1,&quad_);
 for(size_t k=0;k<gpu_.size();k++){
  auto& g=gpu_[k];const auto& mesh=f.batches[k].mesh;g.count=mesh.ix.size();glGenVertexArrays(1,&g.vao);glBindVertexArray(g.vao);
  glGenBuffers(1,&g.vbo);glBindBuffer(GL_ARRAY_BUFFER,g.vbo);glBufferData(GL_ARRAY_BUFFER,mesh.v.size()*sizeof(Vertex),mesh.v.data(),GL_STATIC_DRAW);
  glGenBuffers(1,&g.ibo);glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,g.ibo);glBufferData(GL_ELEMENT_ARRAY_BUFFER,mesh.ix.size()*sizeof(std::uint32_t),mesh.ix.data(),GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,0,sizeof(Vertex),nullptr);glEnableVertexAttribArray(1);glVertexAttribPointer(1,3,GL_FLOAT,0,sizeof(Vertex),(void*)offsetof(Vertex,n));glEnableVertexAttribArray(2);glVertexAttribPointer(2,2,GL_FLOAT,0,sizeof(Vertex),(void*)offsetof(Vertex,uv));
  glEnableVertexAttribArray(9);glVertexAttribPointer(9,4,GL_FLOAT,0,sizeof(Vertex),(void*)offsetof(Vertex,tint));
  glEnableVertexAttribArray(10);glVertexAttribPointer(10,3,GL_FLOAT,0,sizeof(Vertex),(void*)offsetof(Vertex,finish));
  glGenBuffers(1,&g.instances);glBindBuffer(GL_ARRAY_BUFFER,g.instances);
  for(int i=0;i<4;i++){glEnableVertexAttribArray(3+i);glVertexAttribPointer(3+i,4,GL_FLOAT,0,sizeof(Instance),(void*)(sizeof(float)*i*4));glVertexAttribDivisor(3+i,1);}
  glEnableVertexAttribArray(7);glVertexAttribPointer(7,4,GL_FLOAT,0,sizeof(Instance),(void*)offsetof(Instance,color));glVertexAttribDivisor(7,1);
  glEnableVertexAttribArray(8);glVertexAttribPointer(8,4,GL_FLOAT,0,sizeof(Instance),(void*)offsetof(Instance,surface));glVertexAttribDivisor(8,1);
 }

 replaceMaterialTexture(hull_,"racer-liveries-v2.png");
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
 Random rng(99183);std::vector<unsigned char> noise(256*256);for(auto& n:noise)n=rng.next()&255;
 noise_=texture(256,256,GL_R8,GL_RED,GL_UNSIGNED_BYTE,noise.data());glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
 shadow_=texture(2048,2048,GL_DEPTH_COMPONENT24,GL_DEPTH_COMPONENT);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
 glGenFramebuffers(1,&shadowFbo_);glBindFramebuffer(GL_FRAMEBUFFER,shadowFbo_);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,shadow_,0);glDrawBuffer(GL_NONE);glReadBuffer(GL_NONE);checkFbo();
 glGenFramebuffers(1,&msFbo_);glGenRenderbuffers(1,&msColor_);glGenRenderbuffers(1,&msDepth_);
 glGenFramebuffers(1,&hdrFbo_);glGenFramebuffers(2,blurFbo_);
 resize(w,h);glDisable(GL_FRAMEBUFFER_SRGB);
}
Renderer::~Renderer(){for(auto g:gpu_){glDeleteBuffers(1,&g.vbo);glDeleteBuffers(1,&g.ibo);glDeleteBuffers(1,&g.instances);glDeleteVertexArrays(1,&g.vao);}for(GLuint p:{mesh_,depth_,sky_,blur_,post_})glDeleteProgram(p);glDeleteVertexArrays(1,&quad_);for(GLuint t:{hdr_,z_,shadow_,overlay_,noise_,hull_,wood_,environment_[0],environment_[1],environment_[2],environment_[3],blurTex_[0],blurTex_[1]})glDeleteTextures(1,&t);for(GLuint b:{hdrFbo_,msFbo_,shadowFbo_,blurFbo_[0],blurFbo_[1]})glDeleteFramebuffers(1,&b);glDeleteRenderbuffers(1,&msColor_);glDeleteRenderbuffers(1,&msDepth_);}
void Renderer::resize(int w,int h){
 if(w<=0||h<=0||w>8192||h>8192)throw std::runtime_error("Framebuffer dimensions outside 1..8192");
 GLint limit=0;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&limit);if(w>limit||h>limit)throw std::runtime_error("Output exceeds GPU texture limit");
 w_=w;h_=h;
 if(opt_.samples>1){GLint maxSamples=0;glGetIntegerv(GL_MAX_SAMPLES,&maxSamples);opt_.samples=std::min(opt_.samples,maxSamples);
  glBindFramebuffer(GL_FRAMEBUFFER,msFbo_);glBindRenderbuffer(GL_RENDERBUFFER,msColor_);glRenderbufferStorageMultisample(GL_RENDERBUFFER,opt_.samples,GL_RGBA16F,w,h);glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_RENDERBUFFER,msColor_);
  glBindRenderbuffer(GL_RENDERBUFFER,msDepth_);glRenderbufferStorageMultisample(GL_RENDERBUFFER,opt_.samples,GL_DEPTH_COMPONENT24,w,h);glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,msDepth_);checkFbo();
 }

 for(GLuint t:{hdr_,z_,overlay_,blurTex_[0],blurTex_[1]})if(t)glDeleteTextures(1,&t);
 hdr_=texture(w,h,GL_RGBA16F);z_=texture(w,h,GL_DEPTH_COMPONENT24,GL_DEPTH_COMPONENT);overlay_=texture(w,h,GL_RGBA8,GL_RGBA,GL_UNSIGNED_BYTE);
 glBindFramebuffer(GL_FRAMEBUFFER,hdrFbo_);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,hdr_,0);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,z_,0);checkFbo();
 for(int i=0;i<2;i++){blurTex_[i]=texture(std::max(1,w/3),std::max(1,h/3),GL_RGBA16F);glBindFramebuffer(GL_FRAMEBUFFER,blurFbo_[i]);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,blurTex_[i],0);checkFbo();}
 glBindFramebuffer(GL_FRAMEBUFFER,0);lastChapter_=-1;overlayStamp_=-1;
}
void Renderer::uploadOverlay(const Frame& f,const std::string&) {
 long stamp=opt_.titles?long(f.ui.clock*30):-2;
 if(overlayStamp_==stamp&&lastChapter_==f.focus)return;
 overlayStamp_=stamp;lastChapter_=f.focus;
 auto* s=cairo_image_surface_create(CAIRO_FORMAT_ARGB32,w_,h_);auto* c=cairo_create(s);
 cairo_set_operator(c,CAIRO_OPERATOR_CLEAR);cairo_paint(c);cairo_set_operator(c,CAIRO_OPERATOR_OVER);
 if(opt_.titles&&f.ui.page==arcade::Page::Playing)paintGameHud(c,f,w_,h_);
 if(opt_.titles)arcade::paintUi(c,w_,h_,f.ui);
 cairo_surface_flush(s);auto* bytes=cairo_image_surface_get_data(s);int stride=cairo_image_surface_get_stride(s);
 std::vector<unsigned char> rgba(size_t(w_)*h_*4);
 for(int y=0;y<h_;y++)for(int x=0;x<w_;x++){auto* src=bytes+y*stride+x*4;auto* dst=&rgba[(size_t(y)*w_+x)*4];int a=src[3];dst[0]=a?std::min(255,src[2]*255/a):0;dst[1]=a?std::min(255,src[1]*255/a):0;dst[2]=a?std::min(255,src[0]*255/a):0;dst[3]=src[3];}
 tex(overlay_,2);glTexSubImage2D(GL_TEXTURE_2D,0,0,0,w_,h_,GL_RGBA,GL_UNSIGNED_BYTE,rgba.data());cairo_destroy(c);cairo_surface_destroy(s);
}
void Renderer::draw(const Frame& f,const std::string& metrics){
 const bool profile=std::getenv("PRISM_PROFILE");auto now=std::chrono::steady_clock::now();auto tick=[&](const char* phase){if(profile){glFinish();auto p=std::chrono::steady_clock::now();std::cerr<<phase<<": "<<std::chrono::duration<double>(p-now).count()<<"\n";now=p;}};
 uploadOverlay(f,metrics);tick("overlay");
 for(size_t k=0;k<gpu_.size();k++){auto& g=gpu_[k];const auto& a=f.batches[k].instances;glBindBuffer(GL_ARRAY_BUFFER,g.instances);glBufferData(GL_ARRAY_BUFFER,a.size()*sizeof(Instance),a.data(),GL_STREAM_DRAW);}
 M4 lv=ortho(-42,42,-42,42,1,180)*lookAt(f.pengu+V3{-43,68,52},f.pengu);
 M4 vp=perspective(f.fov,float(w_)/h_,.15f,1200)*lookAt(f.eye,f.target,f.up);
 auto geometry=[&](GLuint p,M4 matrix){glUseProgram(p);uniform(p,"vp",matrix);uniform(p,"lightVP",lv);uniform(p,"time",float(f.motionTime));uniform(p,"eye",f.eye);uniform(p,"hero",f.pengu);uniform(p,"chapter",f.chapter);uniform(p,"noiseTex",0);uniform(p,"shadowTex",1);uniform(p,"hullTex",3);uniform(p,"grassTex",4);uniform(p,"metalTex",5);uniform(p,"circuitTex",6);uniform(p,"roadTex",7);uniform(p,"woodTex",8);
 for(size_t k=0;k<gpu_.size();k++){auto& g=gpu_[k];auto count=f.batches[k].instances.size();if(!count)continue;glBindVertexArray(g.vao);glDrawElementsInstanced(GL_TRIANGLES,g.count,GL_UNSIGNED_INT,nullptr,int(count));}};
 glDisable(GL_BLEND);glDisable(GL_CULL_FACE);glEnable(GL_DEPTH_TEST);glDepthMask(GL_TRUE);glDepthFunc(GL_LESS);
 glBindFramebuffer(GL_FRAMEBUFFER,shadowFbo_);glViewport(0,0,2048,2048);glClear(GL_DEPTH_BUFFER_BIT);tex(hull_,3);geometry(depth_,lv);tick("shadow");
 glBindFramebuffer(GL_FRAMEBUFFER,opt_.samples>1?msFbo_:hdrFbo_);glViewport(0,0,w_,h_);glClearColor(.001,.001,.002,1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glDisable(GL_DEPTH_TEST);
 glUseProgram(sky_);tex(noise_,0);uniform(sky_,"noiseTex",0);uniform(sky_,"time",float(f.motionTime));uniform(sky_,"seed",float(f.voyage)+3.f);uniform(sky_,"chapter",f.chapter);uniform(sky_,"eye",f.eye);uniform(sky_,"target",f.target);uniform(sky_,"up",f.up);uniform(sky_,"fov",f.fov);glUniform2f(glGetUniformLocation(sky_,"resolution"),w_,h_);glBindVertexArray(quad_);glDrawArrays(GL_TRIANGLES,0,3);
 tick("sky");glEnable(GL_DEPTH_TEST);tex(noise_,0);tex(shadow_,1);tex(hull_,3);for(int tile=0;tile<4;tile++)tex(environment_[tile],4+tile);tex(wood_,8);geometry(mesh_,vp);glDisable(GL_DEPTH_TEST);tick("meshes");
 if(opt_.samples>1){glBindFramebuffer(GL_READ_FRAMEBUFFER,msFbo_);glBindFramebuffer(GL_DRAW_FRAMEBUFFER,hdrFbo_);glBlitFramebuffer(0,0,w_,h_,0,0,w_,h_,GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT,GL_NEAREST);}
 glBindVertexArray(quad_);glViewport(0,0,std::max(1,w_/3),std::max(1,h_/3));glUseProgram(blur_);uniform(blur_,"image",0);
 for(int pass=0;pass<6;pass++){
  int dst=pass%2;glBindFramebuffer(GL_FRAMEBUFFER,blurFbo_[dst]);tex(pass==0?hdr_:blurTex_[1-dst],0);uniform(blur_,"extract",pass==0?1:0);
  glUniform2f(glGetUniformLocation(blur_,"direction"),(pass%2==0)?3.8f/w_:0,(pass%2)?3.8f/h_:0);glDrawArrays(GL_TRIANGLES,0,3);
 }
 tick("bloom");glBindFramebuffer(GL_FRAMEBUFFER,0);glViewport(outputX_,outputY_,w_,h_);glUseProgram(post_);tex(hdr_,0);tex(blurTex_[1],1);tex(overlay_,2);uniform(post_,"image",0);uniform(post_,"bloom",1);uniform(post_,"overlay",2);uniform(post_,"fade",f.fade);uniform(post_,"brightness",opt_.brightness);uniform(post_,"bloomAmount",opt_.bloom);glUniform2f(glGetUniformLocation(post_,"resolution"),w_,h_);glDrawArrays(GL_TRIANGLES,0,3);
 tick("post");GLenum err=glGetError();if(err)throw std::runtime_error("OpenGL error: "+std::to_string(err));
}
std::vector<unsigned char> Renderer::pixels(){std::vector<unsigned char> out(size_t(w_)*h_*3),bottom(out.size());glPixelStorei(GL_PACK_ALIGNMENT,1);glReadPixels(0,0,w_,h_,GL_RGB,GL_UNSIGNED_BYTE,bottom.data());for(int y=0;y<h_;y++)std::memcpy(out.data()+size_t(y)*w_*3,bottom.data()+size_t(h_-1-y)*w_*3,size_t(w_)*3);return out;}
void png(const std::string& file,const std::vector<unsigned char>& rgb,int w,int h){auto* s=cairo_image_surface_create(CAIRO_FORMAT_ARGB32,w,h);auto* p=cairo_image_surface_get_data(s);int stride=cairo_image_surface_get_stride(s);for(int y=0;y<h;y++)for(int x=0;x<w;x++){const auto* a=&rgb[(size_t(y)*w+x)*3];auto* b=p+y*stride+x*4;b[0]=a[2];b[1]=a[1];b[2]=a[0];b[3]=255;}cairo_surface_mark_dirty(s);auto status=cairo_surface_write_to_png(s,file.c_str());cairo_surface_destroy(s);if(status)throw std::runtime_error("PNG write: "+std::string(cairo_status_to_string(status)));}
}
