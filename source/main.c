#include <stdint.h>
#include <stdbool.h>
#include "gba_input.h"


typedef uint8_t		u8;		typedef int8_t		s8;
typedef uint16_t	u16;	typedef int16_t		s16;
typedef uint32_t	u32;	typedef int32_t		s32;

typedef volatile uint8_t	v_u8;	typedef volatile int8_t     zv_s8;
typedef volatile uint16_t	v_u16;	typedef volatile int16_t 	v_s16;
typedef volatile uint32_t	v_u32;	typedef volatile int32_t 	v_s32;


#define REG_DISPCNT *((v_u32*)(0x04000000))
#define VIDEOMODE_3 0x0003
#define BGMODE_2	0x0400

#define SCREENBUFFER ((v_u16*)(0x06000000))
#define SCREEN_W 240
#define SCREEN_H 160


s32 abs(s32 a_val)
{
	
    s32 mask = a_val >> 31;
	return (a_val ^ mask) - mask;

}

u16 setColour(u8 a_red, u8 a_green, u8 a_blue)
{
	
    return (a_red & 0x1F) | (a_green & 0x1F) << 5 | (a_blue & 0x1f) << 10;

}

void plotPixel( u32 a_x, u32 a_y, u16 a_colour)
{

	SCREENBUFFER[a_y * SCREEN_W + a_x] = a_colour;

}

void drawRect(u32 a_left, u32 a_top, u32 a_width, u32 a_height, u16 a_colour)
{

	for (u32 y = 0; y < a_height; ++y)
	{

		for (u32 x = 0; x < a_width; ++x)
		{
		
        	SCREENBUFFER[ (a_top + y) * SCREEN_W + a_left + x ] = a_colour;
		
        }

	}

}

void drawLine(u32 a_x, u32 a_y, u32 a_x2, u32 a_y2, u16 a_colour)
{

	// Get the horizontal and vertical displacement of the line
	s32 w = a_x2 - a_x; // w is width or horizontal distance
	s32 h = a_y2 - a_y; // h is the height or vertical displacement
	// work out what the change in x and y is with the d in these variables stands for delta
	s32 dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;

	if (w<0) dx1 = dx2 = -1; else if (w>0) dx1 = dx2 = 1;
	if (h<0) dy1 = -1; else if (h>0) dy1 = 1;
	// which is the longest the horizontal or vertical step
	s32 longest = abs(w); // assume that width is the longest displacement
	s32 shortest = abs(h);
	
    if ( shortest > longest )	// oops it's the other way around reverse it
	{

		// use xor to swap longest and shortest around
		longest ^= shortest; shortest ^= longest; longest ^= shortest;
		if (h<0) dy2 = -1; else if (h>0) dy2 = 1;
		dx2 = 0;
	
    }
	// geta  value that is half the longest displacement
	s32 numerator = longest >> 1;
	// for each pixel across the longest span
	for (s32 i = 0; i <= longest; ++i)
    {

		// fill the pixel we're currently at
		plotPixel( a_x, a_y, a_colour);
		// increase the numerator by the shortest span
		numerator += shortest;

		if (numerator>longest)
		{

			// if the numerator is now larger than the longest span
			// subtract the longest value from the numerator
			numerator -= longest;
			// increment x & y by their delta1 values
			a_x += dx1;
			a_y += dy1;

		} else
		{

			// numerator is smaller than the longst side
			// increment x & y by their delta 2 values
			a_x += dx2;
			a_y += dy2;

		}

	}

}

// ? Random number generator
s32 __gba_rand_seed = 42;

s32 seed_gba_rand(s32 a_seed)
{

    s32 old_seed = __gba_rand_seed;
    __gba_rand_seed = a_seed;

    return old_seed;

}

s32 gba_rand()
{

    __gba_rand_seed = 166425 * __gba_rand_seed + 1013904223;

    return (__gba_rand_seed >> 16) & 0x7FFF;

}

s32 gba_rand_range(s32 a_min, s32 a_max)
{

    return (gba_rand() * (a_max - a_min) >> 15) + a_min;

}

typedef struct Ball
{

    s32 x, y, xDir, yDir, size;
    u16 colour;

} Ball;

void StartBall(Ball* a_ball)
{

    while (a_ball -> xDir == 0)
    {

        a_ball->xDir = gba_rand_range(-1, 2);

    }

    a_ball->yDir = gba_rand_range(-1, 2);

}

void InitBall(Ball* a_ball, s32 a_x, s32 a_y, s32 a_size, u16 a_colour)
{

    a_ball->x = a_x;
    a_ball->y = a_y;
    a_ball->size = a_size;
    a_ball->colour = a_colour;
    a_ball->xDir = a_ball->yDir = 0;

    StartBall(a_ball);

}

