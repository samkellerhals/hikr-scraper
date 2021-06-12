#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

struct Locations {
    char *title;
};

const int RANDOM_NUM_LOWER_BOUND = 2000000;


/**
 * Generate random number starting from lower bound range
 * @param n number of random numbers to generate.
 * @return pointer to an array of random numbers.
 */
int *generate_random_numbers(int n)
{
    // allocate space for array of hikr ids on heap, so we can
    // reuse it to generate download URLs.
    int *arrPtr = (int*)malloc(n * sizeof(int));
    if (arrPtr == NULL) {
        printf("Could not allocate memory, exiting.");
        return (int *) EXIT_FAILURE;
    }

    // We need to seed rand() to get random numbers
    srand ( time(NULL) );

    for (int i = 0; i < n; i++) {
        int rand_num = rand() % 10000;
        arrPtr[i] = rand_num + RANDOM_NUM_LOWER_BOUND;
    }

    return arrPtr;
}


/**
 * Download a resource from a URL, and save it to a file using a specified file name.
 * This function also expects an initialised curl instance.
 * @param curl pointer to curl object.
 * @param url pointer to url character array.
 * @param fname pointer to filename character array. File will be saved using this name.
 * @return int HTTP status code of the request.
 */
int download_file(CURL *curl, char *url, char *fname)
{
    FILE *fp = fopen(fname, "wp");
    // set curl options
    long http_code = 0;
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 0);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
    // download file and save to file
    CURLcode result = curl_easy_perform(curl);

    if (result == CURLE_OK) {
        printf("Downloaded %s successfully.", url);
        return http_code;
    } else {
        fprintf(stderr, "Download problem: %s\n", curl_easy_strerror(result));
        return http_code;
    }
}

/**
 * Creates a hikr image url.
 * @param photoId the id of the photo to download.
 * @return Pointer to character array (image url).
 */
char *getImageUrl(int photoId)
{
    char *url = malloc(100);
    snprintf(url, 100, "https://f.hikr.org/files/%dl.jpg", photoId);
    return url;
}

/**
 * Creates a hikr photo url.
 * @param photoId the id of the photo to download.
 * @return Pointer to character array (post url).
 */
char *getPostUrl(int photoId)
{
    char *url = malloc(100);
    snprintf(url, 100, "https://www.hikr.org/gallery/photo%d.html", photoId);
    return url;
}

/**
 * Creates a slice of a character array.
 * @param str pointer to string to slice.
 * @param result pointer to array to store slice.
 * @param start start index of the slice.
 * @param end end index of the slice.
 * @return void
 */
void slice(const char *str, char *result, size_t start, size_t end)
{
    strncpy(result, str + start, end - start);
}

/**
 * Extracts innerHTML string of a link <a> HTML tag (the title of the post in this case)
 * @param string pointer to HTML string.
 * @return a Locations struct with its title field being the extracted innerHTML content of the link.
 */
struct Locations getLinkContent(char *string)
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
    return location;
}

/**
 * Extracts locations/metdata from a HTML page from a given Hikr image post.
 * The current implementation only extracts the post title.
 * @param FILE pointer to the HTML post.
 * @return a Locations struct.
 */
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
    return getLinkContent(targetHtml);
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

            int imgStatus = download_file(curl, imageUrl, imageFileName);
            int postStatus = download_file(curl, postUrl, postFileName);

            // if any of the downloads fail we attempt to download a new image
            if (imgStatus == 404 || postStatus == 404){
                int delImage = remove(imageFileName);
                int delPost = remove(postFileName);
                continue;
            }

            // Extract tour title from HTML and use as filename
            FILE *fp = fopen(postFileName, "r");

            if (fp == NULL) {
                perror("Error opening file.");
                return EXIT_FAILURE;
            }

            struct Locations locations = extractLocationsFromHtml(fp);

            int ret = rename(imageFileName, strcat(locations.title, ".jpg"));
        }
    }
    return EXIT_SUCCESS;
}
