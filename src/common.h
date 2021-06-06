#pragma once

#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <array>
#include <windows.h>
#include <winuser.h>

#include "random.h"

#define PRINT_SEP "==================="
#define PRINT_TITLE(X) cout << PRINT_SEP << endl << X << endl;

using namespace std;

// Probability, keys, release key
typedef tuple<float, vector<unsigned char>, bool> input_group;

struct input_key_config {
    vector<input_group> keys;
    bool wScan = true;
};

struct input_mouse_config {
    bool move = false;
    bool left = false;
    bool right = false;
    bool relative = false;
    long maxX = 100, maxY = 100;
};

struct input_config {
    input_key_config kc{};
    input_mouse_config mc{};
    float chance_key = .8;
    bool simulate = false;
    string title;
};

struct time_config {
    array<uint16_t, 2> delay = {10, 30}; // Min, Max value for delay
};

struct screen_config {
    long x, y, w, h;
};

struct output_config {
    bool verbose = false;
    ofstream *log_file;
};

struct config {
    input_config ic;
    time_config tc;
    screen_config sc;
    output_config oc;
};

bool valdiate_config(config *c) {
    bool success = true;
    float p = 0;
    for (const auto& group: c->ic.kc.keys) {
        p += get<0>(group);
    }
    if (p != 1.) {
        cerr << "Etzlaa is des so, die Wascheinlichkeitneee mus imer einz sein!" << endl;
        success = false;
    }

    return success;
}

string key_name(uint8_t key) {
    stringstream ss;
    if (('A' <= key && key <= 'Z') || ('0' <= key && key <= '9')) {
        ss << key;
    } else if (VK_F1 <= key && key <= VK_F24) {
        ss << "F" << key - VK_F1 + 1;
    } else {
        ss << hex << key + 0;
    }
    return ss.str();
}

void print_key_config(input_key_config kc) {
    PRINT_TITLE("Keyboard:");
    int i = 1;
    for (const auto& group: kc.keys) {
        float p = get<0>(group);
        auto keys = get<1>(group);
        cout << "Grubbe " << i++ << " (" << round(p * 100) << "%):";
        for (auto key: keys) {
            cout << " " << key_name(key);
        }
        cout << endl;
    }
    cout << endl;
}

void print_mouse_config(input_mouse_config mc) {
    PRINT_TITLE("Mouse:");
    cout << "Move - " << (mc.move ? "YES" : "NO") << endl;
    cout << "Left - " << (mc.left ? "YES" : "NO") << endl;
    cout << "Right - " << (mc.right ? "YES" : "NO") << endl;
    cout << "Move relative - " << (mc.relative ? "YES" : "NO") << endl;
    if (mc.relative) {
        cout << "Max move X - " << mc.maxX << endl;
        cout << "Max move Y - " << mc.maxY << endl;
    }
}

void print_input_config(const input_config& ic) {
    print_key_config(ic.kc);
    print_mouse_config(ic.mc);
}

void print_time_config(time_config tc) {
    PRINT_TITLE("Time:");
    cout << "Delay (" << tc.delay[0] << ", " << tc.delay[1] << ") seconds" << endl;
}

void print_screen_config(screen_config sc) {
    PRINT_TITLE("Screen:");
    cout << "Active screen " << sc.w << "x" << sc.h << " pixels" << endl;
}

void print_config(config *c) {
    cout << "CONFIG" << endl;
    print_input_config(c->ic);
    print_time_config(c->tc);
    print_screen_config(c->sc);
}

void log(config *c, string text) {
    if (c->oc.verbose) {
        cout << text;
    } else {
#ifdef DEBUG
        cout << text;
#endif
    }
    if (c->oc.log_file->is_open()) {
        *c->oc.log_file << text;
        c->oc.log_file->flush();
    }
}

void log(config *c, stringstream *text) {
    log(c, text->str());
}

RECT GetDesktopResolution() {
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    return desktop;
}

screen_config get_screen_config() {
    RECT dims = GetDesktopResolution();
    return { dims.left, dims.top, dims.right - dims.left, dims.bottom - dims.top };
}

