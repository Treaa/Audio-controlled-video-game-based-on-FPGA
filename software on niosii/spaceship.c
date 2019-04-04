#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "sys/alt_timestamp.h"
#include <unistd.h>
#include <math.h>
#include <string.h>

#include "VGA.h"
#include "FFT.h"
#include "words.h"

#define NUM_ENEMIES			8
#define PLAYER_MOVE			20
#define PLAY_HEALTH			3

#define MAX_SPAWN_INTERVAL	800
#define MIN_SPAWN_INTERVAL  50
#define SPAWN_INTERVAL_STEP 30

#define AUDIO_CPM_VALUE1	300
#define AUDIO_CPM_VALUE2	1000
#define AUDIO_CPM_VALUE3	1500

#define LOOP_TIME           50

typedef struct player {
	float xpos;
	float ypos;
	float prevx;
	float prevy;
	int health;
} player;

typedef struct enemy {
	float xpos;
	float ypos;
	float prevx;
	float prevy;
	float xvel;
	float yvel;
	int valid;
	int radius;
} enemy;

//function declaration
void clearScreen();
void drawCircle(int x, int y, int r, int color);
void drawPlayer(player* player);
void clearPlayer(player* player);
void initPlayer(player* player);
void drawEnemy(enemy* enemy);
void clearEnemy(enemy* enemy);
void updateEnemies();
void spawnEnemies();
void clearEnemies();
void detectCollision();

void clearTime(int x, int y);
void drawTime(int x, int y);
void drawGameover(int x, int y);
void drawHealth(int health);
void drawHeart(int x, int y);
void clearHeart(int x, int y);
void drawLargeChar(int word[WORDSIZE_L][WORDSIZE_L], int x, int y);
void drawMiddleChar(int word[WORDSIZE_M][WORDSIZE_M], int x, int y);
void drawSmallchar(int word[WORDSIZE_S][WORDSIZE_S], int x, int y);
//

player thePlayer;
enemy  enemies[NUM_ENEMIES];

int    spawnInterval;
struct timeval t1, t2;
double elapsedTime, spawnTime, loopTime;

//main function
int main() {
	//initial VGA and FFT
	init_VGA();
	init_FFT();
	//
	printf("Sart Game?y/n\n");
	char  start[10];
	scanf("%s", start);
	if(strcmp(start, "n") == 0 || strcmp(start, "no") == 0) {
		printf("Quit Game...\n");
		return 0;
	}
	printf("Starting Game...\n");
	elapsedTime = 0;
	spawnTime = 0;
	spawnInterval = MAX_SPAWN_INTERVAL;
	initPlayer(&thePlayer);
	while(1){
		drawHealth(thePlayer.health);
		if(thePlayer.health == 0) {
			clearScreen();
			drawGameover(200,200);
			drawTime(260,280);
			printf("You Lose :(\n");
			printf("You have lasted %f seconds\n", elapsedTime/1000.0);
			printf("do you want to continue?y/n\n");
			char  start[10];
			scanf("%s", start);
			if(strcmp(start, "n") == 0 || strcmp(start, "no") == 0) {
				printf("Quit Game...\n");
				break;
			}
			elapsedTime = 0;
			spawnTime = 0;
			clearEnemies();
			clearScreen();
			spawnInterval = MAX_SPAWN_INTERVAL;
			initPlayer(&thePlayer);
			printf("Starting Game...\n");
		}
		//
		if (elapsedTime - spawnTime > spawnInterval) {
			spawnEnemies();
			spawnTime = elapsedTime;
			if(spawnInterval > MIN_SPAWN_INTERVAL)
				spawnInterval -= SPAWN_INTERVAL_STEP;
		}
		updateEnemies();
		drawPlayer(&thePlayer);
		clearTime(5,5);
		drawTime(5,5);
		detectCollision();
		//get the time
		gettimeofday(&t1, NULL);
		loopTime = 0;
		while(loopTime < LOOP_TIME){
			// assert start signal
			float power[512];
			float power_sum;
			short exp;
      		FFT_START(AUDIO_PROCESS_0_BASE, 1);
      		while(!FFT_DONE(AUDIO_PROCESS_0_BASE));  // Wait for the FFT to finish
      		FFT_START(AUDIO_PROCESS_0_BASE, 0);      // Deassert start
      		exp = FFT_EXP(AUDIO_PROCESS_0_BASE);

      		if(exp < 62) {
      			power_sum = 0;
      			int i;
      			for(i = 0; i < 512; i++) {
              		FFT_ADDR(AUDIO_PROCESS_0_BASE, i);
              		power[i] = (float)(short)FFT_POW(AUDIO_PROCESS_0_BASE);
                  	power_sum += power[i];
              	}
              	printf("cur_volume: %f \n",(power_sum/512)*pow(2, 61-exp));
      		}
      		int audioIntensity;
      		audioIntensity = (power_sum/512)*pow(2, 61-exp);
      		//player move up
      		if( audioIntensity> AUDIO_CPM_VALUE3) {
				thePlayer.prevy = thePlayer.ypos;
				clearPlayer(&thePlayer);
				thePlayer.ypos -= PLAYER_MOVE;
				if (thePlayer.ypos < PLAYER_SIZE)  thePlayer.ypos = PLAYER_SIZE;
				drawPlayer(&thePlayer);      			
      		}
      		//player move down
      		else if((audioIntensity < AUDIO_CPM_VALUE2) && (audioIntensity > AUDIO_CPM_VALUE1)){
				thePlayer.prevy = thePlayer.ypos;
				clearPlayer(&thePlayer);
				thePlayer.ypos += PLAYER_MOVE;
				if (thePlayer.ypos > (479 - PLAYER_SIZE) ) thePlayer.ypos = (479 - PLAYER_SIZE);
				drawPlayer(&thePlayer);		      			
      		}
			gettimeofday(&t2, NULL);
			loopTime  = (t2.tv_sec - t1.tv_sec) * 1000.0;     // sec to ms
			loopTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
		}
		gettimeofday(&t2, NULL);
		elapsedTime += (t2.tv_sec - t1.tv_sec) * 1000.0;     // sec to ms
		elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
	}
	return 0;
}

