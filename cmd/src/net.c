#include "libexec.h"
#include "libvec.h"
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
                String vpn = new_s();
                const_str secret = getenv_checked("SECRET");
                extend_s(&vpn, secret, strlen(secret));
                extend_s(&vpn, "/vpn.conf", sizeof("/vpn.conf"));
                if (!fork()) exldn("sudo", "wg-quick", "up", vpn.data);
        }

        else if (!strcmp(argv[1], "kill")) {
                forked_exldn("sudo", "pkill", "wpa_supplicant");
                forked_exldn("sudo", "pkill", "udhcpc");
                forked_exldn("sudo", "pkill", "wg-quick");
        } else
                usage;
}
