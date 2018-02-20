#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf2any.h"

/*
 * Bourne shell translation from BF, runs at about 950,000 instructions per second.
 *
 * This is a 'current version' translation to the common Bourne shell
 * language.
 *
 * It runs on ash, bash, dash, ksh, lksh, mksh, pdksh-5.2.12+, yash, zsh.
 *
 * It does NOT run on ksh88, pdksh-5.2.3, posh
 *
 * Ash/Dash looks like the fastest at the above speed, Bash is the
 * slowest at about 180,000.
 */

static int do_input = 0;
static int do_output = 0;
static int ind = 0;
#define prv(s,v)        printf("%*s" s "\n", ind*4, "", (v))
#define pr(s)           printf("%*s" s "\n", ind*4, "")

struct be_interface_s be_interface = {};

static void print_string(void);

void
outcmd(int ch, int count)
{
    switch(ch) {
    case '!':
	pr("#!/bin/sh");
	pr("if (eval 'P=1 && : $((M$P=(3))) && Q=500 && : $((M$Q+=1)) &&");
	pr("   : $((M$P+=1)) && [ $((M$P)) = 4 -a $((M$Q)) = 1 ]' ) 2>/dev/null");
	pr("then :");
	pr("else");
	pr("    echo 'ERROR: The shell must be POSIX compatible' >&2");
	pr("    exit 1");
	pr("fi");
	pr("");

	pr("set -f");
	pr("");

	pr("brainfuck() {");
	ind++;
	prv("P=%d", tapeinit);
	break;

    case 'X': pr("echo Infinite Loop 2>&1 ; exit 1"); break;

    case '=': prv(": $((M$P=%d))", count); break;
    case 'B':
	if(bytecell) pr(": $((M$P&=255))");
	pr(": $((V=M$P))");
	break;
    case 'M': prv(": $((M$P+=V*%d))", count); break;
    case 'N': prv(": $((M$P-=V*%d))", count); break;
    case 'S': pr(": $((M$P+=V))"); break;
    case 'Q': prv("[ $V -ne 0 ] && : $((M$P=%d))", count); break;
    case 'm': prv("[ $V -ne 0 ] && : $((M$P+=V*%d))", count); break;
    case 'n': prv("[ $V -ne 0 ] && : $((M$P-=V*%d))", count); break;
    case 's': pr("[ $V -ne 0 ] && : $((M$P+=V))"); break;

    case '+': prv(": $((M$P+=%d))", count); break;
    case '-': prv(": $((M$P-=%d))", count); break;
    case '>': prv(": $((P+=%d))", count); break;
    case '<': prv(": $((P-=%d))", count); break;
    case '.': pr("o $((M$P&255))"); do_output++; break;
    case ',': pr("getch"); do_input++; break;
    case '"': print_string(); break;

    case '[':
	if(bytecell) { pr("while [ $((M$P&=255)) != 0 ] ; do"); }
	else { pr("while [ $((M$P)) != 0 ] ; do"); }
	ind++;
	break;
    case ']': ind--; pr("done"); break;

    case 'I':
	if(bytecell) { pr("if [ $((M$P&=255)) != 0 ] ; then"); }
	else { pr("if [ $((M$P)) != 0 ] ; then"); }
	ind++;
	break;
    case 'E': ind--; pr("fi"); break;

    case '~':
	ind--;
	pr("}");

	if (do_output) {
	    pr("");
	    pr("# shellcheck disable=SC2039,SC1117");
	    pr("if [ \".$(echo -n)\" = .-n ]");
	    pr("then");
	    pr("    echon() { echo \"$1\\c\"; }");
	    pr("    echoe() { echo \"$1\\c\"; }");
	    pr("else");
	    pr("    echon() { echo -n \"$1\"; }");
	    pr("    if [ \".$(echo -e)\" = .-e ]");
	    pr("    then echoe() { echo -n \"$1\"; }");
	    pr("    else # shellcheck disable=SC1075");
	    pr("         if [ \".$(echo -e '\\070\\c')\" = .8 ]");
	    pr("         then echoe() { echo -e \"$1\\c\"; }");
	    pr("         else echoe() { echo -n -e \"$1\"; }");
	    pr("         fi");
	    pr("    fi");
	    pr("fi");
	    pr("if [ \".$(echoe '\\070')\" != .8 ]");
	    pr("then echoe(){ printf \"%%b\" \"$*\" 2>/dev/null ; }");
	    pr("fi");
	    pr("if [ \".$(echoe '\\171')\" = .y ]");
	    pr("then o(){ echoe \"$(printf '\\\\%%03o' \"$1\")\" ; }");
	    pr("else o(){ echoe \"$(printf '\\\\%%04o' \"$1\")\" ; }");
	    pr("fi");
	}

	if (do_input) {
	    pr("");
	    pr("getch() {");
	    pr("    [ \"$goteof\" = \"y\" ] && return;");
	    pr("    [ \"$gotline\" != \"y\" ] && {");
	    pr("        if read -r line");
	    pr("        then");
	    pr("            gotline=y");
	    pr("        else");
	    pr("            goteof=y");
	    pr("            return");
	    pr("        fi");
	    pr("    }");
	    pr("    [ \"$line\" = \"\" ] && {");
	    pr("        gotline=n");
	    pr("        : $((M$P=10))");
	    pr("        return");
	    pr("    }");
	    pr("    A=\"$line\"");
	    pr("    while [ ${#A} -gt 1 ] ; do A=\"${A%%?}\"; done");
	    pr("    line=\"${line#?}\"");
	    pr("    A=$(printf %%d \\'\"$A\")");
	    pr("    : $((M$P=A))");
	    pr("}");
	}

	pr("");
	pr("brainfuck");
	break;
    }
}

static void
print_string(void)
{
    char * str = get_string();
    char buf[256];
    int badchar = 0;
    size_t outlen = 0;

    if (!str) return;

    for(;; str++) {
	if (outlen && (*str == 0 || badchar || outlen > sizeof(buf)-8))
	{
	    buf[outlen] = 0;
	    if (badchar == '\n') {
		prv("echo '%s'", buf);
		badchar = 0;
	    } else {
		do_output++;
		prv("echon '%s'", buf);
	    }
	    outlen = 0;
	}
	if (badchar) {
	    if (badchar == 10)
		pr("echo");
	    else {
		prv("o %d", badchar);
		do_output++;
	    }
	    badchar = 0;
	}
	if (!*str) break;

	if (*str == '-' && outlen == 0) {
	    badchar = (*str & 0xFF);
	} else if (*str >= ' ' && *str <= '~' && *str != '\'') {
	    buf[outlen++] = *str;
	} else if (*str == '\'') {
	    buf[outlen++] = '\'';
	    buf[outlen++] = '\\';
	    buf[outlen++] = *str;
	    buf[outlen++] = '\'';
	} else {
	    badchar = (*str & 0xFF);
	}
    }
}
