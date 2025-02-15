

#include <cassert>

#include "main.hpp"

#include "console.hpp"

// SDL globals`
SDL_Window* gWindow = nullptr;;
SDL_Renderer* gRenderer = nullptr;
SDL_Texture* gTexture = nullptr;


// import gfx from "./gfx.js"
// import Joystick from "./joy.js";
// import type { DPadDirection } from "./joy.js";


// type bool = boolean;
// type str = string;
// type num = number;
// type s32 = number;
// type f32 = number;
// type u8 = number;

// void u8(x:any):u8 {
// 	return x as u8
// }

// void s32(x:num):s32 {
// 	return x as s32
// }

// void int(x:num):s32 {
// 	return std::round(x);
// }


// todo move to util or math or something
f32 places(f32 num, s32 places) {
	const int e = std::pow(10, places);
	return (float)std::round(num * e) / e;
}


// void f32(x:any):f32 {
// 	return x as f32
// }

///

const int NET_FLOAT_PLACES = 2; // decimal places




/////////////////////////////

// const PLAYER_WIDTH = 32
// const PLAYER_HEIGHT = 32

const int PLAYER_WIDTH = 24;
const int PLAYER_HEIGHT = 24;

// const SCREEN_WIDTH = 800
// const SCREEN_HEIGHT = 600

// const SCREEN_WIDTH = 1600
// const SCREEN_HEIGHT = 450

// edit: canvas will fill screen dynamically
int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;

int SCREEN_WIDTH_HALF = SCREEN_WIDTH / 2;
int SCREEN_HEIGHT_HALF = SCREEN_HEIGHT / 2;



// f32 WORLD_WIDTH = SCREEN_WIDTH
// f32 WORLD_HEIGHT = SCREEN_HEIGHT
int WORLD_WIDTH = 1024;
int WORLD_HEIGHT = 1024;


int SCALE = 2;

bool JOY_VISIBLE = true;

////////////////////

// TODO KILL IT WITH FIRE
// void InitArrayWithSize(array: any[], size: s32, constructor: Function) {
// 	for(f32 i = 0; i < size; i ++) {
// 		array[i] = constructor()
// 	}
// }



/////////////////////////////




///////////////////////////////

// interface Player {
// 	x: s32,
// 	y: s32,
// 	// color: Color, // note: no longer stored, but calculated based on ID
// 	alive: bool,
// 	life: s32,
// 	exists: bool,
// 	keyboard: KbdButtonMap,
// 	mouse: PlayerMouse, 	// TODO: unused? SEE ALSO below: InitArrayWithSize(players,...)
// 	deathTimer: s32,
// }




const int PLAYER_MOVE_SPEED = 5;

const int PLAYER_START_LIFE = 100;

Player default_player(bool exists) {
	return { 
		0,0, // pos
		true, // alive // TODO: is alive redundant when we have life?
		PLAYER_START_LIFE,
		exists,
		0 // death timer
		// keyboard: {}, 
		// mouse: {x:-1, y:-1, down:{}},
	};
	// TODO FIXME PORT the mouse?
}

// f32 player : Player;
u32 player_idx = -1;

const int MAX_PLAYERS = 256;

Player players[MAX_PLAYERS];

// players.length = MAX_PLAYERS // ...beautiful

// TODO FIXME PORT
// InitArrayWithSize(players, MAX_PLAYERS, function() : Player {
// 	// todo replace w default player function?
// 	// i think that one generates a living player tho
// 	return { x:0, y:0, /*color: COLOR_ZERO,*/ alive: false, life: 0, exists: false, keyboard: {}, mouse: {x:0, y:0, down: {}}, deathTimer:0}
// })


const s32 BULLET_WIDTH = 8;
const s32 BULLET_HEIGHT = 8;

const s32 BULLET_START_LIFE = 50; // TODO eliminate

const s32 BULLETS_PER_PLAYER = 6;
const s32 MAX_BULLETS = MAX_PLAYERS * BULLETS_PER_PLAYER;

Bullet bullets[MAX_BULLETS]; // TODO PORT FIXME: We're not exhausting the stack, are we?



// TODO FIXME PORT -- yeah i don't know what happened in the conversion here
// Bullet InitArrayWithSize(bullets, MAX_BULLETS) {
// 	return { x:0, y:0, xf:0, yf:0, dx:0, dy:0, alive: false, life: 0 }
// })





/////////////////////////////






SDL_GameController *findController() {
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            return SDL_GameControllerOpen(i);
        }
    }

    return nullptr;
}
SDL_GameController *gController = nullptr;

// Initialization
bool init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    gWindow = SDL_CreateWindow("boxfight", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!gWindow) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!gRenderer) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    
    gController = findController();

    return true;
}

// Load BMP as texture
bool loadMedia() {
    SDL_Surface* loadedSurface = SDL_LoadBMP("res/lilguy.bmp");
    if (!loadedSurface) {
        printf("Unable to load image %s! SDL Error: %s\n", "res/lilguy.bmp", SDL_GetError());
        return false;
    }
    gTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);
    return gTexture != NULL;
}

// Free resources
void closeApp() {
    SDL_DestroyTexture(gTexture);
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    SDL_Quit();
}


/////////


const Uint8* keystate = nullptr;
Uint8* keystate_prev = nullptr;
float x_axis_prev = 0;
float y_axis_prev = 0;

void kbd_copy_prev() {
	int length = 0;
	SDL_GetKeyboardState(&length);
	keystate_prev = (Uint8*) malloc(length * sizeof(Uint8));
	for(int i = 0; i < length; i++) {
		keystate_prev[i] = keystate[i];
	}
}

bool kbd_down(SDL_Scancode code){
	if(keystate == nullptr) {
		std::cout << "DBG: KEYSTATE IS NULL" << std::endl;
		return false;
	}
	return keystate[code];
}

bool kbd_was_down(SDL_Scancode code) {
	if(keystate == nullptr || keystate_prev == nullptr) {
		std::cout << "DBG: KEYSTATE OR KEYSTATE_PREV IS NULL" << std::endl;
		return false;
	}
	return !keystate[code] && keystate_prev[code];
}

bool kbd_pressed(SDL_Scancode code) {
	if(keystate == nullptr || keystate_prev == nullptr) {
		std::cout << "DBG: KEYSTATE OR KEYSTATE_PREV IS NULL" << std::endl;
		return false;
	}
	return keystate[code] && !keystate_prev[code];
}


/////////


void init_game() {
	for(int i = 0; i < MAX_PLAYERS; i++) {
		players[i] = default_player(false);
	}
}



// Main loop function
void mainLoop() {
    Uint32 FrameStart = SDL_GetTicks();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
#ifdef EMSCRIPTEN
                emscripten_cancel_main_loop();
#endif
                closeApp();
                return;

            case SDL_CONTROLLERDEVICEADDED:
                if (!gController) {
                    gController = SDL_GameControllerOpen(event.cdevice.which);
                }
                break;

            case SDL_CONTROLLERDEVICEREMOVED:
                if (gController && event.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gController))) {
                    SDL_GameControllerClose(gController);
                    gController = findController();
                }
                break;

        }
    }

    ////
    // handle input
    keystate = SDL_GetKeyboardState(NULL);

    float x_axis = (float)SDL_GameControllerGetAxis(gController, SDL_CONTROLLER_AXIS_LEFTX) / SDL_JOYSTICK_AXIS_MAX;
    float y_axis = (float)SDL_GameControllerGetAxis(gController, SDL_CONTROLLER_AXIS_LEFTY) / SDL_JOYSTICK_AXIS_MAX;
    // float x_axis = 0;
    // float y_axis = 0;
	float JOY_THRESHOLD = 0.3;

