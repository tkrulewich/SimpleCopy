#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define CHUNK_SIZE 512

int main(int argc, char** argv) {

    // the command requires two arguments passed. If given less show the usage syntax and exit
    if (argc < 3) {
        fprintf(stderr, "Usage: copy sourceFile destinationFile\n");
        exit(1);
    }


    // if the source file doesnt exist alert the user and exit
    if (access(argv[1], F_OK) == -1) {
        fprintf(stderr,"Error file not found: %s!\n", argv[1]);
        exit(1);
    }

    if (access(argv[1], R_OK) == -1) {
        fprintf(stderr, "Error user does not have permission to read file!\n");
        exit(1);
    }

    // if the destination file already exists
    if (access(argv[2], F_OK) == 0) {
        // stores line of input
        char input[25];
        // stores character response if user gave single character
        char response = -1;

        do {
            // ask the user if they want to overwrite the file
            printf("File exists: %s. Overwrite? Y or N: ", argv[2]);

            // get a line of input
            fgets(input, 250, stdin);

            // if the user entered only one character, get the first character
            // \n is included in line we dont want that but we did want to remove it from the input stream so we read
            // the whole line not just one character
            if (strlen(input) == 2) {
                response = input[0];
            }

            // if the user doesnt want to overwrite the file, quit
            if (response == 'N' || response == 'n') {
                exit(0);
            }
        // while the user hasn't indicicated they want to overwrite, repeat the above
        } while (response != 'Y' && response !='y');
    }

    // open the source file as read only
    int sourceFile = open(argv[1], O_RDONLY);
    if (sourceFile == -1) {
        fprintf(stderr, "Failed to open file: %s!\n", argv[1]);
    }


    int got = 0;                // will store the number of bytes read after each read operation
    char buffer[CHUNK_SIZE];    // will store the data after each read operation

    // fileStats will store file information about the source file such as file size and permissions
    struct stat fileStats;
    // read the source file's stats
    if (fstat(sourceFile, &fileStats) != 0) {
        fprintf(stderr, "Failed to read file stats!\n");
    }

    // set size to the size of source file in bytes
    long int size = fileStats.st_size;
    // create the destination file using the same file permissions that the source file had
    int destinationFile = creat(argv[2], fileStats.st_mode);

    if (destinationFile == -1) {
        fprintf(stderr, "Failed to write file: %s!", argv[2]);
    }

    long int bytesRead = 0;     // stores the total number of bytes read
    clock_t t = clock();        // stores the time since the last progress percentage update to the screen

    // initially the progress is 0.0% obviously
    // output the progress
    float progress = 0.0f;
    printf("Progress: %.2f%%", progress);
    // we must flush the output stream for it to display on some systems since we didn't write a \n new line
    fflush(stdout);

    // try to read CHUNK_SIZE bytes from source file. While the number of bytes read was more than 0
    while ((got = read(sourceFile, buffer, CHUNK_SIZE)) > 0) {
        // write we read into the destination file
        write(destinationFile, buffer, got);
        // increment the total number of bytes read
        bytesRead += got;
        // if 1/10th of a second as passed since the last time we updated the progress percentage
        if ((clock() - t) > 100000) {
            // reset the time since last update
            t = clock();
            // calculate progress percent done
            progress = (float) bytesRead / (float) size * (float) 100.0;
            // display the progress to user
            printf("\rProgress: %.2f%%", progress);
            fflush(stdout);
        }
    }

    // should be 100.0 but calculating again just in case
    progress = (float) bytesRead / (float) size * (float) 100.0;
    printf("\rProgress: %.2f%%\n", progress);
    fflush(stdout);

    close(sourceFile);
    close(destinationFile);


    return 0;
}
