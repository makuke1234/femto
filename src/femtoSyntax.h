#ifndef FEMTO_SYNTAX_H
#define FEMTO_SYNTAX_H

#include "common.h"

#define MAX_KWBUF 10

struct femtoLineNode;

enum femtoSyntax
{
	fstxNONE,
	fstxC,
	fstxCPP,
	fstxMD,

	fstxSIZE
};

const char * fSyntaxName(enum femtoSyntax fs);
bool fSyntaxParseAutoAlloc(struct femtoLineNode * restrict node);

bool fSyntaxParseC(struct femtoLineNode * restrict node);
bool fSyntaxParseCpp(struct femtoLineNode * restrict node);
bool fSyntaxParseMd(struct femtoLineNode * restrict node);

#endif
