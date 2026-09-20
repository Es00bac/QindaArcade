// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "renderer.hpp"
#ifdef QINDA_RACING
#include "race.hpp"
#else
#include "battle.hpp"
#endif
#include "input.hpp"
#include "sfx.hpp"
#include <chrono>
#include <csignal>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>
#include "replay.hpp"
#include "racer_profiles.hpp"

namespace arcade {
using namespace sw;
#ifdef QINDA_RACING
constexpr bool Racing=true;using Simulation=Race;
#else
constexpr bool Racing=false;using Simulation=Battle;
#endif
inline volatile std::sig_atomic_t stopped=0;
inline void stop(int){stopped=1;}
inline int integer(const std::string& text,int lo,int hi){std::size_t pos=0;int value=std::stoi(text,&pos);if(pos!=text.size()||value<lo||value>hi)throw std::runtime_error("Invalid integer: "+text);return value;}
inline double number(const std::string& text,double lo,double hi){std::size_t pos=0;double value=std::stod(text,&pos);if(pos!=text.size()||!std::isfinite(value)||value<lo||value>hi)throw std::runtime_error("Invalid number: "+text);return value;}
struct Preferences {
 float volume=.45f,deadzone=.18f;bool fullscreen=false,rumble=true;int wins=0;
 void load(){std::ifstream file(config()/"settings.txt");std::string key;double value;while(file>>key>>value){if(!std::isfinite(value))continue;if(key=="volume")volume=std::clamp(value,0.,1.);if(key=="deadzone")deadzone=std::clamp(value,.05,.45);if(key=="fullscreen")fullscreen=value!=0;if(key=="rumble")rumble=value!=0;if(key=="wins")wins=int(std::clamp(value,0.,1000000.));}}
 void save()const{std::filesystem::create_directories(config());auto temporary=config()/"settings.tmp";{std::ofstream file(temporary);file<<"volume "<<volume<<"\ndeadzone "<<deadzone<<"\nfullscreen "<<fullscreen<<"\nrumble "<<rumble<<"\nwins "<<wins<<'\n';if(!file)throw std::runtime_error("Could not save settings");}std::filesystem::rename(temporary,config()/"settings.txt");}
};
inline int run(int argc,char** argv){
 try{
  Preferences preferences;preferences.load();RenderOptions renderOptions;renderOptions.bloom=.12f;
  int width=1440,height=810,fps=60,stage=Racing?3:1,players=1,benchmark=0;std::array<int,4> characters{0,1,2,3};
  std::uint64_t seed=41;double startTime=0,quitAfter=0;std::string snapshot,replayFile;bool demo=false,quick=false,mute=false,overview=false,garage=false;[[maybe_unused]] float garageAngle=0;
  for(int i=1;i<argc;i++){
   std::string arg=argv[i];auto value=[&](){if(i+1>=argc)throw std::runtime_error("Missing value for "+arg);return std::string(argv[++i]);};
   if(arg=="--help"){
    std::cout<<(Racing?"Qinda Prism Kart":"Qinda Mega Brawl")<<" — native local multiplayer game\n"
     <<"--play  Start immediately   --demo  AI exhibition   --players 1..4\n"
     <<"--stage / --course prism|garden|rooftop|aurora|bliss   --character 0..7\n"
     <<"--fullscreen / --windowed   --size WxH   --fps 10..240   --no-msaa\n"
     <<"--snapshot FILE.png   --start SECONDS   --overview   --benchmark FRAMES\n"
     <<(Racing?"--garage   --garage-angle RADIANS   (kart character / livery inspection)\n":"")
     <<"--quit-after SECONDS   --mute   --volume 0..1   --no-hud   --seed N\n"
     <<"Keyboard: WASD/arrows move; Enter select; Esc pause.\n"
     <<(Racing?"W/A or RT accelerate; S/B or LT brake; RB/E drift; X/J or LB use item.\n":"Space/A jump; J/X attack; K/B special; L/Y heavy; Shift/LB shield; E/RB dodge; R/RT grab; Up+special recover.\n")
     <<"Xbox, PlayStation, Nintendo and generic SDL controllers; Controls menu has a mapping wizard.\n";return 0;
   }else if(arg=="--play")quick=true;
   else if(arg=="--demo")demo=true;
   else if(arg=="--garage"){if(!Racing)throw std::runtime_error("Garage is available in Prism Kart");garage=true;}
   else if(arg=="--garage-angle")garageAngle=number(value(),-6.284,6.284);
   else if(arg=="--fullscreen")preferences.fullscreen=true;
   else if(arg=="--windowed")preferences.fullscreen=false;
   else if(arg=="--players")players=integer(value(),1,4);
   else if(arg=="--character")characters[0]=integer(value(),0,7);
   else if(arg=="--stage"||arg=="--course"){auto s=value();if(s=="prism")stage=0;else if(s=="garden"||s=="eight")stage=1;else if(s=="rooftop"||s=="aurora")stage=2;else if(s=="bliss")stage=3;else stage=integer(s,0,3);}
   else if(arg=="--size"){auto s=value();auto x=s.find('x');if(x==std::string::npos)throw std::runtime_error("Expected WIDTHxHEIGHT");width=integer(s.substr(0,x),640,7680);height=integer(s.substr(x+1),360,4320);}
   else if(arg=="--fps")fps=integer(value(),10,240);
   else if(arg=="--seed")seed=integer(value(),0,2147483647);
   else if(arg=="--snapshot")snapshot=value();
   else if(arg=="--replay")replayFile=value();
   else if(arg=="--start")startTime=number(value(),0,7200);
   else if(arg=="--quit-after")quitAfter=number(value(),.01,36000);
   else if(arg=="--benchmark")benchmark=integer(value(),1,100000);
   else if(arg=="--volume")preferences.volume=number(value(),0,1);
   else if(arg=="--mute")mute=true;
   else if(arg=="--no-hud")renderOptions.titles=false;
   else if(arg=="--no-msaa")renderOptions.samples=1;
   else if(arg=="--overview")overview=true;
   else throw std::runtime_error("Unknown option: "+arg);
  }
  for(int i=1;i<4;i++)characters[i]=(characters[0]+i)%8;
  SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR,"0");
  if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER))throw std::runtime_error(SDL_GetError());
  struct Quit {~Quit(){SDL_EnableScreenSaver();SDL_Quit();}} quit;
  SDL_DisableScreenSaver();
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,24);
  Uint32 flags=SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI;
  if(preferences.fullscreen&&snapshot.empty())flags|=SDL_WINDOW_FULLSCREEN_DESKTOP;
  if(!snapshot.empty())flags|=SDL_WINDOW_HIDDEN;
  struct Window {SDL_Window* window=nullptr;SDL_GLContext gl=nullptr;~Window(){if(gl)SDL_GL_DeleteContext(gl);if(window)SDL_DestroyWindow(window);}} surface;
  surface.window=SDL_CreateWindow(Racing?"Qinda Prism Kart":"Qinda Mega Brawl",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,width,height,flags);
  if(!surface.window)throw std::runtime_error(SDL_GetError());
  surface.gl=SDL_GL_CreateContext(surface.window);if(!surface.gl)throw std::runtime_error(SDL_GetError());
  SDL_GL_SetSwapInterval(benchmark||!snapshot.empty()?0:1);SDL_SetWindowMinimumSize(surface.window,960,540);
  Input input;input.deadzone=preferences.deadzone;input.rumbleEnabled=preferences.rumble;Replay replay(replayFile);
  saver::Sound audio(!mute&&snapshot.empty(),preferences.volume);
  std::cout<<"OpenGL "<<glGetString(GL_VERSION)<<" / "<<glGetString(GL_RENDERER)<<"\nControllers: "<<input.count()<<"\n";
  std::signal(SIGINT,stop);std::signal(SIGTERM,stop);
  Ui ui;ui.racing=Racing;Page page=garage?Page::Setup:demo||quick?Page::Playing:Page::Home,returnPage=Page::Home;
  int selected=0,loadedStage=-1,mappingDevice=0,cupRound=0;bool cup=false,keyboardFirst=false,needsNeutral=false;std::array<int,8> points{};
  std::uint64_t lastSound=0;
  std::unique_ptr<Simulation> simulation;Frame frame;std::vector<std::unique_ptr<Renderer>> renderers;
  auto reset=[&](bool playable){
#ifdef QINDA_RACING
   simulation=std::make_unique<Simulation>(seed,stage);
#else
   simulation=std::make_unique<Simulation>(seed,stage,4);
#endif
   if(playable)simulation->configurePlayers(players,characters);
   else simulation->seek(startTime>0?startTime:18);
   frame=Frame{};initialize(frame);
#ifdef QINDA_RACING
   buildTrackMeshes(frame,simulation->track);
#endif
   loadedStage=stage;renderers.clear();audio.reset();lastSound=0;
  };
  auto show=[&](Page next){page=next;selected=0;needsNeutral=next==Page::Results;ui.notice.clear();};
  auto start=[&](){
   keyboardFirst=players>1&&!input.connected(players-1)&&input.connected(players-2);
   for(int p=1;p<players;p++)if(!input.connected(p-(keyboardFirst?1:0))){ui.notice="Connect a controller for every additional player.";return;}
   demo=false;reset(true);show(Page::Playing);SDL_ShowCursor(SDL_DISABLE);
  };
  reset(quick);if(quick){keyboardFirst=players>1&&!input.connected(players-1)&&input.connected(players-2);if(startTime>0)simulation->seek(startTime);}
  auto levelName=[&]()->std::string{
#ifdef QINDA_RACING
   const char* names[]={"Afterglow Viaduct","Reactor Gardens","Aurora Pass","Bliss Reclamation"};return names[stage];
#else
   const char* names[]={"Prism Terminal","Reactor Garden","Afterglow Rooftop","Bliss Salvage Yard"};return names[stage];
#endif
  };
  auto rows=[&](){
   ui.page=page;ui.selected=selected;ui.exhibition=demo;ui.players=players;ui.title=Racing?"PRISM KART":"MEGA BRAWL";ui.subtitle=Racing?"THE RECLAIMED WORLD GRAND PRIX":"THE QINDA PLATFORM FIGHTER";ui.description=levelName();ui.device=input.name(mappingDevice);ui.playstation=input.playstation(page==Page::Controls?mappingDevice:0);ui.mapping=input.mappingPrompt();ui.rows.clear();ui.standings.clear();
   ui.profileKart.clear();ui.profileRole.clear();ui.profileSponsor.clear();ui.profileStory.clear();
   if(Racing&&page==Page::Setup){int p=selected>=2&&selected<players+2?selected-2:0;const auto& profile=profiles[characters[p]];ui.profileKart=std::string(racerNames[characters[p]])+"  /  "+profile.kart;ui.profileRole=profile.role;ui.profileSponsor=std::string(profile.sponsor)+" "+profile.sponsorLine;ui.profileStory=profile.story;}
   if(page==Page::Home)ui.rows={{Racing?"Single race":"Quick match","01"},{Racing?"Local split-screen":"Local brawl","1—4"},{Racing?"Grand Prix":"Stage select",Racing?"4 RACES":"4 ARENAS"},{"Controllers",""},{"Settings",""},{"Quit",""}};
   if(page==Page::Setup){ui.title="GET READY";ui.subtitle=Racing?(cup?"GRAND PRIX / FOUR COURSES":"RACE SETUP / THREE LAPS"):"MATCH SETUP / THREE STOCKS";ui.rows={{"Players",std::to_string(players)},{Racing?"Course":"Arena",levelName()}};for(int p=0;p<players;p++)ui.rows.push_back({"Player "+std::to_string(p+1),racerNames[characters[p]]});ui.rows.push_back({Racing?"Start race":"Start match","READY"});}
   if(page==Page::Paused){ui.title="PAUSED";ui.subtitle="TAKE YOUR TIME";ui.rows={{"Resume",""},{"Restart",""},{"Controllers",""},{"Settings",""},{"Main menu",""}};}
   if(page==Page::Results){ui.title=Racing?"RACE COMPLETE":"MATCH COMPLETE";ui.subtitle=Racing?(cup?"GRAND PRIX / ROUND "+std::to_string(cupRound+1):"FINAL CLASSIFICATION"):"FINAL RESULT";
#ifdef QINDA_RACING
    auto order=simulation->state().order;
    if(cup&&cupRound==3){ui.title="CUP COMPLETE";ui.subtitle="GRAND PRIX / FINAL STANDINGS";std::stable_sort(order.begin(),order.end(),[&](int a,int b){return points[a]>points[b];});}
    ui.standingsTitle=cup&&cupRound==3?"CHAMPIONSHIP CLASSIFICATION":"RACE CLASSIFICATION";
    for(int id:order){auto& car=simulation->state().cars[id];std::string owner="CPU";for(int p=0;p<players;p++)if(characters[p]==id)owner="P"+std::to_string(p+1);std::ostringstream time;if(car.finishedAt<0)time<<"UNFINISHED";else time<<int(car.finishedAt)/60<<":"<<std::setfill('0')<<std::setw(2)<<int(car.finishedAt)%60<<"."<<std::setw(2)<<int(car.finishedAt*100)%100;ui.standings.push_back({owner+"  "+racerNames[id],cup?std::to_string(points[id])+" PTS":time.str()});}
    ui.description=std::string(racerNames[order[0]])+(cup&&cupRound==3?" wins the Grand Prix":" wins the race");
#else
    auto& s=simulation->state();ui.description=std::string(racerNames[s.fighters[s.winner].id])+" wins";ui.standingsTitle="MATCH CLASSIFICATION / STOCKS · KNOCKOUTS";
    std::array<int,4> order{0,1,2,3};std::stable_sort(order.begin(),order.begin()+s.active,[&](int a,int b){auto score=[&](int i){const auto& f=s.fighters[i];return f.stocks*10000.f+f.kos*500.f-f.damage;};return score(a)>score(b);});
    for(int n=0;n<s.active;n++){int i=order[n];auto& fighter=s.fighters[i];ui.standings.push_back({(i<players?"P"+std::to_string(i+1):"CPU")+std::string("  ")+racerNames[fighter.id],std::to_string(fighter.stocks)+" STOCK  /  "+std::to_string(fighter.kos)+" KO"});}
#endif
    ui.rows.push_back({cup&&cupRound<3?"Next course":"Play again",""});ui.rows.push_back({"Main menu",""});}
   if(page==Page::Settings){ui.title="SETTINGS";ui.subtitle="MAKE YOURSELF AT HOME";ui.rows={{"Sound effects",std::to_string(int(preferences.volume*100))+"%"},{"Stick deadzone",std::to_string(int(preferences.deadzone*100))+"%"},{"Controller vibration",preferences.rumble?"ON":"OFF"},{"Display",preferences.fullscreen?"FULLSCREEN":"WINDOWED"},{"Back",""}};}
   if(page==Page::Controls){ui.title="CONTROLLERS";ui.subtitle="XBOX / PLAYSTATION / USB / BLUETOOTH";bool ps=ui.playstation;ui.rows={{"Device",std::to_string(mappingDevice+1)+" / "+std::to_string(input.count())},{"Move / steer","STICK / WASD"},{Racing?"Accelerate / brake":"Jump / attack",Racing?(ps?"R2 / L2   ·   W / S":"RT / LT   ·   W / S"):(ps?"CROSS / SQUARE":"A / X   ·   SPACE / J")},{Racing?"Drift / item":"Special / heavy",Racing?(ps?"R1 / SQUARE":"RB / X   ·   E / J"):(ps?"CIRCLE / TRIANGLE":"B / Y   ·   K / L")},{Racing?"Item / brake":"Shield / dodge",Racing?(ps?"L1 / CIRCLE":"LB / B   ·   J / S"):(ps?"L1 / R1":"LB / RB   ·   SHIFT / E")},{"Map this controller","12 STEPS"},{"Back",""}};}
   selected=std::clamp(selected,0,std::max(0,int(ui.rows.size())-1));ui.selected=selected;
  };
  rows();using Clock=std::chrono::steady_clock;auto born=Clock::now(),previous=born;int rendered=0;double repeatAt=0;int previousDir=0;bool focused=true;
  while(!stopped){
   auto now=Clock::now();double elapsed=std::chrono::duration<double>(now-born).count();double dt=std::min(.08,std::chrono::duration<double>(now-previous).count());previous=now;ui.clock=elapsed;
   if(quitAfter>0&&elapsed>=quitAfter)break;
   replay.advance(benchmark?double(rendered)/fps:elapsed,surface.window);
   bool click=false;SDL_Event event;
   while(SDL_PollEvent(&event)){
    input.event(event);
    if(event.type==SDL_QUIT)stopped=1;
    if(event.type==SDL_WINDOWEVENT){
     if(event.window.event==SDL_WINDOWEVENT_FOCUS_LOST){focused=false;if(page==Page::Playing&&!demo){show(Page::Paused);ui.notice="Paused while the game is out of focus.";SDL_ShowCursor(SDL_ENABLE);}}
     if(event.window.event==SDL_WINDOWEVENT_FOCUS_GAINED)focused=true;
    }
    if(event.type==SDL_KEYDOWN&&event.key.keysym.sym==SDLK_F11){preferences.fullscreen=!preferences.fullscreen;SDL_SetWindowFullscreen(surface.window,preferences.fullscreen?SDL_WINDOW_FULLSCREEN_DESKTOP:0);preferences.save();}
    if((event.type==SDL_MOUSEMOTION||event.type==SDL_MOUSEBUTTONDOWN)&&page!=Page::Playing&&!input.mapping()){
     int ww,hh;SDL_GetWindowSize(surface.window,&ww,&hh);double scale=std::min(ww/1440.,hh/810.);double x=(event.type==SDL_MOUSEMOTION?event.motion.x:event.button.x)-(ww-1440*scale)*.5,y=(event.type==SDL_MOUSEMOTION?event.motion.y:event.button.y)-(hh-810*scale)*.5;x/=scale;y/=scale;
     int row=int((y-268)/57);if(x>=54&&x<=497&&y>=268&&row>=0&&row<int(ui.rows.size())&&(y-268-row*57)<49){selected=row;if(event.type==SDL_MOUSEBUTTONDOWN&&event.button.button==SDL_BUTTON_LEFT)click=true;}
    }
   }
   if(stopped)break;
   input.poll();
   if(input.disconnected){if(page==Page::Playing&&!demo){show(Page::Paused);ui.notice="Controller disconnected. Reconnect, then resume.";SDL_ShowCursor(SDL_ENABLE);}input.disconnected=false;}
   Control menu=input.state[0];for(int p=1;p<4;p++){menu.pressed|=input.state[p].pressed;if(std::abs(input.state[p].x)>.5f)menu.x=input.state[p].x;if(std::abs(input.state[p].y)>.5f)menu.y=input.state[p].y;}
   bool ready=!needsNeutral||(std::abs(menu.x)<.25f&&std::abs(menu.y)<.25f)||input.navigationX||input.navigationY;
   if(ready)needsNeutral=false;else menu.x=menu.y=0;
   int navSteps=ready?input.navigationY:0;
   int direction=std::abs(menu.y)>.5f?(menu.y>0?1:-1):0;bool nav=navSteps||(direction&&(direction!=previousDir||elapsed>=repeatAt));if(nav){repeatAt=elapsed+(navSteps||direction!=previousDir?.30:.13);}previousDir=direction;
   if(page==Page::Playing){
    if(menu.tapped(Start)||menu.tapped(Back)){show(demo?Page::Home:Page::Paused);SDL_ShowCursor(SDL_ENABLE);}
   }else if(!input.mapping()){
    if(nav&&!ui.rows.empty()){int count=int(ui.rows.size());selected=((selected+(navSteps?navSteps:direction))%count+count)%count;}
    int horizontal=ready?input.navigationX:0;int change=horizontal?(horizontal>0?1:-1):std::abs(menu.x)>.55f?(menu.x>0?1:-1):0;static int lastChange=0;bool adjust=horizontal||(change&&change!=lastChange);lastChange=change;
    bool accept=menu.tapped(South)||click,back=menu.tapped(East)||menu.tapped(Start)||menu.tapped(Back);
    if(back){if(page==Page::Paused){show(Page::Playing);SDL_ShowCursor(SDL_DISABLE);}else if(page==Page::Controls||page==Page::Settings){input.cancelMapping();show(returnPage);}else if(page!=Page::Home){show(Page::Home);reset(false);}}
    else if(page==Page::Home&&accept){
     if(selected<=2){cup=Racing&&selected==2;cupRound=0;points.fill(0);if(selected==0)players=1;else if(selected==1)players=std::min(4,std::max(2,input.count()));if(cup)stage=3;show(Page::Setup);}
     else if(selected==3){returnPage=Page::Home;show(Page::Controls);}else if(selected==4){returnPage=Page::Home;show(Page::Settings);}else stopped=1;
    }else if(page==Page::Setup){
     if((adjust||accept)&&selected==0){players=(players-1+(change<0?3:1))%4+1;if(Racing){std::array<bool,8> used{};for(int p=0;p<players;p++){while(used[characters[p]])characters[p]=(characters[p]+1)%8;used[characters[p]]=true;}}}
     else if((adjust||accept)&&selected==1)stage=(stage+(change<0?3:1))%4;
     else if((adjust||accept)&&selected>=2&&selected<players+2){int p=selected-2;do{characters[p]=(characters[p]+(change<0?7:1))%8;}while(Racing&&std::count(characters.begin(),characters.begin()+players,characters[p])>1);}
     else if(accept&&selected==players+2)start();
    }else if(page==Page::Paused&&accept){if(selected==0){show(Page::Playing);SDL_ShowCursor(SDL_DISABLE);}else if(selected==1)start();else if(selected==2){returnPage=Page::Paused;show(Page::Controls);}else if(selected==3){returnPage=Page::Paused;show(Page::Settings);}else {show(Page::Home);reset(false);}}
    else if(page==Page::Results&&accept){if(selected==0){if(cup&&cupRound<3){cupRound++;stage=(stage+1)%4;}else{cupRound=0;points.fill(0);}start();}else if(selected==1){show(Page::Home);reset(false);}}
    else if(page==Page::Settings&&(adjust||accept)){
     if(selected==0)preferences.volume=std::clamp(preferences.volume+(change<0?-.05f:.05f),0.f,1.f);
     if(selected==1)preferences.deadzone=std::clamp(preferences.deadzone+(change<0?-.025f:.025f),.05f,.45f);
     if(selected==2)preferences.rumble=!preferences.rumble;
     if(selected==3){preferences.fullscreen=!preferences.fullscreen;SDL_SetWindowFullscreen(surface.window,preferences.fullscreen?SDL_WINDOW_FULLSCREEN_DESKTOP:0);}
     if(selected==4)show(returnPage);
     preferences.save();audio.volume(preferences.volume);input.deadzone=preferences.deadzone;input.rumbleEnabled=preferences.rumble;
    }else if(page==Page::Controls&&(adjust||accept)){
     if(selected==0)mappingDevice=(mappingDevice+(change<0?3:1))%4;
     if(selected==5){if(input.connected(mappingDevice))input.beginMapping(mappingDevice);else ui.notice="Connect the controller before mapping it.";}
     if(selected==6)show(returnPage);
    }
   }
   if(loadedStage!=stage)reset(false);
   if(page==Page::Playing){
    if(!demo)for(int p=0;p<players;p++){
     auto c=keyboardFirst?(p==0?input.keyboard:input.pads[p-1]):input.state[p];
#ifdef QINDA_RACING
     simulation->control(characters[p],c);
#else
     simulation->control(p,c);
#endif
    }
    if(focused||!snapshot.empty()||benchmark)simulation->advance(benchmark?1./fps:dt);
    audio.play(simulation->state().sounds);
    for(auto& e:simulation->state().sounds)if(e.serial>lastSound){lastSound=e.serial;if(e.cue==saver::Cue::Hit||e.cue==saver::Cue::Boost||e.cue==saver::Cue::Knockout)for(int p=0;p<players;p++)if(characters[p]==e.character&&(!keyboardFirst||p>0))input.rumble(p-(keyboardFirst?1:0),e.cue==saver::Cue::Knockout?.9f:.40f);}
    bool done=false;
#ifdef QINDA_RACING
    done=true;for(int p=0;p<players;p++)done&=simulation->state().cars[characters[p]].finishedAt>=0;
#else
    done=simulation->state().winner>=0;
#endif
    if(done&&!demo){
#ifdef QINDA_RACING
     static const int awards[]={15,12,10,8,6,4,2,1};for(int i=0;i<8;i++)points[i]+=awards[simulation->state().cars[i].rank-1];
     if(simulation->state().cars[characters[0]].rank==1)preferences.wins++;
#else
     if(simulation->state().winner==0)preferences.wins++;
#endif
     preferences.save();show(Page::Results);SDL_ShowCursor(SDL_ENABLE);
    }
   }else if(page==Page::Home||page==Page::Setup)simulation->advance(dt);
   rows();replay.check(page);int drawableW,drawableH;SDL_GL_GetDrawableSize(surface.window,&drawableW,&drawableH);if(drawableW<=0||drawableH<=0){SDL_Delay(25);continue;}
   int views=Racing&&page==Page::Playing&&!demo?players:1;int cols=views>1?2:1,lines=views>2?2:1;
   int viewW=drawableW/cols,viewH=drawableH/lines;
   if(int(renderers.size())!=views){renderers.clear();for(int p=0;p<views;p++)renderers.push_back(std::make_unique<Renderer>(frame,viewW,viewH,renderOptions));}
   glBindFramebuffer(GL_FRAMEBUFFER,0);glViewport(0,0,drawableW,drawableH);glClearColor(.015f,.025f,.03f,1);glClear(GL_COLOR_BUFFER_BIT);
   for(int p=0;p<views;p++){
    SceneOptions options;
#ifdef QINDA_RACING
    options.focus=characters[p];options.camera=overview||page==Page::Results?Camera::Overview:page==Page::Playing?Camera::Chase:Camera::Front;
    if(page==Page::Setup){int preview=selected>=2&&selected<players+2?selected-2:0;options.gallery=true;options.galleryId=characters[preview];options.galleryAngle=garageAngle;options.galleryMenu=renderOptions.titles;}
    compose(frame,simulation->sample(),simulation->track,options);
#else
    options.camera=overview||page==Page::Playing?Camera::Director:Camera::Fixed;compose(frame,simulation->sample(),options);
    if(page!=Page::Playing){frame.eye.x-=10;frame.target.x-=10;}
#endif
    frame.fade=1;frame.ui=ui;frame.ui.player=p;if(page==Page::Playing)frame.ui.playstation=input.playstation(p-(keyboardFirst?1:0));
    int x=(p%cols)*viewW,y=(lines-1-p/cols)*viewH,w=viewW;
    if(views==3){if(p==0){x=0;w=drawableW;}else{x=(p-1)*viewW;y=0;}}
    auto& renderer=*renderers[p];if(renderer.width()!=w||renderer.height()!=viewH)renderer.resize(w,viewH);
    renderer.outputOffset(x,y);renderer.draw(frame);
   }
   if(!snapshot.empty()||!replay.captures.empty()){
    std::vector<unsigned char> bottom(size_t(drawableW)*drawableH*3),rgb(bottom.size());glPixelStorei(GL_PACK_ALIGNMENT,1);glReadPixels(0,0,drawableW,drawableH,GL_RGB,GL_UNSIGNED_BYTE,bottom.data());
    for(int y=0;y<drawableH;y++)std::copy_n(bottom.data()+size_t(drawableH-1-y)*drawableW*3,drawableW*3,rgb.data()+size_t(y)*drawableW*3);
    if(!snapshot.empty()){png(snapshot,rgb,drawableW,drawableH);std::cout<<"Saved "<<snapshot<<'\n';break;}
    for(auto& file:replay.captures){png(file,rgb,drawableW,drawableH);std::cout<<"Saved "<<file<<'\n';}replay.captures.clear();
   }
   SDL_GL_SwapWindow(surface.window);rendered++;
   if(benchmark&&rendered>=benchmark)break;
   if(!benchmark)std::this_thread::sleep_until(now+std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(1./fps)));
  }
  replay.finish();double seconds=std::chrono::duration<double>(Clock::now()-born).count();std::cout<<"Rendered "<<rendered<<" frames in "<<seconds<<" seconds";if(benchmark)std::cout<<" ("<<rendered/std::max(.001,seconds)<<" fps)";std::cout<<'\n';
  SDL_ShowCursor(SDL_ENABLE);return 0;
 }catch(const std::exception& error){std::cerr<<"Qinda Arcade: "<<error.what()<<'\n';return 1;}
}
}
