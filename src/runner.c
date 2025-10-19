/* Runner
 * Copyright (c) 2025 Kyle MacFarland
 * Code released under an MIT license.
 */

/* Random Notes:
 * Always use ++var and --var. var++ will take up additional assembly instructions for no reason.
 * The 6502 is an 8-bit processor. Only use unsigned char.
 * Do use functions to reuse code. Don't use functions to organize code.
 * Use bitshift operations to multiply and divide by powers of two. (var << n) to multiply (var * 2^n) and (var >> n) to divide (var / 2^n).
 */

#include "lib/neslib.h"
#include "runner.h"

// Nametable A Address Macro
// Converts a tile position at (x,y) to an address usable by Neslib's functions.
#define NTADDR(x,y) (NAMETABLE_A | (((y) << 5)|(x)))

#define RIGHT_EDGE 0xf7
#define PLAYER_X 32
#define PLAYER_RIGHT 48
#define GROUND_Y 176
#define MAX_HEIGHT 30

// Define all variable as global on the Zero Page to make them faster to access.
#pragma bss-name(push, "ZEROPAGE")

// This must be defined here or lib/display.sinc won't work.
int oam_off;

unsigned char sprid = 0x00;
unsigned char i = 0x00;
unsigned char ticks = 0x00;
unsigned char controller = 0x00;
unsigned char player_jumping = 0;
unsigned char player_y = GROUND_Y;
unsigned char jump_vel = 0;
unsigned char score_total = 0;
unsigned char score_hundreds = 0;
unsigned char score_tens = 0;
unsigned char score_ones = 0;
unsigned char obstacle_x = RIGHT_EDGE;
unsigned char prev_obstacle_x = RIGHT_EDGE;
unsigned char wait_time = 0xae;

void write_text(const unsigned char* str, unsigned char x, unsigned char y) {
	vram_adr(NTADDR(y,x));
	i = 0;
	while (str[i]) {
		vram_put(str[i] + 16);
		++i;
	}
}

void write_char(unsigned char c, unsigned char x, unsigned char y) {
	vram_adr(NTADDR(y,x));
	vram_put(c + 16);
}

void main() {
	// Setup Code
	ppu_off();
	pal_bg(bg_palette);
	pal_spr(sprite_palette);
	write_text("SCORE: 000", 2, 2);
	ppu_on_all();

	// Main Loop
	while (1) {
		ppu_wait_nmi();
		
		// Handle Input
		controller = pad_trigger(0);
		if (controller & PAD_A) {
			if (player_y == GROUND_Y) {
				player_jumping = 1;
				jump_vel = 0x0e;
			}
		}
		
		// Update Game Logic
		// Update Obstacle Move
		if (wait_time) {
			--wait_time;
		} else {
			prev_obstacle_x = obstacle_x;
			obstacle_x -= (score_total >> 3) + 3;
			// Take advantage of integer overflow to keep score.
			if (obstacle_x > prev_obstacle_x) {
				obstacle_x = RIGHT_EDGE;
				++score_total;
				++score_ones;
				if (score_ones >= 10) {
					score_ones = 0;
					++score_tens;
					if (score_tens >= 10) {
						++score_hundreds;
					}
				}
				write_char(score_hundreds + '0', 2, 9);
				write_char(score_tens + '0', 2, 10);
				write_char(score_ones + '0', 2, 11);
				wait_time = random_table[score_total];
			}
		}
		// Player Jump Code
		if (player_jumping) {
			player_y -= jump_vel;
			--jump_vel;
			if (!jump_vel) {
				player_jumping = 0;
			}
		} else {
			if (player_y < GROUND_Y) {
				player_y += jump_vel;
				++jump_vel;
			}
		}
		// Check for Collision
		// No need to check if player y-position is higher than the obstacle's bottom y-position since it will always return true.
		if (obstacle_x + 16 > PLAYER_X) {
			if (obstacle_x < PLAYER_RIGHT) {
				if (GROUND_Y < player_y + 16) {
					score_total = 0;
					score_hundreds = 0;
					score_tens = 0;
					score_ones = 0;
					write_char('0', 2, 9);
					write_char('0', 2, 10);
					write_char('0', 2, 11);
					obstacle_x = RIGHT_EDGE;
					wait_time = random_table[score_total];
				}
			}
		}
		
		// Render
		oam_clear();
		sprid = 0x00;
		sprid = oam_meta_spr(PLAYER_X, player_y, sprid, player_sprite[(ticks >> (0x04 - (score_total >> 4))) % 4]);
		if (!wait_time) {
			sprid = oam_meta_spr(obstacle_x, GROUND_Y, sprid, obstacle_sprite);
		}
		oam_hide_rest(sprid);

		++ticks;
	}
}
