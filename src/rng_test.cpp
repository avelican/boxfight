
#include "main.hpp"
#include "console.hpp"

const u32 SEED = 1337 ^ 0xDEADBEEF; // 32-bit seed with optional XOR value



SFC32::SFC32(u32 a, u32 b, u32 c, u32 d)
    : a(a), b(b), c(c), d(d) {}  // Constructor definition

float SFC32::next() {
	u32 t = (static_cast<u32>(a + b) + d);
	d = d + 1;
	a = b ^ (b >> 9);
	b = c + (c << 3);
	c = (c << 21) | (c >> 11);
	c = c + t;
	return static_cast<float>(t) / 4294967296.0f;
}


SFC32 make_rand(u32 seed) {
	// note: optional arg, defaults to SEED
    return {0x9E3779B9, 0x243F6A88, 0xB7E15162, seed};
}

// todo why does this exist lol
SFC32 make_randf(u32 seed) {
	// note: optional arg, defaults to SEED
    return {0x9E3779B9, 0x243F6A88, 0xB7E15162, seed};
}


//////////




SFC32 rand_const = make_rand();

f32 rand_range_const (s32 min, s32 max) {
	s32 range = max-min+1;
	f32 r = rand_const.next();
	return s32(abs(r)) % range + min;
}

int main() {
	
    for(int i = 0; i < 100; i++){
        f32 r = rand_const.next();
        std::cout << r << std::endl;
    }

	
	
}