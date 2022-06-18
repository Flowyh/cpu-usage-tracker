#ifndef PRINTER_H
#define PRINTER_H

#include <unistd.h>

enum Colors {
  COLOR_RESET,
  RED,
  GREEN,
  BLUE,
  YELLOW,
  BLACK,
  WHITE,
  MAGENTA,
  CYAN,
};

enum Modes {
  MODE_RESET,
  MODE_BOLD,
  MODE_ITALIC,
  MODE_UNDERLINE,
  MODE_STRIKE,
};

const char* colors_get(const enum Colors color);
const char* modes_get(const enum Modes mode);

char* printer_histogram_horizontal_bar(const size_t width, const double fill);
char* printer_print_line(const size_t width, const char line_char);

void printer_pretty_cpu_usage(char* names[], const double percentages[], const size_t cores);

#endif // !PRINTER_H
