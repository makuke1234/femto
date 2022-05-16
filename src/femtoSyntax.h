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
	fstxPY,
	fstxJS,
	fstxJSON,
	fstxCSS,
	fstxXML,
	fstxHTML,
	fstxSVG,

	fstxSIZE
};

enum femtoSyntax fSyntaxDetect(const wchar_t * restrict fileName);

const char * fSyntaxName(enum femtoSyntax fs);
bool fSyntaxParseAutoAlloc(struct femtoLineNode * restrict node);

typedef void (*tokeniserFunc_t)(struct femtoLineNode * restrict node, uint32_t start, uint32_t lasti, WORD kwCol);

void checkCToken(struct femtoLineNode * restrict node, uint32_t start, uint32_t lasti, WORD kwCol);
void checkCPPToken(struct femtoLineNode * restrict node, uint32_t start, uint32_t lasti, WORD kwCol);

bool fSyntaxParseNone(struct femtoLineNode * restrict node, const WORD * restrict colors);
bool fSyntaxParseCLike(struct femtoLineNode * restrict node, const WORD * restrict colors, tokeniserFunc_t func);
bool fSyntaxParseMd(struct femtoLineNode * restrict node, const WORD * restrict colors);

bool fSyntaxParsePy(struct femtoLineNode * restrict node, const WORD * restrict colors);

bool fSyntaxParseJS(struct femtoLineNode * restrict node, const WORD * restrict colors);
bool fSyntaxParseJSON(struct femtoLineNode * restrict node, const WORD * restrict colors);
bool fSyntaxParseCSS(struct femtoLineNode * restrict node, const WORD * restrict colors);

bool fSyntaxParseXML(struct femtoLineNode * restrict node, const WORD * restrict colors);


#endif
