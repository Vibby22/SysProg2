struct wordObj* countWords(int fileDesc, const char *filename) {
    char current, prev = '\0';  // Track previous character to handle word boundaries
    struct wordObj *list = NULL;
    int listSize = 0;

    while (read(fileDesc, &current, 1) == 1) {
        int wordSize = 0;
        char *str = calloc(1, 1);  // Start with an empty string and allocate 1 byte

        // **Start a Word if the Character is Alphabetic or a Leading Apostrophe**
        if (isalpha(current) || (current == '\'' && read(fileDesc, &current, 1) == 1 && isalpha(current))) {
            // Allocate space and append the starting character (either alphabetic or valid apostrophe sequence)
            wordSize = 1;
            char *tempStr = realloc(str, wordSize + 1); 
            if (tempStr == NULL) {
                free(str);
                perror("Allocation failed.");
                return NULL;
            }
            str = tempStr;
            str[wordSize - 1] = current; // Append the initial character
            str[wordSize] = '\0';         // Null-terminate
        } else {
            prev = current;
            free(str);
            continue;
        }

        // **Build the Word, Allowing Apostrophes Only if Surrounded by Letters**
        while (read(fileDesc, &current, 1) == 1 && (isalpha(current) || current == '\'' || current == '-')) {
            if (current == '-' && !isalpha(prev)) {
                break;  // Only allow `-` if it’s surrounded by letters
            }
            if (current == '\'' && (!isalpha(prev) || !isalpha(current))) {
                break;  // Only allow `'` if surrounded by letters
            }

            wordSize++;
            char *tempStr = realloc(str, wordSize + 1); // Resize to accommodate the new character
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

        // **Remove Any Trailing Apostrophes or Hyphens**
        // If the word ends with an apostrophe or hyphen, trim it
        while (wordSize > 0 && (str[wordSize - 1] == '-' || str[wordSize - 1] == '\'')) {
            str[--wordSize] = '\0';
        }

        // **Skip Empty Words after Trimming**
        if (wordSize == 0) {
            free(str);
            continue;
        }

        // **Check if Word Exists in List and Update Count if Found**
        int found = 0;
        for (int i = 0; i < listSize; i++) {
            if (strcmp(list[i].str, str) == 0) {
                list[i].count++;
                free(str);  // Free `str` if it's already in the list
                found = 1;
                break;
            }
        }

        // **Add New Word to List if Not Found**
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
