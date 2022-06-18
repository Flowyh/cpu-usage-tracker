#include "../inc/printer.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "../inc/logger.h"

const char* colors_get(const enum Colors color)
{
  if (color < COLOR_RESET || color > CYAN)
    return "";
  
  const char* colors_str[] = {
    "\x1b[0m", // Reset
    "\x1b[31m", // Red
    "\x1b[32m", // Green
    "\x1b[34m", // Blue
    "\x1b[33m", // Yellow
    "\x1b[30m", // Black
    "\x1b[37m", // White
    "\x1b[35m", // Magenta
    "\x1b[36m", // Cyan
  };

  return colors_str[color];
}

const char* modes_get(const enum Modes mode)
{
  if (mode < MODE_RESET || mode > MODE_STRIKE)
    return "";

  const char* modes_str[] = {
    "\e[0m", // Reset
    "\e[1m", // Bold
    "\e[3m", // Italic
    "\e[4m", // Underline
    "\e[9m", // Strike
  };

  return modes_str[mode];
}


#define HISTOGRAM_HORIZONTAL_BAR_EMPTY " "
#define HISTOGRAM_HORIZONTAL_BAR_FILL ":"
#define HISTOGRAM_HORIZONTAL_BAR_LIMIT 2000
#define HISTOGRAM_HORIZONTAL_BAR_SEPARATOR_COLOR WHITE
#define HISTOGRAM_HORIZONTAL_BAR_BASE_COLOR GREEN
#define HISTOGRAM_HORIZONTAL_BAR_MID_VAL 0.5
#define HISTOGRAM_HORIZONTAL_BAR_MID_COLOR YELLOW
#define HISTOGRAM_HORIZONTAL_BAR_CRIT_VAL 0.8
#define HISTOGRAM_HORIZONTAL_BAR_CRIT_COLOR RED

char* printer_histogram_horizontal_bar(const size_t width, const double fill)
{
  char* histogram = malloc(HISTOGRAM_HORIZONTAL_BAR_LIMIT);
  
  histogram[0] = '[';
  histogram[1] = '\0';

  size_t color = HISTOGRAM_HORIZONTAL_BAR_BASE_COLOR;
  strcat(histogram, colors_get(color));
  for (size_t i = 0; i < width; i++)
  {
    double current_block = (double) (i+1) / width;
    if (current_block > fill)
      strcat(histogram, HISTOGRAM_HORIZONTAL_BAR_EMPTY);
    else
    {
      if(current_block >= HISTOGRAM_HORIZONTAL_BAR_CRIT_VAL && color != HISTOGRAM_HORIZONTAL_BAR_CRIT_COLOR)
      {
        color = HISTOGRAM_HORIZONTAL_BAR_CRIT_COLOR;
        strcat(histogram, colors_get(color));
      }
      else if (HISTOGRAM_HORIZONTAL_BAR_CRIT_VAL > current_block && current_block >= HISTOGRAM_HORIZONTAL_BAR_MID_VAL && color != HISTOGRAM_HORIZONTAL_BAR_MID_COLOR)
      {
        color = HISTOGRAM_HORIZONTAL_BAR_MID_COLOR;
        strcat(histogram, colors_get(color));
      }
      strcat(histogram, HISTOGRAM_HORIZONTAL_BAR_FILL);
    }
  }

  strcat(histogram, colors_get(COLOR_RESET));
  strcat(histogram, "]");

  const size_t histogram_len = strlen(histogram);

  char* result = malloc(histogram_len + 1);
  memcpy(&result[0], &histogram[0], histogram_len);
  result[histogram_len] = '\0';

  free(histogram);
  return result;
}

char* printer_print_line(const size_t width, const char line_char)
{
  char* line = malloc(width + 1);
  for(size_t i = 0; i < width; i++)
    line[i] = line_char;
  line[width] = '\0';
  return line;
}

#define PRETTY_CPU_USAGE_HIST_WIDTH 20
#define PRETTY_CPU_USAGE_HORIZONTAL_LINE '-'
#define PRETTY_CPU_USAGE_CORNER '+'
#define PRETTY_CPU_USAGE_VERTICAL_LINE '|'
/*
  Here lies the premiumest spaghetti I've ever written.
  I've wanted to make some things more generic, but I'm too tired to fix it.
  Those randoms additions/substractions were picked experimentally.
*/
void printer_pretty_cpu_usage(char* names[const], const double percentages[const], const size_t cores)
{
  char* datetime_start = "Date: ";
  char* datetime_str = datetime_to_str();
  char* date = malloc(strlen(datetime_start) + strlen(datetime_str) + 1);
  date[0] = '\0';
  strcat(date, datetime_start);
  strcat(date, datetime_str);
  
  size_t longest_name = strlen(names[cores - 1]);
  size_t cpu_usage_space = longest_name + PRETTY_CPU_USAGE_HIST_WIDTH + 2 + 9;
  size_t datetime_pad = (cpu_usage_space - strlen(date)) / 2 + strlen(date) + 1;
  char* top_bot_line = printer_print_line(3 + cpu_usage_space, PRETTY_CPU_USAGE_HORIZONTAL_LINE);

  printf("%c%s%c\n", PRETTY_CPU_USAGE_CORNER, top_bot_line, PRETTY_CPU_USAGE_CORNER);
  printf("%c%*s%*s%c\n", PRETTY_CPU_USAGE_VERTICAL_LINE, (int) (datetime_pad), date, (int) (datetime_pad - strlen(date) + 2), " ", PRETTY_CPU_USAGE_VERTICAL_LINE);
  printf("%c%s%c\n", PRETTY_CPU_USAGE_CORNER, top_bot_line, PRETTY_CPU_USAGE_CORNER);

  for (size_t i = 0; i < cores; i++)
  {
    char* histogram = printer_histogram_horizontal_bar(PRETTY_CPU_USAGE_HIST_WIDTH, percentages[i]);
    size_t percents_width = percentages[i] > 10.0 ? 7 : 6;
    
    printf("%c %s%*s%s %*.2f%%  %c\n", PRETTY_CPU_USAGE_VERTICAL_LINE, names[i], (int) (longest_name - strlen(names[i]) + 1), " ", histogram, (int) percents_width, percentages[i] * 100, PRETTY_CPU_USAGE_VERTICAL_LINE);
    free(histogram);
  }
  printf("%c%s%c\n", PRETTY_CPU_USAGE_CORNER, top_bot_line, PRETTY_CPU_USAGE_CORNER);
  free(datetime_str);
  free(top_bot_line);
  free(date);
}