#include "libexec.h"
#include <assert.h>
#include <unistd.h>

#define usage upanic("Usage: %s [start|kill]", argv[0])

int main(const int argc, Args argv) {
        if (argc != 2) usage;

        if (!strcmp(argv[1], "start")) {
                forked_exldn("sudo", "rfkill", "unblock", "wlan");

                if (!fork())
                        exldn("sudo",
                              "wpa_supplicant",
                              "-i",
                              "wlan0",
                              "-B",
                              "-c",
                              "/etc/wpa_supplicant/wpa_supplicant.conf");

                if (!fork()) exldn("sudo", "udhcpc", "-i", "wlan0");
        }

        else if (!strcmp(argv[1], "kill")) {
                forked_exldn("sudo", "pkill", "wpa_supplicant");
                exldn("sudo", "pkill", "udhcpc");
        } else
                usage;
}
