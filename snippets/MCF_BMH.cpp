#include <type_traits>
#include <cstddef>

#include <iostream>
#include <iomanip>
#include <string>


const std::string text    = "aabababaabbaaababababbbbabbaabbabaabababababbababbabbaa";
const std::string pattern = "bbababbabba";
//const std::string text    = "aaaa__aa__aaa___aaaa_____aaaaa_____aaaaaa_____aaaaaaaa_____aa_______aa";
//const std::string pattern = "__aa_______";
//const std::string text    = "MANPANAMPNAM";
//const std::string pattern = "ANAMPNAM";

namespace MCF {
	template<typename TextBeginT, typename TextEndT, typename PatternBeginT, typename PatternEndT>
	TextBeginT FindSpan(TextBeginT itTextBegin, TextEndT itTextEnd, PatternBeginT itPatternBegin, PatternEndT itPatternEnd){
		const auto nPatternLength = static_cast<std::ptrdiff_t>(itPatternEnd - itPatternBegin);
		if(nPatternLength < 0){
			return itTextEnd;
		}
		if(nPatternLength == 0){
			return itTextBegin;
		}
		const auto nTextCount = static_cast<std::ptrdiff_t>(itTextEnd - itTextBegin);
		if(nTextCount < nPatternLength){
			return itTextEnd;
		}

		// https://en.wikipedia.org/wiki/Boyer-Moore-Horspool_algorithm
		// We store the offsets as small integers using saturation arithmetic for space efficiency. Bits that do not fit into a byte are truncated.
		constexpr unsigned kBcrTableSize = 256;
		__attribute__((__aligned__(64))) short ashBcrTable[kBcrTableSize];
		const std::ptrdiff_t nMaxBcrShift = (nPatternLength <= 0x7FFF) ? nPatternLength : 0x7FFF;
		for(unsigned uIndex = 0; uIndex < kBcrTableSize; ++uIndex){
			ashBcrTable[uIndex] = static_cast<short>(nMaxBcrShift);
		}
		for(std::ptrdiff_t nBcrShift = nMaxBcrShift - 1; nBcrShift > 0; --nBcrShift){
			const auto chGoodChar = itPatternBegin[nPatternLength - nBcrShift - 1];
			ashBcrTable[static_cast<std::make_unsigned_t<decltype(chGoodChar)>>(chGoodChar) % kBcrTableSize] = static_cast<short>(nBcrShift);
		}

		// https://en.wikipedia.org/wiki/Boyer-Moore_string_search_algorithm
		// We create the GSR table from an intermediate table of suffix offsets.
		constexpr unsigned kGsrTableSize = 512;
		__attribute__((__aligned__(64))) short ashGsrTable[kGsrTableSize];
		const std::ptrdiff_t nMaxGsrShift = (nPatternLength <= kGsrTableSize) ? nPatternLength : kGsrTableSize;
		std::ptrdiff_t nGsrCandidateLength = 0;
		ashGsrTable[0] = 1;
		for(std::ptrdiff_t nTestIndex = 1; nTestIndex < nMaxGsrShift; ++nTestIndex){
			const auto chTest = itPatternBegin[nPatternLength - nTestIndex - 1];
			for(;;){
				const auto chCandidateFront = itPatternBegin[nPatternLength - nGsrCandidateLength - 1];
				if(chTest == chCandidateFront){
					++nGsrCandidateLength;
					break;
				}
				if(nGsrCandidateLength == 0){
					break;
				}
				nGsrCandidateLength -= ashGsrTable[nGsrCandidateLength - 1];
			}
			ashGsrTable[nTestIndex] = static_cast<short>(nTestIndex - nGsrCandidateLength + 1);
		}
		std::ptrdiff_t nGsrLastOffset = 1;
		ashGsrTable[0] = 0;
		for(std::ptrdiff_t nTestIndex = 1; nTestIndex < nMaxGsrShift; ++nTestIndex){
			const std::ptrdiff_t nGsrOffset = ashGsrTable[nTestIndex];
			ashGsrTable[nTestIndex] = static_cast<short>(nMaxGsrShift - nGsrCandidateLength);
			if(nGsrLastOffset != nGsrOffset){
				for(std::ptrdiff_t nWriteIndex = nTestIndex - nGsrLastOffset; nWriteIndex > 0; nWriteIndex -= nGsrLastOffset){
					const std::ptrdiff_t nGsrShift = nTestIndex - nWriteIndex;
					if(ashGsrTable[nWriteIndex] <= nGsrShift){
						break;
					}
					ashGsrTable[nWriteIndex] = static_cast<short>(nGsrShift);
				}
			}
			nGsrLastOffset = nGsrOffset;
		}
std::cerr <<"gsr table: ";
for(int i = 0; i < nMaxGsrShift; ++i){
	std::cerr <<ashGsrTable[i] <<", ";
}
std::cerr <<std::endl;

		std::ptrdiff_t nOffset = 0;
		std::ptrdiff_t nKnownMatchEnd = 0;
		std::ptrdiff_t nKnownMatchBegin = 0;
		for(;;){
std::cerr <<"offset = " <<nOffset <<std::endl;
std::cerr <<"  > " <<text <<std::endl;
std::cerr <<"  > " <<std::setw(static_cast<int>(nOffset)) <<"" <<pattern <<std::endl;
			if(nTextCount - nOffset < nPatternLength){
std::cerr <<" -- not found" <<std::endl;
				return itTextEnd;
			}
			std::ptrdiff_t nTestIndex = nPatternLength - 1;
			const auto chLast = itTextBegin[nOffset + nTestIndex];
			for(;;){
				const auto chText = itTextBegin[nOffset + nTestIndex];
				const auto chPattern = itPatternBegin[nTestIndex];
				if(chText != chPattern){
std::cerr <<"  ! " <<std::setw(static_cast<int>(nOffset + nTestIndex)) <<"" <<"^" <<" mismatch" <<std::endl;
					const auto nSuffixLength = nPatternLength - nTestIndex - 1;
					const std::ptrdiff_t nBcrShift = ashBcrTable[static_cast<std::make_unsigned_t<decltype(chLast)>>(chLast) % kBcrTableSize];
					const std::ptrdiff_t nGsrShift = (nSuffixLength < nMaxGsrShift) ? ashGsrTable[nSuffixLength] : 1;
					if(nBcrShift > nGsrShift){
						nOffset += nBcrShift;
						nKnownMatchEnd = nPatternLength - nBcrShift;
						nKnownMatchBegin = nKnownMatchEnd - 1;
					} else {
						nOffset += nGsrShift;
						nKnownMatchEnd = nPatternLength - nGsrShift;
						nKnownMatchBegin = nKnownMatchEnd - nSuffixLength;
					}
std::cerr <<" -- bailing out at " <<nTestIndex <<", bcr = " <<nBcrShift <<", gsr = " <<nGsrShift <<", new_offset = " <<nOffset <<std::endl;
					break;
				}
				if(nTestIndex == nKnownMatchEnd){
std::cerr <<"  @ " <<std::setw(static_cast<int>(nOffset + nKnownMatchBegin)) <<"" <<std::setw(static_cast<int>(nKnownMatchEnd - nKnownMatchBegin)) <<std::setfill('<') <<"" <<std::setfill(' ') <<" skipped" <<std::endl;
					nTestIndex = nKnownMatchBegin;
				}
				if(nTestIndex <= 0){
std::cerr <<" -- found match at " <<nOffset <<std::endl;
					return itTextBegin + nOffset;
				}
				--nTestIndex;
			}
		}
	}
}

int main(){
	auto pos = MCF::FindSpan(text.begin(), text.end(), pattern.begin(), pattern.end());
	std::cout <<"pos = " <<(pos - text.begin()) <<std::endl;
}
