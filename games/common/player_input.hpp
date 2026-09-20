// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
namespace arcade {
enum Button : unsigned { South=0, East, West, North, LB, RB, Start, Back, LT, RT, ButtonCount };
constexpr std::uint32_t bit(Button b){return 1u<<b;}
struct Control {
 float x=0,y=0,throttle=0,brake=0;
 std::uint32_t held=0,pressed=0;
 bool down(Button b)const{return held&bit(b);}
 bool tapped(Button b)const{return pressed&bit(b);}
 void merge(const Control& c){auto pending=pressed;*this=c;pressed|=pending;}
};
}