#ifdef EMSCRIPTEN
	
	if (kbd_pressed(SDL_SCANCODE_1)) {
		std::cout << "1" << std::endl;
	}

    if (kbd_pressed(SDL_SCANCODE_UP) || kbd_pressed(SDL_SCANCODE_W) || y_axis < -JOY_THRESHOLD) {
		std::cout << "DBG: MOVE UP" << std::endl;
		EM_ASM(
			sendMessage('KD KeyW');
		);

	
	}else if(kbd_was_down(SDL_SCANCODE_UP) || kbd_was_down(SDL_SCANCODE_W)
			|| y_axis == 0 && (y_axis_prev < -JOY_THRESHOLD) ) {
		std::cout << "DBG: MOVE UP STOP" << std::endl;
		EM_ASM(
			sendMessage('KU KeyW');
		);
	}
	
	
	if (kbd_pressed(SDL_SCANCODE_DOWN) || kbd_pressed(SDL_SCANCODE_S) || y_axis > JOY_THRESHOLD) {
		std::cout << "DBG: MOVE DOWN" << std::endl;
		EM_ASM(
			sendMessage('KD KeyS');
		);
	}else if(kbd_was_down(SDL_SCANCODE_DOWN) || kbd_was_down(SDL_SCANCODE_S)
			|| y_axis == 0 && (y_axis_prev > JOY_THRESHOLD) ) {
		std::cout << "DBG: MOVE DOWN STOP" << std::endl;
		EM_ASM(
			sendMessage('KU KeyS');
		);
	}

    if (kbd_pressed(SDL_SCANCODE_LEFT) || kbd_pressed(SDL_SCANCODE_A) || x_axis < -JOY_THRESHOLD) {
		std::cout << "DBG: MOVE LEFT" << std::endl;
		EM_ASM(
			sendMessage('KD KeyA');
		);
	} else if(kbd_was_down(SDL_SCANCODE_LEFT) || kbd_was_down(SDL_SCANCODE_A)
			|| x_axis == 0 && (x_axis_prev < -JOY_THRESHOLD) ) {
		std::cout << "DBG: MOVE LEFT STOP" << std::endl;
		EM_ASM(
			sendMessage('KU KeyA');
		);
	}

    if (kbd_pressed(SDL_SCANCODE_RIGHT) || kbd_pressed(SDL_SCANCODE_D) || x_axis > JOY_THRESHOLD) {
		std::cout << "DBG: MOVE RIGHT" << std::endl;
		EM_ASM(
			sendMessage('KD KeyD');
		);
	}else if(kbd_was_down(SDL_SCANCODE_RIGHT) || kbd_was_down(SDL_SCANCODE_D)
			|| x_axis == 0 && (x_axis_prev > JOY_THRESHOLD) ) {
		std::cout << "DBG: MOVE RIGHT STOP" << std::endl;
		EM_ASM(
			sendMessage('KU KeyD');
		);
	}
#endif

    ////
    // update

	update(FrameStart);


	kbd_copy_prev();
	x_axis_prev = x_axis;
	y_axis_prev = y_axis;
	
	///////////
	/// draw

    SDL_RenderClear(gRenderer);

	draw(FrameStart);
    // // SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);
    // const SDL_Rect dstrect = {guy_x, guy_y, guy_width, guy_height};
    // SDL_RenderCopy(gRenderer, gTexture, NULL, &dstrect);
    SDL_RenderPresent(gRenderer);

    // delay next frame
    Uint32 FrameEnd = SDL_GetTicks();
    Uint32 FrameTime = FrameEnd - FrameStart;
    //std::cout << "FrameTime: " << FrameTime << std::endl;
    Uint32 FrameTarget = 16; // 60fps // TODO not quite!
    Uint32 Delay = FrameTarget - FrameTime;
    //std::cout << "Delay: " << Delay << std::endl;
    if(FrameTime < FrameTarget)
    {   
    //  std::cout << "Waiting for: " << Delay << std::endl;
        SDL_Delay(Delay);
    }
    //std::cout<<std::endl;
}

int main(int argc, char* args[]) {
    if (!init()) {
        printf("Failed to initialize!\n");
        return 1;
    }
    if (!loadMedia()) {
        printf("Failed to load media!\n");
        return 1;
    }


	init_game();

#ifdef EMSCRIPTEN

	EM_ASM(
		pathetic_snowflake_done();
	);

	emscripten_set_main_loop(mainLoop, 0, 1);
#else
    while(true){
        mainLoop();
    }
#endif

    return 0;
}



///////////////////////////////



//////////////////
// WebSockets



// TODO FIXME - move the transport stuff into its own thing. Game shouldnt need to worry about it

// const host = window.location.host; // e.g. "localhost:3000"
// const path = window.location.pathname; // e.g. "/12345"
// const url_ws = "ws://" + host + path; // note: we use the path for the room
// const url_wss = "wss://" + host + path;

// f32 ws : WebSocket;
// f32 ws_ws = ws_connect(url_ws);
// f32 ws_wss = ws_connect(url_wss);

// const bool PRINT_MSG_IN = false;
// const bool PRINT_MSG_OUT = false;



// void ws_init() {
// 	// Module.print('connect_websocket');

	
// }

// WebSocket ws_dummy() {
// 	return new WebSocket("wss://asdf");
// 	// note: this will fail, we just need A websocket object
// 	// to placate TS's type system... I'm so sorry,
// 	// this was better than making everything WebSocket | undefined.
// 	// Also, it'll just default to closed, which the system handles properly.
// 	// (And in case WS connection is impossible, it's replaced with this dummy,
// 	// so it doesn't spam reconnect either)
// }

// bool ws_is_encrypted(WebSocket _ws) {
// 	return _ws.url.includes("wss://");
// }

// bool ws_is_connecting(WebSocket _ws) {
// 	return _ws.readyState == 0;
// }

// bool ws_is_connected(WebSocket _ws) {
// 	return _ws.readyState == 1;
// }

// bool ws_is_closing(WebSocket _ws) {
// 	return _ws.readyState == 2;
// }

// bool ws_is_closed(WebSocket _ws) {
// 	return _ws.readyState == 3;
// }

// bool ws_reconnect(WebSocket _ws) {
// 	// ugh ... we just gotta rebuild the whole thing
// 	if(ws_is_encrypted(_ws)){
// 		ws_wss = ws_connect(url_wss);
// 	} else {
// 		ws_ws = ws_connect(url_ws);
// 		// TODO FIXME: this void grosses me out
// 		// ... a global void with special cases for other globals
// 		// throwing away the entire socket also seems silly
// 		// but maybe that's just how it's done
// 	}
// }

// void ws_setup_events(WebSocket _ws) {

// 	// init events
// 	_ws.onopen = void () {
// 		console.log("WebSocket connection opened");
// 		// rule: if I am ws and wss is connected, do nothing.
// 		// rule: if I am ws and wss is not connected, use me as main connection.
// 		//          AND (if wss not connecting) init wss connection.
// 		if ( ! ws_is_encrypted(_ws) ) {
// 			if(ws_is_connected(ws_wss)) {
// 				return; // already have encryption, don't use unencrypted ws
// 			} else {
// 				ws = _ws; // no encryption (yet), use unencrypted ws
// 				if (! ws_is_connecting(ws_wss)) {
// 					// should connect encrypted ws
// 					ws_connect(url_wss);
// 				}
// 			}
// 		} else {
// 			// I (_ws) am encrypted
// 			ws = _ws; // set myself as main ws
// 		}
// 	};

// 	_ws.onmessage = void (event) {
// 		if (PRINT_MSG_IN) {
// 			if (!event.data.includes('POS')) { // antispam
// 				console.log("Received message:", event.data);
// 			}
// 		}
// 		handleMessage(event.data);
// 	};

// 	_ws.onclose = void (event) {
// 		console.warn(`WebSocket closed with code ${event.code}. Reconnecting in 2 seconds...`);
// 		setTimeout(()=>{ws_reconnect(_ws)}, 2000);
// 	};

// 	_ws.onerror = void (error) {
// 		console.error("WebSocket error:", error);
// 		// Handle errors as needed
// 	};
// }

// void ws_connect(url: string) : WebSocket {

// 	console.log('websocket connecting to ' + url);
// 	// connect
// 	try {
// 		f32 _ws = new WebSocket(url);

// 		ws_setup_events(_ws);
// 		return _ws;
// 	} catch {
// 		return ws_dummy();
// 	} // god I love JavaScript
// }

// void sendMessage(msg: str) {
// 	if (!ws) return;
// 	if ( ! (ws.readyState == 1) ) return;
// 	if (!msg.startsWith('MM')){
// 		console.log('OUT: ' + msg);
// 	}
// 	if (PRINT_MSG_OUT) {
// 		if ( ! msg.startsWith('POS')) { // prevent spam
// 			console.log('sent message:')
// 			console.log(msg)
// 			console.log('');
// 		}
// 	}
// 	// msg = JSON.stringify(message)
// 	ws.send(msg);
// }


//////////////////
// Net Send


void net_send_shoot_start(s32 x, s32 y) {
	if(player_idx == -1) return;
#ifdef EMSCRIPTEN
	EM_ASM(
		sendMessage(`SS ${x} ${y}`);
	);
#else
	assert(0); // todo implement udp layer
#endif
}

void net_send_shoot_end() {
	if(player_idx == -1) return;

#ifdef EMSCRIPTEN
	EM_ASM(
		sendMessage(`SE`);
	);
#else
	assert(0); // todo implement udp layer
#endif
}




void net_send_shoot_aim(s32 x, s32 y) {
	const s32 PLACES = 100;
	x = std::round(x*PLACES) / PLACES;
	y = std::round(y*PLACES) / PLACES;
	
	if(player_idx == -1) return;


#ifdef EMSCRIPTEN
	EM_ASM(
		sendMessage(`SA ${x} ${y}`);
	);
#else
	assert(0); // todo implement udp layer
#endif
}


#ifdef EMSCRIPTEN
EM_JS(void, net_send_key_down, (const char* key), {
	// TODO filter for movement keys?
	sendMessage(`KD ${key}`);
});
EM_JS(void, net_send_key_up, (const char* key), {
	sendMessage(`KU ${key}`);
});
#endif



