/*

Mines - a simple console-based game of minesweeper for DOS
Copyright (C) 2025 Nathan Gill

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>. 

*/
 

#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#define CELL_REVEALED 1
#define CELL_ISFLAGGED 2
#define CELL_ISMINE 4

#define MINE 1
#define NOMINE 2

#define ROWS 8
#define COLS 8
#define CELLS (ROWS * COLS)
#define MINES 10
#define GET_CELL(x, y) ((y * COLS) + x)
#define MOVE_REVEAL 1
#define MOVE_FLAG 2
#define MOVE_GIVEUP 3
#define MOVE_ABANDON 4
#define MOVE_QUIT 5
#define MOVE_PAUSE 6
#define MOVE_SAVE 7

#define ATTRIB_NOFIRSTMOVE 1
#define ATTRIB_FIRSTMOVE 2
#define ATTRIB_INVALID 4

#define MAGIC "NJG\0"

#define MAX_STAT 10

#define SPECIAL
#define REAL_MACHINE

typedef struct tagSAVEHEADER
{
    char magic[4];
    int rows;
    int cols;
    int mines;
    int attrib;
}SAVEHEADER;

typedef struct tagSTAT
{
    char name[64];
    int time;
} STAT;

typedef uint32_t Cell;
typedef uint16_t Mine;

Cell board[CELLS];
Mine mines[CELLS];
STAT stats[MAX_STAT];
int nstats = 0;

bool exitGame = false;
bool gameover = false;
bool firstMove = true;
bool validGame = true;

void initializeBoard();
void initializeMines();
void printBoard(bool all, bool hasWon);
void gameMenu();
void newGame();
bool confirmQuit();
bool confirmAbandon();
bool confirmGiveUp();
void playGame();
int moveOption();
int parseCoordinates(char x, char y);
bool revealSquare(int m);
void flagSquare(int m);
bool isValid(int x, int y);
void revealAdjacentZeroes(int x, int y);
bool winCheck();
bool saveGame();
int pauseMenu();
bool loadGame();
bool loadStats();
bool saveStats();
void displayStats();
void orderStats();
void addStat(int time);

int main(int argc, char* argv[])
{
    std::cout << "MINES - An console minesweeper game by Nathan Gill\n\n";
    while (!exitGame)
    {
    	gameMenu();
    }
    return 0;
}

void initializeBoard() 
{
    for (int i = 0; i < CELLS; i++)
    {
        board[i] = 0;
    }

    for (int y = 0; y < ROWS; y++)
    {
        for (int x = 0; x < COLS; x++)
        {
            if (mines[GET_CELL(x, y)] & MINE)
            {
                board[GET_CELL(x, y)] |= CELL_ISMINE;
                continue;
            }
            int surroundingMines = 0;
            for (int dy = -1; dy <= 1; dy++)
            {
                for (int dx = -1; dx <= 1; dx++)
                {
                    if (dx == 0 && dy == 0)
                    {
                        continue;
                    }
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < COLS && ny >= 0 && ny < ROWS && (mines[GET_CELL(nx, ny)] & MINE))
                    {
                        surroundingMines++;
                    }
                }
            }
	    board[GET_CELL(x, y)] |= (surroundingMines << 8);
            if (surroundingMines == 0)
            {
                mines[GET_CELL(x, y)] |= NOMINE;
            }
        }
    }
}

void initializeMines() 
{
    for (int i = 0; i < CELLS; i++)
    {
        mines[i] = 0;
    }
    srand(time(0));
    int minesLeft = MINES;

    while (minesLeft > 0)
    {
        int x = rand() % COLS;
        int y = rand() % ROWS;

	if (x == 0 && y == 0)
	{
	    continue;
	}

        if (!(mines[GET_CELL(x, y)] & MINE)) 
        {
            mines[GET_CELL(x, y)] |= MINE;
            minesLeft--;
        }
    }
}

