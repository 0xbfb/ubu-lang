#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UCC_VERSION "recovery-r1"
#define MAX_LINE 8192
#define MAX_BLOCKS 128

typedef struct {
    int indent;
    char kind[16];
} Block;

static void die(const char *message) {
    fprintf(stderr, "ucc: %s\n", message);
    exit(1);
}

static char *xstrdup(const char *s) {
    size_t n = strlen(s) + 1;
    char *out = (char *)malloc(n);
    if (!out) die("out of memory");
    memcpy(out, s, n);
    return out;
}

static char *trim_left(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    return s;
}

static void trim_right(char *s) {
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) s[--n] = '\0';
}

static int indentation(const char *s) {
    int count = 0;
    while (*s == ' ') { count++; s++; }
    return count;
}

static int starts_with(const char *s, const char *prefix) {
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

static const char *c_type(const char *type) {
    if (strcmp(type, "i64") == 0) return "int64_t";
    if (strcmp(type, "i32") == 0) return "int32_t";
    if (strcmp(type, "bool") == 0) return "bool";
    if (strcmp(type, "str") == 0) return "const char *";
    if (strcmp(type, "char") == 0) return "char";
    if (strcmp(type, "unit") == 0) return "void";
    return "int64_t";
}

static void normalize_identifier(char *s) {
    for (; *s; s++) {
        if (*s == '.') *s = '_';
    }
}

static void replace_word(char *buffer, size_t capacity, const char *from, const char *to) {
    char temp[MAX_LINE * 2];
    size_t from_len = strlen(from), to_len = strlen(to);
    char *src = buffer, *dst = temp;
    size_t remain = sizeof(temp) - 1;
    while (*src && remain > 0) {
        if ((src == buffer || !isalnum((unsigned char)src[-1])) &&
            strncmp(src, from, from_len) == 0 &&
            !isalnum((unsigned char)src[from_len])) {
            if (to_len > remain) break;
            memcpy(dst, to, to_len); dst += to_len; remain -= to_len; src += from_len;
        } else {
            *dst++ = *src++; remain--;
        }
    }
    *dst = '\0';
    if (capacity == 0) return;
    strncpy(buffer, temp, capacity - 1);
    buffer[capacity - 1] = '\0';
}

static void replace_all(char *buffer, size_t capacity, const char *from, const char *to) {
    char temp[MAX_LINE * 2];
    size_t from_len = strlen(from), to_len = strlen(to);
    const char *src = buffer;
    char *dst = temp;
    size_t remain = sizeof(temp) - 1;
    while (*src && remain > 0) {
        if (strncmp(src, from, from_len) == 0) {
            if (to_len > remain) break;
            memcpy(dst, to, to_len);
            dst += to_len;
            remain -= to_len;
            src += from_len;
        } else {
            *dst++ = *src++;
            remain--;
        }
    }
    *dst = '\0';
    if (capacity == 0) return;
    strncpy(buffer, temp, capacity - 1);
    buffer[capacity - 1] = '\0';
}

static void transform_expr(char *expr, size_t capacity) {
    replace_word(expr, capacity, "and", "&&");
    replace_word(expr, capacity, "or", "||");
    replace_word(expr, capacity, "not", "!");
    replace_all(expr, capacity, "std.math.", "std_math_");
    replace_all(expr, capacity, "std.string.", "std_string_");
    replace_all(expr, capacity, "std.range.", "std_range_");
}

static int next_relevant_indent(char **lines, int count, int index) {
    for (int i = index + 1; i < count; i++) {
        char *copy = xstrdup(lines[i]);
        trim_right(copy);
        char *t = trim_left(copy);
        if (*t && *t != '#') {
            int ind = indentation(lines[i]);
            free(copy);
            return ind;
        }
        free(copy);
    }
    return -1;
}

static void emit_indent(FILE *out, int level) {
    for (int i = 0; i < level; i++) fputs("    ", out);
}

static void close_blocks(FILE *out, Block *blocks, int *depth, int target_indent, int preserve_if_for_else) {
    while (*depth > 0) {
        Block *top = &blocks[*depth - 1];
        if (top->indent < target_indent) break;
        if (preserve_if_for_else && top->indent == target_indent && strcmp(top->kind, "if") == 0) break;
        emit_indent(out, *depth - 1);
        fputs("}\n", out);
        (*depth)--;
    }
}

static char **read_lines(const char *path, int *count_out) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "ucc: cannot open %s: %s\n", path, strerror(errno));
        exit(1);
    }
    char **lines = NULL;
    int count = 0, cap = 0;
    char buf[MAX_LINE];
    while (fgets(buf, sizeof(buf), f)) {
        if (count == cap) {
            cap = cap ? cap * 2 : 64;
            char **next = (char **)realloc(lines, (size_t)cap * sizeof(*lines));
            if (!next) die("out of memory");
            lines = next;
        }
        lines[count++] = xstrdup(buf);
    }
    fclose(f);
    *count_out = count;
    return lines;
}

