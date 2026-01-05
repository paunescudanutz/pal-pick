
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

  struct termios raw = *origTermios;
  raw.c_lflag &= ~(ECHO | ICANON);  // no echo, no line buffering
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

  // printf("\0337");       // save cursor pos
  printf("\033[?1049h");  // alternate screen
  printf("\033[2J");      // clear screen
  printf("\033[H");       // cursor home
  fflush(stdout);
}

int clamp(int value, int min, int max) {
  return fmax(min, fmin(value, max));
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
