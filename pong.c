#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define PADDLE_WIDTH 100
#define PADDLE_HEIGHT 20
#define BALL_SIZE 10
#define BLOCK_WIDTH 60
#define BLOCK_HEIGHT 20
#define NUM_BLOCKS_X (WINDOW_WIDTH / BLOCK_WIDTH)
#define NUM_BLOCKS_Y 5

typedef struct {
    int x, y, w, h;
} Paddle;

typedef struct {
    int x, y, dx, dy, size;
} Ball;

typedef struct {
    int x, y, w, h;
    int active;
} Block;

void render(SDL_Renderer *renderer, TTF_Font *font, Paddle *paddle, Ball *ball, Block blocks[], int score, int gameOver, int startScreen) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (startScreen) {
        char startText[] = "Pressione uma tecla para começar!";
        SDL_Color white = {255, 255, 255};
        SDL_Surface *startSurface = TTF_RenderText_Solid(font, startText, white);
        SDL_Texture *startTexture = SDL_CreateTextureFromSurface(renderer, startSurface);
        SDL_Rect startRect = {WINDOW_WIDTH / 2 - startSurface->w / 2, WINDOW_HEIGHT / 2 - startSurface->h / 2, startSurface->w, startSurface->h};
        SDL_RenderCopy(renderer, startTexture, NULL, &startRect);
        SDL_FreeSurface(startSurface);
        SDL_DestroyTexture(startTexture);
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_Rect paddleRect = {paddle->x, paddle->y, paddle->w, paddle->h};
        SDL_RenderFillRect(renderer, &paddleRect);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect ballRect = {ball->x, ball->y, ball->size, ball->size};
        SDL_RenderFillRect(renderer, &ballRect);

        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        for (int i = 0; i < NUM_BLOCKS_X * NUM_BLOCKS_Y; i++) {
            if (blocks[i].active) {
                SDL_Rect blockRect = {blocks[i].x, blocks[i].y, blocks[i].w, blocks[i].h};
                SDL_RenderFillRect(renderer, &blockRect);
            }
        }

        char scoreText[50];
        sprintf(scoreText, "Pontos: %d", score);
        SDL_Color white = {255, 255, 255};
        SDL_Surface *scoreSurface = TTF_RenderText_Solid(font, scoreText, white);
        SDL_Texture *scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurface);
        SDL_Rect scoreRect = {10, 10, scoreSurface->w, scoreSurface->h};
        SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
        SDL_FreeSurface(scoreSurface);
        SDL_DestroyTexture(scoreTexture);

        if (gameOver) {
            char gameOverText[] = "Game Over! Pressione R para reiniciar";
            SDL_Surface *gameOverSurface = TTF_RenderText_Solid(font, gameOverText, white);
            SDL_Texture *gameOverTexture = SDL_CreateTextureFromSurface(renderer, gameOverSurface);
            SDL_Rect gameOverRect = {WINDOW_WIDTH / 2 - gameOverSurface->w / 2, WINDOW_HEIGHT / 2 - gameOverSurface->h / 2, gameOverSurface->w, gameOverSurface->h};
            SDL_RenderCopy(renderer, gameOverTexture, NULL, &gameOverRect);
            SDL_FreeSurface(gameOverSurface);
            SDL_DestroyTexture(gameOverTexture);
        }
    }

    SDL_RenderPresent(renderer);
}

void movePaddle(Paddle *paddle, int direction) {
    if (direction == -1 && paddle->x > 0) {
        paddle->x -= 5;
    } else if (direction == 1 && paddle->x + paddle->w < WINDOW_WIDTH) {
        paddle->x += 5;
    }
}

void moveBall(Ball *ball) {
    ball->x += ball->dx;
    ball->y += ball->dy;

    if (ball->x <= 0 || ball->x + ball->size >= WINDOW_WIDTH) {
        ball->dx = -ball->dx;
    }
    if (ball->y <= 0) {
        ball->dy = -ball->dy;
    }
}