static void free_lines(char **lines, int count) {
    for (int i = 0; i < count; i++) free(lines[i]);
    free(lines);
}

static int emit_c(const char *input, const char *output) {
    int count = 0;
    char **lines = read_lines(input, &count);
    FILE *out = fopen(output, "wb");
    if (!out) {
        fprintf(stderr, "ucc: cannot write %s: %s\n", output, strerror(errno));
        free_lines(lines, count);
        return 1;
    }

    fprintf(out,
        "/* Generated by UbuLang ucc %s. Standalone C; no maintained runtime library. */\n"
        "#include <inttypes.h>\n#include <stdbool.h>\n#include <stdint.h>\n#include <stdio.h>\n\n",
        UCC_VERSION);

    Block blocks[MAX_BLOCKS];
    int depth = 0;
    int function_indent = -1;
    char current_return_type[32] = "int64_t";

    for (int i = 0; i < count; i++) {
        char *raw = xstrdup(lines[i]);
        trim_right(raw);
        int ind = indentation(raw);
        char *t = trim_left(raw);
        if (!*t || *t == '#') { free(raw); continue; }

        if (starts_with(t, "module ") || starts_with(t, "import ")) {
            fprintf(out, "/* %s */\n", t);
            free(raw); continue;
        }
        if (starts_with(t, "type ") || starts_with(t, "primitive ")) {
            fprintf(out, "/* recovery-r1 declaration not lowered: %s */\n", t);
            free(raw); continue;
        }

        int is_else = starts_with(t, "else") && (t[4] == '\0' || isspace((unsigned char)t[4]));
        close_blocks(out, blocks, &depth, ind, is_else);

        if (starts_with(t, "fn ")) {
            close_blocks(out, blocks, &depth, -1, 0);
            char *header = t + 3;
            char *open = strchr(header, '(');
            char *close = strrchr(header, ')');
            char *arrow = strstr(header, "->");
            if (!open || !close || !arrow || close > arrow) {
                fprintf(stderr, "ucc: invalid function header at line %d\n", i + 1);
                fclose(out); free(raw); free_lines(lines, count); return 1;
            }
            *open = '\0';
            char name[256]; snprintf(name, sizeof(name), "%s", header); trim_right(name); normalize_identifier(name);
            *close = '\0';
            char params[MAX_LINE]; snprintf(params, sizeof(params), "%s", open + 1);
            char ret[64]; snprintf(ret, sizeof(ret), "%s", trim_left(arrow + 2)); trim_right(ret);
            snprintf(current_return_type, sizeof(current_return_type), "%s", c_type(ret));
            fprintf(out, "%s %s(", current_return_type, name);
            char *p = strtok(params, ",");
            int first = 1;
            while (p) {
                char *item = trim_left(p); trim_right(item);
                char *colon = strchr(item, ':');
                if (!colon) { fprintf(stderr, "ucc: invalid parameter at line %d\n", i + 1); fclose(out); free(raw); free_lines(lines, count); return 1; }
                *colon = '\0'; trim_right(item);
                char *type = trim_left(colon + 1); trim_right(type);
                if (!first) fputs(", ", out);
                fprintf(out, "%s %s", c_type(type), item);
                first = 0; p = strtok(NULL, ",");
            }
            if (first) fputs("void", out);
            fputs(") {\n", out);
            blocks[depth].indent = ind; snprintf(blocks[depth].kind, sizeof(blocks[depth].kind), "fn"); depth++;
            function_indent = ind;
            free(raw); continue;
        }

        if (starts_with(t, "if ")) {
            char expr[MAX_LINE]; snprintf(expr, sizeof(expr), "%s", t + 3); transform_expr(expr, sizeof(expr));
            emit_indent(out, depth); fprintf(out, "if (%s) {\n", expr);
            blocks[depth].indent = ind; snprintf(blocks[depth].kind, sizeof(blocks[depth].kind), "if"); depth++;
            free(raw); continue;
        }
        if (starts_with(t, "else if ")) {
            if (depth > 0 && strcmp(blocks[depth - 1].kind, "if") == 0) {
                emit_indent(out, depth - 1); fputs("} ", out); depth--;
            } else {
                emit_indent(out, depth); fputs("", out);
            }
            char expr[MAX_LINE]; snprintf(expr, sizeof(expr), "%s", t + 8); transform_expr(expr, sizeof(expr));
            fprintf(out, "else if (%s) {\n", expr);
            blocks[depth].indent = ind; snprintf(blocks[depth].kind, sizeof(blocks[depth].kind), "if"); depth++;
            free(raw); continue;
        }
        if (strcmp(t, "else") == 0) {
            if (depth > 0 && strcmp(blocks[depth - 1].kind, "if") == 0) {
                emit_indent(out, depth - 1); fputs("} else {\n", out);
            } else {
                emit_indent(out, depth); fputs("else {\n", out);
                blocks[depth].indent = ind; snprintf(blocks[depth].kind, sizeof(blocks[depth].kind), "else"); depth++;
            }
            free(raw); continue;
        }
        if (starts_with(t, "while ")) {
            char expr[MAX_LINE]; snprintf(expr, sizeof(expr), "%s", t + 6); transform_expr(expr, sizeof(expr));
            emit_indent(out, depth); fprintf(out, "while (%s) {\n", expr);
            blocks[depth].indent = ind; snprintf(blocks[depth].kind, sizeof(blocks[depth].kind), "while"); depth++;
            free(raw); continue;
        }

        emit_indent(out, depth);
        if (starts_with(t, "let ")) {
            char *decl = t + 4;
            char *eq = strchr(decl, '=');
            if (!eq) { fprintf(stderr, "ucc: invalid let at line %d\n", i + 1); fclose(out); free(raw); free_lines(lines, count); return 1; }
            *eq = '\0'; trim_right(decl);
            char expr[MAX_LINE]; snprintf(expr, sizeof(expr), "%s", trim_left(eq + 1)); transform_expr(expr, sizeof(expr));
            const char *ty = expr[0] == '"' ? "const char *" : (strcmp(expr, "true") == 0 || strcmp(expr, "false") == 0 ? "bool" : "int64_t");
            fprintf(out, "%s %s = %s;\n", ty, decl, expr);
        } else if (starts_with(t, "return ")) {
            char expr[MAX_LINE]; snprintf(expr, sizeof(expr), "%s", t + 7); transform_expr(expr, sizeof(expr));
            fprintf(out, "return %s;\n", expr);
        } else if (starts_with(t, "std.console.println(") || starts_with(t, "console.println(")) {
            char *open = strchr(t, '('); char *close = strrchr(t, ')');
            if (!close) { fprintf(stderr, "ucc: invalid println at line %d\n", i + 1); fclose(out); free(raw); free_lines(lines, count); return 1; }
            *close = '\0';
            char expr[MAX_LINE]; snprintf(expr, sizeof(expr), "%s", open + 1); transform_expr(expr, sizeof(expr));
            if (expr[0] == '"') fprintf(out, "printf(\"%%s\\n\", %s);\n", expr);
            else fprintf(out, "printf(\"%%\" PRId64 \"\\n\", (int64_t)(%s));\n", expr);
        } else {
            char expr[MAX_LINE]; snprintf(expr, sizeof(expr), "%s", t); transform_expr(expr, sizeof(expr));
            int next_ind = next_relevant_indent(lines, count, i);
            int final_in_fn = function_indent >= 0 && (next_ind < 0 || next_ind < ind);
            if (final_in_fn && strcmp(current_return_type, "void") != 0 && !strchr(expr, '=')) {
                fprintf(out, "return %s;\n", expr);
            } else {
                fprintf(out, "%s;\n", expr);
            }
        }
        free(raw);
    }

    close_blocks(out, blocks, &depth, -1, 0);
    fclose(out);
    free_lines(lines, count);
    return 0;
}

