#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define maxCharsInFileSystem 1024000
char currentPath[maxCharsInFileSystem] = "/";

void interfaceHandleCommand(char *command)
{
    const char breakChar[1] = " ";
    char *commandName = strtok(command, breakChar);
    if (!strcmp(commandName, "CD"))
    {
        char *folderDir = strtok(NULL, breakChar);
        if (folderDir == NULL)
        {
            printf("One parameter expected for this command\n");
            return;
        }
        char *invalidParameter = strtok(NULL, breakChar);
        if (invalidParameter != NULL)
        {
            printf("One parameter expected for this command\n");
            return;
        }
        // CD(arq,dir);
        return;
    }

    if (!strcmp(commandName, "DIR"))
    {
        char *invalidParameter = strtok(NULL, breakChar);
        if (invalidParameter != NULL)
        {
            printf("No parameters expected for this command\n");
            return;
        }
        // DIR();
        return;
    }

    if (!strcmp(commandName, "RM"))
    {
        return;
    }

    if (!strcmp(commandName, "MKDIR"))
    {
        char *dirName = strtok(NULL, breakChar);
        if (dirName == NULL)
        {
            printf("One parameter expected for this command\n");
            return;
        }
        char *invalidParameter = strtok(NULL, breakChar);
        if (invalidParameter != NULL)
        {
            printf("One parameter expected for this command\n");
            return;
        }
        // MKDIR(arq,dirName);
        return;
    }

    if (!strcmp(commandName, "MKFILE"))
    {
        char *fileName = strtok(NULL, breakChar);
        if (fileName == NULL)
        {
            printf("One parameter expected for this command\n");
            return;
        }
        char *invalidParameter = strtok(NULL, breakChar);
        if (invalidParameter != NULL)
        {
            printf("One parameter expected for this command\n");
            return;
        }
        // MKFILE(arq, fileName)
        return;
    }

    if (!strcmp(commandName, "EDIT"))
    {
        return;
    }

    if (!strcmp(commandName, "MOVE"))
    {
        return;
    }

    if (!strcmp(commandName, "RENAME"))
    {
        return;
    }

    printf("Unrecognized command\n");
    return;
}

void interfaceLoop()
{
    while (1)
    {
        printf("%s\n", currentPath);
        printf("> ");
        char *command;
        fgets(command, maxCharsInFileSystem, stdin);

        command[strcspn(command, "\n")] = 0;
        // printf("%s\n", command);
        interfaceHandleCommand(command);
    }
}

int main()
{
    interfaceLoop();
}