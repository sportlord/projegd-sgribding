#include <iostream>
#include <Windows.h>
#include <synchapi.h>
#include <string>
#include <sstream>

#define DEBUG 1
#define SIMULATE 0

#include "./games/lol.h"

using namespace std;

//HANDLE ghSemaphore;
bool paused = false;

uint16_t get_uint16_or_default(uint16_t def) {
    uint16_t parsed = def;
    string input;
    getline(cin, input);
    if (!input.empty()) {
        istringstream istr(input);
        istr >> parsed;
    }
    return parsed;
}

bool is_game_active(config c) {
    HWND window = GetForegroundWindow();
    char title[255];
    GetWindowTextA(window, title, 254);
    cout << title << endl;
    return c.ic.title.compare(title) == 0;
}

int main(VOID) {
//    ghSemaphore = CreateSemaphore(nullptr, 1, 1, nullptr);
//
//    if (ghSemaphore == nullptr) {
//        printf("CreateSemaphore error: %lu\n", GetLastError());
//        return 1;
//    }

    cout << "Minimum seconds delay (default: 15, min: 5, max 120): ";

    uint16_t delayMin = get_uint16_or_default(15);
    delayMin = min(120, max(5, delayMin));
    cout << "Maximum seconds delay (default: 45, max 300): ";
    uint16_t delayMax = get_uint16_or_default(45);
    delayMax = min(300, max(delayMin, delayMax));

    cout << "Currently only league is supported..." << endl;

    config config = {
            create_config_lol(),
            {
                    {delayMin, delayMax}
            },
            get_screen_config()
    };


    char show_conf[2]; // Need 2 bytes as cin stores newlines
    cout << "Should I show you the current config? [Ny] ";
    cin.get(show_conf, 2);
    switch (show_conf[0]) {
        case 'y':
        case 'Y':
            print_config(config);
            break;
    }

    screen_config sc = get_screen_config();
    for (;;) {
        if (paused || !is_game_active(config)) {
            Sleep(500);
            continue;
        }
        if (!send_input(config.ic))
            break;
        delay_random(config.tc);
    }

//    CloseHandle(ghSemaphore);

    return 0;
}
