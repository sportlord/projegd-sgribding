#include <iostream>
#include <Windows.h>
#include <synchapi.h>
#include <string>

#include "src/version.h" // Will be created by cmake

#include "../include/argparse/argparse.h"
#include "./games/lol.h"

#define STR1(x)  #x
#define STR(x)  STR1(x)

#define MINDELAY 5

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

bool is_game_active(config *c) {
    HWND window = GetForegroundWindow();
    char title[255];
    GetWindowTextA(window, title, 254);
    return c->ic.title == title;
}

int main(int argc, const char* argv[]) {
//    ghSemaphore = CreateSemaphore(nullptr, 1, 1, nullptr);
//
//    if (ghSemaphore == nullptr) {
//        printf("CreateSemaphore error: %lu\n", GetLastError());
//        return 1;
//    }
    argparse::ArgumentParser parser("Projegd Sgribding " VERSION_STR, "Varieirde wariande eines guden Brograms");
    parser.add_argument("game", "Des Spil des ihr etzala spild", true).count(1).position(0);
    parser.add_argument("-l", "--delay-lower", "Midesdens so lange wadden bis was basirt (min: " STR(MINDELAY) ", max: 120)", false).count(1);
    parser.add_argument("-u", "--delay-upper", "Magsimal so lange wadden bis was basirt (max: 300)",false).count(1);
    parser.add_argument("-f", "--file", "Schreib die Ausgabne in ein Buch",false).count(1);
    parser.add_argument("-v", "--verbose", "Erweiderde ausgabne",false);
    parser.add_argument("-s", "--simulate", "Agdionen nur sumulirn",false);
    parser.enable_help();
    auto err = parser.parse(argc, argv);
    if (err) {
        cout << err << endl;
        return -1;
    }

    if (parser.exists("help")) {
        parser.print_help();
        return 0;
    }

    string game = parser.get<string>("game");
    input_config ic;
    if (game == "lol") {
        ic = create_config_lol();
    } else {
        cout << "Des Schpil kenn ich net" << endl;
        return -2;
    }

    time_config tc = { { 15, 45 } };
    if (parser.exists("delay-lower")) {
        tc.delay[0] = min(120, max(MINDELAY, parser.get<uint16_t>("delay-lower")));
    }
    if (parser.exists("delay-lower")) {
        tc.delay[1] = min(300, max(tc.delay[0], parser.get<uint16_t>("delay-upper")));
    }

    ofstream log_file;
    if (parser.exists("file")) {
        log_file.open(parser.get<string>("file"), ios::out | ios::trunc);
    }

    ic.simulate = parser.exists("simulate");

    config c = {ic, tc, get_screen_config(), { parser.exists("verbose"), &log_file } };

    if (!valdiate_config(&c)) {
        return -3;
    }

    char show_conf[2]; // Need 2 bytes as cin stores newlines
    cout << "Wilsd du noch die Gonfiguration sehne? [Ny] ";
    cin.get(show_conf, 2);
    switch (show_conf[0]) {
        case 'y':
        case 'Y':
            print_config(&c);
            break;
    }

    cout << "Leds Blähhh alder" << endl;

    int sleep_cycles = 0;

    for (;;) {
        if (sleep_cycles > 0) {
            Sleep(100);
        }
        if (paused) {
            continue;
        }
#ifndef SIMULATE
        if (!is_game_active(&c)) {
            log(&c, "Net im Spil. Ich wadde...");
            while (!is_game_active(&c)) {
                Sleep(100);
            }
            log(&c, "un weida\n");
            continue;
        }
#endif
        if (sleep_cycles-- > 0) {
            continue;
        }
        sleep_cycles = delay_random_cycles(&c);
        if (!send_input(&c))
            break;
    }

    if (log_file.is_open()) {
        log_file.close();
    }

//    CloseHandle(ghSemaphore);

    return 0;
}
