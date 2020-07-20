#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <sys/stat.h>
#include <form.h>
#include "WinManage.h"

void printStatus(WINDOW *win, char *query, char *path, char *status) {
    mvwprintw(win, 1, 2, "Path   -> '%s'", path);
    mvwprintw(win, 2, 2, "Query  -> '%s'", query); 
    mvwprintw(win, 3, 2, "Status -> %s", status);  
}

void printPrompt(WINDOW *win, char *prompt) {
    wattron(win, A_BOLD | COLOR_PAIR(3));
    mvwprintw(win, 0, 2, "[ Prompt ]");
    mvwprintw(win, 0, 13, "[ Instruction: %s ]", prompt);
    wattroff(win, A_BOLD | COLOR_PAIR(3));
}

WINDOW* initWin(int length, int width, int y, int x) {
    WINDOW *temp = newwin(length, width, y, x);
    box(temp, 0, 0);
    return temp;
}

void winclear(WINDOW *win) {
	for(int i = 1; i < win->_maxy; i++) {
		for(int j = 1; j < win->_maxx; j++) {
			mvwprintw(win, i, j, " ");
		}
	}
}

/* Will handle all error-checking for user input. 1 is valid, 0 is invalid. */
int inputHandler(const char *win_input, int type) {
    switch(type) {
        /*
         * Normal text case:
         * - Allowed: all characters, nums, symbols
         * - Disallowed: blank
         */
        case 0:
            return(win_input[0] != '\n' && win_input[0] != 0);
            break;
            
		/*
		 * Y/N Binary text case:
		 * - Allowed: Y, y, N, n
		 * - Disallowed: All else
		 */
		case 1:
			return strchr("YyNn", win_input[0]) != NULL ? 1 : 0;
			break;
    }
    return 0;
}