///////
// Net Receive
extern "C" {
void net_recv_key_down(s32 plr_id, char key) {
	if(!players[plr_id].exists) return;
	switch(key){
		case 'w': // note KeyW converted to 'w' on the JS side
			players[plr_id].wasd.w = true;
		break;
		case 'a':
			players[plr_id].wasd.a = true;
		break;
		case 's':
			players[plr_id].wasd.s = true;
		break;
		case 'd':
			players[plr_id].wasd.d = true;
		break;
		default:
            std::cout << key;
			assert(0);
		break;
	}
}
}

extern "C" {
void net_recv_key_up(s32 plr_id, char key, s32 x, s32 y) {
	if(!players[plr_id].exists) return;
	switch(key){
		case 'w': // note KeyW converted to 'w' on the JS side
			players[plr_id].wasd.w = false;
		break;
		case 'a':
			players[plr_id].wasd.a = false;
		break;
		case 's':
			players[plr_id].wasd.s = false;
		break;
		case 'd':
			players[plr_id].wasd.d = false;
		break;
		default:
            std::cout << key;
			assert(0);
		break;
	}
	players[plr_id].x=x;
	players[plr_id].y=y;
}
}


/////////////////


// DOM / Canvas / Input

// f32 canvas = document.getElementById("gameCanvas") as HTMLCanvasElement;
// f32 context = canvas.getContext("2d", { alpha: false }) as CanvasRenderingContext2D;

// resizeCanvas();

// context.imageSmoothingEnabled = false;

// gfx.init(canvas); // TODO DOES THIS WORK?
// gfx.setup(canvas, context);

// joy.init(canvas, SCALE);

// TODO FIXME PORT TOUCH JOYSTICK
// f32 joy_move = new Joystick(canvas, SCALE, canvas.width/4, canvas.height/4*3);
// f32 joy_shoot = new Joystick(canvas, SCALE, canvas.width/4*3, canvas.height/4*3);

// joy_move.is_dpad = true;



// TODO should I move this into joystick?

// f32 prevDirs: DPadDirection[] = [];


// joy_move.setCallback(function() {
// 	if(player_idx == -1) return;
// 	// console.log('a')
// 	if(!joy_move.haveDirsChanged()) return;
// 	// console.log('b')
// 	// console.log(joy_move.x, joy_move.y)
// 	for(f32 dir of joy_move.directions) {
// 		switch(dir) {
// 			case "Right":
// 				// note: We should also maybe set Keyboard.down[key]
// 				// except that that's completely unused o_o
// 				net_send_key_down("ArrowRight");
// 				break;
// 			case "Left":
// 				net_send_key_down("ArrowLeft");
// 				break;
// 			case "Up":
// 				net_send_key_down("ArrowUp");
// 				break;
// 			case "Down":
// 				net_send_key_down("ArrowDown");
// 				break;				
// 		}
// 	}
// 	// send release events
// 	if(joy_move.prevDirs.includes("Left") && !joy_move.directions.includes("Left")) {
// 		net_send_key_up("ArrowLeft");
// 	}
// 	if(joy_move.prevDirs.includes("Right") && !joy_move.directions.includes("Right")) {
// 		net_send_key_up("ArrowRight");
// 	}
// 	if(joy_move.prevDirs.includes("Up") && !joy_move.directions.includes("Up")) {
// 		net_send_key_up("ArrowUp");
// 	}
// 	if(joy_move.prevDirs.includes("Down") && !joy_move.directions.includes("Down")) {
// 		net_send_key_up("ArrowDown");
// 	}

// });

// joy_shoot.setCallback(function() {
// 	if(player_idx == -1) return;
// 	f32 player = players[player_idx]!;
// 	if(joy_shoot.mag == 0) {
// 		net_send_shoot_end();
// 	} else {
// 		console.log(joy_shoot.x, joy_shoot.y)
// 		net_send_shoot_start(joy_shoot.x, joy_shoot.y) 
// 	}
// })

// const MOUSE_LEFT = 0;
// const MOUSE_MIDDLE = 1;
// const MOUSE_RIGHT = 2;


// interface MouseButtonMap {
//     [key: num]: bool;
// }

// const Mouse = {
// 	x: -1,
// 	y: -1,
// 	moved: false,
// 	down: <MouseButtonMap>{},
// 	pressed: <MouseButtonMap>{}, // a button is pressed only for one frame
// };

// NOTE: mouse button 0 = left
// NOTE: mouse button 1 = middle
// NOTE: mouse button 2 = right


// // Note: disabled mouse move event
// window.addEventListener("mousemove", function(event){
// 	Mouse.moved = true;

// 	if(!Mouse.down){
// 		return; // only send MM msg if mouse held
// 	}

// 	// TODO: reduce MM spam?
	
// 	if(player_idx == -1) return;
// 	f32 player = players[player_idx]!;

//     f32 Mouse_x = event.clientX - canvas.offsetLeft;
// 	f32 Mouse_y = event.clientY - canvas.offsetTop;
	
// 	Mouse_x /= SCALE;
// 	Mouse_y /= SCALE;

// 	const diff_x = Mouse_x - canvas.width / 2;
// 	const diff_y = Mouse_y - canvas.height/ 2;
	
	
//     net_send_shoot_aim(diff_x, diff_y); // LATER: laser sights? ;)

// 	// Mouse.x = x;
// 	// Mouse.y = y;
// });

// prevent right click menu
// canvas.addEventListener("contextmenu", function(event){ event.preventDefault(); return false;})

// window.addEventListener("mousedown", function(event){
// 	Mouse.pressed[event.button] = true;
// 	Mouse.down[event.button] = true;

// 	if(player_idx == -1) return;
// 	f32 player = players[player_idx]!;

//     f32 Mouse_x = event.clientX - canvas.offsetLeft;
// 	f32 Mouse_y = event.clientY - canvas.offsetTop;
	
// 	Mouse_x /= SCALE;
// 	Mouse_y /= SCALE;

// 	// const diff_x = f32(Mouse_x - (player.x + PLAYER_WIDTH/2))
// 	// const diff_y = f32(Mouse_y - (player.y + PLAYER_HEIGHT/2))

// 	const diff_x = Mouse_x - canvas.width / 2;
// 	const diff_y = Mouse_y - canvas.height/ 2;

// 	// this.playSound(SFX.EXPLOSION_SHORT)

//     net_send_shoot_start(diff_x, diff_y);
// // });

// canvas.addEventListener("mousedown", function(_event){ 
	
// 	// event.preventDefault(); // prevent clicks from selecting text
// 	// NOTE: this might prevent scroll on mobile? (do we WANT scroll?)
// 	// EDIT: this makes it so you can't leave the text box...
	
// 	// window.getSelection()!.removeAllRanges();
// 	// doesn't work

// });

// window.addEventListener("mouseup",   function(_event){ 
// 	// Mouse.down[event.button] = false;
//     net_send_shoot_end();
// });

// interface KbdButtonMap {
//     [key: str]: bool;
// }

// const Keyboard = 
// {
// 	down: <KbdButtonMap>{},
// 	pressed: <KbdButtonMap>{}, // a button is pressed only for one frame
// };

// window.addEventListener("keydown", function(event){
	
// 	// console.log(event.key)
// 	console.log(event.code)



//     // NOTE: chat disabled
// 	// // do not move player while chatting
// 	// if (document.activeElement == chat_input) return;


// 	// NOTE: we can also use event.code 
// 	// for more precise info,
// 	// such as which AltLeft instead of Alt... but yea



// 	// prevent sending network key if key already down.
// 	// This prevents Windows from spamming key when held.
// 	if (Keyboard.down[event.code]) {
// 		return; // prevent OS spamming keys
// 	}

// 	// yeah... turns out they both suck in different ways. So we just use both
// 	// EDIT: I think this game only uses keycode
// 	Keyboard.down[event.code] = true;
// 	Keyboard.pressed[event.code] = true;

// 	net_send_key_down(event.code);

// 	if (event.key == "ArrowUp" || event.key == "ArrowDown") {
// 		event.preventDefault(); // people complained about scroll up down
// 		// DONE: perhaps a better fix would be to remove the scrollbar altogether
// 	}
// });

// window.addEventListener("keyup", function(event){
// 	// Keyboard.down[event.key] = false;
// 	Keyboard.down[event.code] = false;
// 	net_send_key_up(event.code);
// });



//////////////
// text rendering
// LATER: encapsulate so we can have multiple fonts?

// const font = document.querySelector('#number_font') as HTMLImageElement;
// console.log('font:')
// console.log(font)

// const canvas = document.querySelector('canvas');
// const context = canvas.getContext("2d");

const s32 FONT_BITMAP_WIDTH = 80;
const s32 FONT_BITMAP_HEIGHT = 48;
const s32 CHARS_PER_FONT_ROW = 16;
const s32 CHARS_PER_FONT_COL = 6;
const s32 CHAR_COUNT = CHARS_PER_FONT_ROW * CHARS_PER_FONT_COL;
const s32 CHAR_WIDTH = FONT_BITMAP_WIDTH / CHARS_PER_FONT_ROW;
const s32 CHAR_HEIGHT = FONT_BITMAP_HEIGHT / CHARS_PER_FONT_COL;

