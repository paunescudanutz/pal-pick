
#include <math.h>
#include <stdio.h>

#include "allocators.h"
#include "base.h"
#include "logger.h"

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

typedef struct App {
  Arena* masterArena;
  Box window;  // rename
  Box hueBox;
  Box gradientBox;
  Vec2 gradientCursor;
  Vec2 hueCursor;
  char* gradientDisplay;
  Mode mode;
} App;

int clamp(int value, int min, int max);
Vec3 hsvToRgb(float h, float s, float v);

void drawGradient(char* buffer, Box zone, int hue);
void drawColorInfo(Box zone, Vec3 color);
void drawHueBand(Box zone);
void displayHelp();
void moveCursor(Vec2 pos);
