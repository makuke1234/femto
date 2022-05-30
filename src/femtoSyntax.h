#ifndef FEMTO_SYNTAX_H
#define FEMTO_SYNTAX_H

#include "common.h"

#define MAX_KWBUF 25

#define MAX_C_TOKEN_MEM  600
#define MAX_C_TOKEN_WORD  14

#define MAX_CPP_TOKEN_MEM  810
#define MAX_CPP_TOKEN_WORD  16

#define MAX_PY_TOKEN_MEM  393
#define MAX_PY_TOKEN_WORD  21

#define MAX_JS_TOKEN_MEM  537
#define MAX_JS_TOKEN_WORD  18

#define MAX_RUST_TOKEN_MEM  260
#define MAX_RUST_TOKEN_WORD  10

#define MAX_GO_TOKEN_MEM  140
#define MAX_GO_TOKEN_WORD  11

struct femtoLineNode;

// Enum that defines different supported language syntax codes
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

// Detects syntax from filename
enum femtoSyntax fSyntaxDetect(const wchar * restrict fileName);

// Returns const pointer to syntax name from syntax identifier
const char * fSyntaxName(enum femtoSyntax fs);
// Automatically allocates memory for the syntax parser
bool fSyntaxParseAutoAlloc(struct femtoLineNode * restrict node);

// Tokeniser function signature for fSyntaxParseCLike
typedef void (*tokeniserFunc_t)(struct femtoLineNode * restrict node, u32 start, u32 lasti, WORD kwCol);

// Token parsers
void checkCToken(struct femtoLineNode * restrict node, u32 start, u32 lasti, WORD kwCol);
void checkCPPToken(struct femtoLineNode * restrict node, u32 start, u32 lasti, WORD kwCol);
void checkPyToken(struct femtoLineNode * restrict node, u32 start, u32 lasti, WORD kwCol);
void checkJSToken(struct femtoLineNode * restrict node, u32 start, u32 lasti, WORD kwCol);
void checkRustToken(struct femtoLineNode * restrict node, u32 start, u32 lasti, WORD kwCol);
void checkGoToken(struct femtoLineNode * restrict node, u32 start, u32 lasti, WORD kwCol);

// Syntax parsers

/**
 * @brief Doesn't parse any syntax, just enjoy the view...
 * 
 * @param node Pointer to line node
 * @param colors Colors vector
 * @return true Success
 * @return false Failure
 */
bool fSyntaxParseNone(struct femtoLineNode * restrict node, const WORD * restrict colors);
/**
 * @brief Doesn't parse any syntax, just enjoy the view...
 * 
 * @param node Pointer to line node
 * @param colors Colors vector
 * @param func Tokeniser function pointer
 * @param lang Language identifier
 * @return true Success
 * @return false Failure
 */
bool fSyntaxParseCLike(
	struct femtoLineNode * restrict node,
	const WORD * restrict colors,
	tokeniserFunc_t func,
	enum femtoSyntax lang
);
/**
 * @brief Parses markdown.
 * 
 * @param node Pointer to line node
 * @param colors Colors vector
 * @return true Success
 * @return false Failure
 */
bool fSyntaxParseMd(struct femtoLineNode * restrict node, const WORD * restrict colors);

/**
 * @brief Parses Python... Huh, who uses that?
 * 
 * @param node Pointer to line node
 * @param colors Colors vector
 * @return true Success
 * @return false Failure
 */
bool fSyntaxParsePy(struct femtoLineNode * restrict node, const WORD * restrict colors);

/**
 * @brief Let's parse the ominous JSON syntax. Actually, it's not hard hard to do.
 * 
 * @param node Pointer to line node
 * @param colors Colors vector
 * @return true Success
 * @return false Failure
 */
bool fSyntaxParseJSON(struct femtoLineNode * restrict node, const WORD * restrict colors);
/**
 * @brief Who doesn't need a little CSS here and there?!??
 * 
 * @param node Pointer to line node
 * @param colors Colors vector
 * @return true Success
 * @return false Failure
 */
bool fSyntaxParseCSS(struct femtoLineNode * restrict node, const WORD * restrict colors);

/**
 * @brief HTML is basically the same as XML
 * 
 * @param node Pointer to line node
 * @param colors Colors vector
 * @return true Success
 * @return false Failure
 */
bool fSyntaxParseXML(struct femtoLineNode * restrict node, const WORD * restrict colors);


#endif
