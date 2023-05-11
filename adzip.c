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
        FILE *file = fopen(input, "rb");
        if (file == NULL)
        {
            perror("Error opening file");
            exit(1);
        }

        fprintf(archive, "%s\n", input);
        fprintf(archive, "%d\n", st.st_uid);
        fprintf(archive, "%d\n", st.st_gid);
        fprintf(archive, "%d\n", 0); // size of directory is 0
        // fprintf(archive, "%o\n", st.st_mode);
        char permissions_string[11];
        permissions_string[0] = S_ISDIR(st.st_mode) ? 'd' : '-';
        permissions_string[1] = (st.st_mode & S_IRUSR) ? 'r' : '-';
        permissions_string[2] = (st.st_mode & S_IWUSR) ? 'w' : '-';
        permissions_string[3] = (st.st_mode & S_IXUSR) ? 'x' : '-';
        permissions_string[4] = (st.st_mode & S_IRGRP) ? 'r' : '-';
        permissions_string[5] = (st.st_mode & S_IWGRP) ? 'w' : '-';
        permissions_string[6] = (st.st_mode & S_IXGRP) ? 'x' : '-';
        permissions_string[7] = (st.st_mode & S_IROTH) ? 'r' : '-';
        permissions_string[8] = (st.st_mode & S_IWOTH) ? 'w' : '-';
        permissions_string[9] = (st.st_mode & S_IXOTH) ? 'x' : '-';
        permissions_string[10] = '\0';
        fprintf(archive, "%s\n", permissions_string);

        fclose(file);

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
                    continue;

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
                // fprintf(archive, "%o\n", st.st_mode);
                char permissions_string[11];
                permissions_string[0] = S_ISDIR(st.st_mode) ? 'd' : '-';
                permissions_string[1] = (st.st_mode & S_IRUSR) ? 'r' : '-';
                permissions_string[2] = (st.st_mode & S_IWUSR) ? 'w' : '-';
                permissions_string[3] = (st.st_mode & S_IXUSR) ? 'x' : '-';
                permissions_string[4] = (st.st_mode & S_IRGRP) ? 'r' : '-';
                permissions_string[5] = (st.st_mode & S_IWGRP) ? 'w' : '-';
                permissions_string[6] = (st.st_mode & S_IXGRP) ? 'x' : '-';
                permissions_string[7] = (st.st_mode & S_IROTH) ? 'r' : '-';
                permissions_string[8] = (st.st_mode & S_IWOTH) ? 'w' : '-';
                permissions_string[9] = (st.st_mode & S_IXOTH) ? 'x' : '-';
                permissions_string[10] = '\0';
                fprintf(archive, "%s\n", permissions_string);

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

        // before we write the actual data, write the metadata of this file first
        // write the file name, user id, group id, size and permissions
        // write the full path of the file as the filename
        fprintf(archive, "%s\n", input);
        fprintf(archive, "%d\n", st.st_uid);
        fprintf(archive, "%d\n", st.st_gid);
        fprintf(archive, "%lld\n", st.st_size);
        // fprintf(archive, "%o\n", st.st_mode);
        char permissions_string[11];
        permissions_string[0] = S_ISDIR(st.st_mode) ? 'd' : '-';
        permissions_string[1] = (st.st_mode & S_IRUSR) ? 'r' : '-';
        permissions_string[2] = (st.st_mode & S_IWUSR) ? 'w' : '-';
        permissions_string[3] = (st.st_mode & S_IXUSR) ? 'x' : '-';
        permissions_string[4] = (st.st_mode & S_IRGRP) ? 'r' : '-';
        permissions_string[5] = (st.st_mode & S_IWGRP) ? 'w' : '-';
        permissions_string[6] = (st.st_mode & S_IXGRP) ? 'x' : '-';
        permissions_string[7] = (st.st_mode & S_IROTH) ? 'r' : '-';
        permissions_string[8] = (st.st_mode & S_IWOTH) ? 'w' : '-';
        permissions_string[9] = (st.st_mode & S_IXOTH) ? 'x' : '-';
        permissions_string[10] = '\0';
        fprintf(archive, "%s\n", permissions_string);

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

