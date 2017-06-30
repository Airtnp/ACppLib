#ifndef SN_STRING_CONVERT_H
#define SN_STRING_CONVERT_H

#include "../sn_CommonHeader.h"

namespace sn_String {
	// TODO: add https://www.codeproject.com/Articles/38242/Reading-UTF-with-C-streams
    namespace convert {
        constexpr const std::size_t MAXBUFFERSIZE = 2048;
        inline char* UTF8_to_string(const char* src, char* dest, unsigned int dest_size) {
            wchar_t wbuffer[MAXBUFFERSIZE];
#ifdef __WIN32__ && SN_ENABLE_WINDOWS_API
            ::MultiByteToWideChar(CP_ACP, 0, src, -1, wbuffer, MAXBUFFERSIZE);
            ::WideCharToMultiByte(CP_UTF8, 0, wbuffer, -1, dest, dest_size, NULL, NULL);
#else
            std::mbstowcs(wbuffer, src, 2048);
            std::wcstombs(dest, wbuffer, dest_size);
#endif
            return dest;
        }

        using std::string;
		using std::wstring;
		using std::stringstream;
		using std::setw;
		using std::setfill;
		string wstring_to_utf8(const wstring& str) {
			std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
			return myconv.to_bytes(str);
		}

		wstring gbk_to_wstring(const string& gbk_str) {
			string gbk_locale_name = ".936";
			std::wstring_convert<std::codecvt_byname<wchar_t, char, mbstate_t>> conv(new std::codecvt_byname<wchar_t, char, mbstate_t>(gbk_locale_name));
			return conv.from_bytes(gbk_str);
		}

		//for outputing chinese rows, we use wcout(.imbue(local("chs"))) << utf8_to_wstring(content)
		wstring utf8_to_wstring(const string& str) {
			std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
			return myconv.from_bytes(str);
		}

		wstring string_to_wstring(const string& str) {
			try {
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
				wstring wstr = conv.from_bytes(str);
				return wstr;
			}
			catch (std::range_error&)
			{
				wstring wstr = gbk_to_wstring(str);
				return wstr;
			}
		}

		string wstring_to_string(const wstring& wstr) {
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
			string str = conv.to_bytes(wstr);
			return str;
		}

		//for chinese fields, we use hex(field_name) = wstring_to_hex(L'example')
		string wstring_to_hex(const wstring& str) {
			string u8str = wstring_to_utf8(str);
			stringstream ss;
			ss << std::hex;
			for (unsigned char c : u8str)
				ss << setw(2) << setfill('0') << static_cast<int>(c);
			string hexstr = ss.str();
			hexstr = "\'" + hexstr + "\'";
			return hexstr;
		}

		string wstring_to_unhex(const wstring& str) {
			string unhexstr = "unhex(" + wstring_to_hex(str) + ")";
			return unhexstr;
		}

		string string_to_hex(const string& str) {
			wstring wstr = string_to_wstring(str);
			return wstring_to_hex(wstr);
		}

		string string_to_unhex(const string& str) {
			wstring wstr = string_to_wstring(str);
			return wstring_to_unhex(wstr);
		}

		void hex_print(const std::string& s) {
			std::cout << std::hex << std::setfill('0');
			for(unsigned char c : s)
				std::cout << std::setw(2) << static_cast<int>(c) << ' ';
			std::cout << std::dec << '\n';
		}

#ifdef __WIN32__ && SN_ENABLE_WINDOWS_API
		std::string ConvertFromUtf16ToUtf8(const std::wstring& wstr) {
			std::string convertedString;
			int requiredSize = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, 0, 0, 0, 0);
			if(requiredSize > 0)
			{
				std::vector<char> buffer(requiredSize);
				WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &buffer[0], requiredSize, 0, 0);
				convertedString.assign(buffer.begin(), buffer.end() - 1);
			}
			return convertedString;
		}
		
		std::wstring ConvertFromUtf8ToUtf16(const std::string& str) {
			std::wstring convertedString;
			int requiredSize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, 0, 0);
			if(requiredSize > 0)
			{
				std::vector<wchar_t> buffer(requiredSize);
				MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &buffer[0], requiredSize);
				convertedString.assign(buffer.begin(), buffer.end() - 1);
			}
		
			return convertedString;
		}
#endif
    }
}



#endif