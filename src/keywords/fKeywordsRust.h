#ifndef FKEYWORDS_RUST_H
#define FKEYWORDS_RUST_H

#include "../fCommon.h"

static const wchar * s_keyWordsRust[] = {
	L"as",
	L"break",
	L"const",
	L"continue",
	L"crate",
	L"else",
	L"enum",
	L"extern",
	L"false",
	L"fn",
	L"for",
	L"if",
	L"impl",
	L"in",
	L"let",
	L"loop",
	L"match",
	L"mod",
	L"move",
	L"mut",
	L"pub",
	L"ref",
	L"return",
	L"self",
	L"Self",
	L"static",
	L"struct",
	L"super",
	L"trait",
	L"true",
	L"type",
	L"unsafe",
	L"use",
	L"where",
	L"while",

	L"async",
	L"await",
	L"dyn",

	L"abstract",
	L"become",
	L"box",
	L"do",
	L"final",
	L"macro",
	L"override",
	L"priv",
	L"typeof",
	L"unsized",
	L"virtual",
	L"yield",

	L"try",

	L"union",

	// Built-in types
	L"bool"
	L"char",
	L"str",

	L"i8",
	L"u8",
	L"i16",
	L"u16",
	L"i32",
	L"u32",
	L"i64",
	L"u64",
	L"i128",
	L"u128",
	L"isize",
	L"usize",

	L"f32",
	L"f64",

	L"size_of",
	L"String",

	// Traits
	L"Box",
	L"Rc",
	L"Arc",
	L"Pin",
	L"Deref",
	L"DerefMut",
	L"Drop",
	L"Copy",
	L"Clone",
	L"Send",
	L"Sync",
	L"Unpin",
	L"UnwindSafe",
	L"Sized"
};

#endif