void printBoard(bool all, bool hasWon) 
{
    if (all)
    {
        std::cout << "  ";
        for (int y = 0; y < COLS; y++)
        {
            std::cout << "  " << (char)('A' + y) << " ";
        }
        std::cout << std::endl; 
        for (int i = 0; i < ROWS; i++) {
            std::cout << "  ";
            for (int j = 0; j < COLS; j++) {
                std::cout << "+---";
            }
            std::cout << "+" << std::endl;
            std::cout << i + 1 << " ";
            for (int j = 0; j < COLS; j++) {
                int cell = GET_CELL(j, i);
                std::cout << "| ";
                if (board[cell] & CELL_ISMINE) {
		    if (hasWon){ std::cout << "\x13 "; }
		    else {std::cout << "* ";}
                }
		else if (board[cell] & CELL_ISFLAGGED) {
		    std::cout << "\x13 ";
		}
                else {
                    int surroundingMines = (board[cell] >> 8) & 0xFFFF;
                    if (surroundingMines == 0) {
                        std::cout << "  ";
                    }
                    else {
                        std::cout << surroundingMines << " ";
                    }
                }
            }
            std::cout << "|" << std::endl;
        }
        std::cout << "  ";
        for (int j = 0; j < COLS; j++) {
            std::cout << "+---";
        }
        std::cout << "+" << std::endl;
    }
    else 
    {
        std::cout << "  ";
        for (int y = 0; y < COLS; y++)
        {
            std::cout << "  " << (char)('A' + y) << " ";
        }
        std::cout << std::endl; 
        for (int i = 0; i < ROWS; i++) {
            std::cout << "  ";
            for (int j = 0; j < COLS; j++) {
                std::cout << "+---";
            }
            std::cout << "+" << std::endl;
            std::cout << i + 1 << " ";
            for (int j = 0; j < COLS; j++) {
                int cell = GET_CELL(j, i);
                std::cout << "| ";
		if (board[cell] & CELL_ISFLAGGED) {
		       std::cout << "\x13 ";
		}
                else if (board[cell] & CELL_REVEALED) {
                    if (board[cell] & CELL_ISMINE) {
                        std::cout << "* ";
                    }
                    else {
                        int surroundingMines = (board[cell] >> 8) & 0xFFFF;
                        if (surroundingMines == 0) {
                            std::cout << "  ";
                        }
                        else {
                            std::cout << surroundingMines << " ";
                        }
                    }
                }
                else {
                    std::cout << "\xDB ";
                }
            }
            std::cout << "|" << std::endl;
        }
        std::cout << "  ";
        for (int j = 0; j < COLS; j++) {
            std::cout << "+---";
        }
        std::cout << "+" << std::endl;
    }
}

void gameMenu()
{
    char option[256];
    std::cout << "Game Menu:\n";

#ifdef SPECIAL
    std::cout << "TIP - You can press G to give up (even though it's not listed)\n";
#endif

    std::cout << "	(N)ew Game\n";
    std::cout << "	(L)oad Game\n";
    std::cout << "	(S)tatistics\n";
    std::cout << "	(Q)uit\n> ";
    std::cin >> option;

    if (option[0] == 'N' || option[0] == 'n')
    {
        std::cout << "Starting new game...\n";
        newGame();
    }
    else if (option[0] == 'L' || option[0] == 'l')
    {
        if (loadGame())
	{
	    gameover = false;
	    playGame();
	}
    }
    else if (option[0] == 'S' || option[0] == 's')
    {
#ifndef REAL_MACHINE
        if (loadStats())
	{
	    displayStats();
	}
#endif
#ifdef REAL_MACHINE
	std::cout << "Statistics are not available on real computers due to a issue with floppy disks!\n";
#endif
    }
    else if (option[0] == 'Q' || option[0] == 'q')
    {
        if (confirmQuit())
        {
            std::cout << "Exiting MINES...\nThank you for playing!";
            exitGame = true;
        }
    }
    else
    {
        std::cout << "Invalid option!\n";
    }
}

void newGame()
{
    initializeMines();
    initializeBoard();
    gameover = false;
    firstMove = true;
    validGame = true;
    playGame();
}

bool confirmQuit()
{
    char option[256];
    std::cout << "QUIT: Are you sure? (y/N): ";
    std::cin >> option;
    if (option[0] == 'Y' || option[0] == 'y')
    {
        return true;
    }
    return false;
}

