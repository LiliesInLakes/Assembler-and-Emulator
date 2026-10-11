/*  ==============================================================================
    Author       : Avni Agarwal
    Roll No.     : 2501AI27
    Declaration of Authorship: I hereby declare that this code is my own original work.
    ==============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define MAX_LINE     256
#define MAX_NAME     64
#define MAX_SYMS     1024
#define MAX_MEM      8192
#define MAX_ENTRIES  16384
#define MAX_ERR      200
#define MASK32       0xFFFFFFFFUL

#define OP_DATA  (-1)
#define OP_SET   (-2)
#define OP_BR    17
#define OP_HALT  18

/* Structure for instruction definition */
typedef struct {
    const char *mnemonic;
    int opcode;
    int takes_operand;
} InstructionDef;

/* Structure for symbol table */
typedef struct {
    char name[MAX_NAME];
    int value;
    int used;
    int line;
} Symbol;

/* Structure for parser output (fixed buffers; no dangling pointers) */
typedef struct {
    char label[MAX_NAME];
    char mnemonic[MAX_NAME];
    char operand[MAX_NAME];
    int has_label;
    int has_mnemonic;
    int has_operand;
    int is_comment_only;
    char error[128];
} ParserResult;

/* One line of the listing file (built in pass 2, written at the end) */
typedef struct {
    unsigned long addr;
    int has_word;
    unsigned long word;
    char text[160];
    int sym;            /* symbol defined on this line, or -1 */
    char warn[96];      /* jump warning for this line, or "" */
} Entry;

static const InstructionDef inst_table[] = {
    {"data", OP_DATA, 1},
    {"set",  OP_SET,  1},   /* pseudo-instruction: label: set value */
    {"ldc",   0, 1},
    {"adc",   1, 1},
    {"ldl",   2, 1},
    {"stl",   3, 1},
    {"ldnl",  4, 1},
    {"stnl",  5, 1},
    {"add",   6, 0},
    {"sub",   7, 0},
    {"shl",   8, 0},
    {"shr",   9, 0},
    {"adj",  10, 1},
    {"a2sp", 11, 0},
    {"sp2a", 12, 0},
    {"call", 13, 1},
    {"return",14, 0},
    {"brz",  15, 1},
    {"brlz", 16, 1},
    {"br",   17, 1},
    {"HALT", 18, 0},
    {"mul",  19, 0},
    {"div",  20, 0},
    {NULL,   -1, 0}
};

static Symbol  syms[MAX_SYMS];
static int     nsyms = 0;
static Entry   entries[MAX_ENTRIES];
static int     nentries = 0;
static unsigned long mem[MAX_MEM];
static int     pc = 0;
static int     halt_seen = 0;
static char    errlist[MAX_ERR][192];
static int     nerr = 0;
static int     pass2_ran = 0;

/* ---------------------------------------------------------------- helpers */

static int ci_equal(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++; b++;
    }
    return *a == *b;
}

int is_string_alphanumeric(char *str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        if (!isalnum((unsigned char)str[i])) return 0;
    }
    return 1;
}

char *trim_white_space(char *str) {
    char *end;
    while (*str && isspace((unsigned char)*str)) str++;
    if (*str != 0) {
        end = str + strlen(str) - 1;
        while (end > str && isspace((unsigned char)*end)) end--;
        *++end = '\0';
    }
    return str;
}

static void add_error(int line, const char *msg) {
    if (nerr < MAX_ERR) {
        sprintf(errlist[nerr], "ERROR: Line %d: %s", line, msg);
    }
    nerr++;
}

const InstructionDef *find_instruction(const char *mnemonic) {
    int i = 0;
    while (inst_table[i].mnemonic != NULL) {
        if (ci_equal(inst_table[i].mnemonic, mnemonic)) return &inst_table[i];
        i++;
    }
    return NULL;
}

static int find_symbol(const char *name) {
    int i;
    for (i = 0; i < nsyms; i++) {
        if (strcmp(syms[i].name, name) == 0) return i;
    }
    return -1;
}

static int is_branch(int opcode) {
    return opcode == 13 || opcode == 15 || opcode == 16 || opcode == 17;
}

