type bool = boolean;
type str = string;
type num = number;
type i32 = number;
type f32 = number;
type u8 = number;



////////////////////


let PATHETIC_SNOWFLAKE_INITIALIZED = false; // I hate Emscripten

//////////////////
// WebSockets





const host = window.location.host; // e.g. "localhost:3000"
const path = window.location.pathname; // e.g. "/12345"
const url_ws = "ws://" + host + path; // note: we use the path for the room
const url_wss = "wss://" + host + path;

let ws : WebSocket;
let ws_ws = ws_connect(url_ws);
let ws_wss = ws_connect(url_wss);

const PRINT_MSG_IN = false;
const PRINT_MSG_OUT = false;


function ws_dummy() : WebSocket {
	return new WebSocket("wss://asdf");
	// note: this will fail, we just need A websocket object
	// to placate TS's type system... I'm so sorry,
	// this was better than making everything WebSocket | undefined.
	// Also, it'll just default to closed, which the system handles properly.
	// (And in case WS connection is impossible, it's replaced with this dummy,
	// so it doesn't spam reconnect either)
}

function ws_is_encrypted(_ws: WebSocket): bool {
	return _ws.url.includes("wss://");
}

function ws_is_connecting(_ws: WebSocket) : bool {
	return _ws.readyState == 0;
}

function ws_is_connected(_ws: WebSocket) : bool {
	return _ws.readyState == 1;
}

function ws_is_closing(_ws: WebSocket) : bool {
	return _ws.readyState == 2;
}

function ws_is_closed(_ws: WebSocket) : bool {
	return _ws.readyState == 3;
}

function ws_reconnect(_ws: WebSocket) : void {
	// ugh ... we just gotta rebuild the whole thing
	if(ws_is_encrypted(_ws)){
		ws_wss = ws_connect(url_wss);
	} else {
		ws_ws = ws_connect(url_ws);
		// TODO FIXME: this function grosses me out
		// ... a global function with special cases for other globals
		// throwing away the entire socket also seems silly
		// but maybe that's just how it's done
	}
}

function ws_setup_events(_ws: WebSocket) {

	// init events
	_ws.onopen = function () {
		console.log("WebSocket connection opened");
		// rule: if I am ws and wss is connected, do nothing.
		// rule: if I am ws and wss is not connected, use me as main connection.
		//          AND (if wss not connecting) init wss connection.
		if ( ! ws_is_encrypted(_ws) ) {
			if(ws_is_connected(ws_wss)) {
				return; // already have encryption, don't use unencrypted ws
			} else {
				ws = _ws; // no encryption (yet), use unencrypted ws
				if (! ws_is_connecting(ws_wss)) {
					// should connect encrypted ws
					ws_connect(url_wss);
				}
			}
		} else {
			// I (_ws) am encrypted
			ws = _ws; // set myself as main ws
		}
	};

	_ws.onmessage = function (event) {
		if (PRINT_MSG_IN) {
			if (!event.data.includes('POS')) { // antispam
				console.log("Received message:", event.data);
			}
		}
		handleMessage(event.data);
	};

	_ws.onclose = function (event) {
		console.warn(`WebSocket closed with code ${event.code}. Reconnecting in 2 seconds...`);
		setTimeout(()=>{ws_reconnect(_ws)}, 2000);
	};

	_ws.onerror = function (error) {
		console.error("WebSocket error:", error);
		// Handle errors as needed
	};
}

function ws_connect(url: string) : WebSocket {

	console.log('websocket connecting to ' + url);
	// connect
	try {
		let _ws = new WebSocket(url);

		ws_setup_events(_ws);
		return _ws;
	} catch {
		return ws_dummy();
	} // god I love JavaScript
}

