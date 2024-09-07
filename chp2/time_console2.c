#include <stdio.h>
#include <time.h>

int main(){
    time_t timer;
    time(&timer);

    // prints time converted to human-readable format
    printf("Local time is: %s", ctime(&timer));

    return 0;
}