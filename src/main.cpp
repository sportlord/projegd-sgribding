#include <iostream>
#include <Windows.h>
#include <synchapi.h>
#include <string>

#include "src/version.h" // Will be created by cmake
#include "types.h"

#include "../include/argparse/argparse.h"
#include "./games/lol.h"

#define STR1(x)  #x
#define STR(x)  STR1(x)

#define MINDELAY 3

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

typedef input_config (*config_creator)(VOID);

struct game_info {
    string name;
    config_creator creator;
};


map<string, game_info> GAMES = {
        {"lol", { "League of Legends", &create_config_lol } },
};

int main(int argc, const char* argv[]) {
    SetConsoleOutputCP( CP_UTF8 ); // Meddlfrängisch-Understüdsung


//    ghSemaphore = CreateSemaphore(nullptr, 1, 1, nullptr);
//
//    if (ghSemaphore == nullptr) {
//        printf("CreateSemaphore error: %lu\n", GetLastError());
//        return 1;
//    }
    argparse::ArgumentParser parser("Projegd Sgribding " VERSION_STR, "Varieirde wariande eines guden Brograms");
    parser.add_argument("-g", "--game", "Des Spil des ihr etzala spild", false).count(1);
    parser.add_argument("-l", "--delay-lower", "Midesdens so lange wadden bis was basirt (min: " STR(MINDELAY) ", max: 120)", false).count(1);
    parser.add_argument("-u", "--delay-upper", "Magsimal so lange wadden bis was basirt (max: 300)",false).count(1);
    parser.add_argument("-f", "--file", "Schreib die Ausgabne in ein Buch",false).count(1);
    parser.add_argument("-q", "--quiet", "Gib nich so viel ausne",false);
    parser.add_argument("-v", "--verbose", "Erweiderde Ausgabne",false);
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

    string game;
    if (parser.exists("game")) {
        game = parser.get<string>("game");
    } else {
        cout << "Wähl etzala n Spil aus:" << endl;
        int i = 0;
        vector<string> keys;
        for (auto game: GAMES) {
            keys.insert(keys.begin(), game.first);
            cout << ++i << " - " << game.second.name << endl;
        }
        cout << "Tibbe ne Nummer: ";
        int sel;
        do {
            cin >> sel;
        } while (sel < 1 || sel > i);
        game = keys[sel - 1];
    }
    if (GAMES.find(game) == GAMES.end()) {
        cout << "Des Schpil kenn ich net" << endl;
        return -2;
    }

    input_config ic;
    ic = GAMES[game].creator();

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

    config c = {ic, tc, get_screen_config(), { parser.exists("quiet"), &log_file } };

    if (!valdiate_config(&c)) {
        return -3;
    }

    if (parser.exists("verbose")) {
        print_config(&c);
    }

    cout << "Leds Blähhh alder" << endl;

    int sleep_cycles = 0;

    for (;;) {
        if (sleep_cycles > 0) {
            Sleep(SLEEP_CYCLE);
        }
        if (paused) {
            continue;
        }
#ifndef SIMULATE
        if (!is_game_active(&c)) {
            log(&c, "Net im Spil. Ich wadde...", true);
            while (!is_game_active(&c)) {
                Sleep(SLEEP_CYCLE);
            }
            log(&c, "un weida\n", true);
            continue;
        }
#endif
        if (sleep_cycles-- > 0) {
            continue;
        }
        if (!send_input(&c))
            break;
        sleep_cycles = delay_random_cycles(&c);
    }

    if (log_file.is_open()) {
        log_file.close();
    }

//    CloseHandle(ghSemaphore);

    return 0;
}
