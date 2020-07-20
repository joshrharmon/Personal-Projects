#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <curses.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <dirent.h>
#include <fnmatch.h>
#include "WinManage.h"
#include "BatchLogic.h"

struct searchStruct {
	int initSize, currSize;
	char **results, **resultsExt, *query, *path;
};

/* Variables used for field input and other diagnostic displays */
char *path, *query, *status, prompt[100], input[75], filename[30], buffer[1024];

/* Function used to check if path is valid */
int validPath(const char *path);

/* History functions */
void updateHist(WINDOW *hist, char *historyUpdate, int fd); 

/* Function to recursively search for files */
void fileSearch(struct searchStruct* search, char *path);

/* Function to list results of search in inner_res_win */
void listResults(struct searchStruct* search, WINDOW *res);

int main(int argc, char *argv[]) {
	/* Init search struct */
	struct searchStruct* search = (struct searchStruct*)malloc(sizeof(struct searchStruct));
	
	/* Init variables */
	int maxY, maxX;
	
	/* Declare windows */
	//WINDOW *status_win, *query_win, *res_win, *hist_win;
	
    /* Exit if incorrect execution is used */
    if(argc != 2) {
        printf("Usage: ./BatchDelete [Path to search]\n");
        exit(1);
    }

    /* Copy input supplied as the path */
	asprintf(&path, argv[1]);
	
	/* Initialize struct */
	search->initSize = 20;
	search->currSize = 0;
	search->results = (char**)malloc(sizeof(char*) * search->initSize);
	search->resultsExt = NULL;
	search->path = strdup(path);

    /* Verify path is valid before proceeding */
    if(!validPath(path)) {
        printf("Path '%s' invalid, exiting...\n", path);
        exit(1);
    }

    /* Load history file */
    snprintf(filename, sizeof filename, "history.txt");
    int fdout = open(filename, O_RDWR | O_CREAT | O_APPEND, 0666);
	
	/* Open for reading */
	//FILE *histFile = fopen("history.txt", "r");

    /* If file creation succeeds continue, else exit */
    if(fdout < 0) {
        printf("History file creation failed, extiting...\n");
        exit(-1);
    }

    /* Load initial prompt and status   */
    strcpy(prompt, "Type in a search query");
    asprintf(&status, "Awaiting query...");

    /* Start ncurses session */
    initscr();
    getmaxyx(stdscr, maxY, maxX);
    move(maxY - 2, 2);
    refresh();

    /* Check if terminal supports colors and terminal is appropriate size */
    if(has_colors() == FALSE) {
        endwin();
        printf("Terminal does not support ncurses colors. Exiting...\n");
        exit(1);
    }
    /*else if(maxX < 80 || maxY < 40) {
        endwin();
        printf("Terminal too small, resize to run. Exiting...\n");
        exit(1);
    }   */

    start_color();

    /* Initialize windows */
	WINDOW *header_win = newwin(1, maxX, 0, 0);
    WINDOW *status_win = initWin(5, maxX, 1, 0);
    WINDOW *query_win = initWin(3, maxX, maxY - 3, 0);
    WINDOW *res_win = initWin(maxY - 9, maxX/2 - 1, 6, 0);
	WINDOW *inner_res_win = newwin(maxY - 11, maxX/2 - 4, 7, 1);
    WINDOW *hist_win = initWin(maxY - 9, maxX/2 + 1, 6, (COLS/2) - 1);
	WINDOW *inner_hist_win = newwin(maxY - 11, maxX/2 - 1, 7, COLS/2);
	scrollok(inner_hist_win, true);
	scrollok(inner_res_win, true);

    /* Initialize colors */
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_CYAN, COLOR_BLACK);

    /* Print header message */
	char exitMSG[] = "Enter 'q' to exit";
    wattron(header_win, A_BOLD | COLOR_PAIR(3));
	mvwprintw(header_win, 0, 1, "BatchDelete, a program by Joshua Harmon");
    mvwprintw(header_win, 0, maxX - strlen(exitMSG), "%s", exitMSG);
    wattroff(header_win, A_BOLD | COLOR_PAIR(3));

	/* Print status, history, results, prompt windows */
	wattron(status_win, A_BOLD | COLOR_PAIR(3));
	wattron(hist_win, A_BOLD | COLOR_PAIR(3));
	wattron(res_win, A_BOLD | COLOR_PAIR(3));
	wattron(query_win, A_BOLD | COLOR_PAIR(3));
	
	mvwprintw(status_win, 0, 2, "[ Status ]");
	mvwprintw(hist_win, 0, 2, "[ History ]");
	mvwprintw(res_win, 0, 2, "[ Results ]");
	printPrompt(query_win, prompt);
	
	wattroff(query_win, A_BOLD | COLOR_PAIR(3));
	wattroff(res_win, A_BOLD | COLOR_PAIR(3));
	wattroff(hist_win, A_BOLD | COLOR_PAIR(3));
	wattroff(status_win, A_BOLD | COLOR_PAIR(3));
    printStatus(status_win, query, path, status);

	updateHist(inner_hist_win, "BatchDelete started successfully.\n", fdout);
	
	wnoutrefresh(header_win);
    wnoutrefresh(query_win);
    wnoutrefresh(res_win);
    wnoutrefresh(status_win);
    wnoutrefresh(hist_win);
	wnoutrefresh(inner_res_win);
	wnoutrefresh(inner_hist_win);
    doupdate();

    /* While user has not entered "q", continue running */
    while(input[0] != 'q') {
        if(mvgetstr(maxY - 2, 2, input) != ERR && inputHandler(input, 0)) {
			winclear(inner_res_win);
			search->query = strdup(input);
			fileSearch(search, path);
			listResults(search, inner_res_win);
			asprintf(&query, input);
            asprintf(&status, "Found %d results searching '%s'\n", search->currSize, input);
			updateHist(inner_hist_win, status, fdout);
        } else {
            sprintf(prompt, "Invalid input, try again.");
            printPrompt(query_win, prompt);
        }
		printStatus(status_win, query, path, status);
		wnoutrefresh(status_win);
		wnoutrefresh(query_win);
		wnoutrefresh(inner_hist_win);
		wnoutrefresh(inner_res_win);
		winclear(status_win);
		winclear(query_win);
        doupdate();
    }

	/* User quit program */
	updateHist(inner_hist_win, "User quit successfully.\n", fdout);
	
    /* Free all memory */
	for(int i = 0; i < search->currSize; i++) {
		free(search->results[i]);
	}
	free(search->results);
	free(search->path);
	
	free(status);
	free(query);
	free(path);
	
    delwin(status_win);
    delwin(query_win);
    delwin(hist_win);
	delwin(inner_hist_win);
    delwin(res_win);
	delwin(inner_res_win);
	delwin(header_win);
    endwin();
	close(fdout);
    return 0;
}

