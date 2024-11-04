#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#define VALIDCHAR(current) ((current == '\'' && isalpha(*(&current + 1)) != 0) || isalpha(current) != 0)

struct wordObj {
    char *str;
    int count;
};

// Function to compare words for sorting
int compareWords(const void *a, const void *b) {
    const struct wordObj *wordA = (const struct wordObj *)a;
    const struct wordObj *wordB = (const struct wordObj *)b;
    // Sort by count in descending order, then by lexicographic order if counts are the same
    if (wordA->count != wordB->count) {
        return wordB->count - wordA->count;
    }
    return strcmp(wordA->str, wordB->str);
}

// Function to count unique words in a file and return a dynamically allocated list
struct wordObj* countWords(int fileDesc, int *listSize) {
    char current;
    struct wordObj *list = NULL;
    *listSize = 0;  // Initialize list size to 0
    
    while (read(fileDesc, &current, 1) == 1) {
        int wordSize = 1;
        char *str = calloc(2, wordSize + 1);

        // Check for valid first char, else skip
        if (VALIDCHAR(current) != 0) {
            str[0] = current;
        } else {
            free(str);
            continue;
        }

        // Add each character to the string
        while ((read(fileDesc, &current, 1) == 1) && (current == '-' || VALIDCHAR(current))) {
            if ((current == '-' && isalpha(str[wordSize - 1]) && isalpha(current)) || VALIDCHAR(current)) {
                wordSize++;
                char *tempStr = realloc(str, wordSize + 1);
                if (tempStr == NULL) {
                    free(str);
                    perror("Allocation failed.");
                    return NULL;
                }
                str = tempStr;
                str[wordSize - 1] = current;
            } else {
                break;
            }
        }
        str[wordSize] = '\0';  // Null-terminate the word

        // Check if word already exists in list
        int found = 0;
        for (int i = 0; i < *listSize; i++) {
            if (strcmp(list[i].str, str) == 0) {
                list[i].count++;
                found = 1;
                free(str);
                break;
            }
        }

        // Add new word if not found
        if (!found) {
            struct wordObj *temp = realloc(list, sizeof(struct wordObj) * (*listSize + 1));
            if (temp == NULL) {
                free(list);
                free(str);
                perror("Allocation failed");
                return NULL;
            }
            list = temp;
            list[*listSize].str = str;
            list[*listSize].count = 1;
            (*listSize)++;
        }
    }
    return list;
}

// Function to process each file and display sorted word counts
void processFile(const char *filePath) {
    int fileDesc = open(filePath, O_RDONLY);
    if (fileDesc == -1) {
        perror("Error: Unable to open file");
        return;
    }

    int listSize = 0;
    struct wordObj *list = countWords(fileDesc, &listSize);
    close(fileDesc);

    if (list == NULL || listSize == 0) {
        free(list);
        return;
    }

    // Sort the list of words by frequency and lexicographic order
    qsort(list, listSize, sizeof(struct wordObj), compareWords);

    // Print the sorted list and free memory
    for (int i = 0; i < listSize; i++) {
        printf("%s: %d\n", list[i].str, list[i].count);
        free(list[i].str);
    }
    printf("%d distinct words.\n", listSize);

    free(list);
}

// Directory traversal to process files
void processDirectory(const char *dirPath) {
    struct dirent *entry;
    DIR *dir = opendir(dirPath);

    if (dir == NULL) {
        perror("Error: Unable to open directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }

        char filePath[1024];
        snprintf(filePath, sizeof(filePath), "%s/%s", dirPath, entry->d_name);

        struct stat pathStat;
        stat(filePath, &pathStat);

        if (S_ISDIR(pathStat.st_mode)) {
            processDirectory(filePath);
        } else if (S_ISREG(pathStat.st_mode) && strstr(entry->d_name, ".txt")) {
            processFile(filePath);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Error: Expected one argument, found %d.\n", argc - 1);
     