/* Accepts decimal, hex (0x..) and octal (leading 0), with optional sign. */
static int parse_number(const char *s, long *out) {
    char *end;
    if (!(s[0] == '+' || s[0] == '-' || isdigit((unsigned char)s[0]))) return 0;
    errno = 0;
    *out = strtol(s, &end, 0);
    if (*end != '\0' || errno != 0) return 0;
    return 1;
}

/* ----------------------------------------------------------------- parser
   Shared by both passes. Returns 0 on success, -1 on a syntax error
   (message in result->error). Works on a private copy of the line. */
int parser(const char *line, ParserResult *result) {
    char buf[MAX_LINE * 2];
    char *p, *c, *colon, *lab;
    size_t n;

    if (!line || !result) return -1;

    result->label[0] = '\0';
    result->mnemonic[0] = '\0';
    result->operand[0] = '\0';
    result->error[0] = '\0';
    result->has_label = 0;
    result->has_mnemonic = 0;
    result->has_operand = 0;
    result->is_comment_only = 0;

    if (strlen(line) >= sizeof(buf)) {
        strcpy(result->error, "line too long");
        return -1;
    }
    strcpy(buf, line);

    /* strip comment: everything from the first ';' */
    c = strchr(buf, ';');
    if (c) *c = '\0';

    p = trim_white_space(buf);
    if (*p == '\0') {
        result->is_comment_only = 1;   /* blank or comment-only line */
        return 0;
    }

    /* optional label */
    colon = strchr(p, ':');
    if (colon) {
        *colon = '\0';
        lab = trim_white_space(p);
        if (strlen(lab) >= MAX_NAME || !isalpha((unsigned char)lab[0]) ||
            !is_string_alphanumeric(lab + 1)) {
            strcpy(result->error,
                   "invalid label name (must be alphanumeric, start with a letter)");
            return -1;
        }
        strcpy(result->label, lab);
        result->has_label = 1;
        p = trim_white_space(colon + 1);
        if (*p == '\0') return 0;      /* label-only line */
    }

    /* mnemonic */
    n = strcspn(p, " \t\r\n");
    if (n >= MAX_NAME) {
        strcpy(result->error, "mnemonic too long");
        return -1;
    }
    memcpy(result->mnemonic, p, n);
    result->mnemonic[n] = '\0';
    result->has_mnemonic = 1;
    p = trim_white_space(p + n);

    /* operand (at most one) */
    if (*p != '\0') {
        n = strcspn(p, " \t\r\n");
        if (n >= MAX_NAME) {
            strcpy(result->error, "operand too long");
            return -1;
        }
        memcpy(result->operand, p, n);
        result->operand[n] = '\0';
        result->has_operand = 1;
        p = trim_white_space(p + n);
        if (*p != '\0') {
            strcpy(result->error, "unexpected extra text after operand");
            return -1;
        }
    }
    return 0;
}

/* Reads one line safely; flags lines longer than the buffer. */
static int read_line(FILE *fp, char *buf, int size, int *too_long) {
    int ch;
    *too_long = 0;
    if (!fgets(buf, size, fp)) return 0;
    if (strchr(buf, '\n') == NULL && !feof(fp)) {
        *too_long = 1;
        while ((ch = fgetc(fp)) != '\n' && ch != EOF) { }
    }
    return 1;
}

/* ------------------------------------------------------------------ pass 1
   Builds the symbol table and validates syntax / arity. */
static void define_symbol(const char *name, int value, int line) {
    char msg[128];
    if (find_symbol(name) >= 0) {
        sprintf(msg, "duplicate label '%s'", name);
        add_error(line, msg);
        return;
    }
    if (nsyms >= MAX_SYMS) {
        add_error(line, "too many labels");
        return;
    }
    strcpy(syms[nsyms].name, name);
    syms[nsyms].value = value;
    syms[nsyms].used = 0;
    syms[nsyms].line = line;
    nsyms++;
}