// draw a character(large size)
void drawLargeChar(int word[WORDSIZE_L][WORDSIZE_L], int x, int y)
{
	int i,j;
	for (i = 0; i <  WORDSIZE_L; i++){
		for(j = 0; j < WORDSIZE_L; j++){
			if (word[i][j])
				Vga_Set_Pixel(j+x,(i+y));
		}
	}
}

// draw a character(middle size)
void drawMiddleChar(int word[WORDSIZE_M][WORDSIZE_M], int x, int y)
{
	int i,j;
	for (i = 0; i <  WORDSIZE_M; i++){
		for(j = 0; j < WORDSIZE_M; j++){
			if (word[i][j])
				Vga_Set_Pixel(j+x,(i+y));
		}
	}
}

// draw a character(small size)
void drawSmallChar(int word[WORDSIZE_S][WORDSIZE_S], int x, int y)
{
	int i,j;
	for (i = 0; i <  WORDSIZE_S; i++){
		for(j = 0; j < WORDSIZE_S; j++){
			if (word[i][j])
				Vga_Set_Pixel(j+x,(i+y));
		}
	}
}

// draw a heart
void drawHeart(int x, int y){
	int i,j;
	for (i = 0; i <  20; i++){
		for(j = 0; j < 20; j++){
			if (HEART[i][j])
				Vga_Set_Pixel(j+x,(i+y));
		}
	}
}

// clear a heart
void clearHeart(int x, int y){
	int i,j;
	for (i = 0; i <  20; i++){
		for(j = 0; j < 20; j++){
			if (HEART[i][j])
				Vga_Clr_Pixel(j+x,(i+y));
		}
	}
}

// draw the player's health
void drawHealth(int health){
	if(health == 3){
		drawHeart(600,10);
		drawHeart(580,10);
		drawHeart(560,10);
	}
	else if(health == 2){
		drawHeart(600,10);
		drawHeart(580,10);
		clearHeart(560,10);
	}
	else if(health == 1){
		drawHeart(600,10);
		clearHeart(580,10);
		clearHeart(560,10);
	}
	else {
		clearHeart(600,10);
		clearHeart(580,10);
		clearHeart(560,10);
	}
}

