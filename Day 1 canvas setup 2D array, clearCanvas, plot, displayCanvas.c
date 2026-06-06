#include <stdio.h>
#include <string.h>

/* ─────────────────────────────────────────
   DAY 1: Canvas Setup + Display Function
   - Define canvas (2D char array)
   - clearCanvas() fills with spaces
   - plot() safely writes one character
   - displayCanvas() prints the picture
   ───────────────────────────────────────── */

#define ROWS 24
#define COLS 80

char canvas[ROWS][COLS];

/* Fill every cell with a space */
void clearCanvas() {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            canvas[r][c] = ' ';
}

/* Safe pixel-plot: bounds check before writing */
void plot(int r, int c, char ch) {
    if (r >= 0 && r < ROWS && c >= 0 && c < COLS)
        canvas[r][c] = ch;
}

/* Print the canvas surrounded by a border */
void displayCanvas() {
    printf("\n+");
    for (int c = 0; c < COLS; c++) putchar('-');
    printf("+\n");

    for (int r = 0; r < ROWS; r++) {
        putchar('|');
        for (int c = 0; c < COLS; c++)
            putchar(canvas[r][c]);
        printf("|\n");
    }

    printf("+");
    for (int c = 0; c < COLS; c++) putchar('-');
    printf("+\n");
}

int main() {
    clearCanvas();

    /* Test: write a message on the canvas */
    char *msg = "Day 1: Canvas is working!";
    for (int i = 0; msg[i] != '\0'; i++)
        plot(10, 20 + i, msg[i]);

    /* Test: draw a border of asterisks */
    for (int c = 0; c < COLS; c++) {
        plot(0,        c, '*');
        plot(ROWS - 1, c, '*');
    }
    for (int r = 0; r < ROWS; r++) {
        plot(r, 0,        '*');
        plot(r, COLS - 1, '*');
    }

    displayCanvas();
    printf("\nDay 1 complete: canvas + display function working!\n");
    return 0;
}
