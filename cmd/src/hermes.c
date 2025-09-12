#include "libexec.h"
#include <assert.h>
#include <unistd.h>

int main(void) {
        assert(!setuid(0));
        assert(!setgid(0));

        if (fork_checked()) return 0;
        setsid();
        if (fork_checked()) return 0;
        while (getppid() != 1) usleep(1000);

        forked_exldn("rfkill", "unblock", "wlan");

        pid_t wpa = fork_checked();

        if (!wpa)
                exldn("wpa_supplicant",
                      "-i",
                      "wlan0",
                      "-c"
                      "/etc/wpa_supplicant/wpa_supplicant.conf");

        pid_t dhc = fork_checked();

        if (!dhc) exldn("dhcpcd", "-B", "wlan0");

        fork_wait(wpa);
        fork_wait(dhc);

        upanic("Should never return")
}
