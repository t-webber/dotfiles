#include "lib.h"
#include "libexec.h"
#include "libvec.h"
#include <assert.h>
#include <unistd.h>

#define log(x) printf("\x1b[32m>>> %s:\x1b[0m\n", x)

static void change_vpn_coutry(const_str secret,
                              const_str country_code,
                              const size_t country_code_len,
                              const_str vpn_conf) {
        String new_conf = new_s();
        extend_s(&new_conf, secret, strlen(secret));
        extend_const(&new_conf, "/vpn-");
        extend_s(&new_conf, country_code, country_code_len);
        extend_const(&new_conf, ".conf");
        forked_exldn("ln", "-sf", new_conf.data, vpn_conf);
}

#define change_localtime(town)                                                                     \
        {                                                                                          \
                String dest = new_s();                                                             \
                extend_const(&dest, "/usr/share/zoneinfo/");                                       \
                extend_const(&dest, town);                                                         \
                exldn("sudo", "ln", "-sf", dest.data, "/etc/localtime");                           \
        }

_Noreturn static void start(const_str wpa_conf) {
        log("RFKILL");
        forked_exldn("sudo", "rfkill", "unblock", "wlan");
        log("WPA");
        forked_exldn("sudo", "wpa_supplicant", "-i", "wlan0", "-B", "-c", wpa_conf);
        log("DHCP");
        exldn("sudo", "udhcpc", "-i", "wlan0", "-x", "hostname:GreyBob", "-f");
}

static void vpn(const_str action, const_str vpn_conf) {
        log("VPN");
        forked_exldn("sudo", "wg-quick", action, vpn_conf);
        log("DNS");
        forked_exldn("cat", "/etc/resolv.conf");
}

static void kill(const_str vpn_conf) {
        forked_exldn("sudo", "pkill", "wpa_supplicant");
        forked_exldn("sudo", "pkill", "udhcpc");
        forked_exldn("sudo", "wg-quick", "down", vpn_conf);
}

#define usage "Usage: %s [start|kill|nl|ca|up|ca]"

#define NB_EVENTS 12

#define EVENTS                                                                                     \
        x(startup) x(start) x(kill) x(restart) x(restartup) x(up) x(down) x(list) x(ca) x(nl)      \
            x(edit) x(help)

#define x(name) A##name,
enum Action { EVENTS };
#undef x

#define x(name) [A##name] = #name,
const_str ACTIONS[NB_EVENTS] = {EVENTS};
#undef x

#define cs                                                                                         \
        break;                                                                                     \
        case

__wur __attribute_const__ static enum Action parse(const_str arg) {
        for (enum Action i = 0; i < NB_EVENTS; ++i)
                if (!strcmp(arg, ACTIONS[i])) return i;

        return Ahelp;
}

int main(const int argc, Args argv) {
        if (argc != 2) upanic(usage, argv[0]);

        const_str secret = getenv_checked("SECRET");
        var_prefix(wpa_conf, secret, "/wpa.conf");
        var_prefix(vpn_conf, secret, "/vpn.conf");

        switch (parse(argv[1])) {
        case Astartup:
                vpn("up", vpn_conf);
                start(wpa_conf);
                break;
        case Astart:
                start(wpa_conf);
                break;
        case Akill:
                kill(vpn_conf);
                break;
        case Arestart:
                kill(vpn_conf);
                start(wpa_conf);
                break;
        case Arestartup:
                kill(vpn_conf);
                vpn("up", vpn_conf);
                start(wpa_conf);
                break;
        case Aup:
                vpn("up", vpn_conf);
                break;
        case Adown:
                vpn("down", vpn_conf);
                break;
        case Alist:
                exldn("sudo", "wpa_cli", "list_networks");
        case Aca:
                change_vpn_coutry(secret, "ca", 2, vpn_conf);
                change_localtime("America/New_York");
        case Anl:
                change_vpn_coutry(secret, "nl", 2, vpn_conf);
                change_localtime("Europe/Berlin");
        case Aedit:
                forked_exldn("sudo", "chmod", "ugoa+rwx", wpa_conf);
                exldn("nvim", wpa_conf);
        case Ahelp:
        default:
                upanic(usage, argv[0]);
        }
}