// Disabled because inappropriate for an API that takes a position
// const TARGET_START_X = CHAR_WIDTH; // 
// const TARGET_END_X = canvas.width - CHAR_WIDTH;
// const TARGET_START_Y = CHAR_HEIGHT;
// const MAX_LINE_WIDTH = TARGET_END_X - TARGET_START_X;
// const CHARS_PER_SCREEN_LINE = MAX_LINE_WIDTH / CHAR_WIDTH;

const s32 LINE_HEIGHT = CHAR_HEIGHT * 1.5;

s32 calcTextScale(s32 size) {
	return size/CHAR_HEIGHT;
	// should it be round instead?
}

bool fontReady = false;

// f32 char_widths : number[] = [];
// f32 char_startx : number[] = [];
// void initFont() {
// 	console.log('initFont');
// 	// detects char widths

// 	// // px
// 	// f32 x = 0;
// 	// f32 y = 0;

// 	// mashallah it will be erased
// 	context.drawImage(font, 0, 0);

// 	// iterate over each character
// 	for (s32 i = 0; i < CHAR_COUNT; i++) {
// 		const x_index = i % CHARS_PER_FONT_ROW;
// 		const y_index = std::floor(i / CHARS_PER_FONT_ROW);

// 		f32 x_start = -1;
// 		f32 x_end = -1;
// 		// iterate over each pixel in the character
// 		for (s32 x = x_index*CHAR_WIDTH; x<(x_index+1)*CHAR_WIDTH; x++) {
// 			for (s32 y = y_index*CHAR_HEIGHT; y<(y_index+1)*CHAR_HEIGHT; y++) {
// 				// const letter = String.fromCharCode(i + 32);
// 				// console.log(letter);
// 				const pixel = context.getImageData(x, y, 1, 1).data;
// 				if(pixel[3]! > 0) { // check alpha
// 					if (x_start == -1) {
// 						x_start = x;
// 					}
// 					x_end = x; // gets constantly updated until the last time we see an opaque pixel
// 				}
// 			}	
// 		}
// 		f32 width = x_end - x_start;
// 		width += 1; // 1px wide chars have same start and end index, which would give 0
// 		if(x_start == -1) {
// 			// empty character
// 			x_start = x_index * CHAR_WIDTH; // reset start_x
// 			width = CHAR_WIDTH / 2; // NOTE: adjust space char width here
// 		}
// 		char_startx[i] = x_start;
// 		char_widths[i] = width;
// 		// console.log(char_startx[i], char_widths[i]);
// 	}
// 	context.clearRect(0,0,canvas.width,canvas.height);

// 	fontReady = true;
// }

// void renderTextCaps(x: s32, y: s32, text: str, alpha?: f32, scale?: s32) {
// 	if (!fontReady) return
// 	text = text.toUpperCase();
// 	renderText(x,y, text, alpha, scale);
// }

// void renderTextInvert(x: s32, y: s32, text: str, alpha?: f32, scale?: s32)  {
// 	context.globalCompositeOperation = "difference" 
// 	// context.globalCompositeOperation = "xor" 
	
// 	renderText(x,y, text, alpha, scale);
// 	context.globalCompositeOperation = "source-over" // default
// }


// void renderTextBlack(x: s32, y: s32, text: str, alpha?: f32, scale?: s32)  {
// 	// context.globalCompositeOperation = "difference" 
// 	context.globalCompositeOperation = "xor" 
	
// 	renderText(x,y, text, alpha, scale);
// 	context.globalCompositeOperation = "source-over" // default
// }

void renderTextCam(s32 x, s32 y, std::string text, f32 alpha, s32 scale) {
	f32 camera_x = 0;
	f32 camera_y = 0;
	if (player_idx != -1) {
		camera_x = players[player_idx].x;
		camera_y = players[player_idx].y;	
	}
	f32 xc = x - camera_x + SCREEN_WIDTH/2;
	f32 yc = y - camera_y + SCREEN_HEIGHT/2;
	xc -= PLAYER_WIDTH/2;
	xc = std::round(xc);
	yc -= PLAYER_HEIGHT/2;
	yc = std::round(yc);
	renderText(xc,yc,text,alpha,scale);
}

void renderText(s32 x, s32 y, std::string text, f32 alpha, s32 scale) {
	// defaults:  f32 alpha = 1.0, s32 scale = 1
	// since you can only write them in the header for some reason

	/* 
		// todo fixme port
	
	
	if (!fontReady) return;

	// if (typeof alpha == 'undefined') {
	// 	alpha = 1.0;
	// }

	const auto alpha_saved = context.globalAlpha;
	context.globalAlpha = alpha;

	// if (typeof scale == 'undefined') {
	// 	scale = 1;
	// }

	const s32 TARGET_START_X = x;
	const s32 TARGET_START_Y = y;

	// LATER: add with param
    // NOTE: wat?
	const s32 TARGET_END_X = canvas.width - (CHAR_WIDTH*scale) * 2;

	// console.log('renderText');
	// console.log(text);

	bool word_wrap = false;
	bool char_wrap = false; // near bottom of function

	if (word_wrap) { // word-wrap
		f32 _text = text.split(''); // workaround for frigging javascript
		f32 cur_line_width = 0;
		f32 lastSpaceIndex = -1;
		for (s32 i = 0; i < text.length; i++) {
			if (text[i] == ' ')
				lastSpaceIndex = i;
			const ascii = text.charCodeAt(i);
			const sprite_index = ascii - 32;
			cur_line_width += char_widths[sprite_index]! * scale;
			if (cur_line_width >= TARGET_END_X) { // LATER: ? should this be >= or >
				cur_line_width = 0;
				_text[lastSpaceIndex] = '\n'; // workaround
			}
		}
		text = _text.join(''); // end workaround
	}

	f32 cur_line_width = 0;
	f32 target_x = TARGET_START_X;
	f32 target_y = TARGET_START_Y;
	for (s32 i = 0; i < text.length; i++) {
		const letter = text[i];
		// console.log(letter);
		if (letter == '\n') {
			// console.log('NEWLINE DETECTED. Reset target_x; increment target_y');
			target_x = TARGET_START_X;
			target_y += LINE_HEIGHT * scale;
			continue;
		}
		const ascii = text.charCodeAt(i);
		const sprite_index = ascii - 32;
		// console.log(letter, ascii, sprite_index);
		// note: this could be optimized, modulo is expensive
		// but we probably won't have much text
		// const x_index = sprite_index % CHARS_PER_FONT_ROW;
		const y_index = std::floor(sprite_index / CHARS_PER_FONT_ROW);
		// console.log(x_index, y_index)

		// const x_coord = x_index * CHAR_WIDTH;
		f32 y_coord = y_index * CHAR_HEIGHT;
		const width  = char_widths[sprite_index]!; // LATER: separate variable for source vs target width?
		const startx = char_startx[sprite_index]!;
		context.drawImage(font, startx, y_coord, width, CHAR_HEIGHT, target_x, target_y, width * scale, CHAR_HEIGHT * scale);
		target_x += width * scale;
		target_x += 1 * scale; // padding! // LATER configurable padding. e.g. want a 2px padding (at source pixel level)
		// target_y += 20; // debug
		cur_line_width += width * scale;

		// this should rarely run, since word-wrap above handles most cases.
		// this runs if there was a word with no spaces, longer than the screen
		if (char_wrap && target_x >= TARGET_END_X) {
			target_x = TARGET_START_X;
			target_y += LINE_HEIGHT * scale;
		}
	}

	context.globalAlpha = alpha_saved

	*/
}


//////////////



/////////////////////////////



// void Color(r: u8, g: u8, b: u8, a: u8) : Color {
// 	return { r, g, b, a	}
// }

// const BACKGROUND_COLOR = Color(32, 32, 32, 255); // gray
// const BACKGROUND_COLOR = Color(16, 24, 32, 255); // bluish gray

const Color BACKGROUND_COLOR = {8, 16, 24, 255}; // deep blue

// const BACKGROUND_COLOR = Color(12, 20, 28, 255); // weak compromise




const Color COLOR_ZERO = {0,0,0,0};
const Color COLOR_YELLOW = { 253, 249, 0, 255 };
const Color COLOR_DEAD =  { 255,0,0,128 };
const Color COLOR_FIXME = {255,0,255,255};

Color color_from_index(s32 idx) {
	// TODO FIXME PORT
	return COLOR_FIXME;
	// reset_color_rand(idx); // reset color RNG with index as seed
	// return rng_color_next();
}

// void StupidAlpha(a: s32) : str {
// 	return `${std::floor(a * 100 / 255)}%`
// }


