#include "repl_private.h"
#include <assert.h>

static inline int tcols(void) {
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
		return -1;
	else
		return csbi.srWindow.Right - csbi.srWindow.Left + 1; // Window width
		// return csbi.dwSize.X; // Buffer width
#else
#ifdef TIOCGSIZE
	struct ttysize ts;
	ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
	return ts.ts_cols;
#elif defined(TIOCGWINSZ)
	struct winsize ts;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
	return ts.ws_col;
#else // TIOCGSIZE
	return -1;
#endif // TIOCGSIZE
#endif // _WIN32
}


void replStart(REPL* repl) {
    repl->cx = 0;
    repl->cy = 0;
    repl->linesCount = 1;
    repl->lines[0] = (LineInfo){0, 0};
    stringInit(&repl->buffer);
}

static void computeOffsets(REPL* repl, int start) {
    int offset = repl->lines[start].offset;
    for(int i = start+1; i < repl->linesCount; ++i) {
        repl->lines[i].offset = offset;
        offset += repl->lines[i].length + 1; // +1 for \n
    }
}

static inline void setWidth(REPL* repl, int width) {
    repl->cx = width;
    repl->lines[repl->cy].length = width;
}

static inline int llength(REPL* repl) {
    return repl->lines[repl->cy].length;
}

static inline int loffset(REPL* repl) {
    return repl->lines[repl->cy].offset;
}

//
// void printLine(REPL* repl) {
//     printf("%.*s", llength(repl), repl->buffer.data + loffset(repl));
// }

static inline int min(int a, int b) {
    return a < b ? a : b;
}

static inline int max(int a, int b) {
    return a > b ? a : b;
}

static inline void writeBuffer(REPL* repl, char c) {
    stringInsert(&repl->buffer, repl->cx + loffset(repl), c);
    // printLine(repl);
}

static void setX(REPL* repl, int x) {
    if(x > llength(repl)) x = llength(repl);
    printf("\033[%dD", repl->cx);
    if(x > 0) printf("\033[%dC", x);
    repl->cx = x;
}

static inline void capX(REPL* repl) {
    setX(repl, repl->cx);
}

bool moveUp(REPL* repl, int count) {
    if(repl->cy <= 0) return false;
    printf("\033[%dA", count);
    repl->cy -= 1;
    
    capX(repl);
    return true;
}

bool moveDown(REPL* repl, int count) {
    if(repl->cy >= repl->linesCount-1) return false;
    printf("\033[%dB", count);
    repl->cy += 1;
    
    capX(repl);
    return true;
}

static void debug(REPL* repl) {
    printf("\033[%dD\033[%dB", repl->cx, 20 - repl->cy);
    int len = printf("(%d, %d) -> %d bytes [c%d]", repl->cx, repl->cy, repl->buffer.count, tcols());
    printf("\033[%dD", len);
    printf("\033[%dC\033[%dA", repl->cx, 20 - repl->cy);
}

void moveLeft(REPL* repl, int count) {
    int target = max(0, repl->cx - count);
    count = repl->cx - target;
    
    int cols = tcols();
    int dx = count % cols;
    int dy = count / cols;
    
    debug(repl);
    
    if(dx > 0) printf("\033[%dD", dx);
    if(dy > 0) printf("\033[%dA", dy);
    
    repl->cx -= count;
}

void moveRight(REPL* repl, int count) {
    int target = min(llength(repl), repl->cx + count);
    count = target - repl->cx;
    
    int cols = tcols();
    int dx = count % cols;
    int dy = count / cols;
    
    debug(repl);
    
    if(dx > 0) printf("\033[%dC", dx);
    if(dy > 0) printf("\033[%dB", dy);
    
    repl->cx += count;
}

bool insertNewLine(REPL* repl) {
    // quick path, not much to do!
    if(repl->cx != llength(repl) || repl->cy + 1 != repl->linesCount) return false;
    writeBuffer(repl, '\n');
    putchar('\n');
    
    repl->cy += 1;
    repl->lines[repl->cy].length = 0;
    repl->lines[repl->cy].offset = repl->buffer.count;
    repl->cx = 0;
    repl->linesCount = repl->cy + 1;
    return true;
}


void print(REPL* repl, char c) {
    // putchar(c);
    writeBuffer(repl, c);
    repl->lines[repl->cy].length += 1;
    for(int i = 0; i < llength(repl) - repl->cx; ++i)
        putchar(repl->buffer.data[loffset(repl) + repl->cx + i]);
    repl->cx += 1;
    if(repl->cx != llength(repl))
        printf("\033[%dD", llength(repl) - repl->cx);
    fflush(stdout);
    computeOffsets(repl, repl->cy);
}

void homeLine(REPL* repl) {
    printf("\033[%dD", llength(repl));
    setX(repl, 0);
}

void clearLine(REPL* repl) {
    
    LineInfo line = repl->lines[repl->cy];
    if(line.length <= 0) return;
    printf("\033[%dD", line.length);
    for(int i = 0; i < line.length; ++i) putchar(' ');
    printf("\033[%dD", line.length);
    setWidth(repl, 0);
}

void clearAll(REPL* repl) {
    if(repl->cy > 0) printf("\033[%dA", repl->cy);
    if(repl->cx > 0) printf("\033[%dD", repl->cx);
    
    for(int i = 0; i < repl->linesCount; ++i) {
        LineInfo line = repl->lines[i];
        if(line.length <= 0) return;
        for(int i = 0; i < line.length; ++i) putchar(' ');
        putchar('\n');
    }
    if(repl->linesCount > 0) printf("\033[%dA", repl->linesCount);
    
}
