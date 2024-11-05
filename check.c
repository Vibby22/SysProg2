struct wordObj* countWords(int fileDesc, const char *filename) {
    char current, prev = '\0';  // Initialize `prev` to track previous character
    struct wordObj *list = NULL;
    int listSize = 0;

    while (read(fileDesc, &current, 1) == 1) {
        int wordSize = 0;
        char *str = calloc(1, 1);  // Start with an empty string and allocate 1 byte

        // Check if current character can start a word
        if (isalpha(current)) {
            wordSize = 1;
            char *tempStr = realloc(str, wordSize + 1); // Resize for 1 character + null
            if (tempStr == NULL) {
                free(str);
                perror("Allocation failed.");
                return NULL;
            }
            str = tempStr;
            str[wordSize - 1] = current; // Append current character
            str[wordSize] = '\0';         // Null-terminate
        } else {
            prev = current;
            free(str);
            continue;
        }

        // Build the word by reading additional characters
        while (read(fileDesc, &current, 1) == 1 && (isalpha(current) || current == '-')) {
            if (current == '-' && !isalpha(prev)) {
                break;  // Only allow `-` when surrounded by letters
            }
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
