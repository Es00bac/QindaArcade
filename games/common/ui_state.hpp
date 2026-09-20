// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <string>
#include <vector>
namespace arcade {
enum class Page {Home,Setup,Playing,Paused,Results,Settings,Controls};
struct Row {std::string label,value;};
struct Ui {
 Page page=Page::Home;bool racing=false,playstation=false,exhibition=false;int selected=0,players=1,player=0;
 double clock=0;std::string title,subtitle,description,notice,device="Keyboard",mapping;
 std::vector<Row> rows,standings;
 std::string standingsTitle;
 std::string profileKart,profileRole,profileSponsor,profileStory;
};
}