function sendMessage(msg: str) {
	if (!ws) return;
	if ( ! (ws.readyState == 1) ) return;
	if (!msg.startsWith('MM')){
		console.log('OUT: ' + msg);
	}
	if (PRINT_MSG_OUT) {
		if ( ! msg.startsWith('POS')) { // prevent spam
			console.log('sent message:')
			console.log(msg)
			console.log('');
		}
	}
	// msg = JSON.stringify(message)
	ws.send(msg);
}


//////////////////
// Net Send


// function net_send_shoot_start(x: num, y: num) {
//	if(player_idx == -1) return;
// 	sendMessage(`SS ${x} ${y}`);
// }

// function net_send_shoot_end() {
// 	if(player_idx == -1) return;
// 	sendMessage(`SE`);
// }


// function net_send_shoot_aim(x: num, y: num) {
// 	const PLACES = 100;
// 	x = Math.round(x*PLACES) / PLACES;
// 	y = Math.round(y*PLACES) / PLACES;
	
// 	if(player_idx == -1) return;
// 	sendMessage(`SA ${x} ${y}`);
// }


// function net_send_key_down(key: str) {
// 	// TODO filter for movement keys!
//     sendMessage(`KD ${key}`);
// }


// function net_send_key_up(key: str) {
// 	// TODO filter for movement keys!
//     sendMessage(`KU ${key}`);
// }




///////
// Net Receive

function util_char(ch: str){
    if(ch.length != 0) throw "ch must have length 1";
    return ch.charCodeAt(0);
}

function net_recv_key_down(plr_id: i32, key_code: str) {
    let ch = null;
    if(key_code == "KeyW") ch = 'w';
    if(key_code == "KeyA") ch = 'a';
    if(key_code == "KeyS") ch = 's';
    if(key_code == "KeyD") ch = 'd';
    if(!ch) return;
    ch = util_char(ch);
    // @ts-ignore
    _net_recv_key_down(plr_id, ch);
}

function net_recv_key_up(plr_id: i32, key_code: str) {
    let ch = null;
    if(key_code == "KeyW") ch = 'w';
    if(key_code == "KeyA") ch = 'a';
    if(key_code == "KeyS") ch = 's';
    if(key_code == "KeyD") ch = 'd';
    if(!ch) return;
    ch = util_char(ch);
    // @ts-ignore
    _net_recv_key_up(plr_id, ch);
}


/////////////////

///////////////////////////////

/////////////////


// function net_get_bullet(idx: i32, x: i32, y:i32, dx: i32, dy: i32) : void {
	
// 	// spawn bullet
// 	bullets[idx] = { // todo does this generate garbage? defeats the purpose of obj pooling
// 		x, 
// 		y, 
// 		xf: f32(x), 
// 		yf: f32(y), 
// 		dx, 
// 		dy, 
// 		alive: true, 
// 		life: BULLET_START_LIFE, // TODO eliminate
// 	}
// 	// TODO rename this sfx?
// 	// TODO CLIENTSIDE
// 	playSound(SFX.EXPLOSION_SHORT)
// }


// function net_kill_bullet(idx: i32) : void {
// 	bullets[idx]!.alive = false
// }


// function net_player_died(idx: i32) : void {
// 	console.log('net player died: ' + idx)
// 	players[idx]!.alive = false
// 	const die_msg = `PLAYER ${idx} DIED`;
// 	// (document.querySelector('#chat_log') as HTMLTextAreaElement).value += die_msg + '\n';
// 	console.warn('sam.speak: ' + die_msg);
// 	sam.speak(die_msg)
// }

// function net_delete_player(plr_id: i32) : void {  // delete_player() called from js
// 	players[plr_id]!.exists = false
// }



function pathetic_snowflake_done() {
    PATHETIC_SNOWFLAKE_INITIALIZED = true;
    process_msgs_if_needed();
}

let msgs : str[] = [];

function queue_msg(msg: str) {
    msgs.push(msg);
}

function process_msgs_if_needed() {
    while (msgs.length > 0){
        // @ts-ignore;
        let msg = msgs.shift(); 
        // @ts-ignore;
        handleMessage(msg);
    }
}


