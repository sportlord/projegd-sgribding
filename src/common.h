#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include <Windows.h>
#include <WinUser.h>

#include "random.h"

#define PRINT_SEP "==================="
#define PRINT_TITLE(X) cout << PRINT_SEP << endl << X << endl;

using namespace std;

struct input_key_config {
    // Virtual Key Codes
    vector<uint8_t> basic;
    vector<uint8_t> special;
    float chance_special = .1;
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
    string title;
};

struct time_config {
    array<uint16_t, 2> delay = {10, 30}; // Min, Max value for delay
};

struct screen_config {
    long x, y, w, h;
};

struct config {
    input_config ic{};
    time_config tc{};
    screen_config sc{};
};

void print_key_config(input_key_config kc) {
    PRINT_TITLE("Keyboard:");
    cout << "Basic - ";
    for (uint8_t vcs : kc.basic) {
        cout << vcs << " ";
    }
    cout << endl;
    cout << "Special - ";
    for (uint8_t vcs : kc.special) {
        if ('A' <= vcs && vcs <= 'Z')
            cout << vcs;
        else
            cout << hex << vcs;
        cout << " ";
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

void print_config(config c) {
    cout << "CONFIG" << endl;
    print_input_config(c.ic);
    print_time_config(c.tc);
    print_screen_config(c.sc);
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

bool send_input(input_config ic) {
    INPUT inputs[2] = {0};
    bool need_second = true;
    if (randf() < ic.chance_key) {
        // send key
        inputs[0].type = INPUT_KEYBOARD;
        inputs[0].ki.dwFlags = 0;
        uint8_t key = 0;
        uint8_t idx;
        if (randf() < ic.kc.chance_special) {
            idx = (uint8_t) randl(0, (int) ic.kc.special.size());
            key = ic.kc.special[idx];
        } else {
            idx = (uint8_t) randl(0, (int) ic.kc.basic.size());
            key = ic.kc.basic[idx];
        }
#if DEBUG
        cout << "Sending key ";
        if ('A' <= key && key <= 'Z') {
             cout << key;
        } else {
            cout << hex << key + 0;
        }
        cout << endl;
#endif
        inputs[0].ki.wVk = key;
        inputs[0].ki.wScan = MapVirtualKeyA(key, MAPVK_VK_TO_VSC);
        if (ic.kc.wScan) {
            inputs[0].ki.dwFlags |= KEYEVENTF_SCANCODE;
        }
        memcpy(&inputs[1], &inputs[0], sizeof(INPUT));
        inputs[1].ki.dwFlags |= KEYEVENTF_KEYUP;
    } else {
        // send mouse
        inputs[0].type = INPUT_MOUSE;
        int option_count = ic.mc.move + ic.mc.left + ic.mc.right;
        uint8_t options = ic.mc.move << 0 |  ic.mc.left << 1 | ic.mc.right << 2;
        int option;
        do {
            option = (int) round(randf() * (float) option_count);
        } while ((1 << option & options) == 0);
        if (option == 0) {
            need_second = false;
            inputs[0].mi.dwFlags = MOUSEEVENTF_MOVE;
            long x, y;
            if (ic.mc.relative) {
                inputs[0].mi.dx = (x = randl(-ic.mc.maxX, ic.mc.maxX));
                inputs[0].mi.dy = (y = randl(-ic.mc.maxY, ic.mc.maxY));
            } else {
                screen_config sc = get_screen_config();
                inputs[0].mi.dwFlags |= MOUSEEVENTF_ABSOLUTE;
                inputs[0].mi.dx = MulDiv((x = randl(0, sc.w)), 65536, sc.w);
                inputs[0].mi.dy = MulDiv((y = randl(0, sc.h)), 65536, sc.h);
            }
#if DEBUG
            cout << "Moving mouse ";
            if (ic.mc.relative) {
                cout << "relatively by ";
            } else {
                cout << "to ";
            }
            cout << x << ", " << y << endl;
#endif
        } else {
#if DEBUG
            cout << "Pressing the ";
            if (option == 1) {
                cout << "left";
            } else {
                cout << "right";
            }
            cout << " mouse button" << endl;
#endif
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
#if !SIMULATE
    INPUT send[1] = {};
    send[0] = inputs[0];
    if (SendInput(1, send, sizeof(INPUT)) != 1) return false;
    if (need_second) {
        Sleep(50); // Tiny sleep for Window to register event
        send[0] = inputs[1];
        if (SendInput(1, send, sizeof(INPUT)) != 1) return false;
    }
#endif
    return true;
}

void delay_random(time_config tc) {
    uint16_t delta = abs(tc.delay[1] - tc.delay[0]);
    auto delay = min(tc.delay[0], tc.delay[1]) * 1000 + (uint16_t) round((double) randf() * (double) delta * 1000.);
#if DEBUG
    cout << "Sleeping " << dec << delay + 0 << "ms" << endl;
#endif
    Sleep(delay);
}