#!/bin/bash

mkdir -p out_web
EMFLAGS="-s USE_SDL=2 -s USE_SDL_IMAGE=2 --embed-file=res"
em++ $EMFLAGS -Iinclude -std=c++14 -x c++ -c src/console.cpp -o out_web/console.o

em++ $EMFLAGS --shell-file src/shell_minimal.html -sEXPORTED_FUNCTIONS=_main,_tmp_delme,_net_get_bullet,_net_kill_bullet,_net_player_died,_net_delete_player,_net_set_userid,_net_set_player_state,_net_set_bullet_state,_net_recv_key_down,_net_recv_key_up -Iinclude -std=c++14 src/main.cpp -o out_web/main.html
# cp src/index.html out_web/index.html

# em++ -Iinclude -std=c++14 out/console.o src/console_test.cpp -o out_web/console_test.html
# em++ -Iinclude -std=c++14 out/console.o src/rng_test.cpp -o out_web/rng_test.html

