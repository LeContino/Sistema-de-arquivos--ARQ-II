#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void interfaceHandleCommand(char *command)
{
    char *commandName;
    const char breakChar[1] = " ";

    commandName = strtok(command, breakChar);

    if (!strcmp(commandName, "CD"))
    {
        // printf("%s\n", commandName);

        char *folderDir = strtok(NULL, breakChar);
        if (folderDir == NULL)
        {
            printf("One parameter expected for this command\n");
            return;
        }

        // printf("%s\n", folderDir);

        char *invalidParameter = strtok(NULL, breakChar);
        if (invalidParameter != NULL)
        {
            printf("One parameter expected for this command\n");
            return;
        }

        // printf("call cd function with %s \n", folderDir);

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

        // DIR(arq);

        return;
    }

    if (!strcmp(commandName, "RM"))
    {

        return;
    }

    if (!strcmp(commandName, "MKDIR"))
    {
        // printf("%s\n", commandName);

        char *dirName = strtok(NULL, breakChar);
        if (dirName == NULL)
        {
            printf("One parameter expected for this command\n");
            return;
        }

        // printf("%s\n", folderDir);

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
        printf(">>>>>>>>> ");
        char *command;
        fgets(command, 1024000, stdin);
        // printf("READ: %s", command);

        interfaceHandleCommand(command);
    }
}

int main()
{
    interfaceLoop();
}