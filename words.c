#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#define VALIDCHAR(current, prev) ((current == '\'' && isalpha(prev)!=0) || isalpha(current)!=0)

int lSize = 0;
struct wordObj
{
    char* str;
    int count;
};

int compareWords(const void *a, const void *b) 
{
    const struct wordObj *wordA = (const struct wordObj *)a;
    const struct wordObj *wordB = (const struct wordObj *)b;

    // Sort by count in descending order
    if (wordA->count != wordB->count) {
        return wordB->count - wordA->count; // Descending order
    }

    // If counts are the same, sort by str lexicographically
    return strcmp(wordA->str, wordB->str);
}

struct wordObj* countWords(int fileDesc, const char *filename) {
    char current, prev = '\0';  // Initialize `prev` to track previous character
    struct wordObj *list = NULL;
    int listSize = 0;

    while (read(fileDesc, &current, 1) == 1) {
        int wordSize = 0;
        char *str = calloc(1, 1);  // Start with an empty string and allocate 1 byte

        // Check if current character can start a word
        if (isalpha(current)) {
            wordSize++;
            char *tempStr = realloc(str, wordSize + 1); // Resize for 1 character + null
            if (tempStr == NULL) {
                free(str);
                perror("Allocation failed.");
                return NULL;
            }
            str = tempStr;
            str[wordSize - 1] = current; // Append current character
            str[wordSize] = '\0';         // Null-terminate
        } 
        else if (current = '\'') {
            prev = current;
            if(read(fileDesc, &current, 1)==1 && isalpha(current)) {
                wordSize = 2;
                str = realloc(str, wordSize+1);
                str[wordSize-2] = prev;
                str[wordSize-1] = current;
                str[wordSize] = '\0';
            }
            
        }
        else {
            prev = current;
            free(str);
            continue;
        }

        // Build the word by reading additional characters
        while (read(fileDesc, &current, 1) == 1 && (isalpha(current) || current == '\'' || current == '-')) {
            if (current == '-' && !isalpha(prev)) {
                break;  // Only allow `-` when surrounded by letters
            }

            // if multiple apostrophes in a row, break string
            if(current == '\'' && prev == '\'')
                break;

            wordSize++;
         
            char *tempStr = realloc(str, wordSize + 1); // Resize for each new character
            if (tempStr == NULL) {
                free(str);
                perror("Allocation failed.");
                return NULL;
            }
            str = tempStr;
            str[wordSize - 1] = current; // Append current character
            str[wordSize] = '\0';         // Null-terminate
            prev = current;
        }

        // Trim trailing hyphens
        while (wordSize > 0 && str[wordSize - 1] == '-') {
            str[--wordSize] = '\0';
        }

        // If the word is empty after trimming, continue
        if (wordSize == 0) {
            free(str);
            continue;
        }

        // Check if word already exists in list
        int found = 0;
        for (int i = 0; i < listSize; i++) {
            if (strcmp(list[i].str, str) == 0) {
                list[i].count++;
                free(str);  // Free `str` if it's already in the list
                found = 1;
                break;
            }
        }

        // Add new word if not found
        if (!found) {
            struct wordObj *tempList = realloc(list, sizeof(struct wordObj) * (listSize + 1));
            if (tempList == NULL) {
                for (int i = 0; i < listSize; i++) {
                    free(list[i].str);
                }
                free(list);
                free(str);
                perror("Allocation failed");
                return NULL;
            }
            list = tempList;
            list[listSize].str = str;
            list[listSize].count = 1;
            listSize++;
        }
    }
    lSize = listSize;
    return list;
}

void processFile(const char *filePath) {
    int fileDesc = open(filePath, O_RDONLY);
    if (fileDesc == -1) {
        perror("Error: Unable to open file");
        return;
    }
    
    struct wordObj *list = countWords(fileDesc, filePath);
    close(fileDesc);
    if(list == NULL)
        return;

    printf("%d\n", lSize);
    qsort(list, lSize, sizeof(struct wordObj), compareWords);

    
    for(int i=0;i<lSize; i++)
    {
        struct wordObj temp = list[i];
        printf("%s: %d\n", list[i].str, list[i].count);
        free (temp.str);
    }

    printf("%d distinct words.\n", lSize);
    free(list);
}

void processDirectory(const char *dirPath) {
    struct dirent *entry;
    DIR *dir = opendir(dirPath);

    if (dir == NULL) {
        perror("Error: Unable to open directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Skip entries starting with a period
        if (entry->d_name[0] == '.') {
            continue;
        }

        char filePath[1024];
        snprintf(filePath, sizeof(filePath), "%s/%s", dirPath, entry->d_name);

        struct stat pathStat;
        stat(filePath, &pathStat);

        if (S_ISDIR(pathStat.st_mode)) {
            // Recurse into subdirectory
            processDirectory(filePath);
        } else if (S_ISREG(pathStat.st_mode) && strstr(entry->d_name, ".txt")) {
            // Process regular files ending with ".txt"
            processFile(filePath);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Error: Expected one argument, found %d.\n", argc - 1);
        return -1;
    }

    struct stat pathStat;
    stat(argv[1], &pathStat);

    if (S_ISDIR(pathStat.st_mode)) {
        // Argument is a directory
        processDirectory(argv[1]);
    } else if (S_ISREG(pathStat.st_mode)) {
        // Argument is a regular file
        processFile(argv[1]);
    } else {
        printf("Error: Argument is neither a file nor a directory.\n");
        return -1;
    }

    return 0;
}
