/*
 * NNTPSwitch Curl Helper Functions
 */

#include <curl/curl.h>

struct curlMemStruct {
    char *mem;
    size_t size;
};

size_t curlWriteCallback(void *, size_t, size_t, void *);
char * curlGetURL(const char *, struct curlMemStruct *);
char * curlGetCurlURL(const char *, struct curlMemStruct *, CURL *);
char * curlPostCurlURL(const char *, struct curlMemStruct *, CURL *, const char *);

