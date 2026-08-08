#include "lib.h"
#include "libexec.h"
#include "libfzf.h"
#include "libos.h"
#include "libvec.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#define change_localtime(town)                                                  \
        {                                                                       \
                String dest = new_s();                                          \
                extend_const(&dest, "/usr/share/zoneinfo/");                    \
                extend_const(&dest, town);                                      \
                forked_exldn("sudo", "ln", "-sf", dest.data, "/etc/localtime"); \
        }

static void change_vpn_coutry(const_str secret, const_str selected, const_str vpn_conf) {
        String new_conf = new_s();
        extend_s(&new_conf, secret, strlen(secret));
        push_s(&new_conf, '/');
        extend_s(&new_conf, selected, strlen(selected));
        push_s(&new_conf, 0);
        forked_exldn("ln", "-sf", new_conf.data, vpn_conf);
        if (starts_with_const(selected, "vpn-ca-")) {
                change_localtime("America/New_York");
        } else if (starts_with_const(selected, "vpn-nl-"))
                change_localtime("Europe/Berlin");
}

_Noreturn static void start(const_str wpa_conf) {
        forked_exldn("sudo", "rfkill", "unblock", "wlan");
        forked_exldn("sudo", "wpa_supplicant", "-i", "wlan0", "-B", "-c", wpa_conf);
        if (!fork_checked()) exldn("sudo", "udhcpc", "-i", "wlan0", "-x", "hostname:GreyBob", "-f");
        exit(0);
}

static void vpn(const_str action, const_str vpn_conf) {
        forked_exldn("sudo", "wg-quick", action, vpn_conf);
        forked_exldn("cat", "/etc/resolv.conf");
}

static void kill(const_str vpn_conf) {
        forked_exldn("sudo", "pkill", "wpa_supplicant");
        forked_exldn("sudo", "pkill", "udhcpc");
        forked_exldn("sudo", "wg-quick", "down", vpn_conf);
}

#define NB_EVENTS 17

#define EVENTS                     \
        x(au)      /* startup */   \
            x(a)   /* start */     \
            x(o)   /* kill */      \
            x(oa)  /* restart */   \
            x(oau) /* restartup */ \
            x(ls)  /* list */      \
            x(u)   /* up */        \
            x(d)   /* down */      \
            x(du)  /* down-up */   \
            x(r)   /* resolv */    \
            x(gv)  /* get vpn */   \
            x(sv)  /* set vpn */   \
            x(e)   /* edit */      \
            x(lk)  /* lock */      \
            x(uk)  /* unlock */    \
            x(gk)  /* getlock */   \
            x(h)   /* help */

#define x(name) A##name,
enum Action { EVENTS };
#undef x

#define x(name) [A##name] = #name,
const_str ACTIONS[NB_EVENTS] = {EVENTS};
#undef x

#define cs     \
        break; \
        case

__wur __attribute_const__ static enum Action parse(const_str arg) {
        for (enum Action i = 0; i < NB_EVENTS; ++i)
                if (!strcmp(arg, ACTIONS[i])) return i;

        return Ah;
}

_Noreturn static void usage(const_str prog) {
        String help = new_s();
        extend_s(&help, prog, strlen(prog));
        extend_const(&help, " [");
        for (int i = 0; i < NB_EVENTS; ++i) {
                extend_s(&help, ACTIONS[i], strlen(ACTIONS[i]));
                if (i + 1 < NB_EVENTS) extend_const(&help, "|");
        }
        fprintf(stderr, RED "Usage: %s]\n" RESET, help.data);
        exit(1);
}

static Vec list_locations(const_str secret) {
        Vec locations = alphascan_dir(secret);
        size_t rd = 0, wr = 0;
        for (; rd < locations.len; ++rd)
                if (starts_with_const(locations.data[rd], "vpn-"))
                        locations.data[wr++] = locations.data[rd];
        locations.len = wr;
        push_v(&locations, "invalid");
        return locations;
}

_Noreturn __nonnull(()) static void ex_change_loc(const_str secret, const_str vpn_conf) {
        printf("Previous was ");
        fflush(stdout);
        if (!fork_and_wait()) exldn("readlink", "-f", vpn_conf);
        sleep(2);
        Vec locations = list_locations(secret);
        const_str selected = fzf(locations.data, locations.len);
        if (selected == NULL) upanic("Cancelled");
        change_vpn_coutry(secret, selected, vpn_conf);
        printf("New is ");
        fflush(stdout);
        exldn("readlink", "-f", vpn_conf);
}

int main(const int argc, Args argv) {
        if (argc != 2) usage(argv[0]);

        const_str secret = getenv_checked("SECRET");
        var_prefix(wpa_conf, secret, "/wpa.conf");
        var_prefix(vpn_conf, secret, "/vpn.conf");

        switch (parse(argv[1])) {
        case Aau:
                vpn("up", vpn_conf);
                start(wpa_conf);
                break;
        case Aa:
                start(wpa_conf);
                break;
        case Ao:
                kill(vpn_conf);
                break;
        case Aoa:
                kill(vpn_conf);
                start(wpa_conf);
                break;
        case Aoau:
                kill(vpn_conf);
                vpn("up", vpn_conf);
                start(wpa_conf);
                break;
        case Au:
                vpn("up", vpn_conf);
                break;
        case Ad:
                vpn("down", vpn_conf);
                break;
        case Adu:
                vpn("down", vpn_conf);
                vpn("up", vpn_conf);
                break;
        case Als:
                exldn("sudo", "wpa_cli", "list_networks");
        case Agv:
                exldn("readlink", "-f", vpn_conf);
        case Asv:
                ex_change_loc(secret, vpn_conf);
        case Ae:
                forked_exldn("sudo", "chmod", "ugoa+rwx", wpa_conf);
                exldn("nvim", wpa_conf);
        case Ar:
                exldn("sudo", "cat", "/etc/resolv.conf");
        case Alk:
                exldn("sudo", "chattr", "+i", "/etc/resolv.conf");
        case Auk:
                exldn("sudo", "chattr", "-i", "/etc/resolv.conf");
        case Agk:
                exldn("lsattr", "/etc/resolv.conf");
        case Ah:
        default:
                usage(argv[0]);
        }
}
