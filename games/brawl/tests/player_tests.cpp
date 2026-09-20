#include "battle.hpp"
#include <iostream>
using namespace sw;
namespace sw {struct BattleTest{static BattleState& state(Battle& b){return b.current_;}};}
void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
int main(){try{
 Battle b(41,3,4);b.configurePlayers(4,{0,1,2,3});b.advance(3);
 float start=b.state().fighters[0].x;b.advance(.5);check(b.state().fighters[0].x==start,"A human fighter must not walk under AI control");
 arcade::Control c;c.x=1;b.control(0,c);b.advance(.25);check(b.state().fighters[0].x>start+.5f,"Analog movement must drive the fighter");
 c={};c.pressed=c.held=arcade::bit(arcade::South);b.control(0,c);b.advance(FixedStep);check(b.state().fighters[0].vy>12&&b.state().fighters[0].jumps==1,"First jump");
 c.pressed=0;b.control(0,c);b.advance(.15);check(b.state().fighters[0].jumps==1,"Holding jump cannot repeat it");
 c.pressed=arcade::bit(arcade::South);b.control(0,c);b.advance(FixedStep);check(b.state().fighters[0].jumps==0,"Second jump");
 b.control(0,c);b.advance(FixedStep);check(b.state().fighters[0].jumps==0,"Third jump is unavailable");
 c={};c.y=-1;c.pressed=arcade::bit(arcade::East);b.control(0,c);b.advance(FixedStep);check(b.state().fighters[0].recoveryUsed&&b.state().fighters[0].vy>16,"Manual up-special recovery");
 auto& s=BattleTest::state(b);s.fighters[0].action=Action::Idle;s.fighters[0].stun=0;s.fighters[0].floor=0;s.fighters[0].y=0;s.fighters[0].vx=s.fighters[0].vy=0;
 c={};c.held=arcade::bit(arcade::LB);b.control(0,c);b.advance(FixedStep);check(b.state().fighters[0].action==Action::Shield,"Held shield");
 c={};b.control(0,c);b.advance(FixedStep);check(b.state().fighters[0].action!=Action::Shield,"Shield release");
 for(auto [button,action]:std::array<std::pair<arcade::Button,Action>,5>{{{arcade::West,Action::Jab},{arcade::North,Action::Heavy},{arcade::East,Action::Special},{arcade::RB,Action::Dodge},{arcade::RT,Action::Grab}}}){
  auto& fighter=s.fighters[0];fighter.action=Action::Idle;fighter.stun=fighter.cooldown=fighter.specialCooldown=0;fighter.floor=0;fighter.x=0;fighter.y=0;fighter.vx=fighter.vy=0;
  c={};c.pressed=arcade::bit(button);b.control(0,c);b.advance(FixedStep);check(b.state().fighters[0].action==action,"Each combat button selects its manual action");
 }
 s.fighters[0].x=-21;s.fighters[0].action=Action::Idle;s.fighters[1].x=21;b.control(0,{});b.advance(.3);
 check(b.state().cameraDistance>40,"Combat camera widens for off-stage recovery");
 s.finishedAt=s.roundTime;s.winner=0;auto round=s.round;b.advance(8);check(b.state().round==round,"Playable match must remain at results, never auto-reset");
 std::cout<<"PASS: player movement, jump edges, recovery, guard, attacks, special, dodge, grab, recovery camera, persistent results\n";return 0;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