static int run_command(const char *command) {
    int rc = system(command);
    if (rc != 0) fprintf(stderr, "ucc: command failed: %s\n", command);
    return rc == 0 ? 0 : 1;
}

static void usage(void) {
    puts("UbuLang ucc recovery-r1\n"
         "Usage:\n"
         "  ucc --version\n"
         "  ucc doctor\n"
         "  ucc emit-c INPUT.ubu -o OUTPUT.c\n"
         "  ucc build INPUT.ubu -o OUTPUT [--cc gcc]\n");
}

int main(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "--version") == 0) { puts(UCC_VERSION); return 0; }
    if (argc == 2 && strcmp(argv[1], "doctor") == 0) {
        puts("ucc recovery-r1: compiler executable OK");
#ifdef _WIN32
        return run_command("gcc --version > NUL 2>&1");
#else
        return run_command("gcc --version > /dev/null 2>&1");
#endif
    }
    if (argc < 5) { usage(); return 2; }
    const char *command = argv[1];
    const char *input = argv[2];
    const char *output = NULL;
    const char *cc = "gcc";
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) output = argv[++i];
        else if (strcmp(argv[i], "--cc") == 0 && i + 1 < argc) cc = argv[++i];
    }
    if (!output) die("missing -o OUTPUT");
    if (strcmp(command, "emit-c") == 0) return emit_c(input, output);
    if (strcmp(command, "build") == 0) {
        char temp[MAX_LINE]; snprintf(temp, sizeof(temp), "%s.c", output);
        if (emit_c(input, temp) != 0) return 1;
        char shell[MAX_LINE * 2];
        snprintf(shell, sizeof(shell), "%s -std=c11 -O2 -Wall -Wextra -Wpedantic \"%s\" -o \"%s\"", cc, temp, output);
        return run_command(shell);
    }
    usage(); return 2;
}
