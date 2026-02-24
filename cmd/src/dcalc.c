#include "lib.h"
#include <ctype.h> // IWYU pragma: keep
#include <stdlib.h>
#include <time.h>

// dcalc 14h30m - 6h
// dcalc 10h30m - 11h
// dcalc 23/2/ - 11h

typedef struct {
        int year;
        int month;
        int day;
        int hour;
        int min;
        int sec;
} DT;

const DT MASK_ALL = {1, 1, 1, 1, 1, 1};

__wur static DT new_dt(void) {
        return (DT){0, 0, 0, 0, 0, 0};
}

#define fill_with(field, func)                                                 \
        if (dt.field == 0) {                                                   \
                *wrt = '\0';                                                   \
                dt.field = atoi(buf);                                          \
                dt.field = func;                                               \
                mask->field = 1;                                               \
                wrt = buf;                                                     \
                continue;                                                      \
        }

#define fill(field) fill_with(field, dt.field)

__wur static DT now(void) {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);

        return (DT){tm.tm_year + 1900,
                    tm.tm_mon + 1,
                    tm.tm_mday,
                    tm.tm_hour,
                    tm.tm_min,
                    tm.tm_sec};
}

#define toomany(x) upanic("Invalid input '%s': too many '" x "'", input);

enum ParsingState { HasSlashes, HasModifiers, HasNone };

#define setstate(x)                                                            \
        if (state != x && state != HasNone)                                    \
                upanic("Can't use /,- and y,t,w,d");                           \
        state = x;

__wur __nonnull((2)) static DT parse_dt(const_str input, DT *mask) {
        if (!input) return now();
        DT dt = new_dt();
        char *const buf = malloc(4 * (sizeof(char)));
        char *wrt = buf;
        enum ParsingState state = HasNone;

        for (const char *ch = input; *ch; ++ch) {
                if (isdigit(*ch)) {
                        *wrt++ = *ch;
                        continue;
                } else if (*ch == '/' || *ch == '-') {
                        setstate(HasSlashes);
                        fill(day);
                        fill(month);
                        fill(year);
                        toomany("/ or -");
                } else if (*ch == 'y') {
                        setstate(HasModifiers);
                        fill(year);
                        toomany("y");
                } else if (*ch == 't') {
                        setstate(HasModifiers);
                        fill(month);
                        toomany("t");
                } else if (*ch == 'd') {
                        setstate(HasModifiers);
                        fill(day);
                        toomany("d or w");
                } else if (*ch == 'w') {
                        setstate(HasModifiers);
                        fill_with(day, dt.day * 7);
                        toomany("d or w");
                } else if (*ch == 'h') {
                        fill(hour);
                        toomany("h");
                } else if (*ch == 'm') {
                        fill(min);
                        toomany("m");
                } else if (*ch == 's') {
                        fill(sec);
                        toomany("s");
                }
                upanic("Invalid character %c in %s", *ch, input);
        }
        if (wrt != buf)
                upanic("You can't end datetime with a digit: %s", input);
        return dt;
}

__nonnull() static void print_dt(const DT *const dt,
                                 const DT *const mask,
                                 char *buf) {
#define s(...)                                                                 \
        {                                                                      \
                int n = sprintf(buf, __VA_ARGS__);                             \
                buf += n;                                                      \
        }

        bool d = mask->year || mask->month || mask->day;
        if (mask->day) {
                if (mask->year || mask->month) {
                        s("%02d", dt->day);
                } else {
                        s("%dd", dt->day);
                }
        }
        if (mask->month || (mask->year && mask->day)) s("/%02d", dt->month);
        if (mask->year) s("/%d", dt->year);

        bool t = mask->hour || mask->min || mask->sec;
        if (d && t) s(" ");

        if (t) {
                s("%dh", dt->hour);
                if (mask->min || dt->min || mask->sec) s("%02dm", dt->min);
                if (mask->sec) s("%02ds", dt->sec);
        }
}

__nonnull() static void print_dbg(const DT *const dt) {
        if (getenv("DEBUG")) {
                char m[64];
                print_dt(dt, &MASK_ALL, m);
                puts(m);
        }
}

