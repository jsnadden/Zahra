#pragma once

namespace Zahra
{
	// from https://jrgraphix.net/research/unicode_blocks.php	

	#define Z_UNICODE_RANGE_Latin			{ 0x0020, 0x007F }
	#define Z_UNICODE_RANGE_LatinSupp		{ 0x00A0, 0x00FF }
	#define Z_UNICODE_RANGE_CJKSymbols		{ 0x3000, 0x303F }
	#define Z_UNICODE_RANGE_CJKIdeograms	{ 0x4E00, 0x9FFF }
	#define Z_UNICODE_RANGE_Hiragana		{ 0x3040, 0x309F }
	#define Z_UNICODE_RANGE_Katakana		{ 0x30A0, 0x30FF }
	#define Z_UNICODE_RANGE_KatakanaExt		{ 0x31F0, 0x31FF }
	#define Z_UNICODE_RANGE_HalfWidth		{ 0xFF00, 0xFFEF }
	#define Z_UNICODE_RANGE_Specials		{ 0xFFF0, 0xFFFF }

}