void printMetadata(FILE *archive) // function to print the metadata of the archived files and skip the actual data
{
    // go to the beginning of the file
    fseek(archive, 0, SEEK_SET);

    // read the file name, user id, group id, size and permissions and skip the actual data
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), archive) != NULL) // read until EOF
    {
        int size;
        printf("%s", buffer);
        fgets(buffer, sizeof(buffer), archive);
        fgets(buffer, sizeof(buffer), archive);
        fgets(buffer, sizeof(buffer), archive);
        size = atoi(buffer);
        fgets(buffer, sizeof(buffer), archive);
        printf("Permissions: %s\n", buffer);

        // skip the actual data
        fseek(archive, size, SEEK_CUR);
    }
}

// Create a sturct to store all paths in the archive
typedef struct
{
    char **paths;
    int num_files;
} archive_paths;

// Get the full paths of all files in the archive
archive_paths *getPaths(FILE *archive)
{
    // Go to the beginning of the file
    fseek(archive, 0, SEEK_SET);

    archive_paths *paths = malloc(sizeof(archive_paths));

    // Create a dynamic array to store the file names
    char **file_names = malloc(sizeof(char *));
    int num_files = 0;

    // read the file name, user id, group id, size and permissions and skip the actual data
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), archive) != NULL) // read until EOF
    {
        // printf("File name: %s", buffer);
        // Add the file name to the array
        file_names[num_files] = malloc(sizeof(buffer));
        strcpy(file_names[num_files], buffer);
        num_files++;
        // Skip the rest of the metadata
        int size;
        fgets(buffer, sizeof(buffer), archive);
        fgets(buffer, sizeof(buffer), archive);
        fgets(buffer, sizeof(buffer), archive);
        size = atoi(buffer);
        fgets(buffer, sizeof(buffer), archive);

        // Skip the actual data
        fseek(archive, size, SEEK_CUR);
    }

    // Print the file names in the order of the array
    // for (int i = 0; i < num_files; i++) {
    //     printf("%s", file_names[i]);
    // }
    paths->paths = file_names;
    paths->num_files = num_files;
    return paths;
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
        // if this is a directory, skip it. We don't need to create it
        if (mode[0] == 'd')
        {
            continue;
        }

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
        free(temp);
        fclose(newFile);
    }
}

// Break up a file path into all its "subpaths"
// For ex:
/* test_folder/subfolder2/test2.txt
-> test_folder/subfolder2/
-> test_folder/
*/
// Function primarily generated via ChatGPT but required multiple alterations
// to avoid unnecessary strings in our output array
char **break_up_string(char *input_string)
{
    char **output_strings = malloc(sizeof(char *) * (strlen(input_string) + 1));
    int output_index = 0;
    char *token = strtok(input_string, "/");
    char temp_string[1000] = {0};
    while (token != NULL)
    {
        strcat(temp_string, token);
        strcat(temp_string, "/");
        output_strings[output_index] = malloc(sizeof(char) * (strlen(temp_string) + 1));
        strcpy(output_strings[output_index], temp_string);
        output_index++;
        token = strtok(NULL, "/");
    }
    output_strings[output_index] = malloc(sizeof(char) * (strlen(input_string) + 1));
    strcpy(output_strings[output_index], input_string);
    // Before returning, remove the trailing newlines from each string
    for (int i = 0; i < output_index + 1; i++)
    {
        if (output_strings[i][strlen(output_strings[i]) - 2] == '\n')
        {
            output_strings[i][strlen(output_strings[i]) - 2] = '\0';
        }
    }
    // And remove the last string, which is just the input string
    output_strings[output_index] = NULL;
    return output_strings;
}

