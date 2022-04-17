/*
 * @file klog.c
 * @author Maldus512
 * @brief Small library that implements a circular log buffer. When properly
 * traced (with ASCII representation), `klog_buffer` displays a series of
 * printed lines.
 */

#define KLOG_LINES                                                             \
  100 // Number of lines in the buffer. Adjustable, only limited by available
     // memory
#define KLOG_LINE_SIZE 41 // Length of a single line in characters

static void next_line(void);
static void next_char(void);

unsigned int klog_line_index = 0; // Index of the next line to fill
unsigned int klog_char_index = 0; // Index of the current character in the line
char klog_buffer[KLOG_LINES][KLOG_LINE_SIZE] = {
    0}; // Actual buffer, to be traced in uMPS3

// Print str to klog
void kprint(char *str)
{
  while (*str != '\0') {
    // If there is a newline skip to the next one
    if (*str == '\n') {
      next_line();
      str++;
    }
    // Otherwise just fill the current one
    else {
      klog_buffer[klog_line_index][klog_char_index] = *str++;
      next_char();
    }
  }
}

// Princ a number in decimal numbers !!backwards!!
void kprint_int(int num)
{
  if (num < 0) {
    num = -num;
    klog_buffer[klog_line_index][klog_char_index] = '-';
    next_char();
  }

  do {
    klog_buffer[klog_line_index][klog_char_index] = '0' + (num % 10);
    num /= 10;
    next_char();
  } while (num > 0);
}

// Princ a number in hexadecimal format (best for addresses)
void kprint_hex(unsigned int num)
{
  const char digits[] = "0123456789ABCDEF";

  do {
    klog_buffer[klog_line_index][klog_char_index] = digits[num % 16];
    num /= 16;
    next_char();
  } while (num > 0);
}

// Move onto the next character (and into the next line if the current one
// overflows)
static void next_char(void)
{
  if (++klog_char_index >= KLOG_LINE_SIZE) {
    klog_char_index = 0;
    next_line();
  }
}

// Skip to next line
static void next_line(void)
{
  klog_line_index = (klog_line_index + 1) % KLOG_LINES;
  klog_char_index = 0;
  // Clean out the rest of the line for aesthetic purposes
  for (unsigned int i = 0; i < KLOG_LINE_SIZE; i++) {
    klog_buffer[klog_line_index][i] = ' ';
  }
}
