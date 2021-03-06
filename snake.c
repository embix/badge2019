/*
 * This file is part of badge2019.
 * Copyright 2019 Niels Kristensen <niels@kristensen.io>
 * Copyright 2019 Emil Renner Berthing <esmil@labitat.dk>
 *
 * badge2019 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * badge2019 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with badge2019. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>

#include "leds.h"
#include "timer.h"
#include "events.h"
#include "buttons.h"
#include "display.h"
#include "menu.h"

enum events {
	EV_UP = 1,
	EV_DOWN,
	EV_LEFT,
	EV_RIGHT,
	EV_TICK,
};

enum direction {
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT,
};

static const struct button_config snake_buttons[BTN_MAX] = {
	[BTN_UP]    = { .press = EV_UP, },
	[BTN_DOWN]  = { .press = EV_DOWN, },
	[BTN_LEFT]  = { .press = EV_LEFT, },
	[BTN_RIGHT] = { .press = EV_RIGHT, },
};

static const struct button_config anybutton[BTN_MAX] = {
	[BTN_SUP]    = { .press = 1, },
	[BTN_SMID]   = { .press = 1, },
	[BTN_SDOWN]  = { .press = 1, },
	[BTN_UP]     = { .press = 1, },
	[BTN_DOWN]   = { .press = 1, },
	[BTN_LEFT]   = { .press = 1, },
	[BTN_RIGHT]  = { .press = 1, },
	[BTN_CENTER] = { .press = 1, },
};

struct pos {
	uint8_t x;
	uint8_t y;
};

static void
random_pos(struct pos *p, unsigned int res)
{
	p->x = (rand() % (240/res - 2)) * res + res;
	p->y = (rand() % (240/res - 2)) * res + res;
}

static void
lost(unsigned int score)
{
	char buf[4];

	sprintf(buf, "%u", score);

	dp_fill(0, 0, 240, 240, 0x000);
	dp_puts( 30, 20, 0x00F, 0x000, "Your score:");
	dp_puts(150, 50, 0xCB0, 0x000, buf);

	buttons_config(anybutton);
	event_wait();
}

void
snake(unsigned int res)
{
	struct pos path[256];
	struct pos point;
	struct ticker tick;
	enum direction cdir = DIR_DOWN;
	enum direction ndir = DIR_DOWN;
	unsigned int len = 5;
	unsigned int i;
	uint8_t currentX = 6*res;
	uint8_t currentY = 6*res;

	/* initialize path */
	for (i = 0; i < len; i++) {
		path[i].x = currentX;
		path[i].y = currentY;
	}

	/* '4' guaranteed to be random
	 * -- chosen by fair dice
	 */
	srand(4);

	/* print border */
	dp_fill(0,         0, 240,         res, 0xCB0);
	dp_fill(0,   240-res, 240,         res, 0xCB0);
	dp_fill(0,       res, res, 240 - 2*res, 0xCB0);
	dp_fill(240-res, res, res, 240 - 2*res, 0xCB0);
	/* clear middle part */
	dp_fill(res, res, 240 - 2*res, 240 - 2*res, 0x000);

	/* first point */
	point.x = 240 - 8*res;
	point.y = 240 - 8*res;
	dp_fill(point.x, point.y, res, res, 0xF00);

	buttons_config(snake_buttons);

	ticker_start(&tick, 100, EV_TICK);

	while (1) {
		unsigned int k;

		switch ((enum events)event_wait()) {
		case EV_UP:
			if (cdir != DIR_DOWN)
				ndir = DIR_UP;
			continue;
		case EV_DOWN:
			if (cdir != DIR_UP)
				ndir = DIR_DOWN;
			continue;
		case EV_LEFT:
			if (cdir != DIR_RIGHT)
				ndir = DIR_LEFT;
			continue;
		case EV_RIGHT:
			if (cdir != DIR_LEFT)
				ndir = DIR_RIGHT;
			continue;
		case EV_TICK:
			break;
		}

		cdir = ndir;
		switch (cdir) {
		case DIR_DOWN:  currentY += res; break;
		case DIR_UP:    currentY -= res; break;
		case DIR_LEFT:  currentX -= res; break;
		case DIR_RIGHT: currentX += res; break;
		}

		/* did we eat a point? */
		if (currentX == point.x && currentY == point.y) {
			if (len < (ARRAY_SIZE(path) - 1))
				len += 1;
			/* generate new point */
			random_pos(&point, res);
		}

		/* calculate tail index */
		if (i < len)
			k = ARRAY_SIZE(path) + i - len;
		else
			k = i - len;

		/* clear tail point */
		dp_fill(path[k].x, path[k].y, res, res, 0x000);

		/* paint new head */
		dp_fill(currentX, currentY, res, res, 0xCB0);

		/* did we hit ourself? */
		while (1) {
			k = (k + 1) % ARRAY_SIZE(path);
			if (k == i)
				break;
			if (currentX == path[k].x && currentY == path[k].y) {
				ticker_stop(&tick);
				lost(len - 5);
				return;
			}
		}

		/* did we hit the wall? */
		if (currentX == 0 || currentX == 240-res || currentY == 0 || currentY == 240-res) {
			ticker_stop(&tick);
			lost(len - 5);
			return;
		}

		/* paint red dot last since it might
		 * be inside the snake
		 */
		dp_fill(point.x, point.y, res, res, 0xF00);

		/* record this point */
		path[i].x = currentX;
		path[i].y = currentY;
		i = (i + 1) % ARRAY_SIZE(path);
	}
}

static void snake4(void)  { snake(4); }
static void snake5(void)  { snake(5); }
static void snake6(void)  { snake(6); }
static void snake8(void)  { snake(8); }
static void snake10(void) { snake(10); }

void
snakemenu(void)
{
	static const struct menuitem snakemenu[] = {
		{ .label = "4", .cb = snake4,  },
		{ .label = "5", .cb = snake5,  },
		{ .label = "6", .cb = snake6,  },
		{ .label = "8", .cb = snake8,  },
		{ .label = "10",.cb = snake10, },
	};

	menu(snakemenu, ARRAY_SIZE(snakemenu), 0xCB0, 0x000);
}