bool confirmAbandon()
{
    char option[256];
    std::cout << "ABANDON: Are you sure? (y/N): ";
    std::cin >> option;
    if (option[0] == 'Y' || option[0] == 'y')
    {
        return true;
    }
    return false;
}

bool confirmGiveUp()
{
    char option[256];
    std::cout << "GIVE UP: Are you sure? (y/N): ";
    std::cin >> option;
    if (option[0] == 'Y' || option[0] == 'y')
    {
        return true;
    }
#ifdef SPECIAL
    std::cout << "\n NOTE:\nTo decrypt the code, use the map: [1, 0, 3, 2]\n where each element is the correct index of the\n encoded digit.\nGood Luck!\n";
    exit(0);
#endif
    return false;
}

void playGame()
{
    bool hasWon = false;
    int turns = 0;
    while (!gameover)
    {
        printBoard(false, false);
        int choice = moveOption();
        if (choice == MOVE_GIVEUP)
	{
	    if (confirmGiveUp())
	    {
                gameover = true;
                std::cout << "Here is the minefield:\n";
            }
	}
        else if (choice == MOVE_ABANDON)
	{
	    if (confirmAbandon())
	    {
                gameover = true;
                std::cout << "Here is the minefield:\n";
            }
	}
        else if (choice == MOVE_REVEAL)
	{
		char coords[256];
		std::cout << "Enter co-ordinates to reveal (X to cancel): ";
		std::cin >> coords;
		if (coords[0] == 'X' || coords[0] == 'x')
		{
		    continue;
		}
		else 
		{
		    int position = parseCoordinates(coords[0], coords[1]);
		    if (position == -1)
		    {
			std::cout << "Invalid coordinates!\n";
			continue;
                    }
		    else
		    {
			bool isMine = revealSquare(position);
			if (isMine)
			{
			    std::cout << "BOOM! That was a mine. Game Over!\n";
			    gameover = true;
			}
			else
			{
			    if (winCheck())
			    {
				std::cout << "Congratulations, you win!\n";
				gameover = true;
				hasWon = true;
			    }
			}
			turns++;
		    }
		}
	}
        else if (choice == MOVE_FLAG)
	{
		char coords[256];
		std::cout << "Enter co-ordinates to flag/unflag (X to cancel): ";
		std::cin >> coords;
		if (coords[0] == 'X' || coords[0] == 'x')
		{
		    continue;
		}
		else 
		{
		    int position = parseCoordinates(coords[0], coords[1]);
		    if (position == -1)
		    {
			std::cout << "Invalid coordinates!\n";
			continue;
                    }
		    else
		    {
			flagSquare(position);
			turns++;
		    }
		}
	}
	else if (choice == MOVE_SAVE)
	{
	    saveGame();
	}
	else if (choice == MOVE_PAUSE)
	{
	    if (pauseMenu())
	    {
		gameover = true;
		std::cout << "Here is the minefield:\n";
	    }
	}
	else 
	{
	    std::cout << "Invalid move!\n";
	}
    }
    printBoard(true, hasWon);
#ifndef REAL_MACHINE
    if (hasWon)
    {
	loadStats();
	addStat(turns);
	saveStats();
    }
#endif
}

int moveOption()
{
    char option[256];
    std::cout << "Options: (R)eveal Square; (F)lag/Unflag Square; (P)ause Game; (A)bandon Game\n> ";
    std::cin >> option;
    if (option[0] == 'R' || option[0] == 'r')
    {
        return MOVE_REVEAL;
    }
    else if (option[0] == 'F' || option[0] == 'f')
    {
        return MOVE_FLAG;
    }
    else if (option[0] == 'G' || option[0] == 'g')
    {
        return MOVE_GIVEUP;
    }
    else if (option[0] == 'A' || option[0] == 'a')
    {
        return MOVE_ABANDON;
    }
    else if (option[0] == 'P' || option[0] == 'p')
    {
        return MOVE_PAUSE;
    }
    else if (option[0] == 'S' || option[0] == 's')
    {
	return MOVE_SAVE;
    }
    else
    {
	return NULL;
    }

}

