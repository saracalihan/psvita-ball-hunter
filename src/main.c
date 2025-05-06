#include <psp2/kernel/processmgr.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <psp2/ctrl.h>
#include <SDL2/SDL_audio.h>

#define BALL_SPEED 8
#define BAR_SPEED 4
#define BULLET_SPEED 15
#define COLOR_COUNT 8
#define RADIUS 40
#define BAR_WIDTH (RADIUS*3)
#define BAR_HEIGHT 10
#define BAR_MARGIN BAR_HEIGHT
#define BULLET_COUNT 4

//Screen dimension constants
enum {
  SCREEN_WIDTH  = 960,
  SCREEN_HEIGHT = 544
};

typedef struct {
  int r;
  int g;
  int b;
} Color;

typedef struct {
  int x;
  int y;
} Point;

Color colors[COLOR_COUNT] = {
  {255, 0, 0},   // Red
  {0, 255, 0},   // Green
  {0, 0, 255},   // Blue
  {255, 255, 0}, // Yellow
  {255, 0, 255}, // Magenta
  {0, 255, 255}, // Cyan
  {255, 165, 0}, // Orange
  {128, 0, 128}  // Purple
};

Point bullets[BULLET_COUNT] = {0};

SDL_Window    * gWindow   = NULL;
SDL_Renderer  * gRenderer = NULL;


int currentColorIndex = 0;

void drawCircle(SDL_Renderer *renderer, int ballX, int ballY, int r) {
    for (int w = 0; w < r * 2; w++) {
        for (int h = 0; h < r * 2; h++) {
            int dx = r - w; // Horizontal offset
            int dy = r - h; // Vertical offset
            if ((dx * dx + dy * dy) <= (r * r)) {
                SDL_RenderDrawPoint(renderer, ballX + dx, ballY + dy);
            }
        }
    }
}

int checkXCollision(int cx, int bx){
  return (bx <= cx && cx <= bx + BAR_WIDTH) ? 1 : 0;
}

// map value fo -1 - 1 from 0 - 255
float mapInput(int v){
  return ((float)v/255)*2 - 1;
}

int isGameOver(int circleX, int circleY, int barX){
  // check y collision
  if(circleY + RADIUS < SCREEN_HEIGHT){
    return 0;
  }

  return checkXCollision(circleX, barX) ? 0 : 1; 
}

void generateBeepSound(void *userdata, Uint8 *stream, int len) {
    static int phase = 0;
    int frequency = 440; // Frequency in Hz (A4 note)
    int sampleRate = 44100; // Sample rate in Hz
    int amplitude = 28000; // Amplitude of the wave

    for (int i = 0; i < len / 2; i++) {
        int16_t sample = amplitude * sin(2.0f * M_PI * frequency * phase / sampleRate);
        ((int16_t *)stream)[i] = sample;
        phase++;
    }
}

void playGameOverSound() {
    SDL_AudioSpec desiredSpec, obtainedSpec;
    SDL_memset(&desiredSpec, 0, sizeof(desiredSpec));
    desiredSpec.freq = 44100;
    desiredSpec.format = AUDIO_S16SYS;
    desiredSpec.channels = 1;
    desiredSpec.samples = 2048;
    desiredSpec.callback = generateBeepSound;

    if (SDL_OpenAudio(&desiredSpec, &obtainedSpec) < 0) {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
        return;
    }

    SDL_PauseAudio(0); // Start audio playback
    SDL_Delay(500);    // Play for 500ms
    SDL_CloseAudio();
}

void updateBullets() {
    for (int i = 0; i < BULLET_COUNT; i++) {
        if (bullets[i].y > 0) {
            bullets[i].y -= BULLET_SPEED; // Move bullet upwards
        } else {
            bullets[i].x = 0; // Reset bullet position when it goes off-screen
            bullets[i].y = 0;
        }
    }
}

void drawBullets(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White bullets
    for (int i = 0; i < BULLET_COUNT; i++) {
        if (bullets[i].y > 0) {
            SDL_Rect bulletRect = {
                bullets[i].x - 2, // Center the rectangle horizontally
                bullets[i].y - 5, // Adjust the rectangle vertically
                4,                // Width of the bullet
                10                // Height of the bullet
            };
            SDL_RenderFillRect(renderer, &bulletRect);
        }
    }
}


