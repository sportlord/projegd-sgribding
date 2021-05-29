#pragma once

#include "../common.h"

input_config create_config_lol() {
    input_config c = {};
    c.kc.basic = { 'Q', 'W', 'E' };
    c.kc.special = { 'R', 'D', 'F' };
    c.kc.chance_special = .2;
    c.kc.wScan = true;
    c.mc.right = true;
    c.mc.move = true;
    c.title = "League of Legends (TM) Client";
    return c;
}


