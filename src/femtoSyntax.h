#ifndef FEMTO_SYNTAX_H
#define FEMTO_SYNTAX_H

#include "common.h"

#define MAX_KWBUF 25

#define MAX_C_TOKEN_MEM  169
#define MAX_C_TOKEN_WORD  14

#define MAX_CPP_TOKEN_MEM  289
#define MAX_CPP_TOKEN_WORD  16

#define MAX_PY_TOKEN_MEM  393
#define MAX_PY_TOKEN_WORD  21

#define MAX_JS_TOKEN_MEM  537
#define MAX_JS_TOKEN_WORD  18

#define MAX_RUST_TOKEN_MEM  215
#define MAX_RUST_TOKEN_WORD   8

#define MAX_GO_TOKEN_MEM  140
#define MAX_GO_TOKEN_WORD  11

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
	fstxRust,
	fstxGo,

	fstxSIZE
};

enum femtoSyntax fSyntaxDetect(const wchar_t * restrict fileName);

const char * fSyntaxName(enum femtoSyntax fs);
bool fSyntaxParseAutoAlloc(struct femtoLineNode * restrict node);

typedef void (*tokeniserFunc_t)(struct femtoLineNode * restrict node, uint32_t start, uint32_t lasti, WORD kwCol);

void checkCToken(struct femtoLineNode * restrict node, uint32_t start, uint32_t lasti, WORD kwCol);
void checkCPPToken(struct femtoLineNode * restrict node, uint32_t start, uint32_t lasti, WORD kwCol);
void checkPyToken(struct femtoLineNode * restrict node, uint32_t start, uint32_t lasti, WORD kwCol);
void checkJSToken(struct femtoLineNode * restrict node, uint32_t start, uint32_t lasti, WORD kwCol);
void checkRustToken(struct femtoLineNode * restrict node, uint32_t start, uint32_t lasti, WORD kwCol);
void checkGoToken(struct femtoLineNode * restrict node, uint32_t start, uint32_t lasti, WORD kwCol);

bool fSyntaxParseNone(struct femtoLineNode * restrict node, const WORD * restrict colors);
bool fSyntaxParseCLike(
	struct femtoLineNode * restrict node,
	const WORD * restrict colors,
	tokeniserFunc_t func,
	enum femtoSyntax lang
);
bool fSyntaxParseMd(struct femtoLineNode * restrict node, const WORD * restrict colors);

bool fSyntaxParsePy(struct femtoLineNode * restrict node, const WORD * restrict colors);

bool fSyntaxParseJSON(struct femtoLineNode * restrict node, const WORD * restrict colors);
bool fSyntaxParseCSS(struct femtoLineNode * restrict node, const WORD * restrict colors);

bool fSyntaxParseXML(struct femtoLineNode * restrict node, const WORD * restrict colors);


#endif
