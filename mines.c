#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

// #define DEBUG

static int WIDTH = 8;
static int HEIGHT = 8;
static int MINES = 10;
static unsigned long START = 0;

#define FFLAG 16
#define FREVEAL 32
#define FMINE 64

#define CUROFF_H(x) (1 + (x))
#define CUROFF_V(y) (2 + (y))

typedef unsigned char cell;

int
countMines(cell board[HEIGHT][WIDTH], int x, int y)
{
    int c = 0;
    for (int nx = x - 1; nx <= x + 1; ++nx) {
        for (int ny = y - 1; ny <= y + 1; ++ny) {
            if (nx < 0 || nx >= WIDTH || ny < 0 || ny >= HEIGHT ||
                (nx == x && ny == y))
                continue;
            if (board[ny][nx] & FMINE)
                c++;
        }
    }
    return c;
}

int
countFlags(cell board[HEIGHT][WIDTH], int x, int y)
{
    int c = 0;
    for (int nx = x - 1; nx <= x + 1; ++nx) {
        for (int ny = y - 1; ny <= y + 1; ++ny) {
            if (nx < 0 || nx >= WIDTH || ny < 0 || ny >= HEIGHT ||
                (nx == x && ny == y) || board[ny][nx] & FREVEAL)
                continue;
            if (board[ny][nx] & FFLAG)
                c++;
        }
    }
    return c;
}

void
initBoard(cell board[HEIGHT][WIDTH])
{
    for (int y = 0; y < HEIGHT; ++y)
        for (int x = 0; x < WIDTH; ++x)
            board[y][x] = 0;
}

void
createBoard(cell board[HEIGHT][WIDTH], int ix, int iy)
{
    int n = 0;
    int mines = (MINES > WIDTH * HEIGHT) ? (WIDTH * HEIGHT) : MINES;
    while (n < mines) {
        int x = rand() % WIDTH;
        int y = rand() % HEIGHT;

        if (!(board[y][x] & FMINE) && !(x == ix && x == iy)) {
            board[y][x] |= FMINE;
            n++;
        }
    }

    for (int y = 0; y < HEIGHT; ++y)
        for (int x = 0; x < WIDTH; ++x)
            board[y][x] =
                (board[y][x] & 0xF0) | (countMines(board, x, y) & 0x0F);
}

int
revealCell(cell board[HEIGHT][WIDTH], int x, int y, bool allowChord)
{
    int r = 0;
    int count = board[y][x] & 0xF;

    if (board[y][x] & FREVEAL) {
        if (!allowChord)
            return 0;
        if (count == 0)
            return 0;
        if (count != countFlags(board, x, y))
            return 0;
    } else {
        board[y][x] |= FREVEAL;
        if (board[y][x] & FMINE)
            return -1;
        r++;
        if (count > 0)
            return r;
    }

    for (int nx = x - 1; nx <= x + 1; ++nx) {
        for (int ny = y - 1; ny <= y + 1; ++ny) {
            if (nx < 0 || nx >= WIDTH || ny < 0 || ny >= HEIGHT ||
                (nx == x && ny == y) || board[ny][nx] & FFLAG)
                continue;

            int nr = revealCell(board, nx, ny, false);
            if (nr == -1)
                return -1;
            else
                r += nr;
        }
    }

    return r;
}

void
revealAll(cell board[HEIGHT][WIDTH])
{
    for (int y = 0; y < HEIGHT; ++y)
        for (int x = 0; x < WIDTH; ++x)
            if (!((board[y][x] & FFLAG) && (board[y][x] & FMINE)))
                board[y][x] |= FREVEAL;
}

void
drawCell(char c)
{
    switch (c) {
        case '.':
            printf("\033[1;30m.");
            break;
        case '1':
            printf("\033[34m1");
            break;
        case '2':
            printf("\033[32m2");
            break;
        case '3':
            printf("\033[31m3");
            break;
        case '4':
            printf("\033[36m4");
            break;
        case '5':
            printf("\033[35m5");
            break;
        case '6':
            printf("\033[33m6");
            break;
        case '7':
            printf("\033[37m7");
            break;
        case '8':
            printf("\033[1;37m8");
            break;
        case 'F':
            printf("\033[1;33mF");
            break;
        case 'X':
            printf("\033[1;31mX");
            break;
        default:
            putchar(c);
            return;
    }
    printf("\033[0m");
}

void
drawBoard(cell board[HEIGHT][WIDTH])
{
    printf("\033[%u;%uH", CUROFF_V(0), CUROFF_H(0));

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            short n = board[y][x] & 0xF;

            if (board[y][x] & FREVEAL) {
                if (board[y][x] & FMINE)
                    drawCell('X');
                else if (n == 0)
                    drawCell('.');
                else
                    drawCell((char)n + '0');
            } else if (board[y][x] & FFLAG) {
                drawCell('F');
            } else {
                drawCell('?');
            }
        }
        putchar('\n');
    }
}

void
clearMessage(int sel_x, int sel_y)
{
    printf("\033[%u;%uH\033[2K\033[%u;%uH", CUROFF_V(HEIGHT), 1,
           CUROFF_V(sel_y), CUROFF_H(sel_x));
}

void
drawStatusLine()
{
    printf("\033[H");
    printf("%lu", time(NULL) - START);
}

