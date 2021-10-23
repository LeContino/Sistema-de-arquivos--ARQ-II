#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void interfaceHandleCommand(char *command)
{
    char *token;
    const char breakChar[2] = " ";

    token = strtok(command, breakChar);

    if (token == "CD")
    {
        token = strtok(NULL, breakChar);

        char *folderDir = token;

        token = strtok(NULL, breakChar);
        if (token != NULL)
        {
            printf("One parameter expected for this command\n");
        }
        return;
    }
    //strcmp()
    if (token == "DIR")
    {

        return;
    }

    if (token == "RM")
    {

        return;
    }

    if (token == "MKDIR")
    {

        return;
    }

    if (token == "MKFILE")
    {

        return;
    }

    if (token == "EDIT")
    {

        return;
    }

    if (token == "MOVE")
    {

        return;
    }

    if (token == "RENAME")
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
        fgets(command, 500, stdin);
        printf("READ: %s", command);

        // if (command[0] != '\0')
        // {
        interfaceHandleCommand(command);
        // }
    }
}

int main()
{
    interfaceLoop();
}