#include "libexec.h"
#include "libterm.h"
#include "libvec.h"
#include <assert.h>
#include <unistd.h>

#define usage upanic("Usage: %s [start|kill]", argv[0])

#define log(...)                                                                                   \
        printf(GREEN ">>> " __VA_ARGS__);                                                          \
        printf(RESET "\n")

int main(const int argc, Args argv) {
        if (argc != 2) usage;

        const_str secret = getenv_checked("SECRET");
        var_prefix(wpa_conf, secret, "/wpa.conf");
        var_prefix(vpn_conf, secret, "/vpn.conf");

        if (!strcmp(argv[1], "start")) {

                log("RFKILL");
                forked_exldn("sudo", "rfkill", "unblock", "wlan");

                log("WPA [%s]", wpa_conf);
                forked_exldn("sudo", "wpa_supplicant", "-i", "wlan0", "-B", "-c", wpa_conf);

                log("DHCP");
                forked_exldn("sudo", "udhcpc", "-i", "wlan0", "-x", "hostname:GreyBob");

                log("VPN [%s]", vpn_conf);
                forked_exldn("sudo", "wg-quick", "up", vpn_conf);
        }

        else if (!strcmp(argv[1], "kill")) {
                forked_exldn("sudo", "pkill", "wpa_supplicant");
                forked_exldn("sudo", "pkill", "udhcpc");
                forked_exldn("sudo", "wg-quick", "down", vpn_conf);
        } else
                usage;
}
