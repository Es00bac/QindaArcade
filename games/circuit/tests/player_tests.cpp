#include "race.hpp"
#include <iostream>
using namespace sw;
namespace sw {struct RaceTest{static RaceState& state(Race& r){return r.current_;}};}
void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(){try{
 Race race(41,3);race.configurePlayers(1,{0,1,2,3});race.advance(6);
 check(race.state().cars[0].speed==0&&race.state().cars[0].distance<=-9,"Human racer must wait for throttle");
 arcade::Control c;c.throttle=1;race.control(0,c);race.advance(2);check(race.state().cars[0].speed>10,"Human throttle accelerates");
 float lane=race.state().cars[0].lane;c.x=.7f;c.held=arcade::bit(arcade::RB);race.control(0,c);race.advance(.7);
 check(race.state().cars[0].lane>lane,"Steering changes lateral position");check(race.state().cars[0].driftCharge>.45f,"Drift charge follows held input");
 c.held=0;c.x=0;race.control(0,c);race.advance(FixedStep);check(race.state().cars[0].boost>0,"Releasing charged drift grants boost");
 auto& state=RaceTest::state(race);state.cars[0].item=Item::Shield;state.cars[0].itemAge=0;
 race.advance(2);check(race.state().cars[0].item==Item::Shield,"Human items are not automatically used");
 c.pressed=arcade::bit(arcade::West);race.control(0,c);race.advance(FixedStep);check(race.state().cars[0].item==Item::None&&race.state().cars[0].shield>0,"Manual item activation");
 c={};c.brake=1;race.control(0,c);race.advance(2);check(race.state().cars[0].speed<.1f,"Braking stops the kart");
 state.winner=1;state.finishedAt=state.time;auto round=state.round;race.advance(15);check(race.state().round==round,"Human races never auto-reset at the results screen");
 bool rejected=false;try{race.configurePlayers(2,{0,0,2,3});}catch(...){rejected=true;}check(rejected,"Duplicate local racers rejected");
 Track track(41,3);for(int i=0;i<100;i++){auto p=track.at(track.length*i/100);check(groundHeight(track,p.position.x,p.position.z)<p.position.y-.5f,"Road clearance on Bliss");}
 // Drive every course through the public human-input path, then wait for the field.
 // This catches finish-line blockers and lap/order regressions without teleporting cars.
 for(int course=0;course<4;course++){
  Race complete(41,course);complete.configurePlayers(1,{0,1,2,3});
  for(int tick=0;tick<120*600&&complete.state().cars[0].finishedAt<0;tick++){
   const auto& car=complete.state().cars[0];const auto pose=complete.track.at(car.distance+6);
   arcade::Control drive;drive.throttle=1;
   drive.x=clamp((pose.curvature*car.speed-.9f*car.heading-.12f*car.lane)/(.14f+car.speed*.023f),-1,1);
   complete.control(0,drive);complete.advance(FixedStep);
  }
  const auto& car=complete.state().cars[0];check(car.finishedAt>=0,"Human input can finish three laps on every course");
  const auto finish=car.finishedAt;const auto round=complete.state().round;
  complete.control(0,{});complete.advance(180);
  check(complete.state().cars[0].finishedAt==finish&&complete.state().round==round,"Finish time and result persist");
  for(const auto& other:complete.state().cars)check(other.finishedAt>=0,"Every opponent can cross the finish behind a stopped finisher");
  check(complete.state().cars[0].speed<.1f,"Finished human kart comes to a stop");
 }
 std::cout<<"PASS: throttle, steering, braking, drift boost, manual items, results, Bliss clearance, three-lap completion on all four courses\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
