#include "main.h"

// NOTE: vibe coded
void disableRawMode(struct termios* origTermios) {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, origTermios);

  /* Restore cursor and original screen */
  printf("\033[?1049l");  // leave alternate screen
  printf("\033[?25h");    // show cursor
  printf("\0338");        // restore cursor pos
  fflush(stdout);
}

// NOTE: vibe coded
void enableRawMode(struct termios* origTermios) {
  tcgetattr(STDIN_FILENO, origTermios);

  origTermios->c_lflag &= ~(ECHO | ICANON);  // no echo, no line buffering
  tcsetattr(STDIN_FILENO, TCSAFLUSH, origTermios);

  // printf("\0337");       // save cursor pos
  printf("\033[?1049h");  // alternate screen
  printf("\033[2J");      // clear screen
  printf("\033[H");       // cursor home
  fflush(stdout);
}

void moveCursor(Vec2 pos) {
  printf(MOVE_CURSOR("%d", "%d"), pos.row, pos.col);
}

void drawGradient(char* buffer, Box zone, int hue) {
  int size = 0;
  for (int row = 0; row < zone.height; row++) {
    for (int col = 0; col < zone.width; col++) {
      Vec3 color = hsvToRgb(hue, (1 - (float)(row) / (zone.height - 1)), (1 - (float)(col) / (zone.width - 1)));

      size += sprintf(buffer + size, MOVE_CURSOR("%d", "%d") "\x1b[48;2;%d;%d;%dm ", zone.pos.row + row, zone.pos.col + col, color.r, color.g, color.b);
    }
    size += sprintf(buffer + size, RESET_LINE);
  }
  printf("%s", buffer);
}

void fill(char* screen, Box zone, Vec3 color) {
  int size = 0;

  for (int row = zone.pos.row; row < zone.pos.row + zone.height; row++) {
    size += sprintf(screen + size, MOVE_CURSOR("%d", "%d") "\x1b[48;2;%d;%d;%dm", row, zone.pos.col, color.r, color.g, color.b);
    memset(screen + size, ' ', zone.width);
    size += zone.width;
    size += sprintf(screen + size, RESET_LINE);
  }
}

void drawColorInfo(Box zone, Vec3 color) {
  char sample[10000] = {0};

  Box infoBox = {
      .pos =
          {
              .row = zone.pos.row,
              .col = zone.pos.col + zone.width + 1,
          },
      .size = {6, 3},
  };

  fill(sample, infoBox, color);
  printf("%s", sample);

  int infoTxtRow = infoBox.pos.row + infoBox.height;
  char mouseMove[20] = {0};

  int infoCol = infoBox.pos.col;
  sprintf(mouseMove, MOVE_CURSOR("%d", "%d"), infoTxtRow, infoCol);
  printf("%s      %sR: %d", mouseMove, mouseMove, color.r);
  sprintf(mouseMove, MOVE_CURSOR("%d", "%d"), infoTxtRow + 1, infoCol);
  printf("%s      %sG: %d", mouseMove, mouseMove, color.g);
  sprintf(mouseMove, MOVE_CURSOR("%d", "%d"), infoTxtRow + 2, infoCol);
  printf("%s      %sB: %d", mouseMove, mouseMove, color.b);
}

void drawHueBand(Box zone) {
  for (int row = 0; row < zone.height; row++) {
    for (int col = 0; col < zone.width; col++) {
      float hue = ((float)(col) / (zone.width - 1)) * 360.0f;

      Vec3 color = hsvToRgb(hue, 1.0f, 1.0f);

      moveCursor((Vec2){.row = zone.pos.row + row, .col = zone.pos.col + col});

      printf("\033[48;2;%d;%d;%dm  \033[0m", color.r, color.g, color.b);
    }
  }

  printf("\033[%d;1H", zone.pos.row + zone.height);
}

void displayHelp() {
  printf("palpick (Pallete Picker) - a color picker for the terminal.\n");
  printf(
      "\nAllows you to quickly select a color from a gimp-like color picker "
      "and outputs the resulting rgb color in the format you provide. "
      "Useful for integrating in terminal editors. Allows for selecting "
      "colors and injecting the result directly into the text.\n");
  printf(
      "Think of a place where you would need to select a color for a Vec3 "
      "color = {123, 34, 105} \n");

  printf("\nUsage:\n");
  printf(
      "    h, j, k, l - used to move the cursor for selecting a hue or a "
      "value\n");
  printf("    m - toggle between selecting a hue or selecting a value\n");
  printf("    Enter - select the color and exit\n");

  printf("\nOptions:\n");
  printf("    %-20s %s\n", "-h, --help", "Prints this help");
  printf("    %-20s %s\n", "--x, --y", "X/Y origin (upper left hand corner) of widget. Defaults to 1");
  printf("    %-20s %s\n", "--width, --height",
         "Width and Height of the Hue Bar and Gradient box - not including the "
         "right side info text. Defaults to 20 and 10");
  printf("    %-20s %s\n", "--prefix", "Prepends the final output with this string");
  printf("    %-20s %s\n", "--postfix", "Appends to the final output with this string");
  printf("    %-20s %s\n", "--separator1",
         "The char or string to use as the separator between the Red and Green "
         "components. Defaults to ','");
  printf("    %-20s %s\n", "--separator2",
         "The char or string to use as the separator between the Green and "
         "Blue components. Defaults to ','");
  printf("    %-20s %s\n", "--red-label", "The label before the Red component");
  printf("    %-20s %s\n", "--green-label", "The label before the Gree component");
  printf("    %-20s %s\n", "--blue-label", "The label before the Blue component");
  printf("\nExamples: \n");
  printf(
      "    <command> --pre \"Vec3(\" --post \")\" --separator1 \", \" "
      "--separator2 \", \" --> Vec3(123, 34, 105)\n");
  printf(
      "    <command> --separator1 \"; \" --separator2 \"; \" --red-label "
      "\"R: \" --green-label \"G: \" --blue-label \"B: \" --> R: 123; G: "
      "34; B: 105\n");
}