// Function to compare how many slashes are in two strings
// Used to sort the paths and "subpaths" by length
// Passed as a comparator to C's qsort()
int compare_strings(const void *a, const void *b)
{
    const char *str_a = *(const char **)a;
    const char *str_b = *(const char **)b;

    int slashes_a = 0;
    for (int i = 0; i < strlen(str_a); i++)
    {
        if (str_a[i] == '/')
        {
            slashes_a++;
        }
    }

    int slashes_b = 0;
    for (int i = 0; i < strlen(str_b); i++)
    {
        if (str_b[i] == '/')
        {
            slashes_b++;
        }
    }

    if (slashes_a < slashes_b)
    {
        return -1;
    }
    else if (slashes_a > slashes_b)
    {
        return 1;
    }
    else
    {
        return strcmp(str_a, str_b);
    }
}

// Function to remove duplicates from an array of strings
// Used to remove duplicate paths and "subpaths" while printing
// the hierarchy of files and (sub)directories in our archive
void removeDuplicates(char **arr, int size)
{
    int i, j, k;
    for (i = 0; i < size; ++i)
    {
        for (j = i + 1; j < size;)
        {
            if (strcmp(arr[j], arr[i]) == 0)
            {
                for (k = j; k < size; ++k)
                {
                    arr[k] = arr[k + 1];
                }
                --size;
            }
            else
            {
                ++j;
            }
        }
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

        // split by comma and loop through each file and directory
        char *token = strtok(input_file, ",");
        while (token != NULL)
        {

            // archive the file
            archive_files(token, archive);

            token = strtok(NULL, ",");
        }

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

        char *token = strtok(input_file, ",");
        while (token != NULL)
        {

            // archive the file
            archive_files(token, archive);

            token = strtok(NULL, ",");
        }

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
        FILE *archive = fopen(ad_file, "r");
        // Get the paths of all files in our archive
        // We will use these paths to get the hierarchy of files and directories
        // in our archive
        archive_paths *paths = getPaths(archive);
        fclose(archive);

        // Get all the possible paths in our directory
        // These are the paths broken up by the number of slashes in each file path
        // It currently allows for 100 times the number of file paths we have
        // Since there is no reasonable way we reach a higher depth than that for each file
        char **all_paths = malloc(sizeof(char *) * paths->num_files * 100);
        int all_paths_index = 0;
        if (fork() == 0)
        {
            // Get the paths broken up by slashes and add them to the array
            for (int i = 0; i < paths->num_files; i++)
            {
                char **output_strings = break_up_string(paths->paths[i]);
                for (int j = 0; j < 100; j++)
                {
                    if (output_strings[j] == NULL)
                    {
                        break;
                    }
                    // printf("%s\n", output_strings[j]);
                    all_paths[all_paths_index] = malloc(sizeof(char) * (strlen(output_strings[j]) + 1));
                    strcpy(all_paths[all_paths_index], output_strings[j]);
                    all_paths_index++;
                }
            }
        }
        else
        {
            wait(NULL);
        }

        // Sort all the broken up paths by the number of slashes they have
        // This enables us to print the paths from the "shallowest" to the "deepest"
        // (i.e., from test_folder/ to subfolder3/ etc.)
        qsort(all_paths, all_paths_index, sizeof(all_paths), compare_strings);

        // Remove the duplicates in our broken up paths
        // We are bound to have duplicates since many file paths may
        // come under the same subfolder(s)
        removeDuplicates(all_paths, all_paths_index);

        // Print the unique paths we have got, starting from the "shallowest"
        // to the "deepest"
        for (int i = 0; i < all_paths_index; i++)
        {
            // move on if the string is empty or null
            if (all_paths[i] == NULL || strcmp(all_paths[i], "") == 0)
            {
                continue;
            }
            printf("%s\n", all_paths[i]); // Temporarily print the strings
        }

        exit(0);
    }
    else
    {
        printf("Error: Invalid operation\n");
        exit(1);
    }
}
