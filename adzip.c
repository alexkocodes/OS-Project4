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

        if (strcmp(entry->d_name, "..") == 0)
            continue; // Skip parent

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

// Function to print the hierarchy of files and directories
// inside the specified directory in a human-readable format
void listDirHierarchy(const char *basePath, int depth)
{
    DIR *dir;
    struct dirent *entry;
    struct stat filestat;

    if (!(dir = opendir(basePath)))
    {
        fprintf(stderr, "Failed to open directory %s\n", basePath);
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        char path[1024];
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", basePath, entry->d_name);
        if (lstat(path, &filestat) < 0)
        {
            fprintf(stderr, "Failed to stat %s\n", path);
            continue;
        }

        printf("%*s%s\n", depth * 2, "", entry->d_name);

        if (S_ISDIR(filestat.st_mode))
            listDirHierarchy(path, depth + 1);
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

            // store the directory in the archive as well
            if (S_ISDIR(st.st_mode))
            {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                    continue; // Skip parent
                // before we write the actual data, write the metadata of this directory first
                // write the file name, user id, group id, size and permissions
                // fprintf(archive, "%s\n", entry->d_name);
                // fprintf(archive, "%d\n", st.st_uid);
                // fprintf(archive, "%d\n", st.st_gid);
                // fprintf(archive, "%lld\n", st.st_size);
                // fprintf(archive, "%o\n", st.st_mode & 0777);

                archive_files(path, archive); // recursively call archive_files on the directory
            }

            else
            {
                FILE *file = fopen(path, "rb");
                if (file == NULL)
                {
                    perror("Error opening file");
                    exit(1);
                }

                // before we write the actual data, write the metadata of this file first
                // write the file name, user id, group id, size and permissions
                // write the full path of the file as the filename
                fprintf(archive, "%s\n", path);
                fprintf(archive, "%d\n", st.st_uid);
                fprintf(archive, "%d\n", st.st_gid);
                fprintf(archive, "%lld\n", st.st_size);
                fprintf(archive, "%o\n", st.st_mode & 0777);

                // now write the actual data
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
        FILE *file = fopen(input, "rb");
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

void printMetadata(FILE *archive) // function to print the metadata of the archived files and skip the actual data
{
    // go to the beginning of the file
    fseek(archive, 0, SEEK_SET);

    // read the file name, user id, group id, size and permissions and skip the actual data
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), archive) != NULL) // read until EOF
    {
        int size;
        printf("File name: %s", buffer);
        fgets(buffer, sizeof(buffer), archive);
        printf("User ID: %s", buffer);
        fgets(buffer, sizeof(buffer), archive);
        printf("Group ID: %s", buffer);
        fgets(buffer, sizeof(buffer), archive);
        printf("Size: %s", buffer);
        size = atoi(buffer);
        fgets(buffer, sizeof(buffer), archive);
        printf("Permissions: %s\n", buffer);

        // skip the actual data
        fseek(archive, size, SEEK_CUR);
    }
}

void extractArchive(FILE *archive)
{

    char filename[1024], uid[1024], gid[1024], mode[1024];

    int size;
    // go to the beginning of the file
    fseek(archive, 0, SEEK_SET);

    // read the file name, user id, group id, size and permissions
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), archive) != NULL) // read until EOF
    {
        // read the file name, user id, group id, size and permissions

        strcpy(filename, buffer);
        printf("File name: %s", filename);
        fgets(buffer, sizeof(buffer), archive);
        strcpy(uid, buffer);
        printf("User ID: %s", uid);
        fgets(buffer, sizeof(buffer), archive);
        strcpy(gid, buffer);
        printf("Group ID: %s", gid);
        fgets(buffer, sizeof(buffer), archive);
        size = atoi(buffer);
        printf("Size: %s", buffer);
        fgets(buffer, sizeof(buffer), archive);
        strcpy(mode, buffer);
        printf("Permissions: %s\n", mode);

        // create the file with the same metadata, create the folders as well
        // create the folders
        char *token;
        token = strtok(filename, "/");
        char *temp = malloc(strlen(filename) + 1);
        strcpy(temp, token);
        mkdir(temp, 0777); // create the first folder
        // create the rest of the folders
        while ((token = strtok(NULL, "/")) != NULL)
        {
            // append the next folder to the path
            strcat(temp, "/");
            strcat(temp, token);
            mkdir(temp, 0777);
        }
        // remove the last folder since it should be a file
        rmdir(temp);

        // create each file based on the metadata
        FILE *newFile;
        newFile = fopen(temp, "wb");
        if (newFile == NULL)
        {
            perror("Error creating file");
            exit(1);
        }

        // write the actual data based on the size
        char buffer[size];
        size_t bytes_read;
        bytes_read = fread(buffer, 1, size, archive);
        fwrite(buffer, 1, bytes_read, newFile);

        // fseek(archive, size, SEEK_CUR);
        // close the file
        // free(temp);
        fclose(newFile);
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

    if (strcmp(operation, "-c") == 0) // if the operation is -c, then archive the files
    {
        if (operation == NULL || ad_file == NULL || input_file == NULL)
        {
            printf("Error: Missing arguments\n");
            exit(1);
        }
        FILE *archive = fopen(ad_file, "wb");
        if (archive == NULL)
        {
            perror("Error opening archive file");
            exit(1);
        }

        archive_files(input_file, archive);

        printf("All files archived successfully.\n");
        fclose(archive);
        exit(0);
    }
    if (strcmp(operation, "-a") == 0) // if the operation is -p, then append the files to the archive
    {
        if (operation == NULL || ad_file == NULL || input_file == NULL)
        {
            printf("Error: Missing arguments\n");
            exit(1);
        }
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
    if (strcmp(operation, "-x") == 0) // if the operation is -x, then extract the files
    {
        if (operation == NULL || ad_file == NULL)
        {
            printf("Error: Missing arguments\n");
            exit(1);
        }
        FILE *archive = fopen(ad_file, "rb");
        if (archive == NULL)
        {
            perror("Error opening archive file");
            exit(1);
        }

        extractArchive(archive);

        fclose(archive);

        printf("All files extracted successfully.\n");
        exit(0);
    }
    if (strcmp(operation, "-m") == 0) // if the operation is -m, then print metadata from the archive
    {
        if (operation == NULL || ad_file == NULL)
        {
            printf("Error: Missing arguments\n");
            exit(1);
        }
        // Open the archive file
        FILE *archive = fopen(ad_file, "rb");
        if (archive == NULL)
        {
            perror("Error opening archive file");
            exit(1);
        }

        printMetadata(archive);
        fclose(archive);
        exit(0);
    }
    if (strcmp(operation, "-p") == 0) // if the operation is -p, then print the hierarchy of files and directories
    {
        if (operation == NULL || ad_file == NULL)
        {
            printf("Error: Missing arguments\n");
            exit(1);
        }
        listDirHierarchy(input_file, 0);
        exit(0);
    }
    else
    {
        printf("Error: Invalid operation\n");
        exit(1);
    }
}
