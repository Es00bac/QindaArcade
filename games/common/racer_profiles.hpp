// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
namespace arcade {
struct RacerProfile {const char* kart;const char* role;const char* sponsor;const char* sponsorLine;const char* sticker;const char* story;};
inline constexpr RacerProfile profiles[]={
 {"BREAKWATER","RESCUE TECH / RELUCTANT HERO","OPEN CURRENT","COOPERATIVE","LEAVE NO ONE BEHIND","A retired rescue tug, rebuilt to win back the flood district's equipment."},
 {"PRESSURE DROP","INVENTOR / INSUFFERABLE GENIUS","BOILERMATE","WORKS","MIND THE PRESSURE","A laundry accumulator and a welded tub. Ducké made every unreasonable part."},
 {"REDLINE RIOT","STREET RACER / LOUDMOUTH RIVAL","AFTERHOURS","AUDIO","LOUDER THAN YOUR EXCUSES","A medicine courier's rally build. Vix wants a title the city cannot dismiss."},
 {"LOST & FOUND","SALVAGE COURIER / OPPORTUNIST","SECOND BYTE","SALVAGE","NOT LOST. RELOCATED.","Three donor carts and a parcel cage. Cache is racing to save the salvage yard."},
 {"MOONSHOT","HILLCLIMB ENGINEER / UNDERDOG","SWITCHBACK","INSTRUMENTS","MEASURE TWICE. SEND ONCE.","A hand-built precision racer, funding a new generation of mountain apprentices."},
 {"HOSTILE TAKEOVER","CORPORATE ACE / VILLAIN","VANTA","DYNAMICS","TERMS APPLY","A factory prototype built to make Vanta's takeover look inevitable."},
 {"COMMON GROUND","COMMUNITY MECHANIC / ALLY","MEND & GROW","UNION","FIX IT. DON'T BIN IT.","A neighborhood-built service buggy. Every lap helps protect the garden's water."},
 {"LITTLE CURRENT","WETLAND SCIENTIST / ACTIVIST","RILL RESEARCH","COLLECTIVE","SMALL BUT NOT OPTIONAL","A wheeled research skiff. Axi races for clean streams and a much bigger audience."}
};
}
