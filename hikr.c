#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

struct Locations {
    char *title;
};

/**
 * Generate random number starting from lower bound range
 * Returns pointer to an array of random numbers.
 */
int *generate_random_numbers(int nfiles) {
    int lower_bound = 2000000;


    // allocate space for array of hikr ids on heap, so we can
    // reuse it to generate download URLs.
    int *arrPtr = (int*)malloc(nfiles * sizeof(int));
    if (arrPtr == NULL) {
        printf("Could not allocate memory, exiting.");
        return EXIT_FAILURE;
    }

    // We need to seed rand() to get random numbers
    srand ( time(NULL) );

    for (int i = 0; i < nfiles; i++) {
        int rand_num = rand() % 10000;
        arrPtr[i] = rand_num + lower_bound;
    }

    return arrPtr;
}


/**
 * Download a resource from a URL, and save it to a file using a specified file name.
 * This function also expects an initialised curl instance.
 */
void download_file(CURL *curl, char *url, char *fname) {
    FILE *fp = fopen(fname, "wp");
    // set curl options
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    
    // download file and save to file
    CURLcode result = curl_easy_perform(curl);

    if (result == CURLE_OK) {
        printf("Downloaded %s successfully.", url);
    } else {
        fprintf(stderr, "Download problem: %s\n", curl_easy_strerror(result));
    }
    fclose(fp);
    free(url);
}

/**
 * Create the hikr download strings.
 * @param postId the id of the post to download.
 * @return Pointer to character array (download url).
 */
char *getImageUrl(int photoId)
{
    char *url = malloc(100);
    snprintf(url, 100, "https://f.hikr.org/files/%dl.jpg", photoId);
    return url;
}

/**
 * Create the hikr download strings.
 * @param postId the id of the post to download.
 * @return Pointer to character array (download url).
 */
char *getPostUrl(int photoId)
{
    char *url = malloc(100);
    snprintf(url, 100, "https://www.hikr.org/gallery/photo%d.html", photoId);
    return url;
}

void slice(const char *str, char *result, size_t start, size_t end)
{
    strncpy(result, str + start, end - start);
}

struct Locations findLinkContent(char *string)
{
    // find position of closing tag >
    char *linkStart = strstr(string, "<a");
    char *innerHTMLStart = strstr(linkStart, ">") + 1;
    char *linkEnd = strstr(linkStart, "</a>");

    int innerHTMLEnd = linkEnd - innerHTMLStart;


    // copy innerHTML to struct
    struct Locations location;
    char *title = malloc(innerHTMLEnd + 8); // add four additional bytes to hold .jpg later on during renaming
    strncpy(title, innerHTMLStart + 0, innerHTMLEnd);
    char *filename = malloc(sizeof(title) + 4);
    strcpy(filename, "img/");
    location.title = strcat(filename, title);
    free(title);
    return location;
}

struct Locations extractLocationsFromHtml(FILE *fp)
{
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    rewind(fp);

    char *string = malloc(fsize + 1);
    fread(string, 1, fsize, fp);
    fclose(fp);
    string[fsize] = 0;


    // search for position of first instance of div13
    char *pfound = strstr(string, "div13");

    // search for position of closing tag of div13
    char *pfoundend = strstr(pfound, "</div>");
    int dposfoundend = pfoundend - pfound;  // this is the position at which we can find </div>

    // copy div content into targetHtml array
    char *targetHtml = malloc(dposfoundend + 1);
    slice(pfound, targetHtml, 0, dposfoundend);

    // we now need to process the char array to extract all relevant <a> tags and their innerHTML content
    return findLinkContent(targetHtml);
}


int main(int argc, char **argv) {
    // initialise reusable curl instance
    CURL *curl = curl_easy_init();

    int nfiles = atoi(argv[1]);

    // check if curl initialised properly
    if (!curl) {
        fprintf(stderr, "CURL initialisation failed.\n");
    } else {

        // TODO: make temporary directory and save files to temp directory.

        int * random_numbers = generate_random_numbers(nfiles);

        for (int i = 0; i < nfiles; i++) {
            char postFileName[100], imageFileName[100];
            int photoId = random_numbers[i];
            char *imageUrl = getImageUrl(photoId);
            char *postUrl = getPostUrl(photoId);


            // download files
            snprintf(imageFileName, 100, "img/img%d.jpg", i);
            snprintf(postFileName, 100, "html/post%d.html", i);

            download_file(curl, imageUrl, imageFileName);
            download_file(curl, postUrl, postFileName);

            FILE *fp = fopen(postFileName, "r");

            if (fp == NULL) {
                perror("Error opening file.");
                return EXIT_FAILURE;
            }

            struct Locations locations = extractLocationsFromHtml(fp);

            int ret = rename(imageFileName, strcat(locations.title, ".jpg"));

            // TODO: if download did not succeed print error message and continue

            // TODO: generate PDF using https://github.com/libharu/libharu, call from pdf.c
            // TODO: free other vars
        }
    }

    // perform cleanup before exit
    curl_easy_cleanup(curl);
    return EXIT_SUCCESS;

}