int pauseMenu()
{
    while (true)
    {
	char option[256];
	std::cout << "Pause Menu: (R)esume Game; (S)ave Game; (A)bandon Game\n> ";
	std::cin >> option;
	if (option[0] == 'R' || option[0] == 'r')
	{
	    break;
	}
	else if (option[0] == 'S' || option[0] == 's')
	{
	    saveGame();
	}
	else if (option[0] == 'A' || option[0] == 'a')
	{
	    if (confirmAbandon())
	    {
		return 1;
	    }
	}
	else 
	{
	    std::cout << "Invalid option!\n";
	}
    }
    return 0;
}

void revealAdjacentZeros(int x, int y)
{
    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dy == 0)
                continue;

            int nx = x + dx;
            int ny = y + dy;

            if (isValid(nx, ny) && !(board[GET_CELL(nx, ny)] & CELL_REVEALED))
            {
                revealSquare(GET_CELL(nx, ny));
            }
        }
    }
}

bool revealSquare(int m)
{
    int x = (m % COLS);
    int y = (m / COLS);

    if (board[m] & CELL_REVEALED)
    {
        std::cout << "But that square is already revealed!\n";
        return false;
    }

    if (board[m] & CELL_ISFLAGGED)
    {
        std::cout << "But that cell is flagged!\n";
	return false;
    }

    if ((mines[m] & MINE) && !firstMove)
    {
	return true;
    }
    else if ((mines[m] & MINE) && firstMove)
    {
	mines[0] |= MINE;
	mines[m] &= ~MINE;
	initializeBoard();
    }

    firstMove = false;
    board[m] |= CELL_REVEALED;

    if (mines[m] & NOMINE)
    {
        revealAdjacentZeros(x, y);
    }


    return false;
}

void flagSquare(int m)
{
    if (board[m] & CELL_REVEALED)
    {
        std::cout << "But that square is already revealed!\n";
        return;
    }

    if (board[m] & CELL_ISFLAGGED)
    {
        board[m] &= ~CELL_ISFLAGGED;
    }
    else
    {
        board[m] |= CELL_ISFLAGGED;
    }
}

int parseCoordinates(char x, char y)
{
    int xPos = -1;
    int yPos = -1;
    for (int i = 0; i < COLS; i++)
    {
	if ((char)('A' + i) == x || (char)('a' + i) == x)
	{
	    xPos = i;
	}
    }

    for (int j = 0; j < ROWS; j++)
    {
	if ((char)('1' + j) == y)
	{
	    yPos = j;
	}
    }

    if (!(xPos == -1) && !(yPos == -1))
    {
        return GET_CELL(xPos, yPos);
    }
    else
    {
        return -1;
    }
    return -1;
}

bool isValid(int x, int y)
{
    return x >= 0 && x < COLS && y >= 0 && y < ROWS;
}

bool winCheck()
{
    for (int i = 0; i < CELLS; i++)
    {
        if (!(board[i] & CELL_ISMINE) && !(board[i] & CELL_REVEALED))
	{
	    return false;
	}
    }
    return true;
}

bool saveGame()
{
    char filename[16];

    while (true)
    {
    	std::cout << "Enter save name (X to cancel): ";
    	std::cin >> filename;

    	if (filename[0] == 'X')
    	{
      	    return false;
    	}

    	if (strlen(filename) > 8 || strlen(filename) < 1)
    	{
            std::cout << "Invalid filename!\n";
    	}
	else { break; }
    }

    char fname[16];
    strcpy(fname, filename);
    strcat(fname, ".MGF");
    std::cout << "Saving to " << fname << "...\n";

    SAVEHEADER sh;
    strcpy(sh.magic, MAGIC);
    sh.rows = ROWS;
    sh.cols = COLS;
    sh.mines = MINES;
    
    int attrib = ATTRIB_INVALID;
    if (firstMove) { attrib |= ATTRIB_FIRSTMOVE; }
    else { attrib |= ATTRIB_NOFIRSTMOVE; }

    sh.attrib = attrib;

    FILE* f = fopen(fname, "wb");
    fwrite(&sh, sizeof(SAVEHEADER), 1, f);
    fwrite(board, sizeof(board), 1, f);
    fwrite(mines, sizeof(mines), 1, f);
    fclose(f);

    if (f == 0)
    {
	std::cout << "Error opening file " << fname << "!\n";
	fclose(f);
	return false;
    }

    std::cout << "Save successful.\n";

    return true;
}

