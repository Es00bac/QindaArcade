#include "input.hpp"
#include <iostream>
using namespace arcade;
void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(){try{
 if(SDL_Init(SDL_INIT_EVENTS|SDL_INIT_JOYSTICK))throw std::runtime_error(SDL_GetError());
 int rumbleCount=0;
 SDL_VirtualJoystickDesc desc{};desc.version=SDL_VIRTUAL_JOYSTICK_DESC_VERSION;desc.type=SDL_JOYSTICK_TYPE_GAMECONTROLLER;desc.naxes=6;desc.nbuttons=15;desc.nhats=1;desc.name="Qinda virtual Xbox controller";desc.vendor_id=0x045e;desc.product_id=0x02ea;desc.userdata=&rumbleCount;
 desc.Rumble=[](void* user,Uint16,Uint16){++*static_cast<int*>(user);return 0;};
 int xbox=SDL_JoystickAttachVirtualEx(&desc);check(xbox>=0,"Virtual Xbox attach");
 {Input input(false);SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,"1");
  auto pump=[&](){SDL_Event e;while(SDL_PollEvent(&e))input.event(e);input.poll();};
  auto* joy=SDL_JoystickOpen(xbox);pump();check(input.connected(0),"Controller discovery");
  SDL_JoystickSetVirtualAxis(joy,0,3000);pump();check(input.state[0].x==0,"Deadzone removes drift");
  SDL_JoystickSetVirtualAxis(joy,0,28000);SDL_JoystickSetVirtualButton(joy,0,1);pump();
  check(input.state[0].x>.7f&&input.state[0].tapped(South),"Xbox stick and face button mapping");
  pump();check(!input.state[0].tapped(South)&&input.state[0].down(South),"Button edge occurs once");
  input.rumble(0,.5f);check(rumbleCount>0,"Rumble dispatch");
  desc.name="Qinda virtual DualSense";desc.vendor_id=0x054c;desc.product_id=0x0ce6;
  int ps=SDL_JoystickAttachVirtualEx(&desc);pump();check(input.connected(1),"PlayStation hot-plug");check(input.playstation(1)&&!input.playstation(0),"Device-specific PlayStation and Xbox prompts");
  auto* psjoy=SDL_JoystickOpen(ps);SDL_JoystickSetVirtualButton(psjoy,0,1);pump();check(input.state[1].tapped(South),"PlayStation logical south button");
  SDL_JoystickClose(joy);SDL_JoystickDetachVirtual(xbox);pump();check(input.disconnected&&!input.connected(0)&&input.connected(1),"Removal preserves other player slots");
  desc.name="Qinda virtual Xbox controller";desc.vendor_id=0x045e;desc.product_id=0x02ea;int reconnected=SDL_JoystickAttachVirtualEx(&desc);pump();check(input.connected(0),"Reconnect restores original slot");
  SDL_Event key{};key.type=SDL_KEYDOWN;key.key.keysym.scancode=SDL_SCANCODE_W;input.event(key);input.poll();check(input.keyboard.throttle==1,"Keyboard throttle");
  key.type=SDL_WINDOWEVENT;key.window.event=SDL_WINDOWEVENT_FOCUS_LOST;input.event(key);input.poll();check(input.keyboard.throttle==0,"Focus loss clears held keys");
  key={};key.type=SDL_KEYDOWN;key.key.keysym.scancode=SDL_SCANCODE_SPACE;input.event(key);key.type=SDL_KEYUP;input.event(key);input.poll();check(input.keyboard.tapped(South)&&!input.keyboard.down(South),"Short keyboard taps survive a render stall");
  input.poll();check(!input.keyboard.tapped(South),"Buffered tap is consumed exactly once");
  for(int press=0;press<3;press++){key={};key.type=SDL_KEYDOWN;key.key.keysym.scancode=SDL_SCANCODE_DOWN;input.event(key);key.type=SDL_KEYUP;input.event(key);}input.poll();check(input.navigationY==3,"Rapid menu taps preserve each navigation step");
  SDL_Event tap{};tap.type=SDL_CONTROLLERBUTTONDOWN;tap.cbutton.which=SDL_JoystickGetDeviceInstanceID(reconnected);tap.cbutton.button=SDL_CONTROLLER_BUTTON_X;input.event(tap);tap.type=SDL_CONTROLLERBUTTONUP;input.event(tap);input.poll();check(input.pads[0].tapped(West),"Short controller taps survive a render stall");
  tap={};tap.type=SDL_CONTROLLERAXISMOTION;tap.caxis.which=SDL_JoystickGetDeviceInstanceID(reconnected);tap.caxis.axis=SDL_CONTROLLER_AXIS_TRIGGERRIGHT;tap.caxis.value=30000;input.event(tap);tap.caxis.value=0;input.event(tap);input.poll();check(input.pads[0].tapped(RT),"Short analog trigger presses are buffered");
  desc.name="Qinda virtual unmapped USB joystick";desc.type=SDL_JOYSTICK_TYPE_UNKNOWN;desc.vendor_id=0x1209;desc.product_id=0xabcd;
  int generic=SDL_JoystickAttachVirtualEx(&desc);pump();check(input.connected(2)&&!SDL_IsGameController(generic),"Unmapped generic joystick fallback");
  auto* raw=SDL_JoystickOpen(generic);SDL_JoystickSetVirtualAxis(raw,0,-28000);SDL_JoystickSetVirtualButton(raw,0,1);pump();check(input.pads[2].x<-.7f&&input.pads[2].tapped(South),"Generic stick and button input");
  int vibrations=rumbleCount;input.rumble(2,.4f);check(rumbleCount>vibrations,"Generic joystick vibration dispatch");
  SDL_JoystickSetVirtualAxis(raw,0,0);SDL_JoystickSetVirtualButton(raw,0,0);pump();
  input.beginMapping(2);check(input.mapping(),"Mapping wizard starts on an unmapped device");
  auto instance=SDL_JoystickGetDeviceInstanceID(generic);
  for(int step=0;step<12;step++){
   SDL_Delay(355);SDL_Event e{};
   if(step<2||step>=10){e.type=SDL_JOYAXISMOTION;e.jaxis.which=instance;e.jaxis.axis=step<2?step:step-6;e.jaxis.value=0;input.event(e);e.jaxis.value=30000;}
   else{e.type=SDL_JOYBUTTONDOWN;e.jbutton.which=instance;e.jbutton.button=step-2;}
   input.event(e);
  }
  check(!input.mapping()&&std::filesystem::exists(config()/"controllers.txt")&&SDL_IsGameController(generic),"Custom mapping saves and upgrades raw joystick to a mapped controller");
  SDL_JoystickClose(raw);SDL_JoystickClose(psjoy);
 }
 SDL_Quit();std::cout<<"PASS: virtual Xbox/PlayStation/generic joystick, hot-plug, deadzone, buffered taps, rapid navigation, trigger edges, vibration, keyboard, mapping persistence\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<" / "<<SDL_GetError()<<'\n';SDL_Quit();return 1;}}