void checkBallPaddleCollision(Ball *ball, Paddle *paddle) {
    if (ball->x + ball->size >= paddle->x &&
        ball->x <= paddle->x + paddle->w &&
        ball->y + ball->size >= paddle->y &&
        ball->y <= paddle->y + paddle->h) {
        ball->dy = -ball->dy;
    }
}

void checkBallBlockCollision(Ball *ball, Block blocks[], int *score) {
    for (int i = 0; i < NUM_BLOCKS_X * NUM_BLOCKS_Y; i++) {
        if (blocks[i].active &&
            ball->x + ball->size >= blocks[i].x &&
            ball->x <= blocks[i].x + blocks[i].w &&
            ball->y + ball->size >= blocks[i].y &&
            ball->y <= blocks[i].y + blocks[i].h) {
            ball->dy = -ball->dy;
            blocks[i].active = 0;
            (*score)++;
        }
    }
}

void resetGame(Paddle *paddle, Ball *ball, Block blocks[], int *score) {
    paddle->x = WINDOW_WIDTH / 2 - PADDLE_WIDTH / 2;
    paddle->y = WINDOW_HEIGHT - 30;
    ball->x = WINDOW_WIDTH / 2 - BALL_SIZE / 2;
    ball->y = WINDOW_HEIGHT / 2 - BALL_SIZE / 2;
    ball->dx = 3;
    ball->dy = -3;
    *score = 0;

    for (int i = 0; i < NUM_BLOCKS_X * NUM_BLOCKS_Y; i++) {
        blocks[i].active = 1;
    }
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL não pode ser inicializado! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    if (TTF_Init() == -1) {
        printf("Erro ao inicializar SDL_ttf: %s\n", TTF_GetError());
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("Breakout", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Não foi possível criar a janela! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Não foi possível criar o renderizador! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    TTF_Font *font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 24);
    if (!font) {
        printf("Erro ao carregar a fonte: %s\n", TTF_GetError());
        return -1;
    }

    Paddle paddle = {WINDOW_WIDTH / 2 - PADDLE_WIDTH / 2, WINDOW_HEIGHT - 30, PADDLE_WIDTH, PADDLE_HEIGHT};
    Ball ball = {WINDOW_WIDTH / 2 - BALL_SIZE / 2, WINDOW_HEIGHT / 2 - BALL_SIZE / 2, 3, -3, BALL_SIZE};
    Block blocks[NUM_BLOCKS_X * NUM_BLOCKS_Y];
    for (int i = 0; i < NUM_BLOCKS_X * NUM_BLOCKS_Y; i++) {
        blocks[i].x = (i % NUM_BLOCKS_X) * BLOCK_WIDTH;
        blocks[i].y = (i / NUM_BLOCKS_X) * BLOCK_HEIGHT;
        blocks[i].w = BLOCK_WIDTH;
        blocks[i].h = BLOCK_HEIGHT;
        blocks[i].active = 1;
    }

    int score = 0;
    int gameOver = 0;
    int startScreen = 1;

    SDL_Event e;
    int quit = 0;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
            if (e.type == SDL_KEYDOWN) {
                if (startScreen) {
                    startScreen = 0;
                }
                if (gameOver) {
                    if (e.key.keysym.sym == SDLK_r) {
                        resetGame(&paddle, &ball, blocks, &score);
                        gameOver = 0;
                    }
                }
            }
        }

        if (!startScreen && !gameOver) {
            const Uint8 *keystate = SDL_GetKeyboardState(NULL);
            if (keystate[SDL_SCANCODE_LEFT]) {
                movePaddle(&paddle, -1);
            }
            if (keystate[SDL_SCANCODE_RIGHT]) {
                movePaddle(&paddle, 1);
            }

            moveBall(&ball);
            checkBallPaddleCollision(&ball, &paddle);
            checkBallBlockCollision(&ball, blocks, &score);

            if (ball.y > WINDOW_HEIGHT) {
                gameOver = 1;
            }
        }

        render(renderer, font, &paddle, &ball, blocks, score, gameOver, startScreen);
        SDL_Delay(16);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
