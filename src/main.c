#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "allocators.h"
#include "base.h"
#include "logger.h"

FILE* logFile;

#define MOVE_CURSOR(row, col) "\033[" row ";" col "H"
#define CLEAR_SCREEN "\033[2J"
#define RESET_LINE "\033[0m"
#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"
#define KEY_ENTER 10

#define SEPARATOR "%s"
#define PREFIX "%s"
#define POSTFIX "%s"
#define RED_LABEL "%s"
#define GREEN_LABEL "%s"
#define BLUE_LABEL "%s"

typedef enum Mode {
  HUE,
  GRADIENT,
} Mode;

typedef struct Params {
  char* x;
  char* y;
  char* width;
  char* height;
  char* prefix;
  char* postfix;
  char* firstSeparator;
  char* secondSeparator;
  char* redLabel;
  char* greenLabel;
  char* blueLabel;
} Params;

Vec3 hsvToRgb(float h, float s, float v);
void moveCursor(Vec2 pos);
Vec3 getCurrentColor(Vec2 hueCursor, Vec2 gradientCursor, Box zone);

void drawGradient(char* gradient, Box zone, int hue) {
  int size = 0;
  for (int row = 0; row < zone.height; row++) {
    for (int col = 0; col < zone.width; col++) {
      Vec3 color = hsvToRgb(hue, (1 - (float)(row) / (zone.height - 1)), (1 - (float)(col) / (zone.width - 1)));

      size += sprintf(gradient + size, MOVE_CURSOR("%d", "%d") "\x1b[48;2;%d;%d;%dm ", zone.pos.row + row, zone.pos.col + col, color.r, color.g, color.b);
    }
    size += sprintf(gradient + size, RESET_LINE);
  }
  printf("%s", gradient);
}

