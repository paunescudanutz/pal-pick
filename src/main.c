#include "main.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "allocators.h"

FILE* logFile;
struct termios origTermios;

HSV stateToHSV(App* app) {
  return (HSV){
      .h = (float)(app->hueCursor.col - app->gradientBox.pos.col) / app->gradientBox.size.width * 360.0f,
      .v = 1 - (((float)app->gradientCursor.col - app->gradientBox.pos.col) / (app->gradientBox.size.width - 1)),
      .s = 1 - (((float)app->gradientCursor.row - app->gradientBox.pos.row) / (app->gradientBox.size.height - 1)),
  };
}

Vec3 getCurrentColor(App* app) {
  HSV hsv = stateToHSV(app);
  return hsvToRgb(hsv.h, hsv.s, hsv.v);
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

void initApp(App* app, Arena* arena, Params* p) {
  initArena(arena, MB(1));

  Box window = {
      .pos = {atoi(p->x), atoi(p->y)},
      .size = {atoi(p->width), atoi(p->height)},
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

  *app = (App){
      .mode = GRADIENT,
      .gradientBox = colorGradient,
      .window = window,
      .gradientCursor = {.row = colorGradient.pos.row, .col = colorGradient.pos.col},
      .hueBox = hueBand,
      .hueCursor = window.pos,
      .gradientDisplay = arenaAlloc(arena, window.width * window.height * 2),
      .masterArena = arena,
  };
}

void toggleMode(App* app) {
  if (app->mode == HUE) {
    app->mode = GRADIENT;
    moveCursor(app->gradientCursor);
  } else if (app->mode == GRADIENT) {
    app->mode = HUE;
    moveCursor(app->hueCursor);
  }

  fflush(stdout);
}

void handleGradientSelector(App* app, char c) {
  bool changed = false;

  switch (c) {
    case 'h':
      app->gradientCursor.col--;
      changed = true;
      break;
    case 'l':
      app->gradientCursor.col++;
      changed = true;
      break;
    case 'j':
      app->gradientCursor.row++;
      changed = true;
      break;
    case 'k':
      app->gradientCursor.row--;
      changed = true;
      break;
  }

  if (changed) {
    app->gradientCursor.col = clamp(app->gradientCursor.col, app->gradientBox.pos.col, app->gradientBox.pos.col + app->gradientBox.width - 1);
    app->gradientCursor.row = clamp(app->gradientCursor.row, app->gradientBox.pos.row, app->gradientBox.pos.row + app->gradientBox.height - 1);

    Vec3 color = getCurrentColor(app);
    drawColorInfo(app->window, color);

    moveCursor(app->gradientCursor);
  }
}

void handleHueSelector(App* app, char c) {
  bool changed = false;

  switch (c) {
    case 'h':
      app->hueCursor.col--;
      changed = true;
      break;
    case 'l':
      app->hueCursor.col++;
      changed = true;
      break;
  }

  if (changed) {
    app->hueCursor.col = clamp(app->hueCursor.col, app->window.pos.col, app->window.pos.col + app->window.width - 1);

    HSV hsv = stateToHSV(app);
    Vec3 color = hsvToRgb(hsv.h, hsv.s, hsv.v);

    drawGradient(app->gradientDisplay, app->gradientBox, hsv.h);
    drawColorInfo(app->window, color);

    moveCursor(app->hueCursor);
  }
}

void releaseResources(App* app) {
  fclose(logFile);
}

void printResult(App* app, Params* p) {
  disableRawMode(&origTermios);
  Vec3 color = getCurrentColor(app);
  printf(PREFIX RED_LABEL "%d" SEPARATOR GREEN_LABEL "%d" SEPARATOR BLUE_LABEL "%d" POSTFIX, p->prefix, p->redLabel, color.r, p->firstSeparator, p->greenLabel, color.g, p->secondSeparator, p->blueLabel, color.b, p->postfix);
  releaseResources(app);
}

int main(int argc, char* argv[]) {
  initLogger();

  Params p = {0};

  parseParams(&p, argc, argv);
  enableRawMode(&origTermios);

  Arena arena = {0};
  App app = {0};
  initApp(&app, &arena, &p);

  Vec3 color = getCurrentColor(&app);
  drawHueBand(app.hueBox);
  drawGradient(app.gradientDisplay, app.gradientBox, 0);
  drawColorInfo(app.window, color);
  moveCursor(app.gradientCursor);
  fflush(stdout);

  while (1) {
    char c = getchar();

    if (c == KEY_ENTER) {
      printResult(&app, &p);
      return 0;
    }

    printf(HIDE_CURSOR);

    if (c == 'm') {
      toggleMode(&app);
    }

    if (app.mode == GRADIENT) {
      handleGradientSelector(&app, c);
    } else if (app.mode == HUE) {
      handleHueSelector(&app, c);
    }

    printf(SHOW_CURSOR);
    fflush(stdout);
  }

  releaseResources(&app);
}
