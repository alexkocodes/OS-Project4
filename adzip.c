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

void list_dir(const char *path, int indent) // function to prints out the hierarchy of files and directories
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

void archive_files(char *input, FILE *archive) // function to archive files
{
    // input_dir can be a directory or a file
    // If it's a directory, recursively call archive_files on all files in the directory
    // If it's a file, write the contents of the file to the archive

    struct stat st;
    if (lstat(input, &st) < 0)
    {
        perror("Error getting file/directory info");
        exit(1);
    }

    if (S_ISDIR(st.st_mode))
    {

        DIR *dir = opendir(input);
        if (dir == NULL)
        {
            perror("Error opening input directory");
            exit(1);
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL)
        {
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", input, entry->d_name);

            struct stat st;
            if (lstat(path, &st) < 0)
            {
                perror("Error getting file/directory info");
                exit(1);
            }

            if (S_ISDIR(st.st_mode))
            {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                {
                    continue;
                }

                archive_files(path, archive);
            }
            else
            {
                FILE *file = fopen(path, "r");
                if (file == NULL)
                {
                    perror("Error opening file");
                    exit(1);
                }

                char buffer[1024];
                size_t bytes_read;
                while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
                {
                    fwrite(buffer, 1, bytes_read, archive);
                }

                fclose(file);
            }
        }

        closedir(dir);
    };
    if (S_ISREG(st.st_mode))
    {
        FILE *file = fopen(input, "r");
        if (file == NULL)
        {
            perror("Error opening file");
            exit(1);
        }

        char buffer[1024];
        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0)
        {
            fwrite(buffer, 1, bytes_read, archive);
        }

        fclose(file);
    }
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

    if (operation == NULL || ad_file == NULL || input_file == NULL)
    {
        printf("Error: Missing arguments\n");
        exit(1);
    }
    if (strcmp(operation, "-c") == 0)
    { // if the operation is -c, then archive the files
        FILE *archive = fopen(ad_file, "w");
        if (archive == NULL)
        {
            perror("Error opening archive file");
            exit(1);
        }

        archive_files(input_file, archive);

        fclose(archive);

        printf("All files archived successfully.\n");
        exit(0);
    }
    if (strcmp(operation, "-a") == 0)
    { // if the operation is -p, then append the files to the archive
        FILE *archive = fopen(ad_file, "a");
        if (archive == NULL)
        {
            perror("Error opening archive file");
            exit(1);
        }

        archive_files(input_file, archive);
        fclose(archive);
        exit(0);
    }
    if (strcmp(operation, "-x") == 0)
    { // if the operation is -x, then extract the files
        FILE *archive = fopen(ad_file, "r");
        if (archive == NULL)
        {
            perror("Error opening archive file");
            exit(1);
        }

        char buffer[1024];
        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), archive)) > 0)
        {
            fwrite(buffer, 1, bytes_read, stdout);
        }

        fclose(archive);

        printf("All files extracted successfully.\n");
        exit(0);
    }
    if (strcmp(operation, "-m") == 0) // if the operation is -m, then print metadata from the archive
    {
        // Open the archive file
        FILE *archive = fopen(ad_file, "r");
        if (archive == NULL)
        {
            perror("Error opening archive file");
            exit(1);
        }

        fclose(archive);
        exit(0);
    }
    if (strcmp(operation, "-p") == 0) // if the operation is -p, then print the hierarchy of files and directories
    {
        list_dir(input_file, 0);
        exit(0);
    }
    else
    {
        printf("Error: Invalid operation\n");
        exit(1);
    }
}
