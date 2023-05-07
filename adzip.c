/* Group Parners: Alex Ko (fyk211) and Ritin Malhotra (rm5486)*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <math.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <sys/select.h>
#include <errno.h>
#include <dirent.h>

void list_dir(const char *path, int indent)
{
    DIR *dir = opendir(path);
    if (!dir)
    {
        printf("Failed to open directory %s\n", path);
        return;
    }

    struct dirent *entry;
    struct stat st;

    while ((entry = readdir(dir)) != NULL)
    {
        char full_path[1024];

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        sprintf(full_path, "%s/%s", path, entry->d_name);

        if (stat(full_path, &st) == -1)
        {
            printf("Failed to get file stats for %s\n", full_path);
            continue;
        }

        printf("%*s%s - %o\n", indent, "", entry->d_name, st.st_mode & 0777);

        if (S_ISDIR(st.st_mode))
        {
            list_dir(full_path, indent + 4);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[])
{
    // Declare variables for args (as specified by the assignment)
    char *operation;
    char *ad_file;
    char *input_file;

    // Get the comamnd-line args
    operation = argv[1];
    ad_file = argv[2];
    input_file = argv[3];

    list_dir(input_file, 0);

    exit(0);
}