function handleMessage(msg: str) {

    if(!PATHETIC_SNOWFLAKE_INITIALIZED) {
        queue_msg(msg);
        return;
    } else {
        process_msgs_if_needed();
    }
    
	console.log('RCV: ' + msg)

	if (msg.startsWith('STATE')){
		return handleStateMessage(msg);
	}

	// if (msg.startsWith('SND')) {
	// 	const snd_id = Number(msg.split(' ')[1]);
	// 	return playSoundById(snd_id);
	// }

	if (msg.startsWith('BLT_X')) {
		const idx = Number(msg.split(' ')[1])
        // @ts-ignore
		_net_kill_bullet(idx)
		return;
	}

	if (msg.startsWith('BLT')) {
		const args = msg.split(' ')
		args.shift();
		const idx = Number(args[0])
		const x   = Number(args[1])
		const y   = Number(args[2])
		const dx  = Number(args[3])
		const dy  = Number(args[4])
		// @ts-ignore
		_net_get_bullet(idx, x, y, dx, dy)
		return;
	}

	if (msg.startsWith('KD')) {
		const chunks = msg.split(' ');
		const keycode = chunks[1]!;
		const plr_id = Number(chunks[2]!); // NOTE: When server retransmits KU/KD, it appends origin player ID to message
		// @ts-ignore
		_net_recv_key_down(plr_id, keycode);
		return;
	}

	if (msg.startsWith('KU')) {
		const chunks = msg.split(' ');
		const keycode = chunks[1]!;
		const plr_id = Number(chunks[2]!);
		const x = Number(chunks[3]!);
		const y = Number(chunks[4]!);
		// @ts-ignore
		_net_recv_key_up(plr_id, keycode, x, y);
        // note: logic below moved to C++
		// console.log(players[plr_id]!.x);
		// players[plr_id]!.x=x;
		// console.log(players[plr_id]!.x);
		// players[plr_id]!.y=y;
		return;
	}

	if (msg.startsWith('DIED')) {
		console.log(msg)
		const chunks = msg.split(' ');
		const plr_id = Number(chunks[1]!);
		// @ts-ignore
		_net_player_died(plr_id);
		return;
	}

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

	if (msg.startsWith('JOIN')) {
		const id = Number(msg.split(' ')[1]);
		// players[id]!.exists = true;
        // @ts-ignore
		_net_player_joined(id);
		return;
	}

	if (msg.startsWith("RSPN")) {
		const userid = Number(msg.split(' ')[1]);
		// @ts-ignore
        _net_respawn_player(userid);
		return;
	}
	
	if (msg.startsWith("LEAVE")) {
		const userid = Number(msg.split(' ')[1]);
		// @ts-ignore
		_net_delete_player(userid);
		// const leave_msg  = `Player ${userid} quit`;
		// sam.speak(leave_msg);
		// chat_log(leave_msg)
		return;
	}

	if (msg.startsWith("USERID")){
		const user_id = Number(msg.split(' ')[1]);
		// player_idx = user_id;
        // @ts-ignore
        _net_set_userid(user_id);
        return;
	}

    console.warn('WARNING: UNHANDLED MESSAGE');
    console.log(msg);

}

function handleStateMessage(msg: str) {
	console.log('RCV: {STATE}');
	const json = msg.split(' ')[1]!;
	const state = JSON.parse(json);
	// console.log(state);
	// console.log(json);
	// players = state.players;
	// bullets = state.bullets;

    let plr_idx = 0;
    for(let p of state.players) {
        // @ts-ignore
        _net_set_player_state(plr_idx, p.x, p.y, p.alive, p.life, p.exists, p.deathTimer);
        plr_idx++;
    }

    let blt_idx = 0;
    for(let b of state.bullets) {
        // @ts-ignore
        _net_set_bullet_state(blt_idx, b.x, b.y, b.xf, b.yf, b.dx, b.dy, b.alive, b.life);
        blt_idx++;
    }
}