static void pass1(FILE *fp) {
    ParserResult r;
    char line[MAX_LINE], msg[192];
    const InstructionDef *d;
    long v;
    int line_no = 0, too_long;

    pc = 0;
    while (read_line(fp, line, sizeof(line), &too_long)) {
        line_no++;
        if (too_long) { add_error(line_no, "line too long"); continue; }
        if (parser(line, &r) < 0) { add_error(line_no, r.error); continue; }
        if (r.is_comment_only) continue;

        d = NULL;
        if (r.has_mnemonic) {
            d = find_instruction(r.mnemonic);
            if (!d) {
                sprintf(msg, "unknown mnemonic '%s'", r.mnemonic);
                add_error(line_no, msg);
                if (r.has_label) define_symbol(r.label, pc, line_no);
                continue;
            }
            if (d->takes_operand && !r.has_operand) {
                sprintf(msg, "'%s' requires an operand", r.mnemonic);
                add_error(line_no, msg);
            } else if (!d->takes_operand && r.has_operand) {
                sprintf(msg, "'%s' takes no operand", r.mnemonic);
                add_error(line_no, msg);
            }
        }

        if (d && d->opcode == OP_SET) {            /* label: set <number> */
            if (!r.has_label) {
                add_error(line_no, "'set' needs a label");
            } else if (!r.has_operand || !parse_number(r.operand, &v)) {
                add_error(line_no, "'set' needs a numeric operand");
            } else {
                define_symbol(r.label, (int)v, line_no);
            }
            continue;
        }

        if (r.has_label) define_symbol(r.label, pc, line_no);
        if (d) {
            if (pc >= MAX_MEM) {
                add_error(line_no, "program too large");
                continue;
            }
            pc++;
        }
    }
}

/* ------------------------------------------------------------------ pass 2
   Encodes instructions. Returns -1 error, 0 numeric operand, 1 label. */
static int resolve_operand(const ParserResult *r, long *val, char *err) {
    int idx;
    if (isalpha((unsigned char)r->operand[0])) {
        if (!is_string_alphanumeric((char *)r->operand)) {
            sprintf(err, "invalid label '%s' in operand", r->operand);
            return -1;
        }
        idx = find_symbol(r->operand);
        if (idx < 0) {
            sprintf(err, "undefined label '%s'", r->operand);
            return -1;
        }
        syms[idx].used = 1;
        *val = syms[idx].value;
        return 1;
    }
    if (!parse_number(r->operand, val)) {
        sprintf(err, "invalid number '%s'", r->operand);
        return -1;
    }
    return 0;
}

static void pass2(FILE *fp) {
    ParserResult r;
    char line[MAX_LINE], err[192];
    const InstructionDef *d;
    Entry *e;
    long v;
    int line_no = 0, too_long, kind;

    pass2_ran = 1;
    pc = 0;
    rewind(fp);
    while (read_line(fp, line, sizeof(line), &too_long)) {
        line_no++;
        parser(line, &r);                 /* same parser; errors seen in pass 1 */
        if (r.is_comment_only) continue;
        if (nentries >= MAX_ENTRIES) { add_error(line_no, "too many lines"); return; }

        e = &entries[nentries++];
        memset(e, 0, sizeof(*e));
        e->addr = (unsigned long)pc;
        e->sym = r.has_label ? find_symbol(r.label) : -1;

        if (!r.has_mnemonic) {            /* label-only line */
            sprintf(e->text, "%s:", r.label);
            continue;
        }
        d = find_instruction(r.mnemonic);
        strcpy(e->text, r.mnemonic);
        if (r.has_operand) { strcat(e->text, " "); strcat(e->text, r.operand); }
        if (d->opcode == OP_SET) continue;   /* no code generated */
        if (d->opcode == OP_HALT) halt_seen = 1;

        v = 0;
        if (d->takes_operand) {
            kind = resolve_operand(&r, &v, err);
            if (kind < 0) { add_error(line_no, err); continue; }
            if (d->opcode == OP_DATA) {
                if (sizeof(long) > 4 &&
                    (v < -2147483647L - 1L || v > (long)MASK32)) {
                    add_error(line_no, "data value out of 32-bit range");
                    continue;
                }
            } else {
                if (is_branch(d->opcode) && kind == 1) v = v - (long)(pc + 1);
                if (d->opcode == OP_BR && v < 0) {
                    sprintf(e->warn,
                      "WARN: Line %d: possible infinite loop (backward jump)",
                      line_no);
                }
                if (v < -8388608L || v > 16777215L) {
                    add_error(line_no, "operand out of 24-bit range");
                    continue;
                }
            }
        }

        if (d->opcode == OP_DATA) {
            e->word = (unsigned long)v & MASK32;
        } else {
            e->word = ((((unsigned long)v) & 0xFFFFFFUL) << 8)
                      | (unsigned long)d->opcode;
        }
        e->has_word = 1;
        mem[pc++] = e->word;
    }
}