bool send_input(config *c) {
    stringstream ss;
    INPUT inputs[2] = {0};
    bool need_second = true;

    if (randf() < c->ic.chance_key) {
        // send key
        inputs[0].type = INPUT_KEYBOARD;
        inputs[0].ki.dwFlags = 0;
        uint8_t key = -1;
        uint8_t idx;
        float p = 0;
        float x = randf();
        int i = 0;
        for (const auto& group: c->ic.kc.keys) {
            p += get<0>(group);
            if (p > x) {
                need_second = get<2>(group);
                idx = (uint8_t) randl(0, (int) get<1>(group).size());
                key = get<1>(group)[idx];
                break;
            }
        }
        if (key == (uint8_t) -1) {
            key = get<1>(c->ic.kc.keys[0])[0];
        }

        ss << "Schigge dasde " << key_name(key) << endl;
        log(c, &ss);

        inputs[0].ki.wVk = key;
        inputs[0].ki.wScan = MapVirtualKeyA(key, MAPVK_VK_TO_VSC);
        if (c->ic.kc.wScan) {
            inputs[0].ki.dwFlags |= KEYEVENTF_SCANCODE;
        }
        memcpy(&inputs[1], &inputs[0], sizeof(INPUT));
        inputs[1].ki.dwFlags |= KEYEVENTF_KEYUP;
    } else {
        // send mouse
        inputs[0].type = INPUT_MOUSE;
        int option_count = c->ic.mc.move + c->ic.mc.left + c->ic.mc.right;
        uint8_t options = c->ic.mc.move << 0 | c->ic.mc.left << 1 | c->ic.mc.right << 2;
        int option;
        do {
            option = (int) round(randf() * (float) option_count);
        } while ((1 << option & options) == 0);
        if (option == 0) {
            need_second = false;
            inputs[0].mi.dwFlags = MOUSEEVENTF_MOVE;
            long x, y;
            if (c->ic.mc.relative) {
                inputs[0].mi.dx = (x = randl(-c->ic.mc.maxX, c->ic.mc.maxX));
                inputs[0].mi.dy = (y = randl(-c->ic.mc.maxY, c->ic.mc.maxY));
            } else {
                screen_config sc = get_screen_config();
                inputs[0].mi.dwFlags |= MOUSEEVENTF_ABSOLUTE;
                inputs[0].mi.dx = MulDiv((x = randl(0, sc.w)), 65536, sc.w);
                inputs[0].mi.dy = MulDiv((y = randl(0, sc.h)), 65536, sc.h);
            }

            ss << "Bewegne die Maus ";
            if (c->ic.mc.relative) {
                ss << "um ";
            } else {
                ss << "auf ";
            }
            ss << x << ", " << y << endl;
            log(c, &ss);
        } else {
            ss << "Drügge den ";
            if (option == 1) {
                ss << "lingen";
            } else {
                ss << "rechten";
            }
            ss << " Mausknof" << endl;
            log(c, &ss);

            if (option == 1) {
                inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                memcpy(&inputs[1], &inputs[0], sizeof(INPUT));
                inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
            } else {
                inputs[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
                memcpy(&inputs[1], &inputs[0], sizeof(INPUT));
                inputs[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;
            }
        }
    }
#ifndef SIMULATE
    if (!c->ic.simulate) {
        INPUT send[1] = {};
        send[0] = inputs[0];
        if (SendInput(1, send, sizeof(INPUT)) != 1) return false;
        if (need_second) {
            Sleep(50); // Tiny sleep for Window to register event
            send[0] = inputs[1];
            if (SendInput(1, send, sizeof(INPUT)) != 1) return false;
        }
    }
#endif

    return true;
}

auto delay_random(config *c) {
    uint16_t delta = abs(c->tc.delay[1] - c->tc.delay[0]);
    auto delay = min(c->tc.delay[0], c->tc.delay[1]) * 1000 + (uint16_t) round((double) randf() * (double) delta * 1000.);
    return delay;
}

int delay_random_cycles(config *c) {
    auto delay = delay_random(c);
    stringstream ss;
    ss << "Wadde " << dec << delay + 0 << "ms" << endl;
    log(c, &ss);
    return (int) delay / 100;
}