#define carry(this, higher, modulo)                                            \
        if (sum.this / modulo) {                                               \
                mask.higher = 1;                                               \
                sum.higher += sum.this / modulo;                               \
                sum.this %= modulo;                                            \
        }

#define mod(x) (year % x == 0)

__wur static bool leap_year(const int year) {
        return mod(4)
               && (!mod(100) || (mod(400) && (!mod(1000) || (mod(4000)))));
}

__wur static int days_per_month(const int month, const int year) {
        static const int list[12]
            = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

        if (month == 2) {
                return leap_year(year) ? 29 : 28;
        } else
                return list[month - 1];
}

static void fix_days(DT *const dt, DT *const mask) {
        do {
                int max_days = days_per_month(dt->month, dt->year);
                if (dt->day <= max_days) break;
                mask->month = 1;
                if (dt->month == 12) {
                        dt->month = 1;
                        mask->year = 1;
                        dt->year += 1;
                } else {
                        dt->month += 1;
                }
                dt->day -= max_days;
        } while (1);
}

__nonnull((2)) static void plus(const_str arg1, const_str arg2, char *buf) {
        DT mask = new_dt();
        const DT dt1 = parse_dt(arg1, &mask);
        const DT dt2 = parse_dt(arg2, &mask);

        DT sum = {dt1.year + dt2.year,
                  dt1.month + dt2.month,
                  dt1.day + dt2.day,
                  dt1.hour + dt2.hour,
                  dt1.min + dt2.min,
                  dt1.sec + dt2.sec};

        carry(sec, min, 60);
        carry(min, hour, 60);
        carry(hour, day, 24);
        carry(month, year, 12);

        if (mask.year != 0 || mask.month != 0) { fix_days(&sum, &mask); }

        print_dbg(&mask);

        print_dt(&sum, &mask, buf);
}

#define carrysub(this, higher, modulo)                                         \
        carry(this, higher, modulo);                                           \
        if (sum.this < 0) {                                                    \
                sum.higher -= 1;                                               \
                sum.this += modulo;                                            \
        }

__nonnull((2)) static void minus(const_str arg1, const_str arg2, char *buf) {
        DT mask = new_dt();
        const DT dt1 = parse_dt(arg1, &mask);
        const DT dt2 = parse_dt(arg2, &mask);
        DT sum = {dt1.year - dt2.year,
                  dt1.month - dt2.month,
                  dt1.day - dt2.day,
                  dt1.hour - dt2.hour,
                  dt1.min - dt2.min,
                  dt1.sec - dt2.sec};

        carrysub(sec, min, 60);
        carrysub(min, hour, 60);
        carrysub(hour, day, 24);
        carrysub(month, year, 12);

        if (sum.month == 0) {
                sum.year -= 1;
                sum.month = 12;
                mask.year = mask.month = 1;
        }

        while (sum.day <= 0) {
                if (sum.month == 1) {
                        sum.month = 12;
                        sum.year -= 1;
                        mask.year = 1;
                } else {
                        sum.month -= 1;
                }
                mask.month = 1;
                int nb_days = days_per_month(sum.month, sum.year);
                sum.day += nb_days;
        }

        fix_days(&sum, &mask);

        print_dt(&sum, &mask, buf);
}

#define usage upanic("Usage: %s [[<datetime>] (+|-) <datetime>]", argv[0])

static void process_args(const int argc, Args argv, char *buf) {
        if (argc == 1) {
                const DT dt = now();
                print_dt(&dt, &MASK_ALL, buf);
                return;
        }

        if (argc != 3 && argc != 4) usage;

        const char *arg1 = NULL, *arg2 = NULL, *op;
        if (argc == 4) {
                arg1 = argv[1];
                op = argv[2];
                arg2 = argv[3];
        } else {
                op = argv[1];
                arg2 = argv[2];
        }

        if (!strcmp(op, "+")) {
                plus(arg1, arg2, buf);
                return;
        }
        if (!strcmp(op, "-")) {
                minus(arg1, arg2, buf);
                return;
        }

        usage;
}

#ifndef TEST
int main(const int argc, Args argv) {
        store_usage(argv[0], "", false);
        char buf[64];
        process_args(argc, argv, buf);
        puts(buf);
}
#else
int main(void) {
        printf("blob");
}
#endif