/* ----------------------------------------------------------------- output */

static void write_log(const char *path) {
    FILE *f = fopen(path, "w");
    int i;
    if (!f) { perror(path); return; }

    fprintf(f, "=== Assemble output ===\n--- warnings ---\n");
    for (i = 0; i < nentries; i++) {
        if (entries[i].warn[0]) fprintf(f, "%s\n", entries[i].warn);
    }
    if (pass2_ran) {
        for (i = 0; i < nsyms; i++) {
            if (!syms[i].used)
                fprintf(f, "WARN: Label '%s' defined but never used\n", syms[i].name);
        }
        if (!halt_seen && nerr == 0) fprintf(f, "WARN: no HALT instruction found\n");
    }
    if (nerr > 0) {
        fprintf(f, "--- errors ---\n");
        for (i = 0; i < nerr && i < MAX_ERR; i++) fprintf(f, "%s\n", errlist[i]);
    }
    fprintf(f, "--- labels ---\n");
    for (i = 0; i < nsyms; i++) fprintf(f, "%s : %d\n", syms[i].name, syms[i].value);

    if (nerr == 0) fprintf(f, "Program assembled.\n");
    else fprintf(f, "Assembly failed: %d error(s). No object file produced.\n", nerr);
    fclose(f);
}

static void write_listing(const char *path) {
    FILE *f = fopen(path, "w");
    int i;
    if (!f) { perror(path); return; }
    for (i = 0; i < nentries; i++) {
        Entry *e = &entries[i];
        if (e->has_word) fprintf(f, "%04lX  %08lX  %s", e->addr, e->word, e->text);
        else             fprintf(f, "%04lX  %8s  %s", e->addr, "", e->text);
        if (e->sym >= 0 && !syms[e->sym].used)
            fprintf(f, "  WARN: Label '%s' defined but never used", syms[e->sym].name);
        if (e->warn[0]) fprintf(f, "  %s", e->warn);
        fprintf(f, "\n");
    }
    fclose(f);
}

static void write_object(const char *path) {
    FILE *f = fopen(path, "wb");
    int i;
    if (!f) { perror(path); return; }
    for (i = 0; i < pc; i++) {          /* little-endian, 4 bytes per word */
        fputc((int)(mem[i] & 0xFF), f);
        fputc((int)((mem[i] >> 8) & 0xFF), f);
        fputc((int)((mem[i] >> 16) & 0xFF), f);
        fputc((int)((mem[i] >> 24) & 0xFF), f);
    }
    fclose(f);
}

int main(int argc, char **argv) {
    FILE *fp;
    char base[512], path[528], *dot, *slash;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source.asm>\n", argv[0]);
        return 1;
    }
    fp = fopen(argv[1], "r");
    if (!fp) { perror(argv[1]); return 1; }

    if (strlen(argv[1]) >= sizeof(base)) {
        fprintf(stderr, "file name too long\n");
        fclose(fp);
        return 1;
    }
    strcpy(base, argv[1]);
    dot = strrchr(base, '.');
    slash = strrchr(base, '/');
    if (dot && (!slash || dot > slash)) *dot = '\0';

    pass1(fp);
    if (nerr == 0) pass2(fp);
    fclose(fp);

    sprintf(path, "%s.log", base); write_log(path);
    if (pass2_ran) { sprintf(path, "%s.lst", base); write_listing(path); }
    if (nerr == 0) { sprintf(path, "%s.o", base);   write_object(path); }

    if (nerr == 0) printf("Program assembled. Output: %s.log, %s.lst, %s.o\n", base, base, base);
    else           printf("Assembly failed with %d error(s). See %s.log\n", nerr, base);
    return nerr == 0 ? 0 : 1;
}