void
drawMessage(const char *msg, int sel_x, int sel_y)
{
    int len = strlen(msg);
    printf("\033[%u;%uH", CUROFF_V(HEIGHT), CUROFF_H((WIDTH - len) / 2));
    printf("%s", msg);
    printf("\033[%u;%uH", CUROFF_V(sel_y), CUROFF_H(sel_x));
}

void
draw(cell board[HEIGHT][WIDTH], int sel_x, int sel_y)
{
    drawStatusLine();
    drawBoard(board);

#ifdef DEBUG
    printf("\033[2K");
    if (board[sel_y][sel_x] & FMINE)
        printf("FMINE ");
    if (board[sel_y][sel_x] & FFLAG)
        printf("FFLAG ");
    if (board[sel_y][sel_x] & FREVEAL)
        printf("FREVEAL");
    putchar('\n');
#endif

    printf("\033[%u;%uH", CUROFF_V(sel_y), CUROFF_H(sel_x));
}

void
setupTerm(bool enable)
{
    static bool enabled = false;
    static struct termios old = {};
    struct termios new = {};

    if (enable && !enabled) {
        tcgetattr(STDIN_FILENO, &old);
        new = old;
        new.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &new);
        printf("\033[2J");
        enabled = true;
    } else if (!enable && enabled) {
        printf("\033[m");
        tcsetattr(STDIN_FILENO, TCSANOW, &old);
        enabled = false;
    }
}

void
signal_callback(int signum)
{
    printf("\033[%u;%uH", CUROFF_V(HEIGHT), 1);
    setupTerm(false);
    exit(signum);
}

int
main(int argc, char *argv[])
{
    if (argc > 1) {
        char *s = argv[1];
        unsigned short x = 0;
        char *dim[3] = {s, NULL, NULL};
        while (*s) {
            if (*s == 'X' || *s == 'x') {
                x++;
                *s = '\0';

                if (x < 3)
                    dim[x] = s + 1;
            }
            s++;
        }

        if (*dim[0])
            WIDTH = abs(atoi(dim[0]));
        if (*dim[1])
            HEIGHT = abs(atoi(dim[1]));
        if (*dim[2])
            MINES = abs(atoi(dim[2]));
    }

    srand((START = time(NULL)));
    signal(SIGINT, signal_callback);

    bool exit = false, boardCreated = false;
    int status = 0, sel_x = 0, sel_y = 0, left = WIDTH * HEIGHT - MINES, r;

    cell board[HEIGHT][WIDTH];
    initBoard(board);

    setupTerm(true);

    while (!exit) {
        draw(board, sel_x, sel_y);

        char c = getchar();
        if (c == EOF)
            break;

        int dx = 0, dy = 0;
        switch (c) {
            // nw, n, ne
            case 'y':
                dx = -1;
                dy = -1;
                break;
            case 'k':
                dy = -1;
                break;
            case 'u':
                dx = 1;
                dy = -1;
                break;
            case 'K':
                dy = -sel_y;
                break;

            // w, e
            case 'h':
                dx = -1;
                break;
            case 'l':
                dx = 1;
                break;
            case 'H':
                dx = -sel_x;
                break;
            case 'L':
                dx = WIDTH - sel_x - 1;
                break;

            // sw, s, se
            case 'b':
                dx = -1;
                dy = 1;
                break;
            case 'j':
                dy = 1;
                break;
            case 'n':
                dx = 1;
                dy = 1;
                break;
            case 'J':
                dy = HEIGHT - sel_y - 1;
                break;

            // arrow keys
            case '\033':
                if (getchar() == '[') {
                    switch (getchar()) {
                        case 'A':
                            dy = -1;
                            break;
                        case 'B':
                            dy = 1;
                            break;
                        case 'C':
                            dx = 1;
                            break;
                        case 'D':
                            dx = -1;
                            break;
                    }
                }
                break;

            // reveal
            case ' ':
                if (!boardCreated) {
                    // set up mines on first move
                    createBoard(board, sel_x, sel_y);
                    boardCreated = true;
                }
                if ((r = revealCell(board, sel_x, sel_y, true)) == -1) {
                    exit = true;
                    status = 1;
                }
                left -= r;
                break;

            // flag
            case 'f':
            case 'F':
                board[sel_y][sel_x] ^= FFLAG;
                break;

            // quit
            case 'q':
                drawMessage("QUIT (y/N)", sel_x, sel_y);
                c = getchar();
                clearMessage(sel_x, sel_y);
                if (c == 'y' || c == 'Y') {
                    exit = true;
                    status = 2;
                }
                break;
        }

        int nx = sel_x + dx;
        int ny = sel_y + dy;
        if (nx >= 0 && nx < WIDTH)
            sel_x = nx;
        if (ny >= 0 && ny < HEIGHT)
            sel_y = ny;

        if (left <= 0)
            exit = true;
    }

    revealAll(board);
    draw(board, sel_x, sel_y);

    switch (status) {
        case 0:
            drawMessage("WIN", 0, HEIGHT + 1);
            break;
        case 1:
            drawMessage("LOSE", 0, HEIGHT + 1);
            break;
        case 2:
            printf("\033[%u;%uH", CUROFF_V(HEIGHT), 1);
            break;
    }

    setupTerm(false);
    return 0;
}