int clamp(int value, int min, int max) {
  return fmax(min, fmin(value, max));
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

void drawInfo(Box zone, Vec3 color) {
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

// NOTE: vibe coded
Vec3 hsvToRgb(float h, float s, float v) {
  float c = v * s;                                   // Chroma
  float x = c * (1 - fabs(fmod(h / 60.0f, 2) - 1));  // X value
  float m = v - c;                                   // Match value

  float rp, gp, bp;  // Temporary RGB values
  if (h >= 0 && h < 60) {
    rp = c;
    gp = x;
    bp = 0;
  } else if (h >= 60 && h < 120) {
    rp = x;
    gp = c;
    bp = 0;
  } else if (h >= 120 && h < 180) {
    rp = 0;
    gp = c;
    bp = x;
  } else if (h >= 180 && h < 240) {
    rp = 0;
    gp = x;
    bp = c;
  } else if (h >= 240 && h < 300) {
    rp = x;
    gp = 0;
    bp = c;
  } else {
    rp = c;
    gp = 0;
    bp = x;
  }

  return (Vec3){
      .r = (int)((rp + m) * 255),
      .g = (int)((gp + m) * 255),
      .b = (int)((bp + m) * 255),
  };
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

HSV stateToHSV(Vec2 hueCursor, Vec2 gradientCursor, Box zone) {
  return (HSV){
      .h = (float)(hueCursor.col - zone.pos.col) / zone.size.width * 360.0f,
      .v = 1 - (((float)gradientCursor.col - zone.pos.col) / (zone.size.width - 1)),
      .s = 1 - (((float)gradientCursor.row - zone.pos.row) / (zone.size.height - 1)),
  };
}

Vec3 getCurrentColor(Vec2 hueCursor, Vec2 gradientCursor, Box zone) {
  HSV hsv = stateToHSV(hueCursor, gradientCursor, zone);
  return hsvToRgb(hsv.h, hsv.s, hsv.v);
}

static struct termios orig_termios;

// NOTE: vibe coded
void disableRawMode(void) {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);

  /* Restore cursor and original screen */
  printf("\033[?1049l");  // leave alternate screen
  printf("\033[?25h");    // show cursor
  printf("\0338");        // restore cursor pos
  fflush(stdout);
}

// NOTE: vibe coded
void enableRawMode(void) {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawMode);

  struct termios raw = orig_termios;
  raw.c_lflag &= ~(ECHO | ICANON);  // no echo, no line buffering
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

  // printf("\0337");       // save cursor pos
  printf("\033[?1049h");  // alternate screen
  printf("\033[2J");      // clear screen
  printf("\033[H");       // cursor home
  fflush(stdout);
}

void moveCursor(Vec2 pos) {
  printf(MOVE_CURSOR("%d", "%d"), pos.row, pos.col);
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

void parseParams(Params* p, int argc, char* argv[]) {
  p->x = "1";
  p->y = "1";
  p->width = "30";
  p->height = "15";
  p->prefix = "";
  p->postfix = "";
  p->firstSeparator = ", ";
  p->secondSeparator = ", ";
  p->redLabel = "";
  p->greenLabel = "";
  p->blueLabel = "";

  if (argc > 1) {
    logInfo("parsing input params");
    int i = 1;

    while (i < argc) {
      char* param = argv[i];

      if (strcmp(param, "--help") == 0 || strcmp(param, "-h") == 0) {
        displayHelp();
        exit(0);
      } else if (strcmp(param, "--x") == 0) {
        i++;
        p->x = argv[i];
      } else if (strcmp(param, "--y") == 0) {
        i++;
        p->y = argv[i];
      } else if (strcmp(param, "--width") == 0) {
        i++;
        p->width = argv[i];
      } else if (strcmp(param, "--height") == 0) {
        i++;
        p->height = argv[i];
      } else if (strcmp(param, "--pre") == 0) {
        i++;
        p->prefix = argv[i];
      } else if (strcmp(param, "--post") == 0) {
        i++;
        p->postfix = argv[i];
      } else if (strcmp(param, "--separator1") == 0) {
        i++;
        p->firstSeparator = argv[i];
      } else if (strcmp(param, "--separator2") == 0) {
        i++;
        p->secondSeparator = argv[i];
      } else if (strcmp(param, "--red-label") == 0) {
        i++;
        p->redLabel = argv[i];
      } else if (strcmp(param, "--green-label") == 0) {
        i++;
        p->greenLabel = argv[i];
      } else if (strcmp(param, "--blue-label") == 0) {
        i++;
        p->blueLabel = argv[i];
      } else {
        i++;
      }
    }
  }
}

int main(int argc, char* argv[]) {
  initLogger();

  Params p = {0};

  parseParams(&p, argc, argv);
  enableRawMode();

  Arena arena;
  initArena(&arena, MB(1));

  Box window = {
      .pos = {atoi(p.x), atoi(p.y)},
      .size = {atoi(p.width), atoi(p.height)},
  };

  Box hueBand = {
      .pos = window.pos,
      .size = {window.size.width - 1, 1},
  };

  Box colorGradient = {
      .pos =
          {
              .row = window.pos.row + hueBand.size.height,
              .col = window.pos.col,
          },
      .size =
          {
              .width = window.size.width,
              .height = window.size.height - hueBand.size.height,
          },
  };

  int hueBandHeight = 1;

  Vec2 gradientCursor = {.row = colorGradient.pos.row, .col = colorGradient.pos.col};
  Vec2 hueCursor = window.pos;

  Mode mode = GRADIENT;

  Vec3 color = getCurrentColor(hueCursor, gradientCursor, colorGradient);

  char* gradient = arenaAlloc(&arena, window.width * window.height * 2);

  drawHueBand(hueBand);
  drawGradient(gradient, colorGradient, 0);
  drawInfo(window, color);
  moveCursor(gradientCursor);
  fflush(stdout);

  while (1) {
    char c = getchar();

    if (c == KEY_ENTER) {
      break;
    }

    printf(HIDE_CURSOR);

    if (c == 'm') {
      if (mode == HUE) {
        mode = GRADIENT;
        moveCursor(gradientCursor);
        fflush(stdout);
      } else if (mode == GRADIENT) {
        mode = HUE;
        moveCursor(hueCursor);
        fflush(stdout);
      }
    }

    if (mode == GRADIENT) {
      bool changed = false;
      switch (c) {
        case 'h':
          gradientCursor.col--;
          changed = true;
          break;
        case 'l':
          gradientCursor.col++;
          changed = true;
          break;
        case 'j':
          gradientCursor.row++;
          changed = true;
          break;
        case 'k':
          gradientCursor.row--;
          changed = true;
          break;
      }

      if (changed) {
        gradientCursor.col = clamp(gradientCursor.col, colorGradient.pos.col, colorGradient.pos.col + colorGradient.width - 1);
        gradientCursor.row = clamp(gradientCursor.row, colorGradient.pos.row, colorGradient.pos.row + colorGradient.height - 1);

        Vec3 color = getCurrentColor(hueCursor, gradientCursor, colorGradient);
        drawInfo(window, color);

        moveCursor(gradientCursor);
      }
    } else if (mode == HUE) {
      bool changed = false;
      switch (c) {
        case 'h':
          hueCursor.col--;
          changed = true;
          break;
        case 'l':
          hueCursor.col++;
          changed = true;
          break;
      }

      if (changed) {
        hueCursor.col = clamp(hueCursor.col, window.pos.col, window.pos.col + window.width - 1);

        HSV hsv = stateToHSV(hueCursor, gradientCursor, colorGradient);
        Vec3 color = hsvToRgb(hsv.h, hsv.s, hsv.v);

        drawGradient(gradient, colorGradient, hsv.h);
        drawInfo(window, color);

        moveCursor(hueCursor);
      }
    }

    printf(SHOW_CURSOR);
    fflush(stdout);
  }

  disableRawMode();

  color = getCurrentColor(hueCursor, gradientCursor, colorGradient);

  printf(PREFIX RED_LABEL "%d" SEPARATOR GREEN_LABEL "%d" SEPARATOR BLUE_LABEL "%d" POSTFIX, p.prefix, p.redLabel, color.r, p.firstSeparator, p.greenLabel, color.g, p.secondSeparator, p.blueLabel, color.b, p.postfix);
}
