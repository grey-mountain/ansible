#include <curses.h>
#include <dirent.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILES 10000

int compare(const void *a, const void *b) {
  const char **file1 = (const char **)a;
  const char **file2 = (const char **)b;
  return strcmp(*file1, *file2);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <directory>\n", argv[0]);
    return 1;
  }

  DIR *dir;
  struct dirent *entry;
  char *files[MAX_FILES];
  int count = 0;

  dir = opendir(argv[1]);
  if (dir == NULL) {
    printf("Error opening directory.\n");
    return 1;
  }

  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_REG) {
      files[count] = strdup(entry->d_name);
      count++;
    }
  }

  qsort(files, count, sizeof(char *), compare);

  closedir(dir);

  // Initialize ncurses
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  scrollok(stdscr, TRUE); // Enable scrolling

  int selected = 0;
  int start_line = 0; // Track the starting line of the visible area

  char search_query[100] = ""; // Store the user's search query

  while (1) {
    clear();

    int max_lines = LINES - 1; // Subtract 1 for the command line

    // Update the starting line if the selected line goes below the visible area
    if (selected >= start_line + max_lines) {
      start_line = selected - max_lines + 1;
    }
    // Update the starting line if the selected line goes above the visible area
    else if (selected < start_line) {
      start_line = selected;
    }

    // Filter the files based on the search query
    char filtered_files[MAX_FILES][NAME_MAX];
    int filtered_count = 0;
    for (int i = 0; i < count; i++) {
      if (strstr(files[i], search_query) != NULL) {
        strcpy(filtered_files[filtered_count], files[i]);
        filtered_count++;
      }
    }

    for (int i = start_line; i < filtered_count && i < start_line + max_lines;
         i++) {
      if (i == selected) {
        attron(A_REVERSE);
      }
      mvprintw(i - start_line, 0, "%s", filtered_files[i]);
      attroff(A_REVERSE);
    }

    // Print the input line at the bottom of the terminal
    mvprintw(LINES - 1, 0, "Search: %s", search_query);

    int key = getch();

    switch (key) {
    case KEY_UP: {

      selected--;
      if (selected < 0) {
        selected = filtered_count - 1;
      }
      break;
    }
    case KEY_DOWN: {
      selected++;
      if (selected >= filtered_count) {
        selected = 0;
      }
      break;
    }
    case '\n': {
      char str1[200] = "wal -i ";
      strcat(str1, argv[1]);
      strcat(str1, "/");
      strcat(str1, filtered_files[selected]);
      system(str1);
      break;
    }
    case 'q': // Escape key
      endwin();
      // Free memory allocated for file names
      for (int i = 0; i < count; i++) {
        free(files[i]);
      }
      return 0;
    default:
      // Update the search query based on user input
      if (key == KEY_BACKSPACE) {
        search_query[strlen(search_query) - 1] = '\0';
      } else if (key >= 32 && key <= 126) {
        char ch = (char)key;
        strncat(search_query, &ch, 1);
      }
      selected = 0;
      start_line = 0;
      break;
    }
  }
}
