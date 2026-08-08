#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const char *const DAYS[7] = {
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    "Sunday",
};

int main(void) {
        srand((unsigned)time(NULL));
        printf("%s\n", DAYS[rand() % 7]);
        return 0;
}
