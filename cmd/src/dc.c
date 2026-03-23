#include "lib.h"
#include <ctype.h> // IWYU pragma: keep
#include <string.h>
#include <time.h>

#include <stdlib.h>
#include <time.h>

// dcalc 14h30m - 6h
// dcalc 10h30m - 11h
// dcalc 23/2/ - 11h

typedef enum { Amount, Absolute, NotKnown, Meaningless } AmountType;

static AmountType combine_amount(const AmountType a, const AmountType b) {
        if (a == Meaningless || b == Meaningless)
                upanic("Tried to combine meaningless amounts");
        if (a == Absolute || b == Absolute) return Absolute;
        return Amount;
}

typedef struct {
        int year;
        int month;
        int day;
        int hour;
        int min;
        int sec;
        AmountType amount;
} DT;

const DT MASK_ALL = {1, 1, 1, 1, 1, 1, Meaningless};

__wur static DT new_dt(void) {
        return (DT){0, 0, 0, 0, 0, 0, NotKnown};
}

__wur static bool has_date(const DT *const mask) {
        return (mask->year || mask->month || mask->day);
}

__wur static bool has_time(const DT *const mask) {
        return mask->hour || mask->min || mask->sec;
}

#define force_fill_with(field, val)                                            \
        {                                                                      \
                *wrt = '\0';                                                   \
                dt.field += val;                                               \
                mask->field = 1;                                               \
                wrt = buf;                                                     \
                continue;                                                      \
        }

#define fill_with(field, val)                                                  \
        if (dt.field == 0) { force_fill_with(field, val) }

#define fill(field) fill_with(field, atoi(buf))

__wur static DT now(void) {
#ifndef TEST
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);

        return (DT){tm.tm_year + 1900,
                    tm.tm_mon + 1,
                    tm.tm_mday,
                    tm.tm_hour,
                    tm.tm_min,
                    tm.tm_sec,
                    Absolute};
#else
        return (DT){2021, 2, 25, 11, 42, 37, Absolute};
#endif
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
        bool filled_w = false;

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
                        if (!filled_w) {
                                toomany("d");
                        } else
                                force_fill_with(day, atoi(buf));
                } else if (*ch == 'w') {
                        if (filled_w) toomany("w");
                        filled_w = true;
                        setstate(HasModifiers);
                        force_fill_with(day, atoi(buf) * 7);
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
                upanic("Invalid character '%c' in %s", *ch, input);
        }
        if (wrt != buf)
                upanic("You can't end datetime with a digit: %s", input);

        if (state == HasModifiers)
                dt.amount = Amount;
        else if (state == HasSlashes)
                dt.amount = Absolute;

        return dt;
}

#define s(...)                                                                 \
        {                                                                      \
                int n = sprintf(buf, __VA_ARGS__);                             \
                buf += n;                                                      \
        }

__nonnull() static void print_dt_absolute(const DT *const dt,
                                          const DT *const mask,
                                          char *buf) {
        const bool d = has_date(mask);
        if (d) {
                s("%02d/%02d", dt->day, dt->month);
                if (mask->year) s("/%d", dt->year);
        }

        const bool t = has_time(mask);
        if (d && t) s(" ");

        if (t) {
                s("%dh%02dm", dt->hour, dt->min);
                if (mask->sec) s("%02ds", dt->sec);
        }
}