void SetFillStyle(Color color)  {
	SDL_SetRenderDrawColor(gRenderer, color.r, color.g, color.b, color.a);
}

void ClearBackground(Color color)  {
	DrawRectArgs(0,0,SCREEN_WIDTH, SCREEN_HEIGHT,color);
}



/////////////////////



// NOTE: DISABLED: no collision needed on client
// // thank you kind sir https://www.jeffreythompson.org/collision-detection/rect-rect.php
// void rectRect(r1: Rect, r2: Rect) : bool {
// 	return (r1.x + r1.w >= r2.x &&    // r1 right edge past r2 left
// 	r1.x <= r2.x + r2.w &&    // r1 left edge past r2 right
// 	r1.y + r1.h >= r2.y &&    // r1 top edge past r2 bottom
// 	r1.y <= r2.y + r2.h);    // r1 bottom edge past r2 top
// }

void DrawRect(Rect r, Color c) {

	// SDL_FillRect(dst, &r, SDL_MapRGBA()) // NO! CURSED

	SDL_SetRenderDrawColor(gRenderer, c.r, c.g, c.b, c.a);
	SDL_Rect rect = {r.x, r.y, r.w, r.h};        
	SDL_RenderFillRect(gRenderer, &rect);
	// SDL_RenderPresent(gRenderer);
}

void DrawRectArgs(s32 x, s32 y, s32 w, s32 h, Color c) {

	DrawRect((Rect){x,y,w,h}, c);
}

void DrawRectArgsCam(s32 x, s32 y, s32 w, s32 h, Color color){



	// so I'm offsetting the camera by half
	// the player's size
	// so the player will render in the 
	// middle of the screen.
	// It seems like an ugly hack
	// but I don't see a better solution
	// rn. render players "around" their
	// coord instead of bottom and to the right?
	// but this would complicate player hitboxing...

	v2 c = GetCameraOffset(x,y);
	DrawRectArgs(c.x,c.y,w,h,color);
}

///////////////////////////

// TODO FIXME PORT take a vector? or rethink entirely
v2 GetCameraOffset(s32 x, s32 y) {
	f32 camera_x = 0;
	f32 camera_y = 0;
	if (player_idx != -1) {
		camera_x = players[player_idx].x;
		camera_y = players[player_idx].y;	
	}
	f32 xc = x - camera_x + SCREEN_WIDTH/2;
	f32 yc = y - camera_y + SCREEN_HEIGHT/2;
	xc -= PLAYER_WIDTH/2;
	xc = std::round(xc);
	yc -= PLAYER_HEIGHT/2;
	yc = std::round(yc);
	return {xc,yc};
}

////////////////////////


// void playSound(snd_id: s32)  {
// 	playSound(snd_id);
// }

void you_died() {
	// tts_you_died();
	// TODO FIXME PORT
}



// declare class SamJs {
// 	constructor(x:any)
// 	speak(s:str)
// }

// declare class Sfxr {
// 	toAudio(x:any):any;
// }

// declare var sfxr: Sfxr;

// TODO FIXME PORT SAM
// Sam sam = Sam({pitch:200});
// Sam samDeep = Sam({pitch:142, speed:132});

// void playSound(id) {
// 	console.log('id ' + id)
// 	const sfx = getSfxById(id); // defined in jsfxr_presets.js
// 	// @ts-ignore
// 	sfx.play();
// }

// void playSound(sfx) {
// 	// @ts-ignore
// 	sfx.play();
// }


///////////////////////////////////
// sorry
// LATER: move to separate module
// REFACTOR: move to separate module



// TODO PORT sfxr c version???

// interface sfx {
// 	PLAYER_DEATH: any,
// 	ENEMY_DEATH: any,
// 	EXPLOSION: any,
// 	EXPLOSION_SHORT: any,
// 	HURT: any,
// }

// const SFX = {
// 	PLAYER_DEATH: null,
// 	ENEMY_DEATH: null,
// 	EXPLOSION: null,
// 	EXPLOSION_SHORT: null,
// 	HURT: null,
// };

// const JSFXR_PLAYER_DEATH = {
//   "oldParams": true,
//   "wave_type": 3,
//   "p_env_attack": 0,
//   "p_env_sustain": 0.146,
//   "p_env_punch": 0.34,
//   "p_env_decay": 0.682,
//   "p_base_freq": 0.29191116671533823,
//   "p_freq_limit": 0,
//   "p_freq_ramp": -0.171,
//   "p_freq_dramp": 0,
//   "p_vib_strength": 0,
//   "p_vib_speed": 0,
//   "p_arp_mod": 0,
//   "p_arp_speed": 0,
//   "p_duty": 0,
//   "p_duty_ramp": 0,
//   "p_repeat_speed": 0,
//   "p_pha_offset": 0,
//   "p_pha_ramp": 0,
//   "p_lpf_freq": 1,
//   "p_lpf_ramp": 0,
//   "p_lpf_resonance": 0,
//   "p_hpf_freq": 0,
//   "p_hpf_ramp": 0,
//   "sound_vol": 0.25,
//   "sample_rate": 11025,
//   "sample_size": 8
// }

// SFX.PLAYER_DEATH = sfxr.toAudio(JSFXR_PLAYER_DEATH);
// // SFX.PLAYER_DEATH.play();

// const JSFXR_ENEMY_DEATH = {
//   "oldParams": true,
//   "wave_type": 3,
//   "p_env_attack": 0,
//   "p_env_sustain": 0.146,
//   "p_env_punch": 0.34,
//   "p_env_decay": 0.516,
//   "p_base_freq": 0.29191116671533823,
//   "p_freq_limit": 0,
//   "p_freq_ramp": -0.217,
//   "p_freq_dramp": 0,
//   "p_vib_strength": 0,
//   "p_vib_speed": 0,
//   "p_arp_mod": 0,
//   "p_arp_speed": 0,
//   "p_duty": 0,
//   "p_duty_ramp": 0,
//   "p_repeat_speed": 0,
//   "p_pha_offset": 0,
//   "p_pha_ramp": 0,
//   "p_lpf_freq": 1,
//   "p_lpf_ramp": 0,
//   "p_lpf_resonance": 0,
//   "p_hpf_freq": 0,
//   "p_hpf_ramp": 0,
//   "sound_vol": 0.25,
//   "sample_rate": 11025,
//   "sample_size": 8
// }

// SFX.ENEMY_DEATH = sfxr.toAudio(JSFXR_ENEMY_DEATH);

// const JSFXR_EXPLOSION = {
//   "oldParams": true,
//   "wave_type": 3,
//   "p_env_attack": 0,
//   "p_env_sustain": 0.13319494246236924,
//   "p_env_punch": 0.6648364348154416,
//   "p_env_decay": 0.12139456795379933,
//   "p_base_freq": 0.03665698130227344,
//   "p_freq_limit": 0,
//   "p_freq_ramp": 0.08502019811512262,
//   "p_freq_dramp": 0,
//   "p_vib_strength": 0.5274586566235788,
//   "p_vib_speed": 0.43123974849995844,
//   "p_arp_mod": 0,
//   "p_arp_speed": 0,
//   "p_duty": 0,
//   "p_duty_ramp": 0,
//   "p_repeat_speed": 0.6773385022526437,
//   "p_pha_offset": -0.2588846618005372,
//   "p_pha_ramp": -0.19861840639250428,
//   "p_lpf_freq": 1,
//   "p_lpf_ramp": 0,
//   "p_lpf_resonance": 0,
//   "p_hpf_freq": 0,
//   "p_hpf_ramp": 0,
//   "sound_vol": 0.25,
//   "sample_rate": 44100,
//   "sample_size": 8
// }


// SFX.EXPLOSION = sfxr.toAudio(JSFXR_EXPLOSION);

// const JSFXR_EXPLOSION_SHORT = {
//   "oldParams": true,
//   "wave_type": 3,
//   "p_env_attack": 0,
//   "p_env_sustain": 0.13319494246236924,
//   "p_env_punch": 0.285,
//   "p_env_decay": 0.365,
//   "p_base_freq": 0.03665698130227344,
//   "p_freq_limit": 0,
//   "p_freq_ramp": 0.08502019811512262,
//   "p_freq_dramp": 0,
//   "p_vib_strength": 0.5274586566235788,
//   "p_vib_speed": 0.43123974849995844,
//   "p_arp_mod": 0,
//   "p_arp_speed": 0,
//   "p_duty": 0,
//   "p_duty_ramp": 0,
//   "p_repeat_speed": 0.6773385022526437,
//   "p_pha_offset": -0.2588846618005372,
//   "p_pha_ramp": -0.19861840639250428,
//   "p_lpf_freq": 1,
//   "p_lpf_ramp": 0,
//   "p_lpf_resonance": 0,
//   "p_hpf_freq": 0,
//   "p_hpf_ramp": 0,
//   "sound_vol": 0.25,
//   "sample_rate": 44100,
//   "sample_size": 8
// }

// SFX.EXPLOSION_SHORT = sfxr.toAudio(JSFXR_EXPLOSION_SHORT);


