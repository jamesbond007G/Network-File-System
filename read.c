#include <stdio.h>
#include <stdlib.h>

#define MAX_FILE_SIZE 1000000 // Adjust this size based on your file's expected size

int main()
{
    // Open the file for reading
    FILE *file = fopen("example.txt", "r");

    if (file == NULL)
    {
        fprintf(stderr, "Error opening file.\n");
        return 1;
    }

    // Allocate memory for the buffer
    char *buffer = (char *)malloc(MAX_FILE_SIZE);

    if (buffer == NULL)
    {
        fprintf(stderr, "Error allocating memory.\n");
        fclose(file);
        return 1;
    }

    // Read the file content into the buffer
    size_t bytesRead = fread(buffer, 1, MAX_FILE_SIZE, file);

    // Close the file
    fclose(file);

    // Display the content
    if (bytesRead > 0)
    {
        printf("Content read from file:\n");
        printf("%.*s", (int)bytesRead, buffer); // Print the buffer up to bytesRead
    }
    else
    {
        printf("Empty file or read error.\n");
    }

    // Free the allocated memory
    free(buffer);

    return 0;
}