__nonnull() static void print_dt(const DT *const dt,
                                 const DT *const mask,
                                 char *buf) {
        if (dt->amount == Absolute) {
                print_dt_absolute(dt, mask, buf);
                return;
        }

#define p(x, c)                                                                \
        if (mask->x) { s("%d%s", dt->x, #c); }

        p(year, y);
        p(month, t);
        p(day, d);

        if (has_time(mask) && has_date(mask)) s(" ");

        p(hour, h);

        p(min, m);
        p(sec, s);
}

__nonnull() static void print_dbg(const DT *const dt) {
        if (getenv("DEBUG")) {
                char m[64];
                print_dt(dt, &MASK_ALL, m);
                const AmountType a = dt->amount;
                printf(a == Amount        ? "amount"
                       : a == Absolute    ? "Absolute"
                       : a == Meaningless ? "Meaningless"
                                          : "NotKnown");
                printf(": ");
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
        return mod(4) && (!mod(100) || (mod(400)));
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
                  dt1.sec + dt2.sec,
                  combine_amount(dt1.amount, dt2.amount)};

        carry(sec, min, 60);
        carry(min, hour, 60);
        carry(hour, day, 24);
        carry(month, year, 12);

        if (sum.amount == Absolute) {
                if (sum.month == 0) {
                        sum.month = 12;
                        sum.year -= 1;
                }
                fix_days(&sum, &mask);
        }

        if (getenv("DEBUG")) {
                printf("1 ");
                print_dbg(&dt1);
                printf("2 ");
                print_dbg(&dt2);
                printf("------------\n");
                printf("mask ");
                print_dbg(&mask);
                printf("sum ");
                print_dbg(&sum);
                printf("========================\n");
        }

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
                  dt1.sec - dt2.sec,
                  combine_amount(dt1.amount, dt2.amount)};

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

        if (mask.year != 0 || mask.month != 0) { fix_days(&sum, &mask); }

        print_dt(&sum, &mask, buf);
}

#define usage upanic("Usage: %s [[<datetime>](+|-)<datetime>]", argv[0])

static void process_args(const int argc, Args argv, char *buf) {
        if (argc == 1) {
                const DT dt = now();
                print_dt(&dt, &MASK_ALL, buf);
                return;
        }

        char arg[128];
        char *ptr = arg;
        for (int i = 1; i < argc; ++i) { ptr = stpcpy(ptr, argv[i]); }

        char *arg1 = NULL, *arg2 = NULL;
        char *r = arg;

        for (; *r; ++r) {
                if (*r == '+' || *r == '-') break;
        }

        if (*r == '\0' || *(r + 1) == '\0') usage;

        const bool is_plus = *r == '+';
        *r = '\0';
        arg1 = r == arg ? NULL : arg;
        arg2 = r + 1;

        if (is_plus) {
                plus(arg1, arg2, buf);
        } else {
                minus(arg1, arg2, buf);
        }
}

#ifndef TEST
// #ifdef MAIN

int main(const int argc, Args argv) {
        store_usage(argv[0], "", false);
        char buf[64];
        process_args(argc, argv, buf);
        puts(buf);
}
#else

#define tst(expected, ...)                                                     \
        process_args(sizeof((const_str[]){"", __VA_ARGS__})                    \
                         / sizeof(const_str),                                  \
                     (const_str[]){"", __VA_ARGS__},                           \
                     out);                                                     \
        if (getenv("DEBUG")) printf("out %s\n", out);                          \
        if (strcmp(expected, out))                                             \
                upanic("(expected) '%s' != '%s' (output)", expected, out);

int main(void) {
        char out[64];

        tst("25/02/2021 11h42m37s", );

        /// relative plus

        tst("11h42m38s", "+1s");
        tst("11h43m18s", "+41s");
        tst("11h44m18s", "+101s");
        tst("12h42m38s", "+3601s");
        tst("12h42m37s", "+3600s");
        tst("26/02 11h42m37s", "+86400s");

        tst("11h43m", "+1m");
        tst("12h43m", "+61m");
        tst("12h42m", "+60m");
        tst("26/02 11h42m", "+1440m");

        tst("12h42m", "+1h");
        tst("26/02 12h42m", "+25h");

        tst("26/02", "+1d");
        tst("07/03", "+10d");
        tst("05/03", "+1w1d");
        tst("05/03", "+1d1w");

        /// absolute plus

        // carry

        tst("30/12/2026", "29/12/2026/+1d");

        // precision

        tst("11h41m1s", "11h41m+1s");
        tst("11h41m1s", "11h41m", "+", "1s");
        tst("28/02 11h41m01s", "28/02/11h41m+1s");
        tst("28/02/2026 0h00m01s", "28/02/2026/+1s");
        tst("28/02/2026 0h01m", "28/02/2026/+1m");
        tst("28/02/2026 1h00m", "28/02/2026/+1h");

        tst("28/02/2027", "28/02/2026/+1y");
        tst("28/03/2027", "28/02/2026/+1y1t");
        tst("28/03/2027", "28/02/2026/+1t1y");

        // leap years

        tst("01/03/2026", "28/02/2026/+1d");
        tst("29/02/2020", "28/02/2020/+1d");
        tst("01/03/1900", "28/02/1900/+1d");
        tst("29/02/2000", "28/02/2000/+1d");

        /// amount plus

        tst("2d", "1d+1d");
        tst("2d 1h", "1d+1d1h");
        tst("3d 1h", "1d+1d25h");
        tst("36d", "1d+35d");
        tst("1t36d", "1d+35d1t");

        tst("1d 1h3s", "1h3s+1d");
        tst("1y36d", "1d+35d1y");
        tst("43d", "1d+35d1w");

        tst(20 / 12 / 2025 / +1w);

        printf("dc tests passed\n");
}
#endif