void updateHist(WINDOW *hist, char *historyUpdate, int fd) {
	time_t timer = time(NULL);
    struct tm* tm_info = localtime(&timer);
	ssize_t bwritten;
	size_t timeSize;
	timeSize = strftime(buffer, sizeof(buffer), "[%m/%d/%Y, %I:%M:%S %p] ", tm_info);
	strcat(buffer, historyUpdate);
	if((bwritten = write(fd, buffer, (timeSize + strlen(historyUpdate)))) < 0) {
		perror("Could not write to the history file, exiting...");
		exit(1);
	}
	scroll(hist);
	int lastChar = strlen(buffer);
	if(lastChar > 0 && buffer[lastChar - 1] == '\n') {
		buffer[lastChar - 1] = 0;
	}
	mvwprintw(hist, LINES - 12, 0, buffer);
}

void listResults(struct searchStruct* search, WINDOW *res) {
	for(int i = 0; i < search->currSize; i++) {
		char *pathList;
		asprintf(&pathList, "[ ] %s\n", search->results[i]);
		scroll(res);
		mvwprintw(res, LINES - 12, 0, pathList);
		free(pathList);
	}
}

void fileSearch(struct searchStruct* search, char *path) {
	DIR *sourcePath = opendir(path);
	if(sourcePath == NULL) {
		perror("Failed to open directory.");
	}
	char *wildQuery;
	asprintf(&wildQuery, "*%s*", search->query);		/* Add wildcards on either end of query for matching */
	struct dirent *fileObj;
	while((fileObj = readdir(sourcePath)) != NULL) {
		int pathLen = strlen(path);
		if(path[pathLen - 1] != '/') {			/* If there is no trailing slash at the end of path, add it. */
			strcat(path, "/");
		}
		char *goPath;
		asprintf(&goPath, "%s%s", path, fileObj->d_name);
		if(fileObj->d_type == DT_DIR) {
			if(strcmp(fileObj->d_name, ".") == 0 || strcmp(fileObj->d_name, "..") == 0) {
				continue;
			} 
			fileSearch(search, goPath);
		}
		else if(fnmatch(wildQuery, fileObj->d_name, FNM_CASEFOLD) == 0) {
			if(search->currSize == search->initSize) {
				search->initSize += 20;
				search->resultsExt = (char**)realloc(search->results, (sizeof(char*) * search->initSize));
				if(search->resultsExt != NULL) {
					search->results = search->resultsExt;
				}
				else {
					perror("Memory extension for char* array failed, exiting...");
					exit(-1);
				}
			}
			search->results[search->currSize] = strdup(goPath);
			search->currSize++;
		}
		free(goPath);
	}
	free(wildQuery);
	closedir(sourcePath);
}

int validPath(const char *path) {
    struct stat stats;
    stat(path, &stats);
    if(S_ISDIR(stats.st_mode))
        return 1;
    return 0;
}