// const JSFXR_HURT = {
//   "oldParams": true,
//   "wave_type": 1,
//   "p_env_attack": 0,
//   "p_env_sustain": 0,
//   "p_env_punch": 0,
//   "p_env_decay": 0.194,
//   "p_base_freq": 0.396,
//   "p_freq_limit": 0.201,
//   "p_freq_ramp": -0.296,
//   "p_freq_dramp": -0.079,
//   "p_vib_strength": 0,
//   "p_vib_speed": 0,
//   "p_arp_mod": 0,
//   "p_arp_speed": 0,
//   "p_duty": 1,
//   "p_duty_ramp": 0,
//   "p_repeat_speed": 0,
//   "p_pha_offset": 0,
//   "p_pha_ramp": 0,
//   "p_lpf_freq": 1,
//   "p_lpf_ramp": 0,
//   "p_lpf_resonance": 0,
//   "p_hpf_freq": 0.21582389981362443,
//   "p_hpf_ramp": 0,
//   "sound_vol": 0.362,
//   "sample_rate": 44100,
//   "sample_size": 8
// }

// SFX.HURT = sfxr.toAudio(JSFXR_HURT);





// void getSfxById(id) {
//   switch(id) {
//     case 0: return SFX.PLAYER_DEATH;
//     case 1: return SFX.ENEMY_DEATH;
//     case 2: return SFX.EXPLOSION;
//     case 3: return SFX.EXPLOSION_SHORT;
//     case 4: return SFX.HURT;
//   }
// }


////////////////





///////////////////////////////////

// NOTE: DISABLED chat for now

// void chat_log(msg: str)  {
//     return // CHAT DISABLED
// 	const log_el = document.querySelector('#chat_log') as HTMLTextAreaElement
// 	// prepend
// 	log_el.value = msg + '\n' + log_el.value
// }




// const chat_log = document.querySelector('#chat_log')
// const chat_input = document.querySelector('#chat_input') as HTMLInputElement;
// chat_input.addEventListener('keydown', function(e) {
// 	console.log(e)
// 	const chat_msg = chat_input.value;

// 	// LATER: add a submit key for phones
// 	if (e.key == 'Enter') {
// 		const chat_log = document.querySelector('#chat_log') as HTMLTextAreaElement;
// 		chat_input.value = '';
// 		chat_log.value += 'YOU: ' + chat_msg;
// 		chat_log.value += '\n';	
// 		sendMessage('CHAT ' + chat_msg)
// 		sam.speak(chat_msg)
// 	}
// });



///////////////////////////////








const s32 SFX_PLAYER_DEATH = 0;
const s32 SFX_ENEMY_DEATH = 1;
const s32 SFX_EXPLOSION = 2;
const s32 SFX_EXPLOSION_SHORT = 3;
const s32 SFX_HURT = 4;

void playSoundById(s32 snd_id) { // todo move?

		// TODO FIXME PORT

	// switch (snd_id) {
	// 	//@ts-ignore
	// 	case SFX_PLAYER_DEATH: return SFX.PLAYER_DEATH.play();
	// 	//@ts-ignore
	// 	case SFX_ENEMY_DEATH: return SFX.ENEMY_DEATH.play();
	// //@ts-ignore
	// 	case SFX_EXPLOSION: return SFX.EXPLOSION.play();
	// //@ts-ignore
	// 	case SFX_EXPLOSION_SHORT: return SFX.EXPLOSION_SHORT.play();
	// //@ts-ignore
	// 	case SFX_HURT: return SFX.HURT.play();

	// }
}


extern "C" {
void net_get_bullet(s32 idx, s32 x, s32 y, s32 dx, s32 dy){
	
	// spawn bullet
	bullets[idx] = { // todo does this generate garbage? defeats the purpose of obj pooling
		x, 
		y, 
		(f32)x, // xf 
		(f32)y, // yf
		dx,
		dy, 
		true, // alive
		BULLET_START_LIFE, // TODO eliminate
	};
	// TODO rename this sfx?
	// TODO CLIENTSIDE
	// 	TODO FIXME PORT SOUND
	// playSound(SFX.EXPLOSION_SHORT)
}
}

extern "C" {
void net_kill_bullet(s32 idx) {
	bullets[idx].alive = false;
}
}

extern "C" {
void net_player_died(s32 idx) {
	console.log("net player died: " + std::to_string(idx));
	players[idx].alive = false;
	// const std::string die_msg = "PLAYER " + std::to_string(idx) + " DIED";
	// (document.querySelector('#chat_log') as HTMLTextAreaElement).value += die_msg + '\n';
	// console.warn("sam.speak: " + die_msg);
	
	// TODO FIXME PORT
	// sam.speak(die_msg);
}
}

extern "C"{ 
void net_respawn_player(s32 plr_id) {
	players[plr_id] = default_player(true);
}
}

extern "C" {
void net_player_joined(s32 plr_id) {
	players[plr_id].exists = true;
}
}

extern "C" {
void net_delete_player(s32 plr_id)  {  // delete_player() called from js
	players[plr_id].exists = false;
}	
}

extern "C" {
void net_set_userid(s32 userid) {
	player_idx = userid;
}
}

extern "C" {
void net_set_player_state(
	s32 plr_id, 
	s32 x, 
	s32 y, 
	bool alive, 
	s32 life, 
	bool exists, 
	s32 deathTimer
) {
	players[plr_id].x = x;
	players[plr_id].y = y;
	players[plr_id].alive = alive;
	players[plr_id].life = life;
	players[plr_id].exists = exists;
	players[plr_id].deathTimer = deathTimer;	
}
}
	

extern "C" {
void net_set_bullet_state(
	s32 blt_idx,
    s32 x,
    s32 y,
    f32 xf,
    f32 yf,
    s32 dx,
    s32 dy,
    bool alive,
    s32 life
) {
	bullets[blt_idx].x = x;
	bullets[blt_idx].x = x;
	bullets[blt_idx].xf = xf;
	bullets[blt_idx].yf = yf;
	bullets[blt_idx].dx = dx;
	bullets[blt_idx].dy = dy;
	bullets[blt_idx].alive = alive;	
	bullets[blt_idx].life = life;	
}
}
	
			
void handleMessage(std::string msg) {
    
	// TODO FIXME PORT 

	// if (msg.startsWith('STATE')){
	// 	return handleStateMessage(msg);
	// }

	// console.log('RCV: ' + msg)

	// if (msg.startsWith('SND')) {
	// 	const snd_id = Number(msg.split(' ')[1]);
	// 	return playSoundById(snd_id);
	// }

	// if (msg.startsWith('BLT_X')) {
	// 	const idx = Number(msg.split(' ')[1])
	// 	net_kill_bullet(idx)
	// 	return;
	// }

	// if (msg.startsWith('BLT')) {
	// 	const args = msg.split(' ')
	// 	args.shift();
	// 	const idx = Number(args[0])
	// 	const x   = Number(args[1])
	// 	const y   = Number(args[2])
	// 	const dx  = Number(args[3])
	// 	const dy  = Number(args[4])
	// 	net_get_bullet(idx, x, y, dx, dy)
	// 	return;
	// }

	// if (msg.startsWith('KD')) {
	// 	const chunks = msg.split(' ');
	// 	const keycode = chunks[1]!;
	// 	const plr_id = Number(chunks[2]!); // NOTE: When server retransmits KU/KD, it appends origin player ID to message
	// 	net_recv_key_down(plr_id, keycode);
	// 	return;
	// }

	// if (msg.startsWith('KU')) {
	// 	const chunks = msg.split(' ');
	// 	const keycode = chunks[1]!;
	// 	const plr_id = Number(chunks[2]!);
	// 	const x = Number(chunks[3]!);
	// 	const y = Number(chunks[4]!);
	// 	net_recv_key_up(plr_id, keycode);
	// 	console.log(players[plr_id]!.x);
	// 	players[plr_id]!.x=x;
	// 	console.log(players[plr_id]!.x);
	// 	players[plr_id]!.y=y;
	// 	return;
	// }

	// if(msg.startsWith('SS')) {

	// }

	// if (msg.startsWith('DIED')) {
	// 	console.log(msg)
	// 	const chunks = msg.split(' ');
	// 	const plr_id = Number(chunks[1]!);
	// 	net_player_died(plr_id);
	// 	return;
	// }

	// if (msg.startsWith('TTS')) {
	// 	const chunks = msg.split(' ');
	// 	chunks.shift();
	// 	const txt = chunks.join(' ');
	// 	console.warn('sam.speak: ' + txt);
	// 	sam.speak(txt);
	// 	return;
	// }

	// if (msg.startsWith('TTS_DEEP')) {
	// 	const chunks = msg.split(' ');
	// 	chunks.shift();
	// 	const txt = chunks.join(' ');
	// 	samDeep.speak(txt);
	// 	return;
	// }

	// if (msg.startsWith('JOIN')) {
	// 	const id = Number(msg.split(' ')[1]);
	// 	players[id]!.exists = true;
	// 	return;
	// }

	// if (msg.startsWith("RSPN")) {
	// 	const userid = Number(msg.split(' ')[1]);
	// 	respawn_player(userid);
	// 	return;
	// }
	
	// if (msg.startsWith("LEAVE")) {
	// 	const userid = Number(msg.split(' ')[1]);
	// 	net_delete_player(userid);
	// 	const leave_msg  = `Player ${userid} quit`;
	// 	sam.speak(leave_msg);
	// 	chat_log(leave_msg)
	// 	return;
	// }

	// if (msg.startsWith("USERID")){
	// 	const user_id = Number(msg.split(' ')[1]);
	// 	player_idx = user_id;
	// }





}

