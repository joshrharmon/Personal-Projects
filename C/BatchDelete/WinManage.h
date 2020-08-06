#ifndef WIN_MANAGE_H
#define WIN_MANAGE_H

typedef struct WIN_STRUCT {
    int rows, cols, topx, topy;
} win;

/* Function to quickly create windows and put a box around it */
WINDOW* initWin(int length, int width, int y, int x);

/* Print utility functions */
void printStatus(WINDOW *win, char *query, char *path, char *status);
void printPrompt(WINDOW *win, char *instr);
void winclear(WINDOW *win, int mode);

/* Function to check for valid input */
int inputHandler(const char *input, int type);

#endif