void MoveBall(Ball* a_ball)
{

    a_ball->y += a_ball->yDir;

    if (a_ball->y < 0)
    {

        a_ball->y = 0;
        a_ball->yDir *= -1;

    }

    if (a_ball->y > SCREEN_H - a_ball->size)
    {

        a_ball->y = SCREEN_H - a_ball->size;
        a_ball->yDir *= -1;

    }

    a_ball->x += a_ball->xDir;

    if (a_ball->x < 0 || a_ball->x > SCREEN_W - a_ball->size)
    {

        a_ball->x = (SCREEN_W >> 1) - (a_ball->size >> 1);
        a_ball->y = (SCREEN_H >> 1) - (a_ball->size >> 1);
        a_ball->xDir = 0;
        a_ball->yDir = 0;

        StartBall(a_ball);

    }

}

void DrawBall(Ball* a_ball)
{

    drawRect(a_ball->x, a_ball->y, a_ball->size, a_ball->size, a_ball->colour);

}

void ClearBall(Ball* a_ball)
{

    drawRect(a_ball->x, a_ball->y, a_ball->size, a_ball->size, setColour(0, 0, 0));

}

#define REG_VCOUNT (*(v_u16*)(0x04000006))

void vsync()
{

    while(REG_VCOUNT >= SCREEN_H);
    while(REG_VCOUNT < SCREEN_H);

}

// ? Paddles
typedef struct Paddle
{

    s32 x, y, width, height;
    u16 colour;

} Paddle;

void InitPaddle(Paddle* a_pad, s32 a_x, s32 a_y, s32 a_width, s32 a_height, u16 a_colour)
{

    a_pad->x = a_x;
    a_pad->y = a_y;
    a_pad->width = a_width;
    a_pad->height = a_height;
    a_pad->colour = a_colour;

}

void DrawPaddle(Paddle* a_pad)
{

    drawRect(a_pad->x, a_pad->y, a_pad->width, a_pad->height, a_pad->colour);

}

void ClearPaddle(Paddle* a_pad)
{

    drawRect(a_pad->x, a_pad->y, a_pad->width, a_pad->height, setColour(0, 0, 0));

}

void MovePaddle(Paddle* a_pad, s32 a_val)
{

    a_pad->y += a_val;

    if (a_pad->y < 0)
    {

        a_pad->y = 0;

    }

    if (a_pad->y > SCREEN_H - a_pad->height)
    {

        a_pad->y = SCREEN_H - a_pad->height;

    }

}

void HitPaddles(Ball* a_ball, Paddle* a_pad)
{

    if (a_ball->x == a_pad->x && a_ball->y < a_pad->y + a_pad->height && a_ball->y + a_ball->size > a_pad->y)
    {

        a_ball->xDir *= -1;
        a_ball->yDir = gba_rand_range(-1, 2);

    }

}

int main()
{
	// set GBA rendering context to MODE 3 Bitmap Rendering and enable BG 2
	REG_DISPCNT = VIDEOMODE_3 | BGMODE_2;

    // Clear screen
    drawRect(0, 0, SCREEN_W, SCREEN_H, setColour(0, 0, 0));

    // Create ball
    Ball ball;
    InitBall(&ball, SCREEN_W >> 1, SCREEN_H >> 1, 10, setColour(31, 31, 31));

    // Create paddles
    Paddle p1;
    InitPaddle(&p1, 10, 60, 8, 40, setColour(31, 0, 15));

    Paddle p2;
    InitPaddle(&p2, SCREEN_W - 18, 60, 8, 40, setColour(15, 0, 31));

	while (1) {

        vsync();
        PollKeys();

        ClearBall(&ball);
        ClearPaddle(&p1);
        ClearPaddle(&p2);

		MoveBall(&ball);
        HitPaddles(&ball, &p1);
        HitPaddles(&ball, &p2);

        // Move Paddle
        s16 pDir = 0;
		if(keyDown(UP))
		{

			pDir = -2;

        }

        if(keyDown(DOWN))
		{

			pDir = 2;

        }

        MovePaddle(&p1, pDir);

		pDir = 0;
		if(keyDown(A))
		{

			pDir = -2;

        }

        if(keyDown(B))
		{

			pDir = 2;

        }

        // Call update functions
        MovePaddle(&p2, pDir);

        DrawBall(&ball);
        DrawPaddle(&p1);
        DrawPaddle(&p2);

	}

	return 0;
}