void handleStateMessage(std::string msg) {

	// TODO FIXME PORT 

	// console.log('RCV: {STATE}');
	// const json = msg.split(' ')[1]!;
	// const state = JSON.parse(json);
	// // console.log(state);
	// // console.log(json);
	// players = state.players;
	// bullets = state.bullets;
}

// void handleMessage(msg: str) {

// 	if(msg.startsWith('MSG')) {
// 		return handleUserMessage(msg);
// 	}

// 	// server messages
// 	const chunks = msg.split(' ');
// 	if (chunks.length > 1) {
// 		const cmd = chunks[0];
// 		switch (cmd) {
			
// 			// NOTE: moved to handleUserMessage
// 			// case "POS":
// 			// 	_set_player_pos(Number(chunks[1]), Number(chunks[2]));
// 			// 	// NOTE: emscripten apparently exports WASM functions, prefixed with underscore 
// 			// break;

// 			case "USERID":
// 				set_player_id(Number(chunks[1]))
// 			break;

// 			case "JOIN":
// 				spawn_player(Number(chunks[1])); // no data but gets synced immediately
// 				const join_msg = `Player ${chunks[1]} joined`;
// 				sam.speak(join_msg);
// 				chat_log(join_msg)

// 				// LATER: maybe keep new player disabled until first sync ?
// 			break;

// 			case "LEAVE":
// 				delete_player(Number(chunks[1]));
// 				const leave_msg  = `Player ${chunks[1]} quit`;
// 				sam.speak(leave_msg);
// 				chat_log(leave_msg)

// 			break;
// 		}
// 	}
// }

// void handleUserMessage(msg: str) {
// 	const args = msg.split(' ');
// 	if (args.length > 1) {
// 		// @ts-ignore (unused)
// 		const server_cmd = args.shift(); // MSG
// 		const userid = args.shift();
// 		const cmd = args.shift();
// 		// note: remaining args (if any) now begin at 0
// 		switch (cmd) {
// 			case "POS":
// 				set_player_pos(Number(userid), Number(args[0]), Number(args[1]));
// 			break;
// 			case "CHAT":
// 				const chat_msg = args.join(' ');
// 				const chat_msg_line = `PLAYER ${userid}: ${chat_msg}`
// 				chat_log(chat_msg_line)
// 				sam.speak(chat_msg)
// 			break;
// 			case "BLT":
// 				{
// 					const idx = Number(args[0])
// 					const x = Number(args[1])
// 					const y = Number(args[2])
// 					const dx = Number(args[3])
// 					const dy = Number(args[4])
// 					net_get_bullet(idx, x, y, dx, dy)
// 				}
// 			break;
// 			case "BLT_X":
// 				const idx = Number(args[0])
// 				net_kill_bullet(idx)
// 			break
// 			case "DIED":
// 				const die_msg = `PLAYER ${userid} DIED`;
// 				(document.querySelector('#chat_log') as HTMLTextAreaElement).value += die_msg + '\n';
// 				sam.speak(die_msg)
// 				net_player_died(Number(userid))
// 				playSound(1) // SFX.ENEMY_DEATH
// 			break;	

// 			case "RSPN":
// 				spawn_player(Number(userid)); // no data but gets synced immediately by next POS msg
// 			break;
// 		}
// 	}
// }

void tts_you_died() {
	console.log("tts you died");

	// TODO FIXME PORT
	// window.setTimeout(
	// 	function(){
	// 		samDeep.speak('You died')
	// 	},
	// 	500 // delay so death sfx ends
	// )
}

////////////////////////////



///////////

// void init() {

	// TODO FIXME PORT

	// // SetFillStyle(BACKGROUND_COLOR)
	// ClearBackground(BACKGROUND_COLOR);
	// resizeCanvas();
	// initFont();
	
	// // reset_player() // moved to server

	// canvas.style.display = 'block'
	
	// ws_init()

	// requestAnimationFrame(tick)
	// // setInterval(tick_update, 16.6666 * 2);

// }

// TODO FIXME PORT: better type for time?
void tick(s32 time)  {
	update(time);
	// todo cleanup later
	// console.log(player_idx);
	// console.log(players[player_idx]);
	draw(time);

	// requestAnimationFrame(tick);
}


// void tick_update() {
// 	const time = Number(new Date());
// 	update(time);
// }

///////////



// void random_int(lo : s32, hi: s32) : s32 {
// 	const range = hi-lo;
// 	return std::floor(lo + Math.random() * range)
// }



// void random_u8(lo : s32, hi: s32) : u8 {
// 	return u8(random_int( lo, hi ))
// }

// void random_color() : Color {
// 	return Color (
// 		random_u8( 64, 192 ),
// 		random_u8( 64, 192 ),
// 		random_u8( 64, 192 ),
// 		255
// 	)
// }





s32 rng_color_random_int(s32 lo, s32 hi) {
	return rand_range(lo, hi);
}

// todo rename?
u8 rng_color_random_u8(s32 lo, s32 hi) {
	return u8(rng_color_random_int( lo, hi ));
}

Color rng_color_next() {
	return {
		rng_color_random_u8( 64, 192 ),
		rng_color_random_u8( 64, 192 ),
		rng_color_random_u8( 64, 192 ),
		255
	};
}


//////////////////



s32 _time_prev = 0;
s32 _frame_time = 0;


void update(s32 time)  {
	// std::cout << "UPDATE " << time << std::endl;
	const s32 dt = time - _time_prev;
	// console.log(dt);
	// if ( dt < 16 ) {
	// 	alert('fug')
	// 	return;
	// }  
    
    // reset inputs
	// Mouse.moved = false;
	// Mouse.pressed = {};
	// Keyboard.pressed = {}; // clear pressed keys at the end of each frame

	rand_const_reset();

	// players[0].exists = true;
	// players[0].x++;
	// players[0].y++;
	

	update_players(dt);
	update_bullets(dt);

	// // frame time
	_time_prev = time;
	// // _frame_time_prev = _frame_time;
}

void update_players(s32 dt) {
	// for(f32 player of this.players) {
	for(f32 id = 0; id < MAX_PLAYERS; id++) {
		update_player(id, dt); // TODO
	}
}


// TODO PORT FIXME not strings pls
// bool player_is_key_down(s32 id, std::string key)  {
// 	return players[id].keyboard[key] == true;
// }

void respawn_player(s32 id)  {
	console.log("respawn player" + std::to_string(id));
	players[id] = default_player(true);
}

void update_player(s32 id, s32 dt)  {

		Player player = players[id];
		// if (!player) throw "err"; // All IDs should return a valid player.
		// When a player leaves, we just set player.exists = false

		if (!player.exists) return;

		if(! player.alive ) {
			if(player.deathTimer <= 0 ) {
				player.deathTimer = 0;
				// this.respawn(id); // TODO ?
			} else {
				player.deathTimer -= _frame_time;
				return;
			}
		}


		const f32 f = dt / 16.6666; // speed factor releative to 60HZ


		// TODO FIXME PORT

		// std::cout << player.x << std::endl;
		if(players[id].wasd.w ) {
			players[id].y -= s32(PLAYER_MOVE_SPEED * f);
		}
		if(players[id].wasd.s) {
			players[id].y += s32(PLAYER_MOVE_SPEED * f);
		}
		if(players[id].wasd.a) {
			players[id].x -= s32(PLAYER_MOVE_SPEED * f);
		}
		if(players[id].wasd.d ) {
			players[id].x += s32(PLAYER_MOVE_SPEED * f);
		}
		// std::cout << " move speed " << s32(PLAYER_MOVE_SPEED * f) << std::endl;
		// std::cout << player.x << std::endl;

		// player.x = int(player.x);
		// player.y = int(player.y);
		// TODO: does this make movement less accurate?
		// it introduces error based on frame rate...
		// we should SIMULATE in float,
		// but DRAW in int, (we already do)
		// and SYNC in int. 



		// TODO: Since our movement is simple and linear,
		// Wouldn't it make more sense to have a dx,dy?
		// Then when key is pressed/unpressed, we simply adjust dx,dy?


		// if (player.x < 0) player.x = 0;
		// if (player.y < 0) player.y = 0;
		// if (player.x > WORLD_WIDTH  - PLAYER_WIDTH)  player.x = WORLD_WIDTH  - PLAYER_WIDTH;
		// if (player.y > WORLD_HEIGHT - PLAYER_HEIGHT) player.y = WORLD_HEIGHT - PLAYER_HEIGHT;
		
		

		// // if(Mouse.down[MOUSE_LEFT] ) {
		// if (player_is_mouse_down(id, MOUSE_LEFT)) {
		// 	if(this.canShoot(id)) {
		// 		this.shoot(id);
		// 	}
		// }

		// // collide bullets

		// // for k in 0..<MAX_BULLETS {
		// for ( f32 k = 0; k < MAX_BULLETS; k++) {
		// 	const Bullet = this.bullets[k]!;
		// 	if(! bullet.alive) { continue }

		// 	const r1 = Rect(
		// 		f32(player.x),
		// 		f32(player.y),
		// 		f32(PLAYER_WIDTH),
		// 		f32(PLAYER_HEIGHT)
		// 	)
		// 	const r2 = Rect(
		// 		f32(bullet.x),
		// 		f32(bullet.y),
		// 		f32(BULLET_WIDTH),
		// 		f32(BULLET_HEIGHT)
		// 	)

		// 	if(rectRect(r1, r2)) {
		// 		bullet.alive = false
		// 		this.net_send_bullet_died(s32(k))
		// 		player.life -= BULLET_DAMAGE
				
		// 		// TODO CLIENTSIDE
		// 		this.playSound(SFX_HURT)
		// 		if(player.life <= 0) {
		// 			player.alive = false
		// 			// TODO respawn command?
		// 			this.die(id);
		// 		}
		// 	}
		// }

	}