// draw user's score(time)
void drawTime(int x, int y){
	drawMiddleChar(T_m,x,y);
	drawMiddleChar(I_m,x+20,y);
	drawMiddleChar(M_m,x+40,y);
	drawMiddleChar(E_m,x+60,y);
	drawMiddleChar(POINT,x+80,y);

	int elapsedTime_hun = (((int)elapsedTime)/1000)/100 % 10;
	int elapsedTime_ten = (((int)elapsedTime)/1000)/10 % 10;
	int elapsedTime_indi = (((int)elapsedTime)/1000) % 10;
	//hundreds
	if(elapsedTime_hun == 0)
		drawMiddleChar(NUM_0,x+100,y);
	else if(elapsedTime_hun == 1)
		drawMiddleChar(NUM_1,x+100,y);
	else if(elapsedTime_hun == 2)
		drawMiddleChar(NUM_2,x+100,y);
	else if(elapsedTime_hun == 3)
		drawMiddleChar(NUM_3,x+100,y);
	else if(elapsedTime_hun == 4)
		drawMiddleChar(NUM_4,x+100,y);
	else if(elapsedTime_hun == 5)
		drawMiddleChar(NUM_5,x+100,y);
	else if(elapsedTime_hun == 6)
		drawMiddleChar(NUM_6,x+100,y);
	else if(elapsedTime_hun == 7)
		drawMiddleChar(NUM_7,x+100,y);
	else if(elapsedTime_hun == 8)
		drawMiddleChar(NUM_8,x+100,y);
	else if(elapsedTime_hun == 9)
		drawMiddleChar(NUM_9,x+100,y);
	else
		drawMiddleChar(NUM_0,x+100,y);

	//tens
	if(elapsedTime_ten == 0)
		drawMiddleChar(NUM_0,x+120,y);
	else if(elapsedTime_ten == 1)
		drawMiddleChar(NUM_1,x+120,y);
	else if(elapsedTime_ten == 2)
		drawMiddleChar(NUM_2,x+120,y);
	else if(elapsedTime_ten == 3)
		drawMiddleChar(NUM_3,x+120,y);
	else if(elapsedTime_ten == 4)
		drawMiddleChar(NUM_4,x+120,y);
	else if(elapsedTime_ten == 5)
		drawMiddleChar(NUM_5,x+120,y);
	else if(elapsedTime_ten == 6)
		drawMiddleChar(NUM_6,x+120,y);
	else if(elapsedTime_ten == 7)
		drawMiddleChar(NUM_7,x+120,y);
	else if(elapsedTime_ten == 8)
		drawMiddleChar(NUM_8,x+120,y);
	else if(elapsedTime_ten == 9)
		drawMiddleChar(NUM_9,x+120,y);
	else
		drawMiddleChar(NUM_0,x+120,y);

	//individual
	if(elapsedTime_indi == 0)
		drawMiddleChar(NUM_0,x+140,y);
	else if(elapsedTime_indi == 1)
		drawMiddleChar(NUM_1,x+140,y);
	else if(elapsedTime_indi == 2)
		drawMiddleChar(NUM_2,x+140,y);
	else if(elapsedTime_indi == 3)
		drawMiddleChar(NUM_3,x+140,y);
	else if(elapsedTime_indi == 4)
		drawMiddleChar(NUM_4,x+140,y);
	else if(elapsedTime_indi == 5)
		drawMiddleChar(NUM_5,x+140,y);
	else if(elapsedTime_indi == 6)
		drawMiddleChar(NUM_6,x+140,y);
	else if(elapsedTime_indi == 7)
		drawMiddleChar(NUM_7,x+140,y);
	else if(elapsedTime_indi == 8)
		drawMiddleChar(NUM_8,x+140,y);
	else if(elapsedTime_indi == 9)
		drawMiddleChar(NUM_9,x+140,y);
	else
		drawMiddleChar(NUM_0,x+140,y);

	drawSmallChar(S_s,x+158,(y+3));

}

// clear user's score(time)
void clearTime(int x, int y){
	int i,j;
	for (i = 0; i <  15; i++){
		for(j = 0; j < 60; j++){
			Vga_Clr_Pixel(j+x+100,(i+y));
		}
	}
}

// draw gameover
void drawGameover(int x, int y){
	drawLargeChar(G_l,x,y);
	drawLargeChar(A_l,x+30,y);
	drawLargeChar(M_l,x+60,y);
	drawLargeChar(E_l,x+90,y);

	drawLargeChar(O_l,x+140,y);
	drawLargeChar(V_l,x+170,y);
	drawLargeChar(E_l,x+200,y);
	drawLargeChar(R_l,x+230,y);
}

//clear screen
void clearScreen() {
	int i,j;
	//set all pixels not display
	for(i = 0; i < VGA_WIDTH; i++)  
		for(j = 0; j < VGA_HEIGHT; j++)
			Vga_Clr_Pixel(i, j);
}

//draw a circle
void drawCircle(int x, int y, int r, int color) {
	int xc, yc, col, row;
	for(xc = -r; xc <= r; xc++) {
		for(yc = -r; yc <= r; yc++) {
			col = xc;
			row = yc;
			//add the r to make the edge smoother
			if(col*col+row*row <= r*r+r) {
				col += x;
				row += y;
				//check for valid 640*480
				if(col > VGA_WIDTH-1)	col = VGA_WIDTH-1;
				if(row > VGA_HEIGHT-1)	row = VGA_HEIGHT-1;
				if(col < 0)				col = 0;
				if(row < 0)				row = 0;
				//display pixel
				if(color == PIXEL_ON)
					Vga_Set_Pixel(col, row);
				else
					Vga_Clr_Pixel(col, row);
			}
		}
	}
}

