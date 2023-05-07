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
    DIR *dir = opendir(path); // Open the directory
    if (!dir)
    {
        printf("Failed to open directory %s\n", path);
        return;
    }

    struct dirent *entry; // entry is each file in the directory
    struct stat fileInfo; // Declare stat struct to hold file stats

    while ((entry = readdir(dir)) != NULL) // Iterate through the directory
    {
        char full_path[1024];

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue; // Skip self and parent

        sprintf(full_path, "%s/%s", path, entry->d_name); // Get the full path of the file
        // printf("%s\n", full_path);                        // Print the full path

        if (stat(full_path, &fileInfo) == -1)
        {
            printf("Failed to get file stats for %s\n", full_path);
            continue;
        }

        // Print the file name, user id, group id, size and permissions
        printf("%*s", indent, ""); // Indent by the specified amount
        printf("File name: %s\n", entry->d_name);
        printf("%*s", indent, "");
        printf("User ID: %d\n", fileInfo.st_uid);
        printf("%*s", indent, "");
        printf("Group ID: %d\n", fileInfo.st_gid);
        printf("%*s", indent, "");
        printf("Size: %lld\n", fileInfo.st_size);
        printf("%*s", indent, "");
        printf("Permissions: %o\n\n", fileInfo.st_mode & 0777);

        if (S_ISDIR(fileInfo.st_mode)) // Recursively call list_dir if the file is a directory
        {
            list_dir(full_path, indent + 4); // Indent by 4 spaces
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
