/*
 * =====================================================================================
 *
 *       Filename:  shell.c
 *
 *    Description:  Shell implementation
 *
 *        Version:  1.0
 *        Created:  16/04/08 18:09:22
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Aitor Viana Sanchez (avs), aitor.viana.sanchez@esa.int
 *        Company:  European Space Agency (ESA-ESTEC)
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int cmpstr(const void *vs1, const void *vs2)
{
    char * const *s1 = vs1;
    char * const *s2 = vs2;
    return strcmp(*s1, *s2);
}

int main(void)
{
    /* ensure this list is sorted! */
    char *command[] =
    {
        "exit",
    };
    size_t numcommands = sizeof command / sizeof command[0];
    char line[4096] = {0};
    char safeline[4096] = {0};
    int done = 0;
    int i = 0;
    char *s = line;
    char **t = NULL;
    char *prompt = "skyeye> ";
    char *args = NULL;
    char *nl = NULL;

    while(!done)
    {
        printf("%s", prompt);
        fflush(stdout);
        if(NULL == fgets(line, sizeof line, stdin))
        {
            t = NULL;
            done = 1;
        }
        else
        {
            nl = strchr(line, '\n');
            if(nl != NULL)
            {
                *nl = '\0';
                strcpy(safeline, line);
            }
            else
            {
                int ch;
                printf("Line too long! Ignored.\n");
                while((ch = getchar()) != '\n' && ch != EOF)
                {
                    continue;
                }
                if(ch == EOF)
                {
                    done = 1;
                }
            }
            args = strchr(line, ' ');
            if(args != NULL)
            {
                *args++ = '\0';
            }
            t = bsearch(&s,
                    command,
                    numcommands,
                    sizeof command[0],
                    cmpstr);
        }

        if(!done && t != NULL)
        {
            i = (int)(t - command);
            switch(i)
            {
                case 0:  done = 1;break;
                default: break;
            }
        }
        else
            printf("Unknown command %s. \n", line);
    }

    return 0;
}
