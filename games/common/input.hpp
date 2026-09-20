// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "player_input.hpp"
#include "assets.hpp"
#include <SDL.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>
namespace arcade {
class Input {
 struct Device {
  SDL_GameController* pad=nullptr;SDL_Joystick* joy=nullptr;SDL_JoystickID id=-1;
  std::string guid,name;std::uint32_t pending=0,triggers=0;Uint8 hat=0;
 };
 std::array<Device,4> devices_{};
 std::array<bool,SDL_NUM_SCANCODES> keys_{};
 std::array<bool,SDL_NUM_SCANCODES> keyEdges_{};
 int pendingNavX_=0,pendingNavY_=0;
 std::uint32_t keyboardOld_=0;
 int mappingDevice_=-1,mappingStep_=0;std::string mapping_;
 Uint32 mappingAfter_=0;bool axisReady_=true;
 static constexpr const char* mapFields_[]={"leftx","lefty","a","b","x","y","leftshoulder","rightshoulder","start","back","lefttrigger","righttrigger"};
 void open(int index){
  auto id=SDL_JoystickGetDeviceInstanceID(index);if(id<0)return;
  for(auto& d:devices_)if(d.id==id)return;
  char guid[64];SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(index),guid,sizeof guid);
  Device* slot=nullptr;
  for(auto& d:devices_)if(!d.joy&&d.guid==guid){slot=&d;break;}
  if(!slot)for(auto& d:devices_)if(!d.joy){slot=&d;break;}
  if(!slot)return;
  slot->pad=SDL_IsGameController(index)?SDL_GameControllerOpen(index):nullptr;
  slot->joy=slot->pad?SDL_GameControllerGetJoystick(slot->pad):SDL_JoystickOpen(index);
  if(!slot->joy)return;
  slot->id=id;slot->guid=guid;slot->pending=slot->triggers=0;slot->hat=0;
  const char* name=SDL_JoystickName(slot->joy);slot->name=name?name:"USB controller";
  if(slot->pad)SDL_GameControllerSetPlayerIndex(slot->pad,int(slot-devices_.data()));
 }
 static void close(Device& d){if(d.pad)SDL_GameControllerClose(d.pad);else if(d.joy)SDL_JoystickClose(d.joy);d.pad=nullptr;d.joy=nullptr;d.id=-1;d.pending=d.triggers=0;d.hat=0;}
 float axis(Sint16 value)const{float x=std::clamp(value/32767.f,-1.f,1.f);return std::abs(x)<=deadzone?0:std::copysign((std::abs(x)-deadzone)/(1-deadzone),x);}
public:
 float deadzone=.18f;bool disconnected=false;bool rumbleEnabled=true;
 int navigationX=0,navigationY=0;
 std::array<Control,4> state{};
 std::array<Control,4> pads{};Control keyboard;
 explicit Input(bool loadSavedMappings=true){
  SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,"0");
  SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI,"1");
  SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4_RUMBLE,"1");
  SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5_RUMBLE,"1");
  if(SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER|SDL_INIT_HAPTIC))throw std::runtime_error(SDL_GetError());
  SDL_GameControllerAddMappingsFromFile((assets()/"gamecontrollerdb.txt").c_str());
  if(loadSavedMappings)SDL_GameControllerAddMappingsFromFile((config()/"controllers.txt").c_str());
  for(int i=0;i<SDL_NumJoysticks();++i)open(i);
 }
 ~Input(){for(auto& d:devices_)close(d);SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER|SDL_INIT_HAPTIC);}
 Input(const Input&)=delete;Input& operator=(const Input&)=delete;
 int count()const{int n=0;for(auto& d:devices_)n+=d.joy!=nullptr;return n;}
 bool connected(int i)const{return i>=0&&i<4&&devices_[i].joy;}
 std::string name(int i)const{return connected(i)?devices_[i].name:(i==0?"Keyboard":"Not connected");}
 bool playstation(int i=0)const{if(!connected(i))return false;if(SDL_JoystickGetVendor(devices_[i].joy)==0x054c)return true;if(!devices_[i].pad)return false;auto t=SDL_GameControllerGetType(devices_[i].pad);return t==SDL_CONTROLLER_TYPE_PS3||t==SDL_CONTROLLER_TYPE_PS4||t==SDL_CONTROLLER_TYPE_PS5;}
 bool mapping()const{return mappingDevice_>=0;}
 std::string mappingPrompt()const{
  static const char* steps[]={"Move the steering stick RIGHT", "Move the steering stick DOWN", "Press the SOUTH face button", "Press the EAST face button", "Press the WEST face button", "Press the NORTH face button", "Press the LEFT shoulder", "Press the RIGHT shoulder", "Press START / OPTIONS", "Press BACK / SHARE", "Press the LEFT trigger", "Press the RIGHT trigger"};
  return mapping()?std::to_string(mappingStep_+1)+" / 12   "+steps[mappingStep_]:"";
 }
 void beginMapping(int i){if(!connected(i))return;mappingDevice_=i;mappingStep_=0;mapping_=devices_[i].guid+",Qinda Custom Controller,";mappingAfter_=SDL_GetTicks()+350;axisReady_=true;}
 void cancelMapping(){mappingDevice_=-1;}
 void event(const SDL_Event& e){
  if(e.type==SDL_JOYDEVICEADDED)open(e.jdevice.which);
  if(e.type==SDL_JOYDEVICEREMOVED){for(int i=0;i<4;i++)if(devices_[i].id==e.jdevice.which){close(devices_[i]);disconnected=true;if(mappingDevice_==i)cancelMapping();}}
  if(e.type==SDL_KEYDOWN||e.type==SDL_KEYUP){
   const int key=e.key.keysym.scancode;if(key>=0&&key<SDL_NUM_SCANCODES){
    if(e.type==SDL_KEYDOWN&&!keys_[key]){
     keyEdges_[key]=true;
     if(key==SDL_SCANCODE_LEFT||key==SDL_SCANCODE_A)--pendingNavX_;
     if(key==SDL_SCANCODE_RIGHT||key==SDL_SCANCODE_D)++pendingNavX_;
     if(key==SDL_SCANCODE_UP||key==SDL_SCANCODE_W)--pendingNavY_;
     if(key==SDL_SCANCODE_DOWN||key==SDL_SCANCODE_S)++pendingNavY_;
    }
    keys_[key]=e.type==SDL_KEYDOWN;
   }
  }
  if(e.type==SDL_CONTROLLERBUTTONDOWN)for(auto& d:devices_)if(d.id==e.cbutton.which){
   static constexpr SDL_GameControllerButton buttons[]={SDL_CONTROLLER_BUTTON_A,SDL_CONTROLLER_BUTTON_B,SDL_CONTROLLER_BUTTON_X,SDL_CONTROLLER_BUTTON_Y,SDL_CONTROLLER_BUTTON_LEFTSHOULDER,SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,SDL_CONTROLLER_BUTTON_START,SDL_CONTROLLER_BUTTON_BACK};
   for(unsigned b=0;b<8;b++)if(e.cbutton.button==buttons[b])d.pending|=1u<<b;
   if(e.cbutton.button==SDL_CONTROLLER_BUTTON_DPAD_LEFT)--pendingNavX_;
   if(e.cbutton.button==SDL_CONTROLLER_BUTTON_DPAD_RIGHT)++pendingNavX_;
   if(e.cbutton.button==SDL_CONTROLLER_BUTTON_DPAD_UP)--pendingNavY_;
   if(e.cbutton.button==SDL_CONTROLLER_BUTTON_DPAD_DOWN)++pendingNavY_;
  }
  if(e.type==SDL_JOYBUTTONDOWN)for(auto& d:devices_)if(!d.pad&&d.id==e.jbutton.which&&e.jbutton.button<8)d.pending|=1u<<e.jbutton.button;
  if(e.type==SDL_CONTROLLERAXISMOTION&&(e.caxis.axis==SDL_CONTROLLER_AXIS_TRIGGERLEFT||e.caxis.axis==SDL_CONTROLLER_AXIS_TRIGGERRIGHT))for(auto& d:devices_)if(d.id==e.caxis.which){auto button=bit(e.caxis.axis==SDL_CONTROLLER_AXIS_TRIGGERLEFT?LT:RT);if(e.caxis.value>11468){if(!(d.triggers&button))d.pending|=button;d.triggers|=button;}else d.triggers&=~button;}
  if(e.type==SDL_JOYHATMOTION&&e.jhat.hat==0)for(auto& d:devices_)if(!d.pad&&d.id==e.jhat.which){auto fresh=e.jhat.value&~d.hat;d.hat=e.jhat.value;if(fresh&SDL_HAT_LEFT)--pendingNavX_;if(fresh&SDL_HAT_RIGHT)++pendingNavX_;if(fresh&SDL_HAT_UP)--pendingNavY_;if(fresh&SDL_HAT_DOWN)++pendingNavY_;}
  if(e.type==SDL_WINDOWEVENT&&e.window.event==SDL_WINDOWEVENT_FOCUS_LOST){keys_.fill(false);keyEdges_.fill(false);pendingNavX_=pendingNavY_=0;keyboardOld_=0;state={};for(auto& d:devices_)d.pending=0;}
  if(!mapping())return;
  if(e.type==SDL_KEYDOWN&&e.key.keysym.sym==SDLK_ESCAPE){cancelMapping();keys_[SDL_SCANCODE_ESCAPE]=false;keyEdges_[SDL_SCANCODE_ESCAPE]=false;keyboardOld_&=~bit(Start);return;}
  auto& d=devices_[mappingDevice_];std::string binding;
  if(e.type==SDL_JOYAXISMOTION&&e.jaxis.which==d.id&&std::abs(int(e.jaxis.value))<6000)axisReady_=true;
  if(SDL_GetTicks()<mappingAfter_)return;
  if(e.type==SDL_JOYAXISMOTION&&e.jaxis.which==d.id&&std::abs(int(e.jaxis.value))>23000&&axisReady_&&(mappingStep_<2||mappingStep_>=10)){
   binding=std::string(mappingStep_>=10?(e.jaxis.value<0?"-":"+"):"")+"a"+std::to_string(e.jaxis.axis)+(mappingStep_<2&&e.jaxis.value<0?"~":"");axisReady_=false;
  }
  if(e.type==SDL_JOYBUTTONDOWN&&e.jbutton.which==d.id&&mappingStep_>=2)binding="b"+std::to_string(e.jbutton.button);
  if(binding.empty())return;
  mapping_+=std::string(mapFields_[mappingStep_])+":"+binding+",";mappingAfter_=SDL_GetTicks()+300;
  if(++mappingStep_==12){
   mapping_+="platform:Linux,";
   if(SDL_JoystickNumHats(d.joy)>0)mapping_+="dpup:h0.1,dpright:h0.2,dpdown:h0.4,dpleft:h0.8,";
   std::filesystem::create_directories(config());std::ofstream file(config()/"controllers.txt",std::ios::app);file<<mapping_<<'\n';
   if(!file)throw std::runtime_error("Cannot save controller mapping");
   SDL_GameControllerAddMapping(mapping_.c_str());
   auto id=d.id;close(d);cancelMapping();
   for(int i=0;i<SDL_NumJoysticks();i++)if(SDL_JoystickGetDeviceInstanceID(i)==id)open(i);
  }
 }
 void poll(){
  SDL_GameControllerUpdate();
  navigationX=pendingNavX_;navigationY=pendingNavY_;pendingNavX_=pendingNavY_=0;
  for(int i=0;i<4;i++){
   auto& d=devices_[i];Control c;
   if(d.joy){
    if(d.pad){
     c.x=axis(SDL_GameControllerGetAxis(d.pad,SDL_CONTROLLER_AXIS_LEFTX));c.y=axis(SDL_GameControllerGetAxis(d.pad,SDL_CONTROLLER_AXIS_LEFTY));
     static constexpr SDL_GameControllerButton buttons[]={SDL_CONTROLLER_BUTTON_A,SDL_CONTROLLER_BUTTON_B,SDL_CONTROLLER_BUTTON_X,SDL_CONTROLLER_BUTTON_Y,SDL_CONTROLLER_BUTTON_LEFTSHOULDER,SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,SDL_CONTROLLER_BUTTON_START,SDL_CONTROLLER_BUTTON_BACK};
     for(unsigned b=0;b<8;b++)if(SDL_GameControllerGetButton(d.pad,buttons[b]))c.held|=1u<<b;
     c.brake=std::max(0.f,SDL_GameControllerGetAxis(d.pad,SDL_CONTROLLER_AXIS_TRIGGERLEFT)/32767.f);
     c.throttle=std::max(0.f,SDL_GameControllerGetAxis(d.pad,SDL_CONTROLLER_AXIS_TRIGGERRIGHT)/32767.f);
     if(SDL_GameControllerGetButton(d.pad,SDL_CONTROLLER_BUTTON_DPAD_LEFT))c.x=-1;
     if(SDL_GameControllerGetButton(d.pad,SDL_CONTROLLER_BUTTON_DPAD_RIGHT))c.x=1;
     if(SDL_GameControllerGetButton(d.pad,SDL_CONTROLLER_BUTTON_DPAD_UP))c.y=-1;
     if(SDL_GameControllerGetButton(d.pad,SDL_CONTROLLER_BUTTON_DPAD_DOWN))c.y=1;
    }else{
     if(SDL_JoystickNumAxes(d.joy)>0)c.x=axis(SDL_JoystickGetAxis(d.joy,0));
     if(SDL_JoystickNumAxes(d.joy)>1)c.y=axis(SDL_JoystickGetAxis(d.joy,1));
     for(int b=0;b<std::min(8,SDL_JoystickNumButtons(d.joy));b++)if(SDL_JoystickGetButton(d.joy,b))c.held|=1u<<b;
     if(SDL_JoystickNumHats(d.joy)){auto hat=SDL_JoystickGetHat(d.joy,0);if(hat&SDL_HAT_LEFT)c.x=-1;if(hat&SDL_HAT_RIGHT)c.x=1;if(hat&SDL_HAT_UP)c.y=-1;if(hat&SDL_HAT_DOWN)c.y=1;}
    }
    if(c.brake>.35f)c.held|=bit(LT);
    if(c.throttle>.35f)c.held|=bit(RT);
   }
   // Event edges survive press+release in one frame. Polled state supplies holds,
   // never a second edge when a newly polled event reaches the queue next frame.
   c.pressed=d.pending;d.pending=0;state[i]=c;
  }
  pads=state;Control k;
  k.x=float(keys_[SDL_SCANCODE_D]||keys_[SDL_SCANCODE_RIGHT])-float(keys_[SDL_SCANCODE_A]||keys_[SDL_SCANCODE_LEFT]);
  k.y=float(keys_[SDL_SCANCODE_S]||keys_[SDL_SCANCODE_DOWN])-float(keys_[SDL_SCANCODE_W]||keys_[SDL_SCANCODE_UP]);
  auto bind=[&](Button b,bool yes){if(yes)k.held|=bit(b);};
  bind(South,keys_[SDL_SCANCODE_SPACE]||keys_[SDL_SCANCODE_RETURN]);bind(East,keys_[SDL_SCANCODE_K]);bind(West,keys_[SDL_SCANCODE_J]);bind(North,keys_[SDL_SCANCODE_L]);
  bind(LB,keys_[SDL_SCANCODE_LSHIFT]);bind(RB,keys_[SDL_SCANCODE_E]);bind(Start,keys_[SDL_SCANCODE_ESCAPE]);bind(Back,keys_[SDL_SCANCODE_BACKSPACE]);bind(LT,keys_[SDL_SCANCODE_Q]);bind(RT,keys_[SDL_SCANCODE_R]);
  k.throttle=keys_[SDL_SCANCODE_W]||keys_[SDL_SCANCODE_UP];k.brake=keys_[SDL_SCANCODE_S]||keys_[SDL_SCANCODE_DOWN];
  k.pressed=k.held&~keyboardOld_;
  auto edge=[&](Button b,bool yes){if(yes)k.pressed|=bit(b);};
  edge(South,keyEdges_[SDL_SCANCODE_SPACE]||keyEdges_[SDL_SCANCODE_RETURN]);edge(East,keyEdges_[SDL_SCANCODE_K]);edge(West,keyEdges_[SDL_SCANCODE_J]);edge(North,keyEdges_[SDL_SCANCODE_L]);
  edge(LB,keyEdges_[SDL_SCANCODE_LSHIFT]);edge(RB,keyEdges_[SDL_SCANCODE_E]);edge(Start,keyEdges_[SDL_SCANCODE_ESCAPE]);edge(Back,keyEdges_[SDL_SCANCODE_BACKSPACE]);edge(LT,keyEdges_[SDL_SCANCODE_Q]);edge(RT,keyEdges_[SDL_SCANCODE_R]);
  keyEdges_.fill(false);keyboardOld_=k.held;keyboard=k;
  if(k.x)state[0].x=k.x;
  if(k.y)state[0].y=k.y;
  state[0].throttle=std::max(state[0].throttle,k.throttle);state[0].brake=std::max(state[0].brake,k.brake);state[0].held|=k.held;state[0].pressed|=k.pressed;
  if(mapping()){state={};navigationX=navigationY=0;}
 }
 void rumble(int player,float strength){if(!rumbleEnabled||!connected(player))return;auto& d=devices_[player];auto power=Uint16(std::clamp(strength,0.f,1.f)*42000);if(d.pad)SDL_GameControllerRumble(d.pad,power,power/2,110);else SDL_JoystickRumble(d.joy,power,power/2,110);}
};
}