void update_bullets(s32 dt)  {
	// for i in 0..<MAX_BULLETS {
	for (s32 i = 0; i < MAX_BULLETS; i++) {
		Bullet b = bullets[i];
		if(! b.alive) { 
			continue;
		}
		if(b.life <= 0) { 
			b.alive = false; 
			continue;
			// no need for net send die, since it'll die everywhere when time runs out
			// note: lag makes this terrible, but it's a jam so.. lol
		}

		const f32 f = dt / 16.6666; // speed factor releative to 60HZ

		b.xf += b.dx * f;
		b.yf += b.dy * f;
		b.x = s32(b.xf);
		b.y = s32(b.yf);
		b.xf = places(b.xf, NET_FLOAT_PLACES); // for bandwidth
		b.yf = places(b.yf, NET_FLOAT_PLACES);
		b.life-=1;

		// if(b.x < (-BULLET_WIDTH) || b.y < (-BULLET_HEIGHT) || b.x > SCREEN_WIDTH || b.y > SCREEN_HEIGHT) {
		// 	b.alive = false
		// }
	}
}


void draw(s32 _time)  {
    
	// TODO FIXME PORT
//   reset_constant_rand(); // for bg
    

	SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);

	ClearBackground(BACKGROUND_COLOR);
    

	draw_background();


	// draw player
	draw_players();
	// draw_player(player, s32(player_idx))
	draw_bullets();

	if(JOY_VISIBLE) {
		// TODO FIXME PORT
		// joy_move.draw();
		// joy_shoot.draw();
		// note: there's no convenient way to 
		// enable/disable joy events
		// but mouse does not generate them
		// so that shouldn't be a problem
	}
	

}


void draw_background()  {
	// just random noise so you can see camera move
	// LATER: if random is expensive (shouldn't be?) we can cache it
	// const [ox, oy] = GetCameraOffset(0,0);
	v2 o = GetCameraOffset(0,0);
	for(s32 i = 0; i < 200; i++) {
		const s32 c = 255;
		const s32 v = rand_range_const(32, 64) + rand_range(0, 16);
		
		// this was originally a bug (should have been constant), but the twinkling is beautiful
		// ....happy little accidents
		// it also kind of simulates old flickery arcade games
		const Color color = {u8(c),u8(c),u8(c),u8(v)};
		// const Color color = {u8(c),u8(c),u8(c),0};
		// const Color color = {255,0,255,0};
		
		
		const s32 x = rand_range_const(0, WORLD_WIDTH) + o.x;
		const s32 y = rand_range_const(0, WORLD_HEIGHT) + o.y;
		// const sz = rand_range_const(4,8)
		const s32 sz = rand_range_const(2,4);
		
		DrawRectArgs(x,y,sz,sz, color);
	}
}

void draw_players()  {
	// for i in 0..<MAX_PLAYERS {
	for (s32 i = 0; i < MAX_PLAYERS; i++) {
		
		if(! players[i].exists) { continue; }
		// fmt.println(key, value)
		draw_player(players[i], s32(i));
	}
}

void draw_player(Player p, s32 id) {
	// console.log(p);
	Color color = COLOR_DEAD;
	if (p.alive) {
		// color = rng_color_next(); // NOTE: if you want a player to change colors every frame, you can do this
		// it looks pretty cool
		color = color_from_index(id);
	}
	// std::cout << p.x << " " << p.y << " "
	// << (int)color.r << " " << (int)color.g << " " << (int)color.b << " " 
	// << (int)color.a << std::endl;
	DrawRectArgsCam(p.x, p.y, PLAYER_WIDTH, PLAYER_HEIGHT, color);

	std::string text;
	if ( id == -1) {
		text = "loading..."; 
	} else {
		// TODO FIXME PORT
		// text = String(id).padStart(2, '0');
	}

    s32 textSize = 16; 
    f32 textScale = calcTextScale(textSize);

    const s32 x = ( text.length() == 2 ) 
        ? p.x + 2 
        : p.x - CHAR_WIDTH/2*textScale + 2; // 3 digits centered

    const f32 y =  p.y - PLAYER_HEIGHT + 4;
    
    // renderText(x, y, text, 0.7, textScale);
	renderTextCam(x, y, text, 0.7, textScale);
		

}




void draw_bullets()  {
	for (s32 i = 0; i < MAX_BULLETS; i++) {
	// for i in 0..<MAX_BULLETS {
		if(! bullets[i].alive) { continue; }
		draw_bullet(bullets[i]);
	}
}

void draw_bullet(Bullet b)  {
	f32 bullet_size = std::round(b.life / 6);
	// DrawRectArgsCam(b.x, b.y, BULLET_WIDTH, BULLET_HEIGHT, COLOR_YELLOW)
	DrawRectArgsCam(b.x, b.y, bullet_size, bullet_size, COLOR_YELLOW);

}


/////////////





// TODO FIXME not efficient to set up the distribution every time
s32 rand_range(s32 min, s32 max) {
	std::random_device rd;  
	std::mt19937 rng(rd()); // Mersenne Twister RNG
	std::uniform_int_distribution<int> dist(min, max);
	return dist(rng);
}

const s32 FIXED_SEED = 1337;
std::mt19937 rng_rc(FIXED_SEED); // Mersenne Twister RNG

s32 rand_range_const(s32 min, s32 max) {
	std::uniform_int_distribution<int> dist(min, max);
	return dist(rng_rc);
}

void rand_const_reset() {
	rng_rc.seed(FIXED_SEED);
}



////////////



void resizeCanvas() {
	// return
	console.log("resize");
	// const target_width = canvas.clientWidth;
	// const target_height = canvas.clientHeight;

	// f32 target_width = window.innerWidth;
	// f32 target_height = window.innerHeight;

	// TODO FIXME PORT
	f32 target_width = SCREEN_WIDTH; // TODO FIXME DELETE ME DUMMY
	f32 target_height = SCREEN_HEIGHT; // TODO FIXME DELETE ME DUMMY
	

	if(target_width < target_height) {
		SCALE = 1;
		JOY_VISIBLE = true;
	} else {
		SCALE = 2;
		JOY_VISIBLE = false;
	}
	

	SCREEN_WIDTH = std::floor(target_width / SCALE);
	SCREEN_HEIGHT = std::floor(target_height / SCALE);

	SCREEN_WIDTH_HALF = std::floor(SCREEN_WIDTH/2);
	SCREEN_HEIGHT_HALF = std::floor(SCREEN_HEIGHT/2);
	

	// WORLD_WIDTH = SCREEN_WIDTH; // temporary
	// WORLD_HEIGHT = SCREEN_HEIGHT; 
	
	
	// adjust target; force the render / display sizes to be an integer ratio
	target_width = SCREEN_WIDTH * SCALE;
	target_height = SCREEN_HEIGHT * SCALE;
	

	// TODO FIXME PORT

	// canvas.width = SCREEN_WIDTH
	// canvas.height = SCREEN_HEIGHT
	// canvas.style.width = `${target_width}px`;
	// canvas.style.height =` ${target_height}px`;
	// context.imageSmoothingEnabled = false; // not sure why i need to do this every time, but I do...
}

extern "C" {

void tmp_delme(char* bleh) {
	std::cout << "$$$" << bleh << std::endl;
}

}

// window.addEventListener("resize", resizeCanvas);

// window.onload = init