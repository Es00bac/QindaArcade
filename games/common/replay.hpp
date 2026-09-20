// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <SDL.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <string>
namespace arcade {
// Reproducible native end-to-end QA. Events enter SDL's real event queue.
class Replay {
 struct Event {double at;std::string op,arg;};std::vector<Event> events_;std::size_t cursor_=0;std::vector<SDL_JoystickID> virtual_;
public:
 std::vector<std::string> captures;std::string expectedPage;
 explicit Replay(const std::string& file){if(file.empty())return;std::ifstream input(file);if(!input)throw std::runtime_error("Cannot open input replay: "+file);std::string line;double last=-1;while(std::getline(input,line)){if(line.empty()||line[0]=='#')continue;std::istringstream row(line);Event event;row>>event.at>>event.op;std::getline(row>>std::ws,event.arg);if(!row||event.at<last||event.at<0)throw std::runtime_error("Invalid replay event: "+line);last=event.at;events_.push_back(event);}}
 void advance(double time,SDL_Window* window){
  while(cursor_<events_.size()&&events_[cursor_].at<=time){auto event=events_[cursor_++];
   if(event.op=="keydown"||event.op=="keyup"){
    SDL_Event key{};key.type=event.op=="keydown"?SDL_KEYDOWN:SDL_KEYUP;key.key.windowID=SDL_GetWindowID(window);key.key.state=event.op=="keydown"?SDL_PRESSED:SDL_RELEASED;key.key.keysym.scancode=SDL_GetScancodeFromName(event.arg.c_str());key.key.keysym.sym=SDL_GetKeyFromScancode(key.key.keysym.scancode);if(key.key.keysym.scancode==SDL_SCANCODE_UNKNOWN)throw std::runtime_error("Unknown replay key: "+event.arg);SDL_PushEvent(&key);
   }else if(event.op=="click"){
    SDL_Event click{};click.type=SDL_MOUSEBUTTONDOWN;click.button.windowID=SDL_GetWindowID(window);click.button.button=SDL_BUTTON_LEFT;click.button.state=SDL_PRESSED;
    std::istringstream pos(event.arg);if(!(pos>>click.button.x>>click.button.y))throw std::runtime_error("Invalid replay click");SDL_PushEvent(&click);
   }else if(event.op=="capture"){captures.push_back(event.arg);break;}
   else if(event.op=="expect"){expectedPage=event.arg;break;}
   else if(event.op=="resize"){int w,h;char x;std::istringstream(event.arg)>>w>>x>>h;if(w<640||h<360)throw std::runtime_error("Invalid replay window size");SDL_SetWindowSize(window,w,h);}
   else if(event.op=="connect"){
    SDL_VirtualJoystickDesc d{};d.version=SDL_VIRTUAL_JOYSTICK_DESC_VERSION;d.type=SDL_JOYSTICK_TYPE_GAMECONTROLLER;d.naxes=6;d.nbuttons=15;d.nhats=1;d.name=event.arg=="ps"?"Qinda QA DualSense":"Qinda QA Xbox";d.vendor_id=event.arg=="ps"?0x054c:0x045e;d.product_id=event.arg=="ps"?0x0ce6:0x02ea;int index=SDL_JoystickAttachVirtualEx(&d);if(index<0)throw std::runtime_error(SDL_GetError());virtual_.push_back(SDL_JoystickGetDeviceInstanceID(index));
   }else if(event.op=="disconnect"){
    int slot=std::stoi(event.arg);if(slot<0||slot>=int(virtual_.size()))throw std::runtime_error("Unknown replay controller");for(int i=0;i<SDL_NumJoysticks();i++)if(SDL_JoystickGetDeviceInstanceID(i)==virtual_[slot]){SDL_JoystickDetachVirtual(i);break;}
   }else if(event.op=="quit"){SDL_Event quit{};quit.type=SDL_QUIT;SDL_PushEvent(&quit);}
   else throw std::runtime_error("Unknown replay operation: "+event.op);
  }
 }
 void check(Page page){if(expectedPage.empty())return;static const char* names[]={"home","setup","playing","paused","results","settings","controls"};auto actual=names[int(page)];if(expectedPage!=actual)throw std::runtime_error("Replay expected "+expectedPage+", got "+actual);std::cout<<"PASS replay page "<<actual<<'\n';expectedPage.clear();}
 void finish()const{if(cursor_!=events_.size()||!expectedPage.empty()||!captures.empty())throw std::runtime_error("Replay ended before every event, assertion and capture completed");}
};
}
