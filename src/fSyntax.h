#ifndef FEMTO_SYNTAX_H
#define FEMTO_SYNTAX_H

#include "fCommon.h"

#define MAX_KWBUF 25

#define MAX_C_TOKEN_MEM  615
#define MAX_C_TOKEN_WORD  14

#define MAX_CPP_TOKEN_MEM  1025
#define MAX_CPP_TOKEN_WORD  17

#define MAX_PY_TOKEN_MEM  393
#define MAX_PY_TOKEN_WORD  21

#define MAX_JS_TOKEN_MEM  537
#define MAX_JS_TOKEN_WORD  18

#define MAX_RUST_TOKEN_MEM  260
#define MAX_RUST_TOKEN_WORD  10

#define MAX_GO_TOKEN_MEM  140
#define MAX_GO_TOKEN_WORD  11

struct fLine;
struct fStatHash;

typedef enum fTokenColor
{
	tcTEXT,
	tcSEARCH_RESULT,
	tcHIGHLIGHT,
	tcCOMMENT_LINE,
	tcCOMMENT_BLOCK,
	tcKEYWORD,
	tcSTRING,
	tcSTRING_QUOTE,
	tcCHARACTER,
	tcCHARACTER_QUOTE,
	tcESCAPE,
	tcPREPROC,
	tcNUMBER,
	tcHEX,
	tcOCT,
	tcPUNCTUATION,
	tcMD_HEADING,
	tcMD_ITALIC,
	tcMD_BOLD,
	tcMD_STRIKE,
	tcMD_VALUE,
	tcMD_CONETEXT,
	tcMD_BRACKET,
	tcMD_BRACKETPIC,
	tcMD_LINK,
	tcXML_ID,
	tcCSS_SELECTOR,
	tcCSS_PROPERTY,


	tcNUM_OF_TOKENS

} fTokenColor_e, fTC_e;

// Enum that defines different supported language syntax codes
typedef enum fSyntax
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

} fSyntax_e, fStx_e;

// Detects syntax from filename
fStx_e fStx_detect(const wchar * restrict fileName);

// Returns const pointer to syntax name from syntax identifier
const char * fStx_name(fStx_e fs);
// Automatically allocates memory for the syntax parser
bool fStx_autoAlloc(struct fLine * restrict node);

// Tokeniser function signature for fStx_parseCLike
typedef void (*fStx_tokeniserFunc_t)(struct fLine * restrict node, usize start, usize lasti, WORD kwCol);

// Token parsers
/**
 * @brief Generic token parses, uses prefilled hashmap as look-up table
 * 
 * @param node Pointer to line node
 * @param start Start index
 * @param lasti Stop index
 * @param kwCol Keyword color
 * @param map Pointer to hashmap
 */
void fStx_checkGenericToken(
	struct fLine * restrict node, usize start, usize lasti,
	WORD kwCol,
	const struct fStatHash * restrict map
);

void fStx_checkCToken(struct fLine * restrict node, usize start, usize lasti, WORD kwCol);
void fStx_checkCPPToken(struct fLine * restrict node, usize start, usize lasti, WORD kwCol);
void fStx_checkPyToken(struct fLine * restrict node, usize start, usize lasti, WORD kwCol);
void fStx_checkJSToken(struct fLine * restrict node, usize start, usize lasti, WORD kwCol);
void fStx_checkRustToken(struct fLine * restrict node, usize start, usize lasti, WORD kwCol);
void fStx_checkGoToken(struct fLine * restrict node, usize start, usize lasti, WORD kwCol);

// Syntax parsers

/**
 * @brief Doesn't parse any syntax, just enjoy the view...
 * 
 * @param node Pointer to line node
 * @param colors Colors vector
 * @return true Success
 * @return false Failure
 */
bool fStx_parseNone(struct fLine * restrict node, const WORD * restrict colors);
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
bool fStx_parseCLike(
	struct fLine * restrict node,
	const WORD * restrict colors,
	fStx_tokeniserFunc_t func,
	fStx_e lang
);
/**
 * @brief Parses markdown.
 * 
 * @param node Pointer to line node
 * @param colors Colors vector
 * @return true Success
 * @return false Failure
 */
bool fStx_parseMd(struct fLine * restrict node, const WORD * restrict colors);

/**
 * @brief Parses Python... Huh, who uses that?
 * 
 * @param node Pointer to line node
 * @param colors Colors vector
 * @return true Success
 * @return false Failure
 */
bool fStx_parsePy(struct fLine * restrict node, const WORD * restrict colors);

/**
 * @brief Let's parse the ominous JSON syntax. Actually, it's not hard hard to do.
 * 
 * @param node Pointer to line node
 * @param colors Colors vector
 * @return true Success
 * @return false Failure
 */
bool fStx_parseJSON(struct fLine * restrict node, const WORD * restrict colors);
/**
 * @brief Who doesn't need a little CSS here and there?!??
 * 
 * @param node Pointer to line node
 * @param colors Colors vector
 * @return true Success
 * @return false Failure
 */
bool fStx_parseCSS(struct fLine * restrict node, const WORD * restrict colors);

/**
 * @brief HTML is basically the same as XML
 * 
 * @param node Pointer to line node
 * @param colors Colors vector
 * @return true Success
 * @return false Failure
 */
bool fStx_parseXML(struct fLine * restrict node, const WORD * restrict colors);


#endif
