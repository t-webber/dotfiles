#include "lib.h"

int main(void) {
        const char *const file = getenv("WATCHER");
        if (file == NULL)
                panic("WATCHER variable not set");
}