//display player
void drawPlayer(player* player) {
	int  i, j;
	for(i=0; i<PLAYER_SIZE; i++){
		for(j=0; j<PLAYER_SIZE; j++){
			int col, row;
			col = (int)(player->xpos) + j;
			row = (int)(player->ypos) - PLAYER_SIZE/2 + i;
			//check for valid 640*480
			if(col > VGA_WIDTH-1)	col = VGA_WIDTH-1;
			if(row > VGA_HEIGHT-1)	row = VGA_HEIGHT-1;
			if(col < 0)				col = 0;
			if(row < 0)				row = 0;
			//
			if(SPACESHIP[i][j] == 1)
				Vga_Set_Pixel(col, row);
			else
				Vga_Clr_Pixel(col, row);
		}
	}
}

//clear player
void clearPlayer(player* player) {
	int  i, j;
	for(i=0; i<PLAYER_SIZE; i++){
		for(j=0; j<PLAYER_SIZE; j++){
			int col, row;
			col = (int)(player->xpos) + j;
			row = (int)(player->ypos) - PLAYER_SIZE/2 + i;
			//check for valid 640*480
			if(col > VGA_WIDTH-1)	col = VGA_WIDTH-1;
			if(row > VGA_HEIGHT-1)	row = VGA_HEIGHT-1;
			if(col < 0)				col = 0;
			if(row < 0)				row = 0;
			//
			Vga_Clr_Pixel(col, row);
		}
	}
}

//initial player
void initPlayer(player* player) {
	player->xpos = 100;
	player->ypos = 240;
	player->prevx = player->xpos;
	player->prevy = 240;
	player->health = PLAY_HEALTH;
}

//display enemy
void drawEnemy(enemy* enemy) {
	drawCircle((int)(enemy->xpos), (int)(enemy->ypos), enemy->radius, PIXEL_ON);
}

//clear player
void clearEnemy(enemy* enemy) {
	drawCircle((int)(enemy->xpos), (int)(enemy->ypos), enemy->radius, PIXEL_OFF);
}

//update enemies
void updateEnemies() {
	int i;
	for (i = 0; i < NUM_ENEMIES; i++) {
		if (enemies[i].valid) {
			enemies[i].prevx = enemies[i].xpos;
			enemies[i].prevy = enemies[i].ypos;
			clearEnemy(&enemies[i]);
			enemies[i].xpos += enemies[i].xvel;
			enemies[i].ypos += enemies[i].yvel;
			if (enemies[i].xpos < 2 || enemies[i].ypos > 479 || enemies[i].ypos < 2) {
				enemies[i].valid = 0;
				enemies[i].xvel = 0;
				enemies[i].yvel = 0;
			} else {
				drawEnemy(&enemies[i]);
			}
		}
	}
}

void spawnEnemies() {
	int i;
	for (i = 0; i < NUM_ENEMIES; i++) {
		if (!enemies[i].valid) {
			enemies[i].xpos = 630;
			// range for rand: rand() % (max_number + 1 - minimum_number) + minimum_number
			enemies[i].ypos = rand() % (460 + 1 - 20) + 20;
			enemies[i].prevx = enemies[i].xpos;
			enemies[i].prevy = enemies[i].ypos;
			enemies[i].xvel = -((rand() % 1800) + 600)/200.0;
			enemies[i].yvel = ((rand() % 800) - 400)/200.0;
			enemies[i].valid = 1;
			enemies[i].radius = rand() % (20) + 5;
			break;
		}
	}
}

void clearEnemies() {
	int i;
	for(i = 0; i < NUM_ENEMIES; i++) {
		enemies[i].valid = 0;
	}
}

void detectCollision() {
	int i;
	for (i = 0; i < NUM_ENEMIES; i++) {
		if (enemies[i].valid) {
			if (enemies[i].xpos < thePlayer.xpos + PLAYER_SIZE/4 + enemies[i].radius) {
				if ((enemies[i].ypos - thePlayer.ypos <= enemies[i].radius + PLAYER_SIZE/8) &&
				    (enemies[i].ypos - thePlayer.ypos) >= -(enemies[i].radius + PLAYER_SIZE/8)) {
					clearEnemy(&enemies[i]);
					enemies[i].valid = 0;
					thePlayer.health -= 1;
				}
			}
		}
	}
} 