void shootBullet(int barX) {
    static Uint32 lastShotTime = 0;
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastShotTime >= 300) {
        for (int i = 0; i < BULLET_COUNT; i++) {
            if (bullets[i].y == 0) {
                bullets[i].x = barX + BAR_WIDTH / 2;
                bullets[i].y = SCREEN_HEIGHT - BAR_HEIGHT * 2;
                lastShotTime = currentTime;
                break;
            }
        }
    }
}

int main(int argc, char *argv[])
{
  
  int is_done = 0;
  int ballX = SCREEN_WIDTH / 2;
  int ballY = SCREEN_HEIGHT / 2;

  int ballVX = 1, ballVY=1;

  SceCtrlData ctrl;
  Color currentColor = colors[0];
  SDL_Rect barRect = {
    0,
    SCREEN_HEIGHT - BAR_HEIGHT*2,
    BAR_WIDTH,
    BAR_HEIGHT
  };


  if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
      return -1;

  if ((gWindow = SDL_CreateWindow( "Ball", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN)) == NULL)
    return -1;

  if ((gRenderer = SDL_CreateRenderer( gWindow, -1, 0)) == NULL)
      return -1;

  sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG);

  float barVX = 0;
  while(1){
    // BAR CONTROL
    sceCtrlPeekBufferPositive(0, &ctrl, 1);

    if(ctrl.buttons == SCE_CTRL_CROSS){
      shootBullet(barRect.x); // Attempt to shoot a bullet
    }

    // POSITION CALCULATION
    float newValoX = mapInput(ctrl.lx);
    float newValoY = mapInput(ctrl.ly);
    barVX = 0;

    if(fabs((float)newValoX) >=0.2){
      barVX = newValoX*10.0;
    }

    barRect.x += barVX * BAR_SPEED;
    if(0 > barRect.x){
      barRect.x = 0;
    }
    if( barRect.x + BAR_WIDTH >= SCREEN_WIDTH){
      barRect.x = SCREEN_WIDTH - BAR_WIDTH;
    }

    // BOUNCE ANIMATION
    ballX += ballVX * BALL_SPEED;
    ballY += ballVY * BALL_SPEED;

    // wall
    if(ballX + RADIUS >= SCREEN_WIDTH || ballX - RADIUS <= 0){
      ballVX *= -1;
    }
    if(ballY + RADIUS >= SCREEN_HEIGHT || ballY - RADIUS <= 0){
      ballVY *= -1;
    }

    // bar
    if(ballY + RADIUS >= barRect.y && checkXCollision(ballX, barRect.x) == 1){
      ballVY *= -1;
    }

    if(isGameOver(ballX, ballY, barRect.x) == 1){
      currentColor = colors[++currentColorIndex % COLOR_COUNT];
      ballX = SCREEN_WIDTH/2;
      ballY = SCREEN_HEIGHT/2;
      barRect.x=SCREEN_WIDTH/2;

      playGameOverSound();
    }

    // UPDATE BULLETS
    updateBullets();

    // DRAW SHAPES
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
    SDL_RenderClear(gRenderer);
    SDL_SetRenderDrawColor(gRenderer, currentColor.r, currentColor.g, currentColor.b, 255);
    drawCircle(gRenderer, ballX, ballY, RADIUS);
    
    SDL_SetRenderDrawColor(gRenderer, 200, 200, 200, 255);
    SDL_RenderFillRect( gRenderer, &barRect );

    drawBullets(gRenderer); // Draw bullets

    SDL_RenderPresent(gRenderer);
    SDL_Delay(16); // ~60 FPS
    // if (SDL_PollEvent(NULL)) {
    //     break; // Exit loop on any event
    // }
  }

  SDL_DestroyRenderer( gRenderer );
  SDL_DestroyWindow( gWindow );
  gWindow = NULL;
  gRenderer = NULL;

  SDL_Quit();
  return 0;
}