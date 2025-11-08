
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

// File Operations Function

void copyFile(const char *src, const char *dest, int overwrite){
    FILE *sourceFile = fopen(src, "rb");
    if (sourceFile == NULL) {
        perror("fopen source error");
        return;
    }

    if (!overwrite && access(dest, F_OK) == 0) {
        printf("File %s already exists. Use -f to overwrite.\n", dest);
        fclose(sourceFile);
        return;
    }

    FILE *destFile = fopen(dest, "wb");
    if (destFile == NULL){
        perror("fopen dest error");
        fclose(sourceFile);
        return;
    }

    char buffer [1024];
    size_t n;
    while ((n = fread(buffer,1,sizeof(buffer), sourceFile)) > 0){
        fwrite(buffer,1,n,destFile);
    }   

    fclose(sourceFile);
    fclose(destFile);
}

void displayFileContent(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen error");
        return;
    }

    char buffer[1024];
    int lineCount = 0;
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
        if (++lineCount >= 30) {
            printf("Press Enter to continue...\n");
            getchar(); // Wait for the user to press Enter
            lineCount = 0;
        }
    }

    fclose(file);
}

// Function to evaluate prefix expressions
int evaluatePrefix(char **expr) {
    char *token = strtok(*expr, " ");
    if (token == NULL) return 0;

    // Move the pointer forward for the next token
    *expr += strlen(token) + 1;

    if (strcmp(token, "+") == 0) {
        return evaluatePrefix(expr) + evaluatePrefix(expr);
    } else if (strcmp(token, "-") == 0) {
        return evaluatePrefix(expr) - evaluatePrefix(expr);
    } else {
        return atoi(token);  // Convert the token to an integer
    }
}

int main (){
    char command[100];
    char cwd[1024];
    time_t t;
    struct tm *tmp;
    struct utsname sys_info;

    while (1) {
        printf("Enter a command: ");
        fgets(command, sizeof(command),stdin);

        //remove newline character from input
        command[strcspn(command, "\n")] = 0;

        // Quit
        if (strcmp(command, "quit") == 0){
            printf("Exiting.\n");
            break;

        // Current Directory
        } else if (strcmp(command, "path") == 0){
            if (getcwd(cwd, sizeof(cwd)) != NULL){
                printf("Current working directory: %s\n", cwd);
            } else {
                perror("getcwd() error");
            }

        // Current time
        } else if (strcmp(command, "time") == 0){
            t = time(NULL);
            tmp = localtime(&t);
            if (tmp == NULL) {
                perror("localtime error");
            }else {
                printf("Current date and time: %s", asctime(tmp));
            }

        // System information
        } else if (strcmp(command, "sys") == 0) {
            if (uname(&sys_info) != 0){
                perror("uname error");
            } else {
                printf("System Name: %s\n", sys_info.sysname);
                printf("System Version: %s\n", sys_info.version);
                printf("Machine/CPU Type: %s\n", sys_info.machine);
            }

        // Calculate 
        } else if (strncmp(command, "calculate ", 10) == 0) {
            char *expr = command + 10;
            printf("Result: %d\n", evaluatePrefix(&expr));
        
        // Put command
        } else if (strncmp(command, "put ", 4) == 0){
            char *args = command + 4;
            char *dirname = strtok(args, " ");
            char *filename = strtok(NULL, " ");
            char *flag = strtok(NULL, " ");

            if (dirname && filename) {
                if (mkdir(dirname, 0777) != 0 && errno != EEXIST) {
                    perror ("mkdir error");
                    continue;
                }
                char destPath[1024];
                snprintf(destPath, sizeof(destPath), "%s/%s",dirname, filename);
                copyFile(filename, destPath, flag && strcmp(flag, "-f") == 0);
            } else {
                printf("Usage: put dirname filename [-f]\n");
            }

        // Get Command
        } else if (strncmp(command, "get ", 4) == 0) {
            char *filename = command + 4;
            displayFileContent(filename);

        // Unknown command
        } else {
            printf("Unknown command: %s\n", command);
        }
    }

    return 0;
}