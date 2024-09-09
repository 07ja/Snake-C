#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/select.h>
#include "ANSI-color-codes.h"

#define ROWS 22
#define COLS 30

typedef struct Body {
    int x, y;
    struct Body *next;
} Body;

Body *snake = NULL;


void clear();
int collision();
int consume(int applex, int appley);
void debug();
void grow();
void renderApple(int applex, int appley);
void renderBackground();
void renderSnake();
void updateSnake(int direction);
int userInput();



void clear() {
    printf("\e[2J");    // Clear Screen
    printf("\e[H");     // Move Cursor to Top Left
    return;
}


int collision() {
    if (snake == NULL) {
        return 1;
    }
    if (snake->x <= 0 || snake->x >= COLS - 1 || snake->y <= 0 || snake->y >= ROWS - 1) {
        return 1;
    }

    // Checking if Snake Collides With Itself

    /*Body *current = snake->next;
    while (current != NULL) {
        if (snake->x == current->x && snake->y == current->y) {
            printf("collision\n");
            return 1;
        }
        current = current->next;
    }*/

    return 0;
}


int consume(int applex, int appley) {
    return (snake->x == applex && snake->y == appley);
}


void debug() {
    Body *current = snake;
    while (current != NULL) {
        printf("Current (%d, %d)\n", current->x, current->y);
        //printf("Snake (%d, %d)\n", snake->x, snake->y);
        current = current->next;
    }

    // Debugging Apple Positioning
    // printf("apple %d %d\n", applex, appley);
    // printf("apple render %d %d\n", appley + 1, applex * 2 + 1);
    // printf("snake %d %d\n", snake->x, snake->y);
}


void grow() {
    Body *newBody = malloc(sizeof(Body));

    newBody->next = NULL;

    Body *current = snake;
    while (current->next != NULL) {
        current = current->next;
    }

    newBody->x = current->x;
    newBody->y = current->y;
    newBody->next = NULL;

    current->next = newBody;
}


void renderApple(int applex, int appley) {
    printf("\e[%d;%dH", appley + 1, applex * 2 + 1);
    printf(REDB" " reset);
    return;
}


void renderBackground() {
    printf("\e[?25l");    // Hide cursor
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (i == 0 || i == ROWS - 1 || j == 0 || j == COLS - 1) {
                printf(WHTB"  " reset);    // White Border
            }
            else {
                printf("  ");    // Blank Spaces
            }
        }            
        printf("\n");
    }
    return;
}


void renderSnake() {
    Body *current = snake;
    while (current != NULL) {
        printf("\e[%d;%dH", current->y + 1, current->x * 2 + 1);
        printf(GRNB"  " reset);
        current = current->next;
    }
    return;
}



void updateSnake(int direction) {
    int prevx = snake->x;
    int prevy = snake->y;

    Body *current = snake;
    
    while (current->next != NULL) {
        Body *next = current->next;
        int tempX = next->x;
        int tempY = next->y;
        next->x = prevx;
        next->y = prevy;
        prevx = tempX;
        prevy = tempY;
        current = next;

    }

    switch (direction) {
        case 'w': snake->y--; break;
        case 'a': snake->x--; break;
        case 's': snake->y++; break;
        case 'd': snake->x++; break;
    }
    return;
}

int userInput() {
    struct termios oldTerminal, newTerminal;
    int keyPress;
    tcgetattr(STDIN_FILENO, &oldTerminal);    // Current Terminal Settings
    newTerminal = oldTerminal;
    newTerminal.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newTerminal);    // Disable Canonical Mode and Echo

    fd_set readfds;
    struct timeval timeout;
    int result;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    result = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);

    if (result > 0 && FD_ISSET(STDIN_FILENO, &readfds)) {
        keyPress = getchar();
    } 
    else {
        keyPress = -1;
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldTerminal);    // Restore Old Terminal Settings
    return keyPress;
}




int main() {
    int score = 0;
    srand(time(0));

    printf("\n");
    printf("Welcome to snake in c!\n");

    char play;
    printf("Press Y or N to play! ");
    do {
        scanf(" %c", &play);
    }
    while(play != 'N' && play != 'Y');

    if( play == 'N') {
        printf("\nThanks for playing!\n");
    }
    
    if(play == 'Y') {
        int quit = 0;

        int applex = (rand() % (COLS - 2)) + 1;
        int appley = (rand() % (ROWS - 2)) + 1;

        int direction = 'd';

        for (int i = 0; i < 3; i++) {    // 3 is the Snakes Length
            Body *newBody = malloc(sizeof(Body));
            newBody->x = 3 - i;
            newBody->y = 1;
            newBody->next = snake;
            snake = newBody;
        }

        //Main Game Loop
        while (!quit) {
            int input = userInput();
            if (input != -1) {
                direction = input;
            }

            clear();
            renderBackground();
            renderSnake();
            renderApple(applex, appley);
            fflush(stdout);
            // debug();

            updateSnake(direction);

            if (consume(applex, appley)) {
                grow();
                applex = (rand() % (COLS - 2)) + 1;
                appley = (rand() % (ROWS - 2)) + 1;
                score = score + 10;
            }
            
            quit = collision();

            usleep(50000);    
        }
    }
    
    clear();
    renderBackground();

    printf("\n");
    printf(URED"Game over!\n" reset);
    printf(HCYN"TOTAL SCORE: %d\n" reset, score);
    printf("\n");

    // debug();

    printf("\e[?25h");    // Show Cursor

    Body *current = snake;
    while (current != NULL) {
        Body *next = current -> next;
        free(current);
        current = next;
    }
    
    return 0;
}

