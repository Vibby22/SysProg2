#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#define VALIDCHAR(current) ((current == '\'' && isalpha(*(&current+1))!=0) || isalpha(current)!=0)

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

struct wordObj* countWords(int fileDesc, const char *filename)
{
    char current;
    struct wordObj *list = NULL;
    int listSize = 0;
    
    while (read(fileDesc, &current, 1) == 1) 
    {
        int wordSize = 1;
        char *str = calloc(2, wordSize+1);
        
       //check for valid first char, else reiterate loop
        if(VALIDCHAR(current)!=0)
        {
            str[0] = current;
        }
        else
            continue;

        //add each character to string
        while ((read(fileDesc, &current, 1) == 1) && (current == '-' || VALIDCHAR(current)))
        {
            if((current == '-' && (isalpha(*(&current-1)) != 0 && isalpha(*(&current+1)) != 0)) || VALIDCHAR(current))
            {
                wordSize++;
                char *tempStr = realloc(str, wordSize+1);
                
                //allocation failure
                // if(tempStr == NULL)
                // {
                //     free(str);
                //     perror("Allocation failed.");
                //     return NULL;
                // }
                str = tempStr;
                str[wordSize-1] = current;
            }
        }
        //last char of string is NULL
        str[wordSize] = '\0';


        //if list is empty
        if(list == NULL)
        {
            list = malloc(sizeof(struct wordObj));
            list[0].str = str;
            list[0].count = 1;
            listSize++;
        }

        // list is not empty, traverse for identical words
        else
        {
            for(int i=0; i<listSize; i++)
            {
                if(strcmp(list[i].str, str) == 0)
                {
                    list[i].count++;
                    free(str);
                    break;
                }
            }
            // no identical words
            struct wordObj *temp = realloc(list, sizeof(struct wordObj)*(listSize+1));
            // if(temp == NULL)
            // {
            //     free(list);
            //     free(str);
            //     perror("Allocation failed");
            //     return NULL;
            // }

            list = temp;
            list[listSize].str = str;
            list[listSize].count = 1;
            listSize++;
            //printf("%s : %d\n", list[listSize].str, list[listSize].count);
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
    //qsort(list, lSize, sizeof(struct wordObj), compareWords);
    
    for(int i=0;i<lSize; i++)
    {
        printf("%s: %d\n", list[i].str, list[i].count);
        //free (temp.str);
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
