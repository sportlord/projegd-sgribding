#pragma once

#include "../common.h"

input_config create_config_lol() {
    input_config c = {};
    c.kc.keys = { { .6, { 'Q', 'W', 'E' }, true }, { .2, { 'R', 'D', 'F' }, true }, { .1, { '1', '2', '3', '5', '6', '7' }, true }, { .1, { VK_F2, VK_F3, VK_F4, VK_F5 }, false } };
    c.kc.wScan = true;
    c.mc.right = true;
    c.mc.move = true;
    c.title = "League of Legends (TM) Client";
    return c;
}