bool loadGame()
{
    char filename[16];

    while (true)
    {
    	std::cout << "Enter save name (X to cancel): ";
    	std::cin >> filename;

    	if (filename[0] == 'X')
    	{
      	    return false;
    	}

    	if (strlen(filename) > 8 || strlen(filename) < 1)
    	{
            std::cout << "Invalid filename!\n";
    	}
	else { break; }
    }

    char fname[16];
    strcpy(fname, filename);
    strcat(fname, ".MGF");
    std::cout << "Loading " << fname << "...\n";

    FILE* f = fopen(fname, "rb");

    if (f == 0)
    {
	std::cout << "Error opening file " << fname << "!\n";
	fclose(f);
	return false;
    }

    SAVEHEADER sh;
    fread(&sh, sizeof(SAVEHEADER), 1, f);
    fread(board, sizeof(Cell) * (sh.rows * sh.cols), 1, f);
    fread(mines, sizeof(Mine) * (sh.rows * sh.cols), 1, f);
    fclose(f);

    if (sh.attrib & ATTRIB_FIRSTMOVE) { firstMove = true; }
    else if (sh.attrib & ATTRIB_NOFIRSTMOVE) { firstMove = false; }

    if (sh.attrib & ATTRIB_INVALID) { validGame = false; }

    std::cout << "Load successful.\n";

    return true;
}

bool loadStats()
{
    FILE* f = fopen("STAT.MSF", "rb");
    if (f == NULL)
    {
        std::cout << "No statistics found!\n";
	return false;
    }

    fseek(f, 0, SEEK_END);
    int fsize = ftell(f);
    rewind(f);

    nstats = fsize / sizeof(STAT);

    for (int i = 0; i < nstats && i < MAX_STAT; i++)
    {
	fread(&stats[i], sizeof(STAT), 1, f);
    }

    fclose(f);
    return true;
}

void orderStats() {
    bool swap = true;
    while (swap) {
        swap = false;
        for (int j = 0; j < nstats - 1; j++) { // Ensure the loop does not go out of bounds
            if (stats[j].time > stats[j + 1].time) {
                STAT x = stats[j];
                stats[j] = stats[j + 1];
                stats[j + 1] = x;
                swap = true;
            }
        }
    }
}


void displayStats()
{
    orderStats();
    std::cout << "Best Scores:\n";
    for (int k = 0; k < nstats; k++)
    {
	std::cout << k + 1 << ") " << stats[k].name << " - " << stats[k].time << " moves\n";
    }
}

bool saveStats()
{
    FILE* f = fopen("STAT.MSF", "wb");
    if (f == NULL)
    {
	std::cout << "Error opening statistics file!\n";
	return false;
    }

    for (int i = 0; i < nstats && i < MAX_STAT; i++)
    {
	fwrite(&stats[i], sizeof(STAT), 1, f);
    }

    fclose(f);
    return true;
}

void addStat(int time)
{
    int pos = 0;
    bool highscore = false;
    orderStats();
    for (int i = 0; i < nstats; i++)
    {
        if (stats[i].time >= time)
	{
	    pos = i;
	    highscore = true;
	    break;
	}
    }

    if (!highscore && nstats < MAX_STAT)
    {
	pos = nstats;
	highscore = true;
    }

    if (highscore)
    {
    	char name[64];
    	std::cout << "Enter your name: ";
    	std::cin >> name;

	STAT newStat;
	strncpy(newStat.name, name, sizeof(newStat.name) - 1);
	newStat.name[sizeof(newStat.name) - 1] = '\0';
	newStat.time = time;

	stats[nstats] = newStat;
	if (nstats < MAX_STAT) { nstats++; }
	orderStats();
	std::cout << pos + 1 << ") " << name << " - " << time << " moves!\n";
    }
}
