#ifndef FEMTO_SYNTAX_H
#define FEMTO_SYNTAX_H

#include "common.h"

#define MAX_KWBUF 20

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

typedef void (*tokeniserFunc_t)(struct femtoLineNode * restrict node, uint32_t start, uint32_t lasti, WORD kwCol);

void checkCToken(struct femtoLineNode * restrict node, uint32_t start, uint32_t lasti, WORD kwCol);
void checkCPPToken(struct femtoLineNode * restrict node, uint32_t start, uint32_t lasti, WORD kwCol);

bool fSyntaxParseNone(struct femtoLineNode * restrict node, const WORD * restrict colors);
bool fSyntaxParseCLike(struct femtoLineNode * restrict node, const WORD * restrict colors, tokeniserFunc_t func);
bool fSyntaxParseMd(struct femtoLineNode * restrict node, const WORD * restrict colors);

#endif
