#include "FFileParser.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#pragma region(variable)
const unsigned char TIXML_UTF_LEAD_0 = 0xefU;
const unsigned char TIXML_UTF_LEAD_1 = 0xbbU;
const unsigned char TIXML_UTF_LEAD_2 = 0xbfU;
static char _retParser[1024]{ 0, };
static char _iniSubBuff[1024]{ 0, };
static char _xmlRetBuff[1024]{ 0, };
#pragma endregion

FILE* TiXmlFOpen(const char* filename, const char* mode)
{
#if defined(_MSC_VER) && (_MSC_VER >= 1400 )
	FILE* fp = 0;
	errno_t err = fopen_s(&fp, filename, mode);
	if (!err && fp)
		return fp;
	return 0;
#else
	return fopen(filename, mode);
#endif
}

#pragma region(control class)
class iniParser
{
public:
	iniParser(char* cmd)
	{
		int commaPos = FindIndex(cmd, ',', 0);
		_section = new char[commaPos + 1]{ 0, };
		_key = new char[strlen(cmd) - commaPos + 1]{ 0, };
		sprintf_s(_section, commaPos + 1, "%s", SubStr(cmd, 0, commaPos));
		sprintf_s(_key, strlen(cmd) - commaPos + 1, "%s", SubStr(cmd, commaPos + 1, strlen(cmd) - commaPos - 1));
	}
	~iniParser()
	{
		if (_section)	delete[] _section;
		if (_key)		delete[] _key;
	}
private:
	char* _section;
	char* _key;
	int FindIndex(char* msg, char ch, int offset)
	{
		char* temp = msg;
		*(temp = temp + offset);
		while (1)
		{
			if (*(temp) == 0)
				return -1;
			if (*(temp++) == ch)
				break;
		}

		return (int)(temp - msg) - 1;
	}
	void Erase(char* msg, unsigned int startIndex, unsigned int count)
	{
		char* temp = msg;
		if (strlen(temp) <= startIndex)
			return;

		if (strlen(temp) <= startIndex + count)
			count = strlen(temp) - startIndex;

		char* left = new char[startIndex + 1]{ 0, };
		for (unsigned int i = 0; i < startIndex; i++)
		{
			left[i] = *(temp++);
		}
		for (unsigned int i = 0; i < count; i++)
		{
			*(temp++);
		}
		unsigned int rightLength = strlen(temp);
		char* right = new char[rightLength + 1]{ 0, };
		for (unsigned int i = 0; i < rightLength; i++)
		{
			right[i] = *(temp++);
		}
		sprintf_s(msg, strlen(msg), "%s%s", left, right);
		delete[] left;
		delete[] right;
	}
	char* SubStr(char* msg, unsigned int startIndex, unsigned int count)
	{
		ZeroMemory(_iniSubBuff, sizeof(_iniSubBuff));
		char* temp = msg;
		for (unsigned int i = 0; i < startIndex; i++)
		{
			if (*(temp) == 0)
				return temp;
			*(temp++);
		}
		if (*(temp) == 0)
			return temp;
		for (unsigned int i = 0; i < count; i++)
		{
			_iniSubBuff[i] = *(temp++);
			if (*(temp) == 0)
				return _iniSubBuff;
		}

		return _iniSubBuff;
	}
public:
	char* GetString(char* defValue, char* filePath)
	{
		if (_access_s(filePath, 0) != 0)
		{
			return defValue;
		}

		GetPrivateProfileStringA(_section, _key, defValue, _retParser, 1024, filePath);
		return _retParser;
	}
	int GetInt(int defValue, char* filePath)
	{
		if (_access_s(filePath, 0) != 0)
		{
			return defValue;
		}

		char defTemp[1024] = { 0, };
		sprintf_s(defTemp, 1024, "%d", defValue);
		int ret = atoi(GetString(defTemp, filePath));
		return ret;
	}
	double GetDouble(double defValue, char* filePath)
	{
		if (_access_s(filePath, 0) != 0)
		{
			return defValue;
		}

		char defTemp[1024] = { 0, };
		sprintf_s(defTemp, 1024, "%.4f", defValue);
		return atof(GetString(defTemp, filePath));
	}

	void SetString(char* value, char* filePath)
	{
		if (_access_s(filePath, 0) != 0)
		{
			return;
		}

		WritePrivateProfileStringA(_section, _key, value, filePath);
	}
	void SetInt(int value, char* filePath)
	{
		if (_access_s(filePath, 0) != 0)
		{
			return;
		}

		char temp[1024] = { 0, };
		sprintf_s(temp, 1024, "%d", value);
		SetString(temp, filePath);
	}
	void SetDouble(double value, char* filePath)
	{
		if (_access_s(filePath, 0) != 0)
		{
			return;
		}

		char temp[1024] = { 0, };
		sprintf_s(temp, 1024, "%.4f", value);
		SetString(temp, filePath);
	}
};
class xmlParser
{
public:
	xmlParser(char* cmd)
	{
		_count = 0;
		ZeroMemory(_sepChar, sizeof(_sepChar));
		ZeroMemory(_tokenChar, sizeof(_tokenChar));
		SplitChar(cmd, (char*)",");
	}
	~xmlParser()
	{
		ReleaseAllToken();
	}
private:
	int  _count;
	char _sepChar[50];
	char* _tokenChar[100];

	void ReleaseAllToken()
	{
		for (int i = 0; i < _count; i++)
		{
			if (_tokenChar[i])
			{
				delete[] _tokenChar[i];
				_tokenChar[i] = NULL;
			}
		}
	}
	int FindChar(char* source, char* sep)
	{
		char* addr = source;

		int sepSize = strlen((char*)sep);

		while (memcmp(addr, sep, sepSize) != 0)
		{
			if (*(addr++) == 0x0)
				return -1;
		}

		return (int)(addr - source);
	}
	void SplitChar(char* source, char* sep)
	{
		SetSeperatorChar(sep);
		ReleaseAllToken();

		char* temp = source;
		int		sepSize = strlen((char*)_sepChar);
		int		pos = 0;
		int		token = 0;

		while (true)
		{
			int splitSize = FindChar((temp + pos), _sepChar);
			if (splitSize != -1)
			{
				_tokenChar[token] = new char[splitSize + 1];
				memcpy(_tokenChar[token], temp + pos, splitSize * sizeof(*source));
				memset((void*)&_tokenChar[token++][splitSize], 0x00, sizeof(*source));
				pos += (sepSize + splitSize);
			}
			else
			{
				int length = strlen((char*)temp + pos);
				_tokenChar[token] = new char[length + 1];
				memcpy(_tokenChar[token], temp + pos, length * sizeof(*source));
				memset((void*)&_tokenChar[token++][length], 0x00, sizeof(*source));
				break;
			}
		}
		_count = token;
	}
	char* GetTokenChar(int index)
	{
		static char* szEmpty = (char*)"";
		if (index >= _count)
			return (char*)szEmpty;

		return _tokenChar[index];
	}
	void SetSeperatorChar(char* sep)
	{
		strcpy_s((char*)_sepChar, 50, (const char*)sep);
	}
	int GetSplitSize()
	{
		return _count;
	}

	char* ANSIToUTF8(char* ansiString)
	{
		int	uniCodeLength, utf8Length;
		wchar_t* uniCodeStr;
		char* UTF8String = NULL;

		uniCodeLength = MultiByteToWideChar(CP_ACP, 0, ansiString, strlen(ansiString), NULL, NULL);
		uniCodeStr = SysAllocStringLen(NULL, uniCodeLength);
		MultiByteToWideChar(CP_ACP, 0, ansiString, strlen(ansiString), uniCodeStr, uniCodeLength);

		utf8Length = WideCharToMultiByte(CP_UTF8, 0, uniCodeStr, -1, UTF8String, 0, NULL, NULL);
		UTF8String = new char[utf8Length + 1];
		WideCharToMultiByte(CP_UTF8, 0, uniCodeStr, -1, UTF8String, utf8Length, NULL, NULL);

		ZeroMemory(_xmlRetBuff, sizeof(_xmlRetBuff));
		sprintf_s(_xmlRetBuff, 1024, "%s", UTF8String);
		delete[] UTF8String;

		return _xmlRetBuff;
	}
	char* UTF8toANSI(char* utf8String)
	{
		wchar_t* unicodeStr;
		char* ansiStr;
		int unicodeLength;

		unicodeLength = MultiByteToWideChar(CP_UTF8, 0, utf8String, strlen(utf8String) + 1, NULL, NULL);
		unicodeStr = SysAllocStringLen(NULL, unicodeLength);

		MultiByteToWideChar(CP_UTF8, 0, utf8String, strlen(utf8String) + 1, unicodeStr, unicodeLength);

		unicodeLength = WideCharToMultiByte(CP_ACP, 0, unicodeStr, -1, NULL, 0, NULL, NULL);
		ansiStr = new char[unicodeLength];

		WideCharToMultiByte(CP_ACP, 0, unicodeStr, -1, ansiStr, unicodeLength, NULL, NULL);
		SysFreeString(unicodeStr);

		ZeroMemory(_xmlRetBuff, sizeof(_xmlRetBuff));
		sprintf_s(_xmlRetBuff, 1024, "%s", ansiStr);
		delete[] ansiStr;

		return _xmlRetBuff;
	}
	bool IsUTF8Encode(char* data)
	{
		if (NULL == data)
		{
			return FALSE;
		}

		unsigned char* bytes = (unsigned char*)data;
		while (*bytes)
		{
			if ((//ASCII
				bytes[0] == 0x09 ||
				bytes[0] == 0x0A ||
				bytes[0] == 0x0D ||
				(0x20 <= bytes[0] && bytes[0] <= 0x7E)
				)
				)
			{
				bytes += 1;
				continue;
			}
			if ((// non-overlong 2-byte
				(0xC2 <= bytes[0] && bytes[0] <= 0xDF) &&
				(0x80 <= bytes[1] && bytes[1] <= 0xBF)
				)
				)
			{
				bytes += 2;
				continue;
			}
			if ((// excluding overlongs
				bytes[0] == 0xE0 &&
				(0xA0 <= bytes[1] && bytes[1] <= 0xBF) &&
				(0x80 <= bytes[2] && bytes[2] <= 0xBF)
				) ||
				(// straight 3-byte
					((0xE1 <= bytes[0] && bytes[0] <= 0xEC) ||
						bytes[0] == 0xEE ||
						bytes[0] == 0xEF) &&
					(0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
					(0x80 <= bytes[2] && bytes[2] <= 0xBF)
					) ||
				(// excluding surrogates
					bytes[0] == 0xED &&
					(0x80 <= bytes[1] && bytes[1] <= 0x9F) &&
					(0x80 <= bytes[2] && bytes[2] <= 0xBF)
					)
				)
			{
				bytes += 3;
				continue;
			}
			if ((// planes 1-3
				bytes[0] == 0xF0 &&
				(0x90 <= bytes[1] && bytes[1] <= 0xBF) &&
				(0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
				(0x80 <= bytes[3] && bytes[3] <= 0xBF)
				) ||
				(// planes 4-15
					(0xF1 <= bytes[0] && bytes[0] <= 0xF3) &&
					(0x80 <= bytes[1] && bytes[1] <= 0xBF) &&
					(0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
					(0x80 <= bytes[3] && bytes[3] <= 0xBF)
					) ||
				(// planes 16
					bytes[0] == 0xF4 &&
					(0x80 <= bytes[1] && bytes[1] <= 0x8F) &&
					(0x80 <= bytes[2] && bytes[2] <= 0xBF) &&
					(0x80 <= bytes[3] && bytes[3] <= 0xBF)
					)

				) {
				bytes += 4;
				continue;
			}
			return false;
		}
		return true;
	}
public:
	char* GetString(char* defValue, char* filePath)
	{
		if (_access_s(filePath, 0) != 0)
		{
			return defValue;
		}

		int index = 0;
		TiXmlDocument doc;
		doc.LoadFile(filePath);
		TiXmlElement* root = doc.FirstChildElement(GetTokenChar(index++));
		if (root == NULL)
		{
			return defValue;
		}

		TiXmlElement* Sub = root->FirstChildElement(GetTokenChar(index++));
		if (Sub == NULL)
		{
			return defValue;
		}

		for (int i = index; i < GetSplitSize(); i++)
		{
			Sub = Sub->FirstChildElement(GetTokenChar(i));
			if (Sub == NULL)
			{
				return defValue;
			}
		}

		TiXmlHandle handle = TiXmlHandle(Sub);
		TiXmlElement* temp = handle.Element();
		if (temp == NULL)
		{
			return defValue;
		}

		sprintf_s(_retParser, 1024, "%s", temp->GetText());

		bool isOrgUTF8 = IsUTF8Encode(defValue);
		bool isRetUTF8 = IsUTF8Encode(_retParser);
		if (isOrgUTF8 == true && isRetUTF8 == false)
		{
			sprintf_s(_retParser, 1024, "%s", UTF8toANSI(_retParser));
		}
		else if (isOrgUTF8 == false && isRetUTF8 == true)
		{
			sprintf_s(_retParser, 1024, "%s", ANSIToUTF8(_retParser));
		}

		return _retParser;
	}
	int GetInt(int defValue, char* filePath)
	{
		if (_access_s(filePath, 0) != 0)
		{
			return defValue;
		}

		char defTemp[1024] = { 0, };
		sprintf_s(defTemp, 1024, "%d", defValue);
		return atoi(GetString(defTemp, filePath));
	}
	double GetDouble(double defValue, char* filePath)
	{
		if (_access_s(filePath, 0) != 0)
		{
			return defValue;
		}

		char defTemp[1024] = { 0, };
		sprintf_s(defTemp, 1024, "%.4f", defValue);
		return atof(GetString(defTemp, filePath));
	}

	void SetString(char* value, char* filePath)
	{
		if (_access_s(filePath, 0) != 0)
		{
			return;
		}

		int index = 0;
		TiXmlDocument doc;
		doc.LoadFile("C:\\Temp\\Temp.xml");
		TiXmlElement* root = doc.FirstChildElement(GetTokenChar(index++));
		if (root == NULL)
		{
			return;
		}

		TiXmlElement* Sub = root->FirstChildElement(GetTokenChar(index++));
		if (Sub == NULL)
		{
			return;
		}
		for (int i = index; i < GetSplitSize(); i++)
		{
			Sub = Sub->FirstChildElement(GetTokenChar(i));
			if (Sub == NULL)
			{
				return;
			}
		}

		char ret[1024] = { 0, };
		bool isOrgUTF8 = IsUTF8Encode((char*)Sub->GetText());
		bool isRetUTF8 = IsUTF8Encode(value);
		if (isOrgUTF8 == true && isRetUTF8 == false)
		{
			sprintf_s(ret, 1024, "%s", ANSIToUTF8(value));
		}
		else if (isOrgUTF8 == false && isRetUTF8 == true)
		{
			sprintf_s(ret, 1024, "%s", UTF8toANSI(value));
		}
		else
		{
			sprintf_s(ret, 1024, "%s", value);
		}

		Sub->FirstChild()->SetValue(ret);
		doc.SaveFile();
	}
	void SetInt(int value, char* filePath)
	{
		if (_access_s(filePath, 0) != 0)
		{
			return;
		}

		char temp[1024] = { 0, };
		sprintf_s(temp, 1024, "%d", value);
		SetString(temp, filePath);
	}
	void SetDouble(double value, char* filePath)
	{
		if (_access_s(filePath, 0) != 0)
		{
			return;
		}

		char temp[1024] = { 0, };
		sprintf_s(temp, 1024, "%.4f", value);
		SetString(temp, filePath);
	}

#pragma region still xml
	class TiXmlDocument;
	class TiXmlElement;
	class TiXmlComment;
	class TiXmlUnknown;
	class TiXmlAttribute;
	class TiXmlText;
	class TiXmlDeclaration;
	class TiXmlBase;
	class TiXmlString
	{
	public:
		// The size type used
		typedef size_t size_type;

		// Error value for find primitive
		static const size_type npos; // = -1;


		// TiXmlString empty constructor
		TiXmlString() : rep_(&nullrep_)
		{
		}

		// TiXmlString copy constructor
		TiXmlString(const TiXmlString& copy) : rep_(0)
		{
			init(copy.length());
			memcpy(start(), copy.data(), length());
		}

		// TiXmlString constructor, based on a string
		explicit TiXmlString(const char* copy) : rep_(0)
		{
			init(static_cast<size_type>(strlen(copy)));
			memcpy(start(), copy, length());
		}

		// TiXmlString constructor, based on a string
		explicit TiXmlString(const char* str, size_type len) : rep_(0)
		{
			init(len);
			memcpy(start(), str, len);
		}

		// TiXmlString destructor
		~TiXmlString()
		{
			quit();
		}

		TiXmlString& operator = (const char* copy)
		{
			return assign(copy, (size_type)strlen(copy));
		}

		TiXmlString& operator = (const TiXmlString& copy)
		{
			return assign(copy.start(), copy.length());
		}

		// += operator. Maps to append
		TiXmlString& operator += (const char* suffix)
		{
			return append(suffix, static_cast<size_type>(strlen(suffix)));
		}

		// += operator. Maps to append
		TiXmlString& operator += (char single)
		{
			return append(&single, 1);
		}

		// += operator. Maps to append
		TiXmlString& operator += (const TiXmlString& suffix)
		{
			return append(suffix.data(), suffix.length());
		}


		// Convert a TiXmlString into a null-terminated char *
		const char* c_str() const { return rep_->str; }

		// Convert a TiXmlString into a char * (need not be null terminated).
		const char* data() const { return rep_->str; }

		// Return the length of a TiXmlString
		size_type length() const { return rep_->size; }

		// Alias for length()
		size_type size() const { return rep_->size; }

		// Checks if a TiXmlString is empty
		bool empty() const { return rep_->size == 0; }

		// Return capacity of string
		size_type capacity() const { return rep_->capacity; }


		// single char extraction
		const char& at(size_type index) const
		{
			assert(index < length());
			return rep_->str[index];
		}

		// [] operator
		char& operator [] (size_type index) const
		{
			assert(index < length());
			return rep_->str[index];
		}

		// find a char in a string. Return TiXmlString::npos if not found
		size_type find(char lookup) const
		{
			return find(lookup, 0);
		}

		// find a char in a string from an offset. Return TiXmlString::npos if not found
		size_type find(char tofind, size_type offset) const
		{
			if (offset >= length()) return npos;

			for (const char* p = c_str() + offset; *p != '\0'; ++p)
			{
				if (*p == tofind) return static_cast<size_type>(p - c_str());
			}
			return npos;
		}

		void clear()
		{
			//Lee:
			//The original was just too strange, though correct:
			//	TiXmlString().swap(*this);
			//Instead use the quit & re-init:
			quit();
			init(0, 0);
		}

		/*	Function to reserve a big amount of data when we know we'll need it. Be aware that this
			function DOES NOT clear the content of the TiXmlString if any exists.
		*/
		void reserve(size_type cap);

		TiXmlString& assign(const char* str, size_type len);

		TiXmlString& append(const char* str, size_type len);

		void swap(TiXmlString& other)
		{
			Rep* r = rep_;
			rep_ = other.rep_;
			other.rep_ = r;
		}

	private:

		void init(size_type sz) { init(sz, sz); }
		void set_size(size_type sz) { rep_->str[rep_->size = sz] = '\0'; }
		char* start() const { return rep_->str; }
		char* finish() const { return rep_->str + rep_->size; }

		struct Rep
		{
			size_type size, capacity;
			char str[1];
		};

		void init(size_type sz, size_type cap)
		{
			if (cap)
			{
				// Lee: the original form:
				//	rep_ = static_cast<Rep*>(operator new(sizeof(Rep) + cap));
				// doesn't work in some cases of new being overloaded. Switching
				// to the normal allocation, although use an 'int' for systems
				// that are overly picky about structure alignment.
				const size_type bytesNeeded = sizeof(Rep) + cap;
				const size_type intsNeeded = (bytesNeeded + sizeof(int) - 1) / sizeof(int);
				rep_ = reinterpret_cast<Rep*>(new int[intsNeeded]);

				rep_->str[rep_->size = sz] = '\0';
				rep_->capacity = cap;
			}
			else
			{
				rep_ = &nullrep_;
			}
		}

		void quit()
		{
			if (rep_ != &nullrep_)
			{
				// The rep_ is really an array of ints. (see the allocator, above).
				// Cast it back before delete, so the compiler won't incorrectly call destructors.
				delete[](reinterpret_cast<int*>(rep_));
			}
		}

		Rep* rep_;
		static Rep nullrep_;

	};
	class TiXmlOutStream : public TiXmlString
	{
	public:

		// TiXmlOutStream << operator.
		TiXmlOutStream& operator << (const TiXmlString& in)
		{
			*this += in;
			return *this;
		}

		// TiXmlOutStream << operator.
		TiXmlOutStream& operator << (const char* in)
		{
			*this += in;
			return *this;
		}

	};
	class TiXmlParsingData;
	struct TiXmlCursor
	{
		TiXmlCursor() { Clear(); }
		void Clear() { row = col = -1; }

		int row;	// 0 based.
		int col;	// 0 based.
	};
	enum
	{
		TIXML_SUCCESS,
		TIXML_NO_ATTRIBUTE,
		TIXML_WRONG_TYPE
	};
	enum TiXmlEncoding
	{
		TIXML_ENCODING_UNKNOWN,
		TIXML_ENCODING_UTF8,
		TIXML_ENCODING_LEGACY
	};

	class TiXmlVisitor
	{
	public:
		virtual ~TiXmlVisitor() {}

		/// Visit a document.
		virtual bool VisitEnter(const TiXmlDocument& /*doc*/) { return true; }
		/// Visit a document.
		virtual bool VisitExit(const TiXmlDocument& /*doc*/) { return true; }

		/// Visit an element.
		virtual bool VisitEnter(const TiXmlElement& /*element*/, const TiXmlAttribute* /*firstAttribute*/) { return true; }
		/// Visit an element.
		virtual bool VisitExit(const TiXmlElement& /*element*/) { return true; }

		/// Visit a declaration
		virtual bool Visit(const TiXmlDeclaration& /*declaration*/) { return true; }
		/// Visit a text node
		virtual bool Visit(const TiXmlText& /*text*/) { return true; }
		/// Visit a comment node
		virtual bool Visit(const TiXmlComment& /*comment*/) { return true; }
		/// Visit an unknown node
		virtual bool Visit(const TiXmlUnknown& /*unknown*/) { return true; }
	};
	class TiXmlBase
	{
		friend class TiXmlNode;
		friend class TiXmlElement;
		friend class TiXmlDocument;

	public:
		TiXmlBase() : userData(0) {}
		virtual ~TiXmlBase() {}

		/**	All TinyXml classes can print themselves to a filestream
			or the string class (TiXmlString in non-STL mode, std::string
			in STL mode.) Either or both cfile and str can be null.

			This is a formatted print, and will insert
			tabs and newlines.

			(For an unformatted stream, use the << operator.)
		*/
		virtual void Print(FILE* cfile, int depth) const = 0;

		/**	The world does not agree on whether white space should be kept or
			not. In order to make everyone happy, these global, static functions
			are provided to set whether or not TinyXml will condense all white space
			into a single space or not. The default is to condense. Note changing this
			value is not thread safe.
		*/
		static void SetCondenseWhiteSpace(bool condense) { condenseWhiteSpace = condense; }

		/// Return the current white space setting.
		static bool IsWhiteSpaceCondensed() { return condenseWhiteSpace; }

		/** Return the position, in the original source file, of this node or attribute.
			The row and column are 1-based. (That is the first row and first column is
			1,1). If the returns values are 0 or less, then the parser does not have
			a row and column value.

			Generally, the row and column value will be set when the TiXmlDocument::Load(),
			TiXmlDocument::LoadFile(), or any TiXmlNode::Parse() is called. It will NOT be set
			when the DOM was created from operator>>.

			The values reflect the initial load. Once the DOM is modified programmatically
			(by adding or changing nodes and attributes) the new values will NOT update to
			reflect changes in the document.

			There is a minor performance cost to computing the row and column. Computation
			can be disabled if TiXmlDocument::SetTabSize() is called with 0 as the value.

			@sa TiXmlDocument::SetTabSize()
		*/
		int Row() const { return location.row + 1; }
		int Column() const { return location.col + 1; }	///< See Row()

		void  SetUserData(void* user) { userData = user; }	///< Set a pointer to arbitrary user data.
		void* GetUserData() { return userData; }	///< Get a pointer to arbitrary user data.
		const void* GetUserData() const { return userData; }	///< Get a pointer to arbitrary user data.

		// Table that returs, for a given lead byte, the total number of bytes
		// in the UTF-8 sequence.
		static const int utf8ByteTable[256];

		virtual const char* Parse(const char* p,
			TiXmlParsingData* data,
			TiXmlEncoding encoding /*= TIXML_ENCODING_UNKNOWN */) = 0;

		/** Expands entities in a string. Note this should not contian the tag's '<', '>', etc,
			or they will be transformed into entities!
		*/
		static void EncodeString(const TiXmlString& str, TiXmlString* out);

		enum
		{
			TIXML_NO_ERROR = 0,
			TIXML_ERROR,
			TIXML_ERROR_OPENING_FILE,
			TIXML_ERROR_PARSING_ELEMENT,
			TIXML_ERROR_FAILED_TO_READ_ELEMENT_NAME,
			TIXML_ERROR_READING_ELEMENT_VALUE,
			TIXML_ERROR_READING_ATTRIBUTES,
			TIXML_ERROR_PARSING_EMPTY,
			TIXML_ERROR_READING_END_TAG,
			TIXML_ERROR_PARSING_UNKNOWN,
			TIXML_ERROR_PARSING_COMMENT,
			TIXML_ERROR_PARSING_DECLARATION,
			TIXML_ERROR_DOCUMENT_EMPTY,
			TIXML_ERROR_EMBEDDED_NULL,
			TIXML_ERROR_PARSING_CDATA,
			TIXML_ERROR_DOCUMENT_TOP_ONLY,

			TIXML_ERROR_STRING_COUNT
		};

	protected:

		static const char* SkipWhiteSpace(const char*, TiXmlEncoding encoding);

		inline static bool IsWhiteSpace(char c)
		{
			return (isspace((unsigned char)c) || c == '\n' || c == '\r');
		}
		inline static bool IsWhiteSpace(int c)
		{
			if (c < 256)
				return IsWhiteSpace((char)c);
			return false;	// Again, only truly correct for English/Latin...but usually works.
		}

#ifdef TIXML_USE_STL
		static bool	StreamWhiteSpace(std::istream* in, TiXmlString* tag);
		static bool StreamTo(std::istream* in, int character, TiXmlString* tag);
#endif

		/*	Reads an XML name into the string provided. Returns
			a pointer just past the last character of the name,
			or 0 if the function has an error.
		*/
		static const char* ReadName(const char* p, TiXmlString* name, TiXmlEncoding encoding);

		/*	Reads text. Returns a pointer past the given end tag.
			Wickedly complex options, but it keeps the (sensitive) code in one place.
		*/
		static const char* ReadText(const char* in,				// where to start
			TiXmlString* text,			// the string read
			bool ignoreWhiteSpace,		// whether to keep the white space
			const char* endTag,			// what ends this text
			bool ignoreCase,			// whether to ignore case in the end tag
			TiXmlEncoding encoding);	// the current encoding

// If an entity has been found, transform it into a character.
		static const char* GetEntity(const char* in, char* value, int* length, TiXmlEncoding encoding);

		// Get a character, while interpreting entities.
		// The length can be from 0 to 4 bytes.
		inline static const char* GetChar(const char* p, char* _value, int* length, TiXmlEncoding encoding)
		{
			assert(p);
			if (encoding == TIXML_ENCODING_UTF8)
			{
				*length = utf8ByteTable[*((const unsigned char*)p)];
				assert(*length >= 0 && *length < 5);
			}
			else
			{
				*length = 1;
			}

			if (*length == 1)
			{
				if (*p == '&')
					return GetEntity(p, _value, length, encoding);
				*_value = *p;
				return p + 1;
			}
			else if (*length)
			{
				//strncpy( _value, p, *length );	// lots of compilers don't like this function (unsafe),
													// and the null terminator isn't needed
				for (int i = 0; p[i] && i < *length; ++i) {
					_value[i] = p[i];
				}
				return p + (*length);
			}
			else
			{
				// Not valid text.
				return 0;
			}
		}

		// Return true if the next characters in the stream are any of the endTag sequences.
		// Ignore case only works for english, and should only be relied on when comparing
		// to English words: StringEqual( p, "version", true ) is fine.
		static bool StringEqual(const char* p,
			const char* endTag,
			bool ignoreCase,
			TiXmlEncoding encoding);

		static const char* errorString[TIXML_ERROR_STRING_COUNT];

		TiXmlCursor location;

		/// Field containing a generic user pointer
		void* userData;

		// None of these methods are reliable for any language except English.
		// Good for approximation, not great for accuracy.
		static int IsAlpha(unsigned char anyByte, TiXmlEncoding encoding);
		static int IsAlphaNum(unsigned char anyByte, TiXmlEncoding encoding);
		inline static int ToLower(int v, TiXmlEncoding encoding)
		{
			if (encoding == TIXML_ENCODING_UTF8)
			{
				if (v < 128) return tolower(v);
				return v;
			}
			else
			{
				return tolower(v);
			}
		}
		static void ConvertUTF32ToUTF8(unsigned long input, char* output, int* length);

	private:
		TiXmlBase(const TiXmlBase&);				// not implemented.
		void operator=(const TiXmlBase& base);	// not allowed.

		struct Entity
		{
			const char* str;
			unsigned int	strLength;
			char		    chr;
		};
		enum
		{
			NUM_ENTITY = 5,
			MAX_ENTITY_LENGTH = 6

		};
		static Entity entity[NUM_ENTITY];
		static bool condenseWhiteSpace;
	};
	class TiXmlNode : public TiXmlBase
	{
		friend class TiXmlDocument;
		friend class TiXmlElement;

	public:
#ifdef TIXML_USE_STL	

		/** An input stream operator, for every class. Tolerant of newlines and
			formatting, but doesn't expect them.
		*/
		friend std::istream& operator >> (std::istream& in, TiXmlNode& base);

		/** An output stream operator, for every class. Note that this outputs
			without any newlines or formatting, as opposed to Print(), which
			includes tabs and new lines.

			The operator<< and operator>> are not completely symmetric. Writing
			a node to a stream is very well defined. You'll get a nice stream
			of output, without any extra whitespace or newlines.

			But reading is not as well defined. (As it always is.) If you create
			a TiXmlElement (for example) and read that from an input stream,
			the text needs to define an element or junk will result. This is
			true of all input streams, but it's worth keeping in mind.

			A TiXmlDocument will read nodes until it reads a root element, and
			all the children of that root element.
		*/
		friend std::ostream& operator<< (std::ostream& out, const TiXmlNode& base);

		/// Appends the XML node or attribute to a std::string.
		friend std::string& operator<< (std::string& out, const TiXmlNode& base);

#endif

		/** The types of XML nodes supported by TinyXml. (All the
				unsupported types are picked up by UNKNOWN.)
		*/
		enum NodeType
		{
			TINYXML_DOCUMENT,
			TINYXML_ELEMENT,
			TINYXML_COMMENT,
			TINYXML_UNKNOWN,
			TINYXML_TEXT,
			TINYXML_DECLARATION,
			TINYXML_TYPECOUNT
		};

		virtual ~TiXmlNode();

		/** The meaning of 'value' changes for the specific type of
			TiXmlNode.
			@verbatim
			Document:	filename of the xml file
			Element:	name of the element
			Comment:	the comment text
			Unknown:	the tag contents
			Text:		the text string
			@endverbatim

			The subclasses will wrap this function.
		*/
		const char* Value() const { return value.c_str(); }

#ifdef TIXML_USE_STL
		/** Return Value() as a std::string. If you only use STL,
			this is more efficient than calling Value().
			Only available in STL mode.
		*/
		const std::string& ValueStr() const { return value; }
#endif

		const TiXmlString& ValueTStr() const { return value; }

		/** Changes the value of the node. Defined as:
			@verbatim
			Document:	filename of the xml file
			Element:	name of the element
			Comment:	the comment text
			Unknown:	the tag contents
			Text:		the text string
			@endverbatim
		*/
		void SetValue(const char* _value) { value = _value; }

#ifdef TIXML_USE_STL
		/// STL std::string form.
		void SetValue(const std::string& _value) { value = _value; }
#endif

		/// Delete all the children of this node. Does not affect 'this'.
		void Clear();

		/// One step up the DOM.
		TiXmlNode* Parent() { return parent; }
		const TiXmlNode* Parent() const { return parent; }

		const TiXmlNode* FirstChild()	const { return firstChild; }	///< The first child of this node. Will be null if there are no children.
		TiXmlNode* FirstChild() { return firstChild; }
		const TiXmlNode* FirstChild(const char* value) const;			///< The first child of this node with the matching 'value'. Will be null if none found.
		/// The first child of this node with the matching 'value'. Will be null if none found.
		TiXmlNode* FirstChild(const char* _value) {
			// Call through to the const version - safe since nothing is changed. Exiting syntax: cast this to a const (always safe)
			// call the method, cast the return back to non-const.
			return const_cast<TiXmlNode*> ((const_cast<const TiXmlNode*>(this))->FirstChild(_value));
		}
		const TiXmlNode* LastChild() const { return lastChild; }		/// The last child of this node. Will be null if there are no children.
		TiXmlNode* LastChild() { return lastChild; }

		const TiXmlNode* LastChild(const char* value) const;			/// The last child of this node matching 'value'. Will be null if there are no children.
		TiXmlNode* LastChild(const char* _value) {
			return const_cast<TiXmlNode*> ((const_cast<const TiXmlNode*>(this))->LastChild(_value));
		}

#ifdef TIXML_USE_STL
		const TiXmlNode* FirstChild(const std::string& _value) const { return FirstChild(_value.c_str()); }	///< STL std::string form.
		TiXmlNode* FirstChild(const std::string& _value) { return FirstChild(_value.c_str()); }	///< STL std::string form.
		const TiXmlNode* LastChild(const std::string& _value) const { return LastChild(_value.c_str()); }	///< STL std::string form.
		TiXmlNode* LastChild(const std::string& _value) { return LastChild(_value.c_str()); }	///< STL std::string form.
#endif

/** An alternate way to walk the children of a node.
	One way to iterate over nodes is:
	@verbatim
		for( child = parent->FirstChild(); child; child = child->NextSibling() )
	@endverbatim

	IterateChildren does the same thing with the syntax:
	@verbatim
		child = 0;
		while( child = parent->IterateChildren( child ) )
	@endverbatim

	IterateChildren takes the previous child as input and finds
	the next one. If the previous child is null, it returns the
	first. IterateChildren will return null when done.
*/
		const TiXmlNode* IterateChildren(const TiXmlNode* previous) const;
		TiXmlNode* IterateChildren(const TiXmlNode* previous) {
			return const_cast<TiXmlNode*>((const_cast<const TiXmlNode*>(this))->IterateChildren(previous));
		}

		/// This flavor of IterateChildren searches for children with a particular 'value'
		const TiXmlNode* IterateChildren(const char* value, const TiXmlNode* previous) const;
		TiXmlNode* IterateChildren(const char* _value, const TiXmlNode* previous) {
			return const_cast<TiXmlNode*>((const_cast<const TiXmlNode*>(this))->IterateChildren(_value, previous));
		}

#ifdef TIXML_USE_STL
		const TiXmlNode* IterateChildren(const std::string& _value, const TiXmlNode* previous) const { return IterateChildren(_value.c_str(), previous); }	///< STL std::string form.
		TiXmlNode* IterateChildren(const std::string& _value, const TiXmlNode* previous) { return IterateChildren(_value.c_str(), previous); }	///< STL std::string form.
#endif

/** Add a new node related to this. Adds a child past the LastChild.
	Returns a pointer to the new object or NULL if an error occured.
*/
		TiXmlNode* InsertEndChild(const TiXmlNode& addThis);


		/** Add a new node related to this. Adds a child past the LastChild.

			NOTE: the node to be added is passed by pointer, and will be
			henceforth owned (and deleted) by tinyXml. This method is efficient
			and avoids an extra copy, but should be used with care as it
			uses a different memory model than the other insert functions.

			@sa InsertEndChild
		*/
		TiXmlNode* LinkEndChild(TiXmlNode* addThis);

		/** Add a new node related to this. Adds a child before the specified child.
			Returns a pointer to the new object or NULL if an error occured.
		*/
		TiXmlNode* InsertBeforeChild(TiXmlNode* beforeThis, const TiXmlNode& addThis);

		/** Add a new node related to this. Adds a child after the specified child.
			Returns a pointer to the new object or NULL if an error occured.
		*/
		TiXmlNode* InsertAfterChild(TiXmlNode* afterThis, const TiXmlNode& addThis);

		/** Replace a child of this node.
			Returns a pointer to the new object or NULL if an error occured.
		*/
		TiXmlNode* ReplaceChild(TiXmlNode* replaceThis, const TiXmlNode& withThis);

		/// Delete a child of this node.
		bool RemoveChild(TiXmlNode* removeThis);

		/// Navigate to a sibling node.
		const TiXmlNode* PreviousSibling() const { return prev; }
		TiXmlNode* PreviousSibling() { return prev; }

		/// Navigate to a sibling node.
		const TiXmlNode* PreviousSibling(const char*) const;
		TiXmlNode* PreviousSibling(const char* _prev) {
			return const_cast<TiXmlNode*>((const_cast<const TiXmlNode*>(this))->PreviousSibling(_prev));
		}

#ifdef TIXML_USE_STL
		const TiXmlNode* PreviousSibling(const std::string& _value) const { return PreviousSibling(_value.c_str()); }	///< STL std::string form.
		TiXmlNode* PreviousSibling(const std::string& _value) { return PreviousSibling(_value.c_str()); }	///< STL std::string form.
		const TiXmlNode* NextSibling(const std::string& _value) const { return NextSibling(_value.c_str()); }	///< STL std::string form.
		TiXmlNode* NextSibling(const std::string& _value) { return NextSibling(_value.c_str()); }	///< STL std::string form.
#endif

/// Navigate to a sibling node.
		const TiXmlNode* NextSibling() const { return next; }
		TiXmlNode* NextSibling() { return next; }

		/// Navigate to a sibling node with the given 'value'.
		const TiXmlNode* NextSibling(const char*) const;
		TiXmlNode* NextSibling(const char* _next) {
			return const_cast<TiXmlNode*>((const_cast<const TiXmlNode*>(this))->NextSibling(_next));
		}

		/** Convenience function to get through elements.
			Calls NextSibling and ToElement. Will skip all non-Element
			nodes. Returns 0 if there is not another element.
		*/
		const TiXmlElement* NextSiblingElement() const;
		TiXmlElement* NextSiblingElement() {
			return const_cast<TiXmlElement*>((const_cast<const TiXmlNode*>(this))->NextSiblingElement());
		}

		/** Convenience function to get through elements.
			Calls NextSibling and ToElement. Will skip all non-Element
			nodes. Returns 0 if there is not another element.
		*/
		const TiXmlElement* NextSiblingElement(const char*) const;
		TiXmlElement* NextSiblingElement(const char* _next) {
			return const_cast<TiXmlElement*>((const_cast<const TiXmlNode*>(this))->NextSiblingElement(_next));
		}

#ifdef TIXML_USE_STL
		const TiXmlElement* NextSiblingElement(const std::string& _value) const { return NextSiblingElement(_value.c_str()); }	///< STL std::string form.
		TiXmlElement* NextSiblingElement(const std::string& _value) { return NextSiblingElement(_value.c_str()); }	///< STL std::string form.
#endif

/// Convenience function to get through elements.
		const TiXmlElement* FirstChildElement()	const;
		TiXmlElement* FirstChildElement() {
			return const_cast<TiXmlElement*>((const_cast<const TiXmlNode*>(this))->FirstChildElement());
		}

		/// Convenience function to get through elements.
		const TiXmlElement* FirstChildElement(const char* _value) const;
		TiXmlElement* FirstChildElement(const char* _value) {
			return const_cast<TiXmlElement*>((const_cast<const TiXmlNode*>(this))->FirstChildElement(_value));
		}

#ifdef TIXML_USE_STL
		const TiXmlElement* FirstChildElement(const std::string& _value) const { return FirstChildElement(_value.c_str()); }	///< STL std::string form.
		TiXmlElement* FirstChildElement(const std::string& _value) { return FirstChildElement(_value.c_str()); }	///< STL std::string form.
#endif

/** Query the type (as an enumerated value, above) of this node.
	The possible types are: TINYXML_DOCUMENT, TINYXML_ELEMENT, TINYXML_COMMENT,
							TINYXML_UNKNOWN, TINYXML_TEXT, and TINYXML_DECLARATION.
*/
		int Type() const { return type; }

		/** Return a pointer to the Document this node lives in.
			Returns null if not in a document.
		*/
		const TiXmlDocument* GetDocument() const;
		TiXmlDocument* GetDocument() {
			return const_cast<TiXmlDocument*>((const_cast<const TiXmlNode*>(this))->GetDocument());
		}

		/// Returns true if this node has no children.
		bool NoChildren() const { return !firstChild; }

		virtual const TiXmlDocument* ToDocument()    const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
		virtual const TiXmlElement* ToElement()     const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
		virtual const TiXmlComment* ToComment()     const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
		virtual const TiXmlUnknown* ToUnknown()     const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
		virtual const TiXmlText* ToText()        const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
		virtual const TiXmlDeclaration* ToDeclaration() const { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.

		virtual TiXmlDocument* ToDocument() { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
		virtual TiXmlElement* ToElement() { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
		virtual TiXmlComment* ToComment() { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
		virtual TiXmlUnknown* ToUnknown() { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
		virtual TiXmlText* ToText() { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.
		virtual TiXmlDeclaration* ToDeclaration() { return 0; } ///< Cast to a more defined type. Will return null if not of the requested type.

		/** Create an exact duplicate of this node and return it. The memory must be deleted
			by the caller.
		*/
		virtual TiXmlNode* Clone() const = 0;

		/** Accept a hierchical visit the nodes in the TinyXML DOM. Every node in the
			XML tree will be conditionally visited and the host will be called back
			via the TiXmlVisitor interface.

			This is essentially a SAX interface for TinyXML. (Note however it doesn't re-parse
			the XML for the callbacks, so the performance of TinyXML is unchanged by using this
			interface versus any other.)

			The interface has been based on ideas from:

			- http://www.saxproject.org/
			- http://c2.com/cgi/wiki?HierarchicalVisitorPattern

			Which are both good references for "visiting".

			An example of using Accept():
			@verbatim
			TiXmlPrinter printer;
			tinyxmlDoc.Accept( &printer );
			const char* xmlcstr = printer.CStr();
			@endverbatim
		*/
		virtual bool Accept(TiXmlVisitor* visitor) const = 0;

	protected:
		TiXmlNode(NodeType _type);

		// Copy to the allocated object. Shared functionality between Clone, Copy constructor,
		// and the assignment operator.
		void CopyTo(TiXmlNode* target) const;

#ifdef TIXML_USE_STL
		// The real work of the input operator.
		virtual void StreamIn(std::istream* in, TiXmlString* tag) = 0;
#endif

		// Figure out what is at *p, and parse it. Returns null if it is not an xml node.
		TiXmlNode* Identify(const char* start, TiXmlEncoding encoding);

		TiXmlNode* parent;
		NodeType		type;

		TiXmlNode* firstChild;
		TiXmlNode* lastChild;

		TiXmlString	value;

		TiXmlNode* prev;
		TiXmlNode* next;

	private:
		TiXmlNode(const TiXmlNode&);				// not implemented.
		void operator=(const TiXmlNode& base);	// not allowed.
	};
	class TiXmlAttribute : public TiXmlBase
	{
		friend class TiXmlAttributeSet;

	public:
		/// Construct an empty attribute.
		TiXmlAttribute() : TiXmlBase()
		{
			document = 0;
			prev = next = 0;
		}

#ifdef TIXML_USE_STL
		/// std::string constructor.
		TiXmlAttribute(const std::string& _name, const std::string& _value)
		{
			name = _name;
			value = _value;
			document = 0;
			prev = next = 0;
		}
#endif

		/// Construct an attribute with a name and value.
		TiXmlAttribute(const char* _name, const char* _value)
		{
			name = _name;
			value = _value;
			document = 0;
			prev = next = 0;
		}

		const char* Name()  const { return name.c_str(); }		///< Return the name of this attribute.
		const char* Value() const { return value.c_str(); }		///< Return the value of this attribute.
#ifdef TIXML_USE_STL
		const std::string& ValueStr() const { return value; }				///< Return the value of this attribute.
#endif
		int				IntValue() const;									///< Return the value of this attribute, converted to an integer.
		double			DoubleValue() const;								///< Return the value of this attribute, converted to a double.

		// Get the tinyxml string representation
		const TiXmlString& NameTStr() const { return name; }

		/** QueryIntValue examines the value string. It is an alternative to the
			IntValue() method with richer error checking.
			If the value is an integer, it is stored in 'value' and
			the call returns TIXML_SUCCESS. If it is not
			an integer, it returns TIXML_WRONG_TYPE.

			A specialized but useful call. Note that for success it returns 0,
			which is the opposite of almost all other TinyXml calls.
		*/
		int QueryIntValue(int* _value) const;
		/// QueryDoubleValue examines the value string. See QueryIntValue().
		int QueryDoubleValue(double* _value) const;

		void SetName(const char* _name) { name = _name; }				///< Set the name of this attribute.
		void SetValue(const char* _value) { value = _value; }				///< Set the value.

		void SetIntValue(int _value);										///< Set the value from an integer.
		void SetDoubleValue(double _value);								///< Set the value from a double.

#ifdef TIXML_USE_STL
/// STL std::string form.
		void SetName(const std::string& _name) { name = _name; }
		/// STL std::string form.	
		void SetValue(const std::string& _value) { value = _value; }
#endif

		/// Get the next sibling attribute in the DOM. Returns null at end.
		const TiXmlAttribute* Next() const;
		TiXmlAttribute* Next() {
			return const_cast<TiXmlAttribute*>((const_cast<const TiXmlAttribute*>(this))->Next());
		}

		/// Get the previous sibling attribute in the DOM. Returns null at beginning.
		const TiXmlAttribute* Previous() const;
		TiXmlAttribute* Previous() {
			return const_cast<TiXmlAttribute*>((const_cast<const TiXmlAttribute*>(this))->Previous());
		}

		bool operator==(const TiXmlAttribute& rhs) const { return rhs.name.c_str() == name.c_str(); }
		bool operator<(const TiXmlAttribute& rhs)	 const { return name.c_str() < rhs.name.c_str(); }
		bool operator>(const TiXmlAttribute& rhs)  const { return name.c_str() > rhs.name.c_str(); }

		/*	Attribute parsing starts: first letter of the name
							 returns: the next char after the value end quote
		*/
		virtual const char* Parse(const char* p, TiXmlParsingData* data, TiXmlEncoding encoding);

		// Prints this Attribute to a FILE stream.
		virtual void Print(FILE* cfile, int depth) const {
			Print(cfile, depth, 0);
		}
		void Print(FILE* cfile, int depth, TiXmlString* str) const;

		// [internal use]
		// Set the document pointer so the attribute can report errors.
		void SetDocument(TiXmlDocument* doc) { document = doc; }

	private:
		TiXmlAttribute(const TiXmlAttribute&);				// not implemented.
		void operator=(const TiXmlAttribute& base);	// not allowed.

	public:
		TiXmlDocument* document;	// A pointer back to a document, for error reporting.
		TiXmlString name;
		TiXmlString value;
		TiXmlAttribute* prev;
		TiXmlAttribute* next;
	};
	class TiXmlAttributeSet
	{
	public:
		TiXmlAttributeSet();
		~TiXmlAttributeSet();

		void Add(TiXmlAttribute* attribute);
		void Remove(TiXmlAttribute* attribute);

		const TiXmlAttribute* First()	const { return (sentinel.next == &sentinel) ? 0 : sentinel.next; }
		TiXmlAttribute* First() { return (sentinel.next == &sentinel) ? 0 : sentinel.next; }
		const TiXmlAttribute* Last() const { return (sentinel.prev == &sentinel) ? 0 : sentinel.prev; }
		TiXmlAttribute* Last() { return (sentinel.prev == &sentinel) ? 0 : sentinel.prev; }

		TiXmlAttribute* Find(const char* _name) const;
		TiXmlAttribute* FindOrCreate(const char* _name);

#	ifdef TIXML_USE_STL
		TiXmlAttribute* Find(const std::string& _name) const;
		TiXmlAttribute* FindOrCreate(const std::string& _name);
#	endif


	private:
		//*ME:	Because of hidden/disabled copy-construktor in TiXmlAttribute (sentinel-element),
		//*ME:	this class must be also use a hidden/disabled copy-constructor !!!
		TiXmlAttributeSet(const TiXmlAttributeSet&);	// not allowed
		void operator=(const TiXmlAttributeSet&);	// not allowed (as TiXmlAttribute)

		TiXmlAttribute sentinel;
	};
	class TiXmlElement : public TiXmlNode
	{
	public:
		/// Construct an element.
		TiXmlElement(const char* in_value);

#ifdef TIXML_USE_STL
		/// std::string constructor.
		TiXmlElement(const std::string& _value);
#endif

		TiXmlElement(const TiXmlElement&);

		TiXmlElement& operator=(const TiXmlElement& base);

		virtual ~TiXmlElement();

		/** Given an attribute name, Attribute() returns the value
			for the attribute of that name, or null if none exists.
		*/
		const char* Attribute(const char* name) const;

		/** Given an attribute name, Attribute() returns the value
			for the attribute of that name, or null if none exists.
			If the attribute exists and can be converted to an integer,
			the integer value will be put in the return 'i', if 'i'
			is non-null.
		*/
		const char* Attribute(const char* name, int* i) const;

		/** Given an attribute name, Attribute() returns the value
			for the attribute of that name, or null if none exists.
			If the attribute exists and can be converted to an double,
			the double value will be put in the return 'd', if 'd'
			is non-null.
		*/
		const char* Attribute(const char* name, double* d) const;

		/** QueryIntAttribute examines the attribute - it is an alternative to the
			Attribute() method with richer error checking.
			If the attribute is an integer, it is stored in 'value' and
			the call returns TIXML_SUCCESS. If it is not
			an integer, it returns TIXML_WRONG_TYPE. If the attribute
			does not exist, then TIXML_NO_ATTRIBUTE is returned.
		*/
		int QueryIntAttribute(const char* name, int* _value) const;
		/// QueryUnsignedAttribute examines the attribute - see QueryIntAttribute().
		int QueryUnsignedAttribute(const char* name, unsigned* _value) const;
		/** QueryBoolAttribute examines the attribute - see QueryIntAttribute().
			Note that '1', 'true', or 'yes' are considered true, while '0', 'false'
			and 'no' are considered false.
		*/
		int QueryBoolAttribute(const char* name, bool* _value) const;
		/// QueryDoubleAttribute examines the attribute - see QueryIntAttribute().
		int QueryDoubleAttribute(const char* name, double* _value) const;
		/// QueryFloatAttribute examines the attribute - see QueryIntAttribute().
		int QueryFloatAttribute(const char* name, float* _value) const {
			double d;
			int result = QueryDoubleAttribute(name, &d);
			if (result == TIXML_SUCCESS) {
				*_value = (float)d;
			}
			return result;
		}

#ifdef TIXML_USE_STL
		/// QueryStringAttribute examines the attribute - see QueryIntAttribute().
		int QueryStringAttribute(const char* name, std::string* _value) const {
			const char* cstr = Attribute(name);
			if (cstr) {
				*_value = std::string(cstr);
				return TIXML_SUCCESS;
			}
			return TIXML_NO_ATTRIBUTE;
		}

		/** Template form of the attribute query which will try to read the
			attribute into the specified type. Very easy, very powerful, but
			be careful to make sure to call this with the correct type.

			NOTE: This method doesn't work correctly for 'string' types that contain spaces.

			@return TIXML_SUCCESS, TIXML_WRONG_TYPE, or TIXML_NO_ATTRIBUTE
		*/
		template< typename T > int QueryValueAttribute(const std::string& name, T* outValue) const
		{
			const TiXmlAttribute* node = attributeSet.Find(name);
			if (!node)
				return TIXML_NO_ATTRIBUTE;

			std::stringstream sstream(node->ValueStr());
			sstream >> *outValue;
			if (!sstream.fail())
				return TIXML_SUCCESS;
			return TIXML_WRONG_TYPE;
		}

		int QueryValueAttribute(const std::string& name, std::string* outValue) const
		{
			const TiXmlAttribute* node = attributeSet.Find(name);
			if (!node)
				return TIXML_NO_ATTRIBUTE;
			*outValue = node->ValueStr();
			return TIXML_SUCCESS;
		}
#endif

		/** Sets an attribute of name to a given value. The attribute
			will be created if it does not exist, or changed if it does.
		*/
		void SetAttribute(const char* name, const char* _value);

#ifdef TIXML_USE_STL
		const std::string* Attribute(const std::string& name) const;
		const std::string* Attribute(const std::string& name, int* i) const;
		const std::string* Attribute(const std::string& name, double* d) const;
		int QueryIntAttribute(const std::string& name, int* _value) const;
		int QueryDoubleAttribute(const std::string& name, double* _value) const;

		/// STL std::string form.
		void SetAttribute(const std::string& name, const std::string& _value);
		///< STL std::string form.
		void SetAttribute(const std::string& name, int _value);
		///< STL std::string form.
		void SetDoubleAttribute(const std::string& name, double value);
#endif

		/** Sets an attribute of name to a given value. The attribute
			will be created if it does not exist, or changed if it does.
		*/
		void SetAttribute(const char* name, int value);

		/** Sets an attribute of name to a given value. The attribute
			will be created if it does not exist, or changed if it does.
		*/
		void SetDoubleAttribute(const char* name, double value);

		/** Deletes an attribute with the given name.
		*/
		void RemoveAttribute(const char* name);
#ifdef TIXML_USE_STL
		void RemoveAttribute(const std::string& name) { RemoveAttribute(name.c_str()); }	///< STL std::string form.
#endif

		const TiXmlAttribute* FirstAttribute() const { return attributeSet.First(); }		///< Access the first attribute in this element.
		TiXmlAttribute* FirstAttribute() { return attributeSet.First(); }
		const TiXmlAttribute* LastAttribute()	const { return attributeSet.Last(); }		///< Access the last attribute in this element.
		TiXmlAttribute* LastAttribute() { return attributeSet.Last(); }

		/** Convenience function for easy access to the text inside an element. Although easy
			and concise, GetText() is limited compared to getting the TiXmlText child
			and accessing it directly.

			If the first child of 'this' is a TiXmlText, the GetText()
			returns the character string of the Text node, else null is returned.

			This is a convenient method for getting the text of simple contained text:
			@verbatim
			<foo>This is text</foo>
			const char* str = fooElement->GetText();
			@endverbatim

			'str' will be a pointer to "This is text".

			Note that this function can be misleading. If the element foo was created from
			this XML:
			@verbatim
			<foo><b>This is text</b></foo>
			@endverbatim

			then the value of str would be null. The first child node isn't a text node, it is
			another element. From this XML:
			@verbatim
			<foo>This is <b>text</b></foo>
			@endverbatim
			GetText() will return "This is ".

			WARNING: GetText() accesses a child node - don't become confused with the
					 similarly named TiXmlHandle::Text() and TiXmlNode::ToText() which are
					 safe type casts on the referenced node.
		*/
		const char* GetText() const;

		/// Creates a new Element and returns it - the returned element is a copy.
		virtual TiXmlNode* Clone() const;
		// Print the Element to a FILE stream.
		virtual void Print(FILE* cfile, int depth) const;

		/*	Attribtue parsing starts: next char past '<'
							 returns: next char past '>'
		*/
		virtual const char* Parse(const char* p, TiXmlParsingData* data, TiXmlEncoding encoding);

		virtual const TiXmlElement* ToElement()     const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
		virtual TiXmlElement* ToElement() { return this; } ///< Cast to a more defined type. Will return null not of the requested type.

		/** Walk the XML tree visiting this node and all of its children.
		*/
		virtual bool Accept(TiXmlVisitor* visitor) const;

	protected:

		void CopyTo(TiXmlElement* target) const;
		void ClearThis();	// like clear, but initializes 'this' object as well

		// Used to be public [internal use]
#ifdef TIXML_USE_STL
		virtual void StreamIn(std::istream* in, TiXmlString* tag);
#endif
		/*	[internal use]
			Reads the "value" of the element -- another element, or text.
			This should terminate with the current end tag.
		*/
		const char* ReadValue(const char* in, TiXmlParsingData* prevData, TiXmlEncoding encoding);

	private:
		TiXmlAttributeSet attributeSet;
	};
	class TiXmlComment : public TiXmlNode
	{
	public:
		/// Constructs an empty comment.
		TiXmlComment() : TiXmlNode(TiXmlNode::TINYXML_COMMENT) {}
		/// Construct a comment from text.
		TiXmlComment(const char* _value) : TiXmlNode(TiXmlNode::TINYXML_COMMENT) {
			SetValue(_value);
		}
		TiXmlComment(const TiXmlComment&);
		TiXmlComment& operator=(const TiXmlComment& base);

		virtual ~TiXmlComment() {}

		/// Returns a copy of this Comment.
		virtual TiXmlNode* Clone() const;
		// Write this Comment to a FILE stream.
		virtual void Print(FILE* cfile, int depth) const;

		/*	Attribtue parsing starts: at the ! of the !--
							 returns: next char past '>'
		*/
		virtual const char* Parse(const char* p, TiXmlParsingData* data, TiXmlEncoding encoding);

		virtual const TiXmlComment* ToComment() const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
		virtual		  TiXmlComment* ToComment() { return this; } ///< Cast to a more defined type. Will return null not of the requested type.

		/** Walk the XML tree visiting this node and all of its children.
		*/
		virtual bool Accept(TiXmlVisitor* visitor) const;

	protected:
		void CopyTo(TiXmlComment* target) const;

		// used to be public
#ifdef TIXML_USE_STL
		virtual void StreamIn(std::istream* in, TiXmlString* tag);
#endif
		//	virtual void StreamOut( TIXML_OSTREAM * out ) const;

	private:

	};
	class TiXmlText : public TiXmlNode
	{
		friend class TiXmlElement;
	public:
		/** Constructor for text element. By default, it is treated as
			normal, encoded text. If you want it be output as a CDATA text
			element, set the parameter _cdata to 'true'
		*/
		TiXmlText(const char* initValue) : TiXmlNode(TiXmlNode::TINYXML_TEXT)
		{
			SetValue(initValue);
			cdata = false;
		}
		virtual ~TiXmlText() {}

#ifdef TIXML_USE_STL
		/// Constructor.
		TiXmlText(const std::string& initValue) : TiXmlNode(TiXmlNode::TINYXML_TEXT)
		{
			SetValue(initValue);
			cdata = false;
		}
#endif

		TiXmlText(const TiXmlText& copy) : TiXmlNode(TiXmlNode::TINYXML_TEXT) { copy.CopyTo(this); }
		TiXmlText& operator=(const TiXmlText& base) { base.CopyTo(this); return *this; }

		// Write this text object to a FILE stream.
		virtual void Print(FILE* cfile, int depth) const;

		/// Queries whether this represents text using a CDATA section.
		bool CDATA() const { return cdata; }
		/// Turns on or off a CDATA representation of text.
		void SetCDATA(bool _cdata) { cdata = _cdata; }

		virtual const char* Parse(const char* p, TiXmlParsingData* data, TiXmlEncoding encoding);

		virtual const TiXmlText* ToText() const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
		virtual TiXmlText* ToText() { return this; } ///< Cast to a more defined type. Will return null not of the requested type.

		/** Walk the XML tree visiting this node and all of its children.
		*/
		virtual bool Accept(TiXmlVisitor* content) const;

	protected:
		///  [internal use] Creates a new Element and returns it.
		virtual TiXmlNode* Clone() const;
		void CopyTo(TiXmlText* target) const;

		bool Blank() const;	// returns true if all white space and new lines
		// [internal use]
#ifdef TIXML_USE_STL
		virtual void StreamIn(std::istream* in, TiXmlString* tag);
#endif

	private:
		bool cdata;			// true if this should be input and output as a CDATA style text element
	};
	class TiXmlDeclaration : public TiXmlNode
	{
	public:
		/// Construct an empty declaration.
		TiXmlDeclaration() : TiXmlNode(TiXmlNode::TINYXML_DECLARATION) {}

#ifdef TIXML_USE_STL
		/// Constructor.
		TiXmlDeclaration(const std::string& _version,
			const std::string& _encoding,
			const std::string& _standalone);
#endif

		/// Construct.
		TiXmlDeclaration(const char* _version,
			const char* _encoding,
			const char* _standalone);

		TiXmlDeclaration(const TiXmlDeclaration& copy);
		TiXmlDeclaration& operator=(const TiXmlDeclaration& copy);

		virtual ~TiXmlDeclaration() {}

		/// Version. Will return an empty string if none was found.
		const char* Version() const { return version.c_str(); }
		/// Encoding. Will return an empty string if none was found.
		const char* Encoding() const { return encoding.c_str(); }
		/// Is this a standalone document?
		const char* Standalone() const { return standalone.c_str(); }

		/// Creates a copy of this Declaration and returns it.
		virtual TiXmlNode* Clone() const;
		// Print this declaration to a FILE stream.
		virtual void Print(FILE* cfile, int depth, TiXmlString* str) const;
		virtual void Print(FILE* cfile, int depth) const {
			Print(cfile, depth, 0);
		}

		virtual const char* Parse(const char* p, TiXmlParsingData* data, TiXmlEncoding encoding);

		virtual const TiXmlDeclaration* ToDeclaration() const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
		virtual TiXmlDeclaration* ToDeclaration() { return this; } ///< Cast to a more defined type. Will return null not of the requested type.

		/** Walk the XML tree visiting this node and all of its children.
		*/
		virtual bool Accept(TiXmlVisitor* visitor) const;

	protected:
		void CopyTo(TiXmlDeclaration* target) const;
		// used to be public
#ifdef TIXML_USE_STL
		virtual void StreamIn(std::istream* in, TiXmlString* tag);
#endif

	private:

		TiXmlString version;
		TiXmlString encoding;
		TiXmlString standalone;
	};
	class TiXmlUnknown : public TiXmlNode
	{
	public:
		TiXmlUnknown() : TiXmlNode(TiXmlNode::TINYXML_UNKNOWN) {}
		virtual ~TiXmlUnknown() {}

		TiXmlUnknown(const TiXmlUnknown& copy) : TiXmlNode(TiXmlNode::TINYXML_UNKNOWN) { copy.CopyTo(this); }
		TiXmlUnknown& operator=(const TiXmlUnknown& copy) { copy.CopyTo(this); return *this; }

		/// Creates a copy of this Unknown and returns it.
		virtual TiXmlNode* Clone() const;
		// Print this Unknown to a FILE stream.
		virtual void Print(FILE* cfile, int depth) const;

		virtual const char* Parse(const char* p, TiXmlParsingData* data, TiXmlEncoding encoding);

		virtual const TiXmlUnknown* ToUnknown()     const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
		virtual TiXmlUnknown* ToUnknown() { return this; } ///< Cast to a more defined type. Will return null not of the requested type.

		/** Walk the XML tree visiting this node and all of its children.
		*/
		virtual bool Accept(TiXmlVisitor* content) const;

	protected:
		void CopyTo(TiXmlUnknown* target) const;

#ifdef TIXML_USE_STL
		virtual void StreamIn(std::istream* in, TiXmlString* tag);
#endif

	private:

	};
	class TiXmlDocument : public TiXmlNode
	{
	public:
		/// Create an empty document, that has no name.
		TiXmlDocument();
		/// Create a document with a name. The name of the document is also the filename of the xml.
		TiXmlDocument(const char* documentName);

#ifdef TIXML_USE_STL
		/// Constructor.
		TiXmlDocument(const std::string& documentName);
#endif

		TiXmlDocument(const TiXmlDocument& copy);
		TiXmlDocument& operator=(const TiXmlDocument& copy);

		virtual ~TiXmlDocument() {}

		/** Load a file using the current document value.
			Returns true if successful. Will delete any existing
			document data before loading.
		*/
		bool LoadFile(TiXmlEncoding encoding = TIXML_ENCODING_UNKNOWN);
		/// Save a file using the current document value. Returns true if successful.
		bool SaveFile() const;
		/// Load a file using the given filename. Returns true if successful.
		bool LoadFile(const char* filename, TiXmlEncoding encoding = TIXML_ENCODING_UNKNOWN);
		/// Save a file using the given filename. Returns true if successful.
		bool SaveFile(const char* filename) const;
		/** Load a file using the given FILE*. Returns true if successful. Note that this method
			doesn't stream - the entire object pointed at by the FILE*
			will be interpreted as an XML file. TinyXML doesn't stream in XML from the current
			file location. Streaming may be added in the future.
		*/
		bool LoadFile(FILE*, TiXmlEncoding encoding = TIXML_ENCODING_UNKNOWN);
		/// Save a file using the given FILE*. Returns true if successful.
		bool SaveFile(FILE*) const;

#ifdef TIXML_USE_STL
		bool LoadFile(const std::string& filename, TiXmlEncoding encoding = TIXML_DEFAULT_ENCODING)			///< STL std::string version.
		{
			return LoadFile(filename.c_str(), encoding);
		}
		bool SaveFile(const std::string& filename) const		///< STL std::string version.
		{
			return SaveFile(filename.c_str());
		}
#endif

		/** Parse the given null terminated block of xml data. Passing in an encoding to this
			method (either TIXML_ENCODING_LEGACY or TIXML_ENCODING_UTF8 will force TinyXml
			to use that encoding, regardless of what TinyXml might otherwise try to detect.
		*/
		virtual const char* Parse(const char* p, TiXmlParsingData* data = 0, TiXmlEncoding encoding = TIXML_ENCODING_UNKNOWN);

		/** Get the root element -- the only top level element -- of the document.
			In well formed XML, there should only be one. TinyXml is tolerant of
			multiple elements at the document level.
		*/
		const TiXmlElement* RootElement() const { return FirstChildElement(); }
		TiXmlElement* RootElement() { return FirstChildElement(); }

		/** If an error occurs, Error will be set to true. Also,
			- The ErrorId() will contain the integer identifier of the error (not generally useful)
			- The ErrorDesc() method will return the name of the error. (very useful)
			- The ErrorRow() and ErrorCol() will return the location of the error (if known)
		*/
		bool Error() const { return error; }

		/// Contains a textual (english) description of the error if one occurs.
		const char* ErrorDesc() const { return errorDesc.c_str(); }

		/** Generally, you probably want the error string ( ErrorDesc() ). But if you
			prefer the ErrorId, this function will fetch it.
		*/
		int ErrorId()	const { return errorId; }

		/** Returns the location (if known) of the error. The first column is column 1,
			and the first row is row 1. A value of 0 means the row and column wasn't applicable
			(memory errors, for example, have no row/column) or the parser lost the error. (An
			error in the error reporting, in that case.)

			@sa SetTabSize, Row, Column
		*/
		int ErrorRow() const { return errorLocation.row + 1; }
		int ErrorCol() const { return errorLocation.col + 1; }	///< The column where the error occured. See ErrorRow()

		/** SetTabSize() allows the error reporting functions (ErrorRow() and ErrorCol())
			to report the correct values for row and column. It does not change the output
			or input in any way.

			By calling this method, with a tab size
			greater than 0, the row and column of each node and attribute is stored
			when the file is loaded. Very useful for tracking the DOM back in to
			the source file.

			The tab size is required for calculating the location of nodes. If not
			set, the default of 4 is used. The tabsize is set per document. Setting
			the tabsize to 0 disables row/column tracking.

			Note that row and column tracking is not supported when using operator>>.

			The tab size needs to be enabled before the parse or load. Correct usage:
			@verbatim
			TiXmlDocument doc;
			doc.SetTabSize( 8 );
			doc.Load( "myfile.xml" );
			@endverbatim

			@sa Row, Column
		*/
		void SetTabSize(int _tabsize) { tabsize = _tabsize; }

		int TabSize() const { return tabsize; }

		/** If you have handled the error, it can be reset with this call. The error
			state is automatically cleared if you Parse a new XML block.
		*/
		void ClearError() {
			error = false;
			errorId = 0;
			errorDesc = "";
			errorLocation.row = errorLocation.col = 0;
			//errorLocation.last = 0; 
		}

		/** Write the document to standard out using formatted printing ("pretty print"). */
		void Print() const { Print(stdout, 0); }

		/* Write the document to a string using formatted printing ("pretty print"). This
			will allocate a character array (new char[]) and return it as a pointer. The
			calling code pust call delete[] on the return char* to avoid a memory leak.
		*/
		//char* PrintToMemory() const; 

		/// Print this Document to a FILE stream.
		virtual void Print(FILE* cfile, int depth = 0) const;
		// [internal use]
		void SetError(int err, const char* errorLocation, TiXmlParsingData* prevData, TiXmlEncoding encoding);

		virtual const TiXmlDocument* ToDocument()    const { return this; } ///< Cast to a more defined type. Will return null not of the requested type.
		virtual TiXmlDocument* ToDocument() { return this; } ///< Cast to a more defined type. Will return null not of the requested type.

		/** Walk the XML tree visiting this node and all of its children.
		*/
		virtual bool Accept(TiXmlVisitor* content) const;

	protected:
		// [internal use]
		virtual TiXmlNode* Clone() const;
#ifdef TIXML_USE_STL
		virtual void StreamIn(std::istream* in, TiXmlString* tag);
#endif

	private:
		void CopyTo(TiXmlDocument* target) const;

		bool error;
		int  errorId;
		TiXmlString errorDesc;
		int tabsize;
		TiXmlCursor errorLocation;
		bool useMicrosoftBOM;		// the UTF-8 BOM were found when read. Note this, and try to write.
	};
	class TiXmlHandle
	{
	public:
		/// Create a handle from any node (at any depth of the tree.) This can be a null pointer.
		TiXmlHandle(TiXmlNode* _node) { this->node = _node; }
		/// Copy constructor
		TiXmlHandle(const TiXmlHandle& ref) { this->node = ref.node; }
		TiXmlHandle operator=(const TiXmlHandle& ref) { if (&ref != this) this->node = ref.node; return *this; }

		/// Return a handle to the first child node.
		TiXmlHandle FirstChild() const;
		/// Return a handle to the first child node with the given name.
		TiXmlHandle FirstChild(const char* value) const;
		/// Return a handle to the first child element.
		TiXmlHandle FirstChildElement() const;
		/// Return a handle to the first child element with the given name.
		TiXmlHandle FirstChildElement(const char* value) const;

		/** Return a handle to the "index" child with the given name.
			The first child is 0, the second 1, etc.
		*/
		TiXmlHandle Child(const char* value, int index) const;
		/** Return a handle to the "index" child.
			The first child is 0, the second 1, etc.
		*/
		TiXmlHandle Child(int index) const;
		/** Return a handle to the "index" child element with the given name.
			The first child element is 0, the second 1, etc. Note that only TiXmlElements
			are indexed: other types are not counted.
		*/
		TiXmlHandle ChildElement(const char* value, int index) const;
		/** Return a handle to the "index" child element.
			The first child element is 0, the second 1, etc. Note that only TiXmlElements
			are indexed: other types are not counted.
		*/
		TiXmlHandle ChildElement(int index) const;

#ifdef TIXML_USE_STL
		TiXmlHandle FirstChild(const std::string& _value) const { return FirstChild(_value.c_str()); }
		TiXmlHandle FirstChildElement(const std::string& _value) const { return FirstChildElement(_value.c_str()); }

		TiXmlHandle Child(const std::string& _value, int index) const { return Child(_value.c_str(), index); }
		TiXmlHandle ChildElement(const std::string& _value, int index) const { return ChildElement(_value.c_str(), index); }
#endif

		/** Return the handle as a TiXmlNode. This may return null.
		*/
		TiXmlNode* ToNode() const { return node; }
		/** Return the handle as a TiXmlElement. This may return null.
		*/
		TiXmlElement* ToElement() const { return ((node && node->ToElement()) ? node->ToElement() : 0); }
		/**	Return the handle as a TiXmlText. This may return null.
		*/
		TiXmlText* ToText() const { return ((node && node->ToText()) ? node->ToText() : 0); }
		/** Return the handle as a TiXmlUnknown. This may return null.
		*/
		TiXmlUnknown* ToUnknown() const { return ((node && node->ToUnknown()) ? node->ToUnknown() : 0); }

		/** @deprecated use ToNode.
			Return the handle as a TiXmlNode. This may return null.
		*/
		TiXmlNode* Node() const { return ToNode(); }
		/** @deprecated use ToElement.
			Return the handle as a TiXmlElement. This may return null.
		*/
		TiXmlElement* Element() const { return ToElement(); }
		/**	@deprecated use ToText()
			Return the handle as a TiXmlText. This may return null.
		*/
		TiXmlText* Text() const { return ToText(); }
		/** @deprecated use ToUnknown()
			Return the handle as a TiXmlUnknown. This may return null.
		*/
		TiXmlUnknown* Unknown() const { return ToUnknown(); }

	private:
		TiXmlNode* node;
	};
	class TiXmlPrinter : public TiXmlVisitor
	{
	public:
		TiXmlPrinter() : depth(0), simpleTextPrint(false),
			buffer(), indent("    "), lineBreak("\n") {}

		virtual bool VisitEnter(const TiXmlDocument& doc);
		virtual bool VisitExit(const TiXmlDocument& doc);

		virtual bool VisitEnter(const TiXmlElement& element, const TiXmlAttribute* firstAttribute);
		virtual bool VisitExit(const TiXmlElement& element);

		virtual bool Visit(const TiXmlDeclaration& declaration);
		virtual bool Visit(const TiXmlText& text);
		virtual bool Visit(const TiXmlComment& comment);
		virtual bool Visit(const TiXmlUnknown& unknown);

		/** Set the indent characters for printing. By default 4 spaces
			but tab (\t) is also useful, or null/empty string for no indentation.
		*/
		void SetIndent(const char* _indent) { indent = _indent ? _indent : ""; }
		/// Query the indention string.
		const char* Indent() { return indent.c_str(); }
		/** Set the line breaking string. By default set to newline (\n).
			Some operating systems prefer other characters, or can be
			set to the null/empty string for no indenation.
		*/
		void SetLineBreak(const char* _lineBreak) { lineBreak = _lineBreak ? _lineBreak : ""; }
		/// Query the current line breaking string.
		const char* LineBreak() { return lineBreak.c_str(); }

		/** Switch over to "stream printing" which is the most dense formatting without
			linebreaks. Common when the XML is needed for network transmission.
		*/
		void SetStreamPrinting() {
			indent = "";
			lineBreak = "";
		}
		/// Return the result.
		const char* CStr() { return buffer.c_str(); }
		/// Return the length of the result string.
		size_t Size() { return buffer.size(); }

#ifdef TIXML_USE_STL
		/// Return the result.
		const std::string& Str() { return buffer; }
#endif

	private:
		void DoIndent() {
			for (int i = 0; i < depth; ++i)
				buffer += indent;
		}
		void DoLineBreak() {
			buffer += lineBreak;
		}

		int depth;
		bool simpleTextPrint;
		TiXmlString buffer;
		TiXmlString indent;
		TiXmlString lineBreak;
	};
	class TiXmlParsingData
	{
		friend class TiXmlDocument;
	public:
		void Stamp(const char* now, TiXmlEncoding encoding);

		const TiXmlCursor& Cursor() const { return cursor; }

	private:
		// Only used by the document!
		TiXmlParsingData(const char* start, int _tabsize, int row, int col)
		{
			assert(start);
			stamp = start;
			tabsize = _tabsize;
			cursor.row = row;
			cursor.col = col;
		}

		TiXmlCursor		cursor;
		const char* stamp;
		int				tabsize;
	};

#pragma endregion
private:

};
#pragma endregion

#pragma region(org class)
char _fileParserPath[512];
FFileParser::eMODE _mode;
FFileParser::FFileParser(char* filePath, int mode)
{
	_lib = Fmhha();
	sprintf_s(_fileParserPath, 512, "%s", filePath);
	_mode = (eMODE)mode;
}
FFileParser::FFileParser(wchar_t* filePath, int mode)
{
	_lib = Fmhha();
	int nTemp = WideCharToMultiByte(CP_ACP, 0, filePath, -1, NULL, 0, NULL, NULL);
	char* pszTemp = new char[nTemp];
	WideCharToMultiByte(CP_ACP, 0, filePath, -1, pszTemp, nTemp, NULL, NULL);
	sprintf_s(_fileParserPath, 512, "%s", pszTemp);
	delete[] pszTemp;
	_mode = (eMODE)mode;
}
FFileParser::~FFileParser()
{

}

char* FFileParser::GetString(char* cmd, char* defData)
{
	if (_lib.LibraryPermit() == false)
	{
		return defData;
	}
	switch (_mode)
	{
	case eMODE::INI:
	{
		iniParser pas(cmd);
		return pas.GetString(defData, _fileParserPath);
	}
	break;
	case eMODE::XML:
	{
		xmlParser pas(cmd);
		return pas.GetString(defData, _fileParserPath);
	}
	break;
	}

	return defData;
}
void FFileParser::GetString(char* cmd, char* defData, char* returnValue)
{
	returnValue = GetString(cmd, defData);
}
char* FFileParser::GetStringW(wchar_t* cmd, wchar_t* defData)
{
	char cmdTemp[512] = { 0, };
	char defTemp[512] = { 0, };
	int nTemp = WideCharToMultiByte(CP_ACP, 0, cmd, -1, NULL, 0, NULL, NULL);
	char* pszTemp = new char[nTemp];
	WideCharToMultiByte(CP_ACP, 0, cmd, -1, pszTemp, nTemp, NULL, NULL);
	sprintf_s(cmdTemp, 512, "%s", pszTemp);
	delete[] pszTemp;

	nTemp = WideCharToMultiByte(CP_ACP, 0, defData, -1, NULL, 0, NULL, NULL);
	pszTemp = new char[nTemp];
	WideCharToMultiByte(CP_ACP, 0, defData, -1, pszTemp, nTemp, NULL, NULL);
	sprintf_s(defTemp, 512, "%s", pszTemp);
	delete[] pszTemp;

	return GetString(cmdTemp, defTemp);
}
void FFileParser::GetStringW(wchar_t* cmd, wchar_t* defData, wchar_t* returnValue)
{
	char cmdTemp[512] = { 0, };
	char defTemp[512] = { 0, };
	char ret[512] = { 0, };
	int nTemp = WideCharToMultiByte(CP_ACP, 0, cmd, -1, NULL, 0, NULL, NULL);
	char* pszTemp = new char[nTemp];
	WideCharToMultiByte(CP_ACP, 0, cmd, -1, pszTemp, nTemp, NULL, NULL);
	sprintf_s(cmdTemp, 512, "%s", pszTemp);
	delete[] pszTemp;

	nTemp = WideCharToMultiByte(CP_ACP, 0, defData, -1, NULL, 0, NULL, NULL);
	pszTemp = new char[nTemp];
	WideCharToMultiByte(CP_ACP, 0, defData, -1, pszTemp, nTemp, NULL, NULL);
	sprintf_s(defTemp, 512, "%s", pszTemp);
	delete[] pszTemp;

	GetString(cmdTemp, defTemp, ret);

	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, ret, strlen(ret), returnValue, 512);
}
int FFileParser::GetInt(char* cmd, int defData)
{
	if (_lib.LibraryPermit() == false)
	{
		return defData;
	}
	switch (_mode)
	{
	case eMODE::INI:
	{
		iniParser pas(cmd);
		return pas.GetInt(defData, _fileParserPath);
	}
	break;
	case eMODE::XML:
	{
		xmlParser pas(cmd);
		return pas.GetInt(defData, _fileParserPath);
	}
	break;
	}

	return defData;
}
void FFileParser::GetInt(char* cmd, int defData, int& returnValue)
{
	returnValue = GetInt(cmd, defData);
}
int FFileParser::GetIntW(wchar_t* cmd, int defData)
{
	char cmdTemp[512] = { 0, };
	int nTemp = WideCharToMultiByte(CP_ACP, 0, cmd, -1, NULL, 0, NULL, NULL);
	char* pszTemp = new char[nTemp];
	WideCharToMultiByte(CP_ACP, 0, cmd, -1, pszTemp, nTemp, NULL, NULL);
	sprintf_s(cmdTemp, 512, "%s", pszTemp);
	delete[] pszTemp;

	return GetInt(cmdTemp, defData);
}
void FFileParser::GetIntW(wchar_t* cmd, int defData, int& returnValue)
{
	char cmdTemp[512] = { 0, };
	int nTemp = WideCharToMultiByte(CP_ACP, 0, cmd, -1, NULL, 0, NULL, NULL);
	char* pszTemp = new char[nTemp];
	WideCharToMultiByte(CP_ACP, 0, cmd, -1, pszTemp, nTemp, NULL, NULL);
	sprintf_s(cmdTemp, 512, "%s", pszTemp);
	delete[] pszTemp;

	GetInt(cmdTemp, defData, returnValue);
}
double FFileParser::GetDouble(char* cmd, double defData)
{
	if (_lib.LibraryPermit() == false)
	{
		return defData;
	}
	switch (_mode)
	{
	case eMODE::INI:
	{
		iniParser pas(cmd);
		return pas.GetDouble(defData, _fileParserPath);
	}
	break;
	case eMODE::XML:
	{
		xmlParser pas(cmd);
		return pas.GetDouble(defData, _fileParserPath);
	}
	break;
	}

	return defData;
}
void FFileParser::GetDouble(char* cmd, double defData, double& returnValue)
{
	returnValue = GetDouble(cmd, defData);
}
double FFileParser::GetDoubleW(wchar_t* cmd, double defData)
{
	char cmdTemp[512] = { 0, };
	int nTemp = WideCharToMultiByte(CP_ACP, 0, cmd, -1, NULL, 0, NULL, NULL);
	char* pszTemp = new char[nTemp];
	WideCharToMultiByte(CP_ACP, 0, cmd, -1, pszTemp, nTemp, NULL, NULL);
	sprintf_s(cmdTemp, 512, "%s", pszTemp);
	delete[] pszTemp;

	return GetDouble(cmdTemp, defData);
}
void FFileParser::GetDoubleW(wchar_t* cmd, double defData, double& returnValue)
{
	char cmdTemp[512] = { 0, };
	int nTemp = WideCharToMultiByte(CP_ACP, 0, cmd, -1, NULL, 0, NULL, NULL);
	char* pszTemp = new char[nTemp];
	WideCharToMultiByte(CP_ACP, 0, cmd, -1, pszTemp, nTemp, NULL, NULL);
	sprintf_s(cmdTemp, 512, "%s", pszTemp);
	delete[] pszTemp;

	GetDouble(cmdTemp, defData, returnValue);
}

void FFileParser::SetString(char* cmd, char* data)
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}
	switch (_mode)
	{
	case eMODE::INI:
	{
		iniParser pas(cmd);
		return pas.SetString(data, _fileParserPath);
	}
	break;
	case eMODE::XML:
	{
		xmlParser pas(cmd);
		return pas.SetString(data, _fileParserPath);
	}
	break;
	}
}
void FFileParser::SetStringW(wchar_t* cmd, wchar_t* data)
{
	char cmdTemp[512] = { 0, };
	char dataTemp[512] = { 0, };
	int nTemp = WideCharToMultiByte(CP_ACP, 0, cmd, -1, NULL, 0, NULL, NULL);
	char* pszTemp = new char[nTemp];
	WideCharToMultiByte(CP_ACP, 0, cmd, -1, pszTemp, nTemp, NULL, NULL);
	sprintf_s(cmdTemp, 512, "%s", pszTemp);
	delete[] pszTemp;

	nTemp = WideCharToMultiByte(CP_ACP, 0, data, -1, NULL, 0, NULL, NULL);
	pszTemp = new char[nTemp];
	WideCharToMultiByte(CP_ACP, 0, data, -1, pszTemp, nTemp, NULL, NULL);
	sprintf_s(dataTemp, 512, "%s", pszTemp);
	delete[] pszTemp;

	SetString(cmdTemp, dataTemp);
}
void FFileParser::SetInt(char* cmd, int data)
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}
	switch (_mode)
	{
	case eMODE::INI:
	{
		iniParser pas(cmd);
		return pas.SetInt(data, _fileParserPath);
	}
	break;
	case eMODE::XML:
	{
		xmlParser pas(cmd);
		return pas.SetInt(data, _fileParserPath);
	}
	break;
	}
}
void FFileParser::SetIntW(wchar_t* cmd, int data)
{
	char cmdTemp[512] = { 0, };
	char dataTemp[512] = { 0, };
	int nTemp = WideCharToMultiByte(CP_ACP, 0, cmd, -1, NULL, 0, NULL, NULL);
	char* pszTemp = new char[nTemp];
	WideCharToMultiByte(CP_ACP, 0, cmd, -1, pszTemp, nTemp, NULL, NULL);
	sprintf_s(cmdTemp, 512, "%s", pszTemp);
	delete[] pszTemp;

	SetInt(cmdTemp, data);
}
void FFileParser::SetIntStr(char* cmd, char* data)
{
	SetInt(cmd, atoi(data));
}
void FFileParser::SetIntStrW(wchar_t* cmd, wchar_t* data)
{
	char cmdTemp[512] = { 0, };
	int nTemp = WideCharToMultiByte(CP_ACP, 0, cmd, -1, NULL, 0, NULL, NULL);
	char* pszTemp = new char[nTemp];
	WideCharToMultiByte(CP_ACP, 0, cmd, -1, pszTemp, nTemp, NULL, NULL);
	sprintf_s(cmdTemp, 512, "%s", pszTemp);
	delete[] pszTemp;

	SetInt(cmdTemp, _wtoi(data));
}
void FFileParser::SetDouble(char* cmd, double data)
{
	if (_lib.LibraryPermit() == false)
	{
		return;
	}
	switch (_mode)
	{
	case eMODE::INI:
	{
		iniParser pas(cmd);
		return pas.SetDouble(data, _fileParserPath);
	}
	break;
	case eMODE::XML:
	{
		xmlParser pas(cmd);
		return pas.SetDouble(data, _fileParserPath);
	}
	break;
	}
}
void FFileParser::SetDoubleW(wchar_t* cmd, double data)
{
	char cmdTemp[512] = { 0, };
	int nTemp = WideCharToMultiByte(CP_ACP, 0, cmd, -1, NULL, 0, NULL, NULL);
	char* pszTemp = new char[nTemp];
	WideCharToMultiByte(CP_ACP, 0, cmd, -1, pszTemp, nTemp, NULL, NULL);
	sprintf_s(cmdTemp, 512, "%s", pszTemp);
	delete[] pszTemp;

	SetDouble(cmdTemp, data);
}
void FFileParser::SetDoubleStr(char* cmd, char* data)
{
	SetDouble(cmd, atof(data));
}
void FFileParser::SetDoubleStrW(wchar_t* cmd, wchar_t* data)
{
	char cmdTemp[512] = { 0, };
	int nTemp = WideCharToMultiByte(CP_ACP, 0, cmd, -1, NULL, 0, NULL, NULL);
	char* pszTemp = new char[nTemp];
	WideCharToMultiByte(CP_ACP, 0, cmd, -1, pszTemp, nTemp, NULL, NULL);
	sprintf_s(cmdTemp, 512, "%s", pszTemp);
	delete[] pszTemp;

	SetDouble(cmdTemp, _wtof(data));
}
#pragma endregion

#pragma region(node)
xmlParser::TiXmlNode::TiXmlNode(NodeType _type) : TiXmlBase()
{
	parent = 0;
	type = _type;
	firstChild = 0;
	lastChild = 0;
	prev = 0;
	next = 0;
}
xmlParser::TiXmlNode::~TiXmlNode()
{
	TiXmlNode* node = firstChild;
	TiXmlNode* temp = 0;

	while (node)
	{
		temp = node;
		node = node->next;
		delete temp;
	}
}
void xmlParser::TiXmlNode::CopyTo(TiXmlNode* target) const
{
	target->SetValue(value.c_str());
	target->userData = userData;
	target->location = location;
}
void xmlParser::TiXmlNode::Clear()
{
	TiXmlNode* node = firstChild;
	TiXmlNode* temp = 0;

	while (node)
	{
		temp = node;
		node = node->next;
		delete temp;
	}

	firstChild = 0;
	lastChild = 0;
}
xmlParser::TiXmlNode* xmlParser::TiXmlNode::LinkEndChild(TiXmlNode* node)
{
	assert(node->parent == 0 || node->parent == this);
	assert(node->GetDocument() == 0 || node->GetDocument() == this->GetDocument());

	if (node->Type() == TiXmlNode::TINYXML_DOCUMENT)
	{
		delete node;
		if (GetDocument())
			GetDocument()->SetError(TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN);
		return 0;
	}

	node->parent = this;

	node->prev = lastChild;
	node->next = 0;

	if (lastChild)
		lastChild->next = node;
	else
		firstChild = node;			// it was an empty list.

	lastChild = node;
	return node;
}
xmlParser::TiXmlNode* xmlParser::TiXmlNode::InsertEndChild(const TiXmlNode& addThis)
{
	if (addThis.Type() == TiXmlNode::TINYXML_DOCUMENT)
	{
		if (GetDocument())
			GetDocument()->SetError(TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN);
		return 0;
	}
	TiXmlNode* node = addThis.Clone();
	if (!node)
		return 0;

	return LinkEndChild(node);
}
xmlParser::TiXmlNode* xmlParser::TiXmlNode::InsertBeforeChild(TiXmlNode* beforeThis, const TiXmlNode& addThis)
{
	if (!beforeThis || beforeThis->parent != this) {
		return 0;
	}
	if (addThis.Type() == TiXmlNode::TINYXML_DOCUMENT)
	{
		if (GetDocument())
			GetDocument()->SetError(TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN);
		return 0;
	}

	TiXmlNode* node = addThis.Clone();
	if (!node)
		return 0;
	node->parent = this;

	node->next = beforeThis;
	node->prev = beforeThis->prev;
	if (beforeThis->prev)
	{
		beforeThis->prev->next = node;
	}
	else
	{
		assert(firstChild == beforeThis);
		firstChild = node;
	}
	beforeThis->prev = node;
	return node;
}
xmlParser::TiXmlNode* xmlParser::TiXmlNode::InsertAfterChild(TiXmlNode* afterThis, const TiXmlNode& addThis)
{
	if (!afterThis || afterThis->parent != this) {
		return 0;
	}
	if (addThis.Type() == TiXmlNode::TINYXML_DOCUMENT)
	{
		if (GetDocument())
			GetDocument()->SetError(TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN);
		return 0;
	}

	TiXmlNode* node = addThis.Clone();
	if (!node)
		return 0;
	node->parent = this;

	node->prev = afterThis;
	node->next = afterThis->next;
	if (afterThis->next)
	{
		afterThis->next->prev = node;
	}
	else
	{
		assert(lastChild == afterThis);
		lastChild = node;
	}
	afterThis->next = node;
	return node;
}
xmlParser::TiXmlNode* xmlParser::TiXmlNode::ReplaceChild(TiXmlNode* replaceThis, const TiXmlNode& withThis)
{
	if (!replaceThis)
		return 0;

	if (replaceThis->parent != this)
		return 0;

	if (withThis.ToDocument()) {
		// A document can never be a child.	Thanks to Noam.
		xmlParser::TiXmlDocument* document = GetDocument();
		if (document)
			document->SetError(TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN);
		return 0;
	}

	TiXmlNode* node = withThis.Clone();
	if (!node)
		return 0;

	node->next = replaceThis->next;
	node->prev = replaceThis->prev;

	if (replaceThis->next)
		replaceThis->next->prev = node;
	else
		lastChild = node;

	if (replaceThis->prev)
		replaceThis->prev->next = node;
	else
		firstChild = node;

	delete replaceThis;
	node->parent = this;
	return node;
}
bool xmlParser::TiXmlNode::RemoveChild(TiXmlNode* removeThis)
{
	if (!removeThis) {
		return false;
	}

	if (removeThis->parent != this)
	{
		assert(0);
		return false;
	}

	if (removeThis->next)
		removeThis->next->prev = removeThis->prev;
	else
		lastChild = removeThis->prev;

	if (removeThis->prev)
		removeThis->prev->next = removeThis->next;
	else
		firstChild = removeThis->next;

	delete removeThis;
	return true;
}
const xmlParser::TiXmlNode* xmlParser::TiXmlNode::FirstChild(const char* _value) const
{
	const TiXmlNode* node;
	for (node = firstChild; node; node = node->next)
	{
		if (strcmp(node->Value(), _value) == 0)
			return node;
	}
	return 0;
}
const xmlParser::TiXmlNode* xmlParser::TiXmlNode::LastChild(const char* _value) const
{
	const TiXmlNode* node;
	for (node = lastChild; node; node = node->prev)
	{
		if (strcmp(node->Value(), _value) == 0)
			return node;
	}
	return 0;
}
const xmlParser::TiXmlNode* xmlParser::TiXmlNode::IterateChildren(const TiXmlNode* previous) const
{
	if (!previous)
	{
		return FirstChild();
	}
	else
	{
		assert(previous->parent == this);
		return previous->NextSibling();
	}
}
const xmlParser::TiXmlNode* xmlParser::TiXmlNode::IterateChildren(const char* val, const TiXmlNode* previous) const
{
	if (!previous)
	{
		return FirstChild(val);
	}
	else
	{
		assert(previous->parent == this);
		return previous->NextSibling(val);
	}
}
const xmlParser::TiXmlNode* xmlParser::TiXmlNode::NextSibling(const char* _value) const
{
	const TiXmlNode* node;
	for (node = next; node; node = node->next)
	{
		if (strcmp(node->Value(), _value) == 0)
			return node;
	}
	return 0;
}
const xmlParser::TiXmlNode* xmlParser::TiXmlNode::PreviousSibling(const char* _value) const
{
	const TiXmlNode* node;
	for (node = prev; node; node = node->prev)
	{
		if (strcmp(node->Value(), _value) == 0)
			return node;
	}
	return 0;
}
const xmlParser::TiXmlElement* xmlParser::TiXmlNode::FirstChildElement() const
{
	const TiXmlNode* node;

	for (node = FirstChild();
		node;
		node = node->NextSibling())
	{
		if (node->ToElement())
			return node->ToElement();
	}
	return 0;
}
const xmlParser::TiXmlElement* xmlParser::TiXmlNode::FirstChildElement(const char* _value) const
{
	const TiXmlNode* node;

	for (node = FirstChild(_value);
		node;
		node = node->NextSibling(_value))
	{
		if (node->ToElement())
			return node->ToElement();
	}
	return 0;
}
const xmlParser::TiXmlElement* xmlParser::TiXmlNode::NextSiblingElement() const
{
	const TiXmlNode* node;

	for (node = NextSibling();
		node;
		node = node->NextSibling())
	{
		if (node->ToElement())
			return node->ToElement();
	}
	return 0;
}
const xmlParser::TiXmlElement* xmlParser::TiXmlNode::NextSiblingElement(const char* _value) const
{
	const TiXmlNode* node;

	for (node = NextSibling(_value);
		node;
		node = node->NextSibling(_value))
	{
		if (node->ToElement())
			return node->ToElement();
	}
	return 0;
}
const xmlParser::TiXmlDocument* xmlParser::TiXmlNode::GetDocument() const
{
	const TiXmlNode* node;

	for (node = this; node; node = node->parent)
	{
		if (node->ToDocument())
			return node->ToDocument();
	}
	return 0;
}
xmlParser::TiXmlNode* xmlParser::TiXmlNode::Identify(const char* p, TiXmlEncoding encoding)
{
	TiXmlNode* returnNode = 0;

	p = SkipWhiteSpace(p, encoding);
	if (!p || !*p || *p != '<')
	{
		return 0;
	}

	p = SkipWhiteSpace(p, encoding);

	if (!p || !*p)
	{
		return 0;
	}

	// What is this thing? 
	// - Elements start with a letter or underscore, but xml is reserved.
	// - Comments: <!--
	// - Decleration: <?xml
	// - Everthing else is unknown to tinyxml.
	//

	const char* xmlHeader = { "<?xml" };
	const char* commentHeader = { "<!--" };
	const char* dtdHeader = { "<!" };
	const char* cdataHeader = { "<![CDATA[" };

	if (StringEqual(p, xmlHeader, true, encoding))
	{
#ifdef DEBUG_PARSER
		TIXML_LOG("XML parsing Declaration\n");
#endif
		returnNode = new TiXmlDeclaration();
	}
	else if (StringEqual(p, commentHeader, false, encoding))
	{
#ifdef DEBUG_PARSER
		TIXML_LOG("XML parsing Comment\n");
#endif
		returnNode = new TiXmlComment();
	}
	else if (StringEqual(p, cdataHeader, false, encoding))
	{
#ifdef DEBUG_PARSER
		TIXML_LOG("XML parsing CDATA\n");
#endif
		TiXmlText* text = new TiXmlText("");
		text->SetCDATA(true);
		returnNode = text;
	}
	else if (StringEqual(p, dtdHeader, false, encoding))
	{
#ifdef DEBUG_PARSER
		TIXML_LOG("XML parsing Unknown(1)\n");
#endif
		returnNode = new TiXmlUnknown();
	}
	else if (IsAlpha(*(p + 1), encoding)
		|| *(p + 1) == '_')
	{
#ifdef DEBUG_PARSER
		TIXML_LOG("XML parsing Element\n");
#endif
		returnNode = new TiXmlElement("");
	}
	else
	{
#ifdef DEBUG_PARSER
		TIXML_LOG("XML parsing Unknown(2)\n");
#endif
		returnNode = new TiXmlUnknown();
	}

	if (returnNode)
	{
		// Set the parent, so it can report errors
		returnNode->parent = this;
	}
	return returnNode;
}
#pragma endregion

#pragma region(element)
xmlParser::TiXmlElement::TiXmlElement(const char* _value)
	: TiXmlNode(TiXmlNode::TINYXML_ELEMENT)
{
	firstChild = lastChild = 0;
	value = _value;
}
xmlParser::TiXmlElement::TiXmlElement(const TiXmlElement& copy)
	: TiXmlNode(TiXmlNode::TINYXML_ELEMENT)
{
	firstChild = lastChild = 0;
	copy.CopyTo(this);
}
xmlParser::TiXmlElement::~TiXmlElement()
{
	ClearThis();
}
xmlParser::TiXmlElement& xmlParser::TiXmlElement::operator=(const TiXmlElement& base)
{
	ClearThis();
	base.CopyTo(this);
	return *this;
}
void xmlParser::TiXmlElement::RemoveAttribute(const char* name)
{
#ifdef TIXML_USE_STL
	TiXmlString str(name);
	TiXmlAttribute* node = attributeSet.Find(str);
#else
	TiXmlAttribute* node = attributeSet.Find(name);
#endif
	if (node)
	{
		attributeSet.Remove(node);
		delete node;
	}
}
void xmlParser::TiXmlElement::ClearThis()
{
	Clear();
	while (attributeSet.First())
	{
		TiXmlAttribute* node = attributeSet.First();
		attributeSet.Remove(node);
		delete node;
	}
}
const char* xmlParser::TiXmlElement::Attribute(const char* name) const
{
	const TiXmlAttribute* node = attributeSet.Find(name);
	if (node)
		return node->Value();
	return 0;
}
const char* xmlParser::TiXmlElement::Attribute(const char* name, int* i) const
{
	const TiXmlAttribute* attrib = attributeSet.Find(name);
	const char* result = 0;

	if (attrib) {
		result = attrib->Value();
		if (i) {
			attrib->QueryIntValue(i);
		}
	}
	return result;
}
const char* xmlParser::TiXmlElement::Attribute(const char* name, double* d) const
{
	const TiXmlAttribute* attrib = attributeSet.Find(name);
	const char* result = 0;

	if (attrib) {
		result = attrib->Value();
		if (d) {
			attrib->QueryDoubleValue(d);
		}
	}
	return result;
}
int xmlParser::TiXmlElement::QueryIntAttribute(const char* name, int* ival) const
{
	const TiXmlAttribute* attrib = attributeSet.Find(name);
	if (!attrib)
		return TIXML_NO_ATTRIBUTE;
	return attrib->QueryIntValue(ival);
}
int xmlParser::TiXmlElement::QueryUnsignedAttribute(const char* name, unsigned* value) const
{
	const TiXmlAttribute* node = attributeSet.Find(name);
	if (!node)
		return TIXML_NO_ATTRIBUTE;

	int ival = 0;
	int result = node->QueryIntValue(&ival);
	*value = (unsigned)ival;
	return result;
}
int xmlParser::TiXmlElement::QueryBoolAttribute(const char* name, bool* bval) const
{
	const TiXmlAttribute* node = attributeSet.Find(name);
	if (!node)
		return TIXML_NO_ATTRIBUTE;

	int result = TIXML_WRONG_TYPE;
	if (StringEqual(node->Value(), "true", true, TIXML_ENCODING_UNKNOWN)
		|| StringEqual(node->Value(), "yes", true, TIXML_ENCODING_UNKNOWN)
		|| StringEqual(node->Value(), "1", true, TIXML_ENCODING_UNKNOWN))
	{
		*bval = true;
		result = TIXML_SUCCESS;
	}
	else if (StringEqual(node->Value(), "false", true, TIXML_ENCODING_UNKNOWN)
		|| StringEqual(node->Value(), "no", true, TIXML_ENCODING_UNKNOWN)
		|| StringEqual(node->Value(), "0", true, TIXML_ENCODING_UNKNOWN))
	{
		*bval = false;
		result = TIXML_SUCCESS;
	}
	return result;
}
int xmlParser::TiXmlElement::QueryDoubleAttribute(const char* name, double* dval) const
{
	const TiXmlAttribute* attrib = attributeSet.Find(name);
	if (!attrib)
		return TIXML_NO_ATTRIBUTE;
	return attrib->QueryDoubleValue(dval);
}
void xmlParser::TiXmlElement::SetAttribute(const char* name, int val)
{
	TiXmlAttribute* attrib = attributeSet.FindOrCreate(name);
	if (attrib) {
		attrib->SetIntValue(val);
	}
}
void xmlParser::TiXmlElement::SetDoubleAttribute(const char* name, double val)
{
	TiXmlAttribute* attrib = attributeSet.FindOrCreate(name);
	if (attrib) {
		attrib->SetDoubleValue(val);
	}
}
void xmlParser::TiXmlElement::SetAttribute(const char* cname, const char* cvalue)
{
	TiXmlAttribute* attrib = attributeSet.FindOrCreate(cname);
	if (attrib) {
		attrib->SetValue(cvalue);
	}
}
void xmlParser::TiXmlElement::Print(FILE* cfile, int depth) const
{
	int i;
	assert(cfile);
	for (i = 0; i < depth; i++) {
		fprintf(cfile, "    ");
	}

	fprintf(cfile, "<%s", value.c_str());

	const TiXmlAttribute* attrib;
	for (attrib = attributeSet.First(); attrib; attrib = attrib->Next())
	{
		fprintf(cfile, " ");
		attrib->Print(cfile, depth);
	}

	// There are 3 different formatting approaches:
	// 1) An element without children is printed as a <foo /> node
	// 2) An element with only a text child is printed as <foo> text </foo>
	// 3) An element with children is printed on multiple lines.
	TiXmlNode* node;
	if (!firstChild)
	{
		fprintf(cfile, " />");
	}
	else if (firstChild == lastChild && firstChild->ToText())
	{
		fprintf(cfile, ">");
		firstChild->Print(cfile, depth + 1);
		fprintf(cfile, "</%s>", value.c_str());
	}
	else
	{
		fprintf(cfile, ">");

		for (node = firstChild; node; node = node->NextSibling())
		{
			if (!node->ToText())
			{
				fprintf(cfile, "\n");
			}
			node->Print(cfile, depth + 1);
		}
		fprintf(cfile, "\n");
		for (i = 0; i < depth; ++i) {
			fprintf(cfile, "    ");
		}
		fprintf(cfile, "</%s>", value.c_str());
	}
}
void xmlParser::TiXmlElement::CopyTo(TiXmlElement* target) const
{
	// superclass:
	TiXmlNode::CopyTo(target);

	// Element class: 
	// Clone the attributes, then clone the children.
	const TiXmlAttribute* attribute = 0;
	for (attribute = attributeSet.First();
		attribute;
		attribute = attribute->Next())
	{
		target->SetAttribute(attribute->Name(), attribute->Value());
	}

	TiXmlNode* node = 0;
	for (node = firstChild; node; node = node->NextSibling())
	{
		target->LinkEndChild(node->Clone());
	}
}
bool xmlParser::TiXmlElement::Accept(TiXmlVisitor* visitor) const
{
	if (visitor->VisitEnter(*this, attributeSet.First()))
	{
		for (const TiXmlNode* node = FirstChild(); node; node = node->NextSibling())
		{
			if (!node->Accept(visitor))
				break;
		}
	}
	return visitor->VisitExit(*this);
}
xmlParser::TiXmlNode* xmlParser::TiXmlElement::Clone() const
{
	TiXmlElement* clone = new TiXmlElement(Value());
	if (!clone)
		return 0;

	CopyTo(clone);
	return clone;
}
const char* xmlParser::TiXmlElement::GetText() const
{
	const TiXmlNode* child = this->FirstChild();
	if (child) {
		const TiXmlText* childText = child->ToText();
		if (childText) {
			return childText->Value();
		}
	}
	return 0;
}

const char* xmlParser::TiXmlElement::Parse(const char* p, TiXmlParsingData* data, TiXmlEncoding encoding)
{
	p = SkipWhiteSpace(p, encoding);
	TiXmlDocument* document = GetDocument();

	if (!p || !*p)
	{
		if (document) document->SetError(TIXML_ERROR_PARSING_ELEMENT, 0, 0, encoding);
		return 0;
	}

	if (data)
	{
		data->Stamp(p, encoding);
		location = data->Cursor();
	}

	if (*p != '<')
	{
		if (document) document->SetError(TIXML_ERROR_PARSING_ELEMENT, p, data, encoding);
		return 0;
	}

	p = SkipWhiteSpace(p + 1, encoding);

	// Read the name.
	const char* pErr = p;

	p = ReadName(p, &value, encoding);
	if (!p || !*p)
	{
		if (document)	document->SetError(TIXML_ERROR_FAILED_TO_READ_ELEMENT_NAME, pErr, data, encoding);
		return 0;
	}

	TiXmlString endTag("</");
	endTag += value;

	// Check for and read attributes. Also look for an empty
	// tag or an end tag.
	while (p && *p)
	{
		pErr = p;
		p = SkipWhiteSpace(p, encoding);
		if (!p || !*p)
		{
			if (document) document->SetError(TIXML_ERROR_READING_ATTRIBUTES, pErr, data, encoding);
			return 0;
		}
		if (*p == '/')
		{
			++p;
			// Empty tag.
			if (*p != '>')
			{
				if (document) document->SetError(TIXML_ERROR_PARSING_EMPTY, p, data, encoding);
				return 0;
			}
			return (p + 1);
		}
		else if (*p == '>')
		{
			// Done with attributes (if there were any.)
			// Read the value -- which can include other
			// elements -- read the end tag, and return.
			++p;
			p = ReadValue(p, data, encoding);		// Note this is an Element method, and will set the error if one happens.
			if (!p || !*p) {
				// We were looking for the end tag, but found nothing.
				// Fix for [ 1663758 ] Failure to report error on bad XML
				if (document) document->SetError(TIXML_ERROR_READING_END_TAG, p, data, encoding);
				return 0;
			}

			// We should find the end tag now
			// note that:
			// </foo > and
			// </foo> 
			// are both valid end tags.
			if (StringEqual(p, endTag.c_str(), false, encoding))
			{
				p += endTag.length();
				p = SkipWhiteSpace(p, encoding);
				if (p && *p && *p == '>') {
					++p;
					return p;
				}
				if (document) document->SetError(TIXML_ERROR_READING_END_TAG, p, data, encoding);
				return 0;
			}
			else
			{
				if (document) document->SetError(TIXML_ERROR_READING_END_TAG, p, data, encoding);
				return 0;
			}
		}
		else
		{
			// Try to read an attribute:
			TiXmlAttribute* attrib = new TiXmlAttribute();
			if (!attrib)
			{
				return 0;
			}

			attrib->SetDocument(document);
			pErr = p;
			p = attrib->Parse(p, data, encoding);

			if (!p || !*p)
			{
				if (document) document->SetError(TIXML_ERROR_PARSING_ELEMENT, pErr, data, encoding);
				delete attrib;
				return 0;
			}

			// Handle the strange case of double attributes:
#ifdef TIXML_USE_STL
			TiXmlAttribute* node = attributeSet.Find(attrib->NameTStr());
#else
			TiXmlAttribute* node = attributeSet.Find(attrib->Name());
#endif
			if (node)
			{
				if (document) document->SetError(TIXML_ERROR_PARSING_ELEMENT, pErr, data, encoding);
				delete attrib;
				return 0;
			}

			attributeSet.Add(attrib);
		}
	}
	return p;
}
const char* xmlParser::TiXmlElement::ReadValue(const char* p, TiXmlParsingData* data, TiXmlEncoding encoding)
{
	TiXmlDocument* document = GetDocument();

	// Read in text and elements in any order.
	const char* pWithWhiteSpace = p;
	p = SkipWhiteSpace(p, encoding);

	while (p && *p)
	{
		if (*p != '<')
		{
			// Take what we have, make a text element.
			TiXmlText* textNode = new TiXmlText("");

			if (!textNode)
			{
				return 0;
			}

			if (TiXmlBase::IsWhiteSpaceCondensed())
			{
				p = textNode->Parse(p, data, encoding);
			}
			else
			{
				// Special case: we want to keep the white space
				// so that leading spaces aren't removed.
				p = textNode->Parse(pWithWhiteSpace, data, encoding);
			}

			if (!textNode->Blank())
				LinkEndChild(textNode);
			else
				delete textNode;
		}
		else
		{
			// We hit a '<'
			// Have we hit a new element or an end tag? This could also be
			// a TiXmlText in the "CDATA" style.
			if (StringEqual(p, "</", false, encoding))
			{
				return p;
			}
			else
			{
				TiXmlNode* node = Identify(p, encoding);
				if (node)
				{
					p = node->Parse(p, data, encoding);
					LinkEndChild(node);
				}
				else
				{
					return 0;
				}
			}
		}
		pWithWhiteSpace = p;
		p = SkipWhiteSpace(p, encoding);
	}

	if (!p)
	{
		if (document) document->SetError(TIXML_ERROR_READING_ELEMENT_VALUE, 0, 0, encoding);
	}
	return p;
}
#pragma endregion

#pragma region(document)
xmlParser::TiXmlDocument::TiXmlDocument() : TiXmlNode(TiXmlNode::TINYXML_DOCUMENT)
{
	tabsize = 4;
	useMicrosoftBOM = false;
	ClearError();
}
xmlParser::TiXmlDocument::TiXmlDocument(const char* documentName) : TiXmlNode(TiXmlNode::TINYXML_DOCUMENT)
{
	tabsize = 4;
	useMicrosoftBOM = false;
	value = documentName;
	ClearError();
}
xmlParser::TiXmlDocument::TiXmlDocument(const TiXmlDocument& copy) : TiXmlNode(TiXmlNode::TINYXML_DOCUMENT)
{
	copy.CopyTo(this);
}
xmlParser::TiXmlDocument& xmlParser::TiXmlDocument::operator=(const TiXmlDocument& copy)
{
	Clear();
	copy.CopyTo(this);
	return *this;
}
bool xmlParser::TiXmlDocument::LoadFile(TiXmlEncoding encoding)
{
	return LoadFile(Value(), encoding);
}
bool xmlParser::TiXmlDocument::SaveFile() const
{
	return SaveFile(Value());
}
bool xmlParser::TiXmlDocument::LoadFile(const char* _filename, TiXmlEncoding encoding)
{
	TiXmlString filename(_filename);
	value = filename;

	// reading in binary mode so that tinyxml can normalize the EOL
	FILE* file = TiXmlFOpen(value.c_str(), "rb");

	if (file)
	{
		bool result = LoadFile(file, encoding);
		fclose(file);
		return result;
	}
	else
	{
		SetError(TIXML_ERROR_OPENING_FILE, 0, 0, TIXML_ENCODING_UNKNOWN);
		return false;
	}
}
bool xmlParser::TiXmlDocument::LoadFile(FILE* file, TiXmlEncoding encoding)
{
	if (!file)
	{
		SetError(TIXML_ERROR_OPENING_FILE, 0, 0, TIXML_ENCODING_UNKNOWN);
		return false;
	}

	// Delete the existing data:
	Clear();
	location.Clear();

	// Get the file size, so we can pre-allocate the string. HUGE speed impact.
	long length = 0;
	fseek(file, 0, SEEK_END);
	length = ftell(file);
	fseek(file, 0, SEEK_SET);

	// Strange case, but good to handle up front.
	if (length <= 0)
	{
		SetError(TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN);
		return false;
	}

	// Subtle bug here. TinyXml did use fgets. But from the XML spec:
	// 2.11 End-of-Line Handling
	// <snip>
	// <quote>
	// ...the XML processor MUST behave as if it normalized all line breaks in external 
	// parsed entities (including the document entity) on input, before parsing, by translating 
	// both the two-character sequence #xD #xA and any #xD that is not followed by #xA to 
	// a single #xA character.
	// </quote>
	//
	// It is not clear fgets does that, and certainly isn't clear it works cross platform. 
	// Generally, you expect fgets to translate from the convention of the OS to the c/unix
	// convention, and not work generally.

	/*
	while( fgets( buf, sizeof(buf), file ) )
	{
		data += buf;
	}
	*/

	char* buf = new char[length + 1];
	buf[0] = 0;

	if (fread(buf, length, 1, file) != 1) {
		delete[] buf;
		SetError(TIXML_ERROR_OPENING_FILE, 0, 0, TIXML_ENCODING_UNKNOWN);
		return false;
	}

	// Process the buffer in place to normalize new lines. (See comment above.)
	// Copies from the 'p' to 'q' pointer, where p can advance faster if
	// a newline-carriage return is hit.
	//
	// Wikipedia:
	// Systems based on ASCII or a compatible character set use either LF  (Line feed, '\n', 0x0A, 10 in decimal) or 
	// CR (Carriage return, '\r', 0x0D, 13 in decimal) individually, or CR followed by LF (CR+LF, 0x0D 0x0A)...
	//		* LF:    Multics, Unix and Unix-like systems (GNU/Linux, AIX, Xenix, Mac OS X, FreeBSD, etc.), BeOS, Amiga, RISC OS, and others
	//		* CR+LF: DEC RT-11 and most other early non-Unix, non-IBM OSes, CP/M, MP/M, DOS, OS/2, Microsoft Windows, Symbian OS
	//		* CR:    Commodore 8-bit machines, Apple II family, Mac OS up to version 9 and OS-9

	const char* p = buf;	// the read head
	char* q = buf;			// the write head
	const char CR = 0x0d;
	const char LF = 0x0a;

	buf[length] = 0;
	while (*p) {
		assert(p < (buf + length));
		assert(q <= (buf + length));
		assert(q <= p);

		if (*p == CR) {
			*q++ = LF;
			p++;
			if (*p == LF) {		// check for CR+LF (and skip LF)
				p++;
			}
		}
		else {
			*q++ = *p++;
		}
	}
	assert(q <= (buf + length));
	*q = 0;

	Parse(buf, 0, encoding);

	delete[] buf;
	return !Error();
}
bool xmlParser::TiXmlDocument::SaveFile(const char* filename) const
{
	// The old c stuff lives on...
	FILE* fp = TiXmlFOpen(filename, "w");
	if (fp)
	{
		bool result = SaveFile(fp);
		fclose(fp);
		return result;
	}
	return false;
}
bool xmlParser::TiXmlDocument::SaveFile(FILE* fp) const
{
	if (useMicrosoftBOM)
	{
		const unsigned char TIXML_UTF_LEAD_0 = 0xefU;
		const unsigned char TIXML_UTF_LEAD_1 = 0xbbU;
		const unsigned char TIXML_UTF_LEAD_2 = 0xbfU;

		fputc(TIXML_UTF_LEAD_0, fp);
		fputc(TIXML_UTF_LEAD_1, fp);
		fputc(TIXML_UTF_LEAD_2, fp);
	}
	Print(fp, 0);
	return (ferror(fp) == 0);
}
void xmlParser::TiXmlDocument::CopyTo(TiXmlDocument* target) const
{
	TiXmlNode::CopyTo(target);

	target->error = error;
	target->errorId = errorId;
	target->errorDesc = errorDesc;
	target->tabsize = tabsize;
	target->errorLocation = errorLocation;
	target->useMicrosoftBOM = useMicrosoftBOM;

	TiXmlNode* node = 0;
	for (node = firstChild; node; node = node->NextSibling())
	{
		target->LinkEndChild(node->Clone());
	}
}
xmlParser::TiXmlNode* xmlParser::TiXmlDocument::Clone() const
{
	TiXmlDocument* clone = new TiXmlDocument();
	if (!clone)
		return 0;

	CopyTo(clone);
	return clone;
}
void xmlParser::TiXmlDocument::Print(FILE* cfile, int depth) const
{
	assert(cfile);
	for (const TiXmlNode* node = FirstChild(); node; node = node->NextSibling())
	{
		node->Print(cfile, depth);
		fprintf(cfile, "\n");
	}
}
bool xmlParser::TiXmlDocument::Accept(xmlParser::TiXmlVisitor* visitor) const
{
	if (visitor->VisitEnter(*this))
	{
		for (const TiXmlNode* node = FirstChild(); node; node = node->NextSibling())
		{
			if (!node->Accept(visitor))
				break;
		}
	}
	return visitor->VisitExit(*this);
}
#pragma endregion

#pragma region(attribute)
const xmlParser::TiXmlAttribute* xmlParser::TiXmlAttribute::Next() const
{
	// We are using knowledge of the sentinel. The sentinel
	// have a value or name.
	if (next->value.empty() && next->name.empty())
		return 0;
	return next;
}
const xmlParser::TiXmlAttribute* xmlParser::TiXmlAttribute::Previous() const
{
	// We are using knowledge of the sentinel. The sentinel
	// have a value or name.
	if (prev->value.empty() && prev->name.empty())
		return 0;
	return prev;
}
void xmlParser::TiXmlAttribute::Print(FILE* cfile, int /*depth*/, TiXmlString* str) const
{
	TiXmlString n, v;

	EncodeString(name, &n);
	EncodeString(value, &v);

	if (value.find('\"') == TiXmlString::npos) {
		if (cfile) {
			fprintf(cfile, "%s=\"%s\"", n.c_str(), v.c_str());
		}
		if (str) {
			(*str) += n; (*str) += "=\""; (*str) += v; (*str) += "\"";
		}
	}
	else {
		if (cfile) {
			fprintf(cfile, "%s='%s'", n.c_str(), v.c_str());
		}
		if (str) {
			(*str) += n; (*str) += "='"; (*str) += v; (*str) += "'";
		}
	}
}
int xmlParser::TiXmlAttribute::QueryIntValue(int* ival) const
{
	if (sscanf_s(value.c_str(), "%d", ival) == 1)
		return TIXML_SUCCESS;
	return TIXML_WRONG_TYPE;
}
int xmlParser::TiXmlAttribute::QueryDoubleValue(double* dval) const
{
	if (sscanf_s(value.c_str(), "%lf", dval) == 1)
		return TIXML_SUCCESS;
	return TIXML_WRONG_TYPE;
}
void xmlParser::TiXmlAttribute::SetIntValue(int _value)
{
	char buf[64];
#if defined(_snprintf_s)		
	_snprintf_s(buf, sizeof(buf), "%d", _value);
#else
	sprintf_s(buf, 64, "%d", _value);
#endif
	SetValue(buf);
}
void xmlParser::TiXmlAttribute::SetDoubleValue(double _value)
{
	char buf[256];
#if defined(_snprintf_s)		
	_snprintf_s(buf, sizeof(buf), "%g", _value);
#else
	sprintf_s(buf, 256, "%g", _value);
#endif
	SetValue(buf);
}
int xmlParser::TiXmlAttribute::IntValue() const
{
	return atoi(value.c_str());
}
double  xmlParser::TiXmlAttribute::DoubleValue() const
{
	return atof(value.c_str());
}
const char* xmlParser::TiXmlAttribute::Parse(const char* p, TiXmlParsingData* data, TiXmlEncoding encoding)
{
	p = SkipWhiteSpace(p, encoding);
	if (!p || !*p) return 0;

	if (data)
	{
		data->Stamp(p, encoding);
		location = data->Cursor();
	}
	// Read the name, the '=' and the value.
	const char* pErr = p;
	p = ReadName(p, &name, encoding);
	if (!p || !*p)
	{
		if (document) document->SetError(TIXML_ERROR_READING_ATTRIBUTES, pErr, data, encoding);
		return 0;
	}
	p = SkipWhiteSpace(p, encoding);
	if (!p || !*p || *p != '=')
	{
		if (document) document->SetError(TIXML_ERROR_READING_ATTRIBUTES, p, data, encoding);
		return 0;
	}

	++p;	// skip '='
	p = SkipWhiteSpace(p, encoding);
	if (!p || !*p)
	{
		if (document) document->SetError(TIXML_ERROR_READING_ATTRIBUTES, p, data, encoding);
		return 0;
	}

	const char* end;
	const char SINGLE_QUOTE = '\'';
	const char DOUBLE_QUOTE = '\"';

	if (*p == SINGLE_QUOTE)
	{
		++p;
		end = "\'";		// single quote in string
		p = ReadText(p, &value, false, end, false, encoding);
	}
	else if (*p == DOUBLE_QUOTE)
	{
		++p;
		end = "\"";		// double quote in string
		p = ReadText(p, &value, false, end, false, encoding);
	}
	else
	{
		// All attribute values should be in single or double quotes.
		// But this is such a common error that the parser will try
		// its best, even without them.
		value = "";
		while (p && *p											// existence
			&& !IsWhiteSpace(*p)								// whitespace
			&& *p != '/' && *p != '>')							// tag end
		{
			if (*p == SINGLE_QUOTE || *p == DOUBLE_QUOTE) {
				// [ 1451649 ] Attribute values with trailing quotes not handled correctly
				// We did not have an opening quote but seem to have a 
				// closing one. Give up and throw an error.
				if (document) document->SetError(TIXML_ERROR_READING_ATTRIBUTES, p, data, encoding);
				return 0;
			}
			value += *p;
			++p;
		}
	}
	return p;
}
#pragma endregion

#pragma region(comment)
xmlParser::TiXmlComment::TiXmlComment(const TiXmlComment& copy) : TiXmlNode(TiXmlNode::TINYXML_COMMENT)
{
	copy.CopyTo(this);
}
xmlParser::TiXmlComment& xmlParser::TiXmlComment::operator=(const TiXmlComment& base)
{
	Clear();
	base.CopyTo(this);
	return *this;
}
void xmlParser::TiXmlComment::Print(FILE* cfile, int depth) const
{
	assert(cfile);
	for (int i = 0; i < depth; i++)
	{
		fprintf(cfile, "    ");
	}
	fprintf(cfile, "<!--%s-->", value.c_str());
}
void xmlParser::TiXmlComment::CopyTo(TiXmlComment* target) const
{
	TiXmlNode::CopyTo(target);
}
bool xmlParser::TiXmlComment::Accept(TiXmlVisitor* visitor) const
{
	return visitor->Visit(*this);
}
xmlParser::TiXmlNode* xmlParser::TiXmlComment::Clone() const
{
	TiXmlComment* clone = new TiXmlComment();

	if (!clone)
		return 0;

	CopyTo(clone);
	return clone;
}
const char* xmlParser::TiXmlComment::Parse(const char* p, TiXmlParsingData* data, TiXmlEncoding encoding)
{
	TiXmlDocument* document = GetDocument();
	value = "";

	p = SkipWhiteSpace(p, encoding);

	if (data)
	{
		data->Stamp(p, encoding);
		location = data->Cursor();
	}
	const char* startTag = "<!--";
	const char* endTag = "-->";

	if (!StringEqual(p, startTag, false, encoding))
	{
		if (document)
			document->SetError(TIXML_ERROR_PARSING_COMMENT, p, data, encoding);
		return 0;
	}
	p += strlen(startTag);

	// [ 1475201 ] TinyXML parses entities in comments
	// Oops - ReadText doesn't work, because we don't want to parse the entities.
	// p = ReadText( p, &value, false, endTag, false, encoding );
	//
	// from the XML spec:
	/*
	 [Definition: Comments may appear anywhere in a document outside other markup; in addition,
				  they may appear within the document type declaration at places allowed by the grammar.
				  They are not part of the document's character data; an XML processor MAY, but need not,
				  make it possible for an application to retrieve the text of comments. For compatibility,
				  the string "--" (double-hyphen) MUST NOT occur within comments.] Parameter entity
				  references MUST NOT be recognized within comments.

				  An example of a comment:

				  <!-- declarations for <head> & <body> -->
	*/

	value = "";
	// Keep all the white space.
	while (p && *p && !StringEqual(p, endTag, false, encoding))
	{
		value.append(p, 1);
		++p;
	}
	if (p && *p)
		p += strlen(endTag);

	return p;
}
#pragma endregion

#pragma region(text)
void xmlParser::TiXmlText::Print(FILE* cfile, int depth) const
{
	assert(cfile);
	if (cdata)
	{
		int i;
		fprintf(cfile, "\n");
		for (i = 0; i < depth; i++) {
			fprintf(cfile, "    ");
		}
		fprintf(cfile, "<![CDATA[%s]]>\n", value.c_str());	// unformatted output
	}
	else
	{
		TiXmlString buffer;
		EncodeString(value, &buffer);
		fprintf(cfile, "%s", buffer.c_str());
	}
}
void xmlParser::TiXmlText::CopyTo(TiXmlText* target) const
{
	TiXmlNode::CopyTo(target);
	target->cdata = cdata;
}
bool xmlParser::TiXmlText::Accept(TiXmlVisitor* visitor) const
{
	return visitor->Visit(*this);
}
xmlParser::TiXmlNode* xmlParser::TiXmlText::Clone() const
{
	TiXmlText* clone = 0;
	clone = new TiXmlText("");

	if (!clone)
		return 0;

	CopyTo(clone);
	return clone;
}
const char* xmlParser::TiXmlText::Parse(const char* p, TiXmlParsingData* data, TiXmlEncoding encoding)
{
	value = "";
	TiXmlDocument* document = GetDocument();

	if (data)
	{
		data->Stamp(p, encoding);
		location = data->Cursor();
	}

	const char* const startTag = "<![CDATA[";
	const char* const endTag = "]]>";

	if (cdata || StringEqual(p, startTag, false, encoding))
	{
		cdata = true;

		if (!StringEqual(p, startTag, false, encoding))
		{
			if (document)
				document->SetError(TIXML_ERROR_PARSING_CDATA, p, data, encoding);
			return 0;
		}
		p += strlen(startTag);

		// Keep all the white space, ignore the encoding, etc.
		while (p && *p
			&& !StringEqual(p, endTag, false, encoding)
			)
		{
			value += *p;
			++p;
		}

		TiXmlString dummy;
		p = ReadText(p, &dummy, false, endTag, false, encoding);
		return p;
	}
	else
	{
		bool ignoreWhite = true;

		const char* end = "<";
		p = ReadText(p, &value, ignoreWhite, end, false, encoding);
		if (p && *p)
			return p - 1;	// don't truncate the '<'
		return 0;
	}
}
bool xmlParser::TiXmlText::Blank() const
{
	for (unsigned i = 0; i < value.length(); i++)
		if (!IsWhiteSpace(value[i]))
			return false;
	return true;
}
#pragma endregion

#pragma region(declaration)
xmlParser::TiXmlDeclaration::TiXmlDeclaration(const char* _version, const char* _encoding, const char* _standalone)
	: TiXmlNode(TiXmlNode::TINYXML_DECLARATION)
{
	version = _version;
	encoding = _encoding;
	standalone = _standalone;
}
xmlParser::TiXmlDeclaration::TiXmlDeclaration(const TiXmlDeclaration& copy)
	: TiXmlNode(TiXmlNode::TINYXML_DECLARATION)
{
	copy.CopyTo(this);
}
xmlParser::TiXmlDeclaration& xmlParser::TiXmlDeclaration::operator=(const TiXmlDeclaration& copy)
{
	Clear();
	copy.CopyTo(this);
	return *this;
}
void xmlParser::TiXmlDeclaration::Print(FILE* cfile, int /*depth*/, TiXmlString* str) const
{
	if (cfile) fprintf(cfile, "<?xml ");
	if (str)	 (*str) += "<?xml ";

	if (!version.empty()) {
		if (cfile) fprintf(cfile, "version=\"%s\" ", version.c_str());
		if (str) { (*str) += "version=\""; (*str) += version; (*str) += "\" "; }
	}
	if (!encoding.empty()) {
		if (cfile) fprintf(cfile, "encoding=\"%s\" ", encoding.c_str());
		if (str) { (*str) += "encoding=\""; (*str) += encoding; (*str) += "\" "; }
	}
	if (!standalone.empty()) {
		if (cfile) fprintf(cfile, "standalone=\"%s\" ", standalone.c_str());
		if (str) { (*str) += "standalone=\""; (*str) += standalone; (*str) += "\" "; }
	}
	if (cfile) fprintf(cfile, "?>");
	if (str)	 (*str) += "?>";
}
void xmlParser::TiXmlDeclaration::CopyTo(TiXmlDeclaration* target) const
{
	TiXmlNode::CopyTo(target);

	target->version = version;
	target->encoding = encoding;
	target->standalone = standalone;
}
bool xmlParser::TiXmlDeclaration::Accept(TiXmlVisitor* visitor) const
{
	return visitor->Visit(*this);
}
xmlParser::TiXmlNode* xmlParser::TiXmlDeclaration::Clone() const
{
	TiXmlDeclaration* clone = new TiXmlDeclaration();

	if (!clone)
		return 0;

	CopyTo(clone);
	return clone;
}
const char* xmlParser::TiXmlDeclaration::Parse(const char* p, TiXmlParsingData* data, TiXmlEncoding _encoding)
{
	p = SkipWhiteSpace(p, _encoding);
	// Find the beginning, find the end, and look for
	// the stuff in-between.
	TiXmlDocument* document = GetDocument();
	if (!p || !*p || !StringEqual(p, "<?xml", true, _encoding))
	{
		if (document) document->SetError(TIXML_ERROR_PARSING_DECLARATION, 0, 0, _encoding);
		return 0;
	}
	if (data)
	{
		data->Stamp(p, _encoding);
		location = data->Cursor();
	}
	p += 5;

	version = "";
	encoding = "";
	standalone = "";

	while (p && *p)
	{
		if (*p == '>')
		{
			++p;
			return p;
		}

		p = SkipWhiteSpace(p, _encoding);
		if (StringEqual(p, "version", true, _encoding))
		{
			TiXmlAttribute attrib;
			p = attrib.Parse(p, data, _encoding);
			version = attrib.Value();
		}
		else if (StringEqual(p, "encoding", true, _encoding))
		{
			TiXmlAttribute attrib;
			p = attrib.Parse(p, data, _encoding);
			encoding = attrib.Value();
		}
		else if (StringEqual(p, "standalone", true, _encoding))
		{
			TiXmlAttribute attrib;
			p = attrib.Parse(p, data, _encoding);
			standalone = attrib.Value();
		}
		else
		{
			// Read over whatever it is.
			while (p && *p && *p != '>' && !IsWhiteSpace(*p))
				++p;
		}
	}
	return 0;
}
#pragma endregion

#pragma region(unknown)
void xmlParser::TiXmlUnknown::Print(FILE* cfile, int depth) const
{
	for (int i = 0; i < depth; i++)
		fprintf(cfile, "    ");
	fprintf(cfile, "<%s>", value.c_str());
}
void xmlParser::TiXmlUnknown::CopyTo(TiXmlUnknown* target) const
{
	TiXmlNode::CopyTo(target);
}
bool xmlParser::TiXmlUnknown::Accept(TiXmlVisitor* visitor) const
{
	return visitor->Visit(*this);
}
xmlParser::TiXmlNode* xmlParser::TiXmlUnknown::Clone() const
{
	TiXmlUnknown* clone = new TiXmlUnknown();

	if (!clone)
		return 0;

	CopyTo(clone);
	return clone;
}
const char* xmlParser::TiXmlUnknown::Parse(const char* p, TiXmlParsingData* data, TiXmlEncoding encoding)
{
	TiXmlDocument* document = GetDocument();
	p = SkipWhiteSpace(p, encoding);

	if (data)
	{
		data->Stamp(p, encoding);
		location = data->Cursor();
	}
	if (!p || !*p || *p != '<')
	{
		if (document) document->SetError(TIXML_ERROR_PARSING_UNKNOWN, p, data, encoding);
		return 0;
	}
	++p;
	value = "";

	while (p && *p && *p != '>')
	{
		value += *p;
		++p;
	}

	if (!p)
	{
		if (document)
			document->SetError(TIXML_ERROR_PARSING_UNKNOWN, 0, 0, encoding);
	}
	if (p && *p == '>')
		return p + 1;
	return p;
}
#pragma endregion

#pragma region(attribute set)
xmlParser::TiXmlAttributeSet::TiXmlAttributeSet()
{
	sentinel.next = &sentinel;
	sentinel.prev = &sentinel;
}
xmlParser::TiXmlAttributeSet::~TiXmlAttributeSet()
{
	assert(sentinel.next == &sentinel);
	assert(sentinel.prev == &sentinel);
}
void xmlParser::TiXmlAttributeSet::Add(TiXmlAttribute* addMe)
{
#ifdef TIXML_USE_STL
	assert(!Find(TiXmlString(addMe->Name())));	// Shouldn't be multiply adding to the set.
#else
	assert(!Find(addMe->Name()));	// Shouldn't be multiply adding to the set.
#endif

	addMe->next = &sentinel;
	addMe->prev = sentinel.prev;

	sentinel.prev->next = addMe;
	sentinel.prev = addMe;
}
void xmlParser::TiXmlAttributeSet::Remove(TiXmlAttribute* removeMe)
{
	TiXmlAttribute* node;

	for (node = sentinel.next; node != &sentinel; node = node->next)
	{
		if (node == removeMe)
		{
			node->prev->next = node->next;
			node->next->prev = node->prev;
			node->next = 0;
			node->prev = 0;
			return;
		}
	}
	assert(0);		// we tried to remove a non-linked attribute.
}
xmlParser::TiXmlAttribute* xmlParser::TiXmlAttributeSet::Find(const char* name) const
{
	for (TiXmlAttribute* node = sentinel.next; node != &sentinel; node = node->next)
	{
		if (strcmp(node->name.c_str(), name) == 0)
			return node;
	}
	return 0;
}
xmlParser::TiXmlAttribute* xmlParser::TiXmlAttributeSet::FindOrCreate(const char* _name)
{
	TiXmlAttribute* attrib = Find(_name);
	if (!attrib) {
		attrib = new TiXmlAttribute();
		Add(attrib);
		attrib->SetName(_name);
	}
	return attrib;
}
#pragma endregion

#pragma region(handle)
xmlParser::TiXmlHandle xmlParser::TiXmlHandle::FirstChild() const
{
	if (node)
	{
		TiXmlNode* child = node->FirstChild();
		if (child)
			return TiXmlHandle(child);
	}
	return TiXmlHandle(0);
}
xmlParser::TiXmlHandle xmlParser::TiXmlHandle::FirstChild(const char* value) const
{
	if (node)
	{
		TiXmlNode* child = node->FirstChild(value);
		if (child)
			return TiXmlHandle(child);
	}
	return TiXmlHandle(0);
}
xmlParser::TiXmlHandle xmlParser::TiXmlHandle::FirstChildElement() const
{
	if (node)
	{
		TiXmlElement* child = node->FirstChildElement();
		if (child)
			return TiXmlHandle(child);
	}
	return TiXmlHandle(0);
}
xmlParser::TiXmlHandle xmlParser::TiXmlHandle::FirstChildElement(const char* value) const
{
	if (node)
	{
		TiXmlElement* child = node->FirstChildElement(value);
		if (child)
			return TiXmlHandle(child);
	}
	return TiXmlHandle(0);
}
xmlParser::TiXmlHandle xmlParser::TiXmlHandle::Child(int count) const
{
	if (node)
	{
		int i;
		TiXmlNode* child = node->FirstChild();
		for (i = 0;
			child && i < count;
			child = child->NextSibling(), ++i)
		{
			// nothing
		}
		if (child)
			return TiXmlHandle(child);
	}
	return TiXmlHandle(0);
}
xmlParser::TiXmlHandle xmlParser::TiXmlHandle::Child(const char* value, int count) const
{
	if (node)
	{
		int i;
		TiXmlNode* child = node->FirstChild(value);
		for (i = 0;
			child && i < count;
			child = child->NextSibling(value), ++i)
		{
			// nothing
		}
		if (child)
			return TiXmlHandle(child);
	}
	return TiXmlHandle(0);
}
xmlParser::TiXmlHandle xmlParser::TiXmlHandle::ChildElement(int count) const
{
	if (node)
	{
		int i;
		TiXmlElement* child = node->FirstChildElement();
		for (i = 0;
			child && i < count;
			child = child->NextSiblingElement(), ++i)
		{
			// nothing
		}
		if (child)
			return TiXmlHandle(child);
	}
	return TiXmlHandle(0);
}
xmlParser::TiXmlHandle xmlParser::TiXmlHandle::ChildElement(const char* value, int count) const
{
	if (node)
	{
		int i;
		TiXmlElement* child = node->FirstChildElement(value);
		for (i = 0;
			child && i < count;
			child = child->NextSiblingElement(value), ++i)
		{
			// nothing
		}
		if (child)
			return TiXmlHandle(child);
	}
	return TiXmlHandle(0);
}
#pragma endregion

#pragma region(print)
bool xmlParser::TiXmlPrinter::VisitEnter(const TiXmlDocument&)
{
	return true;
}
bool xmlParser::TiXmlPrinter::VisitExit(const TiXmlDocument&)
{
	return true;
}
bool xmlParser::TiXmlPrinter::VisitEnter(const TiXmlElement& element, const TiXmlAttribute* firstAttribute)
{
	DoIndent();
	buffer += "<";
	buffer += element.Value();

	for (const TiXmlAttribute* attrib = firstAttribute; attrib; attrib = attrib->Next())
	{
		buffer += " ";
		attrib->Print(0, 0, &buffer);
	}

	if (!element.FirstChild())
	{
		buffer += " />";
		DoLineBreak();
	}
	else
	{
		buffer += ">";
		if (element.FirstChild()->ToText()
			&& element.LastChild() == element.FirstChild()
			&& element.FirstChild()->ToText()->CDATA() == false)
		{
			simpleTextPrint = true;
			// no DoLineBreak()!
		}
		else
		{
			DoLineBreak();
		}
	}
	++depth;
	return true;
}
bool xmlParser::TiXmlPrinter::VisitExit(const TiXmlElement& element)
{
	--depth;
	if (!element.FirstChild())
	{
		// nothing.
	}
	else
	{
		if (simpleTextPrint)
		{
			simpleTextPrint = false;
		}
		else
		{
			DoIndent();
		}
		buffer += "</";
		buffer += element.Value();
		buffer += ">";
		DoLineBreak();
	}
	return true;
}
bool xmlParser::TiXmlPrinter::Visit(const TiXmlText& text)
{
	if (text.CDATA())
	{
		DoIndent();
		buffer += "<![CDATA[";
		buffer += text.Value();
		buffer += "]]>";
		DoLineBreak();
	}
	else if (simpleTextPrint)
	{
		TiXmlString str;
		TiXmlBase::EncodeString(text.ValueTStr(), &str);
		buffer += str;
	}
	else
	{
		DoIndent();
		TiXmlString str;
		TiXmlBase::EncodeString(text.ValueTStr(), &str);
		buffer += str;
		DoLineBreak();
	}
	return true;
}
bool xmlParser::TiXmlPrinter::Visit(const TiXmlDeclaration& declaration)
{
	DoIndent();
	declaration.Print(0, 0, &buffer);
	DoLineBreak();
	return true;
}
bool xmlParser::TiXmlPrinter::Visit(const TiXmlComment& comment)
{
	DoIndent();
	buffer += "<!--";
	buffer += comment.Value();
	buffer += "-->";
	DoLineBreak();
	return true;
}
bool xmlParser::TiXmlPrinter::Visit(const TiXmlUnknown& unknown)
{
	DoIndent();
	buffer += "<";
	buffer += unknown.Value();
	buffer += ">";
	DoLineBreak();
	return true;
}
#pragma endregion

#pragma region(base)
void xmlParser::TiXmlBase::ConvertUTF32ToUTF8(unsigned long input, char* output, int* length)
{
	const unsigned long BYTE_MASK = 0xBF;
	const unsigned long BYTE_MARK = 0x80;
	const unsigned long FIRST_BYTE_MARK[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

	if (input < 0x80)
		*length = 1;
	else if (input < 0x800)
		*length = 2;
	else if (input < 0x10000)
		*length = 3;
	else if (input < 0x200000)
		*length = 4;
	else
	{
		*length = 0; return;
	}	// This code won't covert this correctly anyway.

	output += *length;

	// Scary scary fall throughs.
	switch (*length)
	{
	case 4:
		--output;
		*output = (char)((input | BYTE_MARK) & BYTE_MASK);
		input >>= 6;
	case 3:
		--output;
		*output = (char)((input | BYTE_MARK) & BYTE_MASK);
		input >>= 6;
	case 2:
		--output;
		*output = (char)((input | BYTE_MARK) & BYTE_MASK);
		input >>= 6;
	case 1:
		--output;
		*output = (char)(input | FIRST_BYTE_MARK[*length]);
	}
}
int xmlParser::TiXmlBase::IsAlpha(unsigned char anyByte, TiXmlEncoding /*encoding*/)
{
	// This will only work for low-ascii, everything else is assumed to be a valid
	// letter. I'm not sure this is the best approach, but it is quite tricky trying
	// to figure out alhabetical vs. not across encoding. So take a very 
	// conservative approach.

//	if ( encoding == TIXML_ENCODING_UTF8 )
//	{
	if (anyByte < 127)
		return isalpha(anyByte);
	else
		return 1;	// What else to do? The unicode set is huge...get the english ones right.
//	}
//	else
//	{
//		return isalpha( anyByte );
//	}
}
int xmlParser::TiXmlBase::IsAlphaNum(unsigned char anyByte, TiXmlEncoding /*encoding*/)
{
	// This will only work for low-ascii, everything else is assumed to be a valid
	// letter. I'm not sure this is the best approach, but it is quite tricky trying
	// to figure out alhabetical vs. not across encoding. So take a very 
	// conservative approach.

//	if ( encoding == TIXML_ENCODING_UTF8 )
//	{
	if (anyByte < 127)
		return isalnum(anyByte);
	else
		return 1;	// What else to do? The unicode set is huge...get the english ones right.
//	}
//	else
//	{
//		return isalnum( anyByte );
//	}
}
const char* xmlParser::TiXmlBase::SkipWhiteSpace(const char* p, TiXmlEncoding encoding)
{
	if (!p || !*p)
	{
		return 0;
	}
	if (encoding == TIXML_ENCODING_UTF8)
	{
		while (*p)
		{
			const unsigned char* pU = (const unsigned char*)p;

			// Skip the stupid Microsoft UTF-8 Byte order marks
			if (*(pU + 0) == TIXML_UTF_LEAD_0
				&& *(pU + 1) == TIXML_UTF_LEAD_1
				&& *(pU + 2) == TIXML_UTF_LEAD_2)
			{
				p += 3;
				continue;
			}
			else if (*(pU + 0) == TIXML_UTF_LEAD_0
				&& *(pU + 1) == 0xbfU
				&& *(pU + 2) == 0xbeU)
			{
				p += 3;
				continue;
			}
			else if (*(pU + 0) == TIXML_UTF_LEAD_0
				&& *(pU + 1) == 0xbfU
				&& *(pU + 2) == 0xbfU)
			{
				p += 3;
				continue;
			}

			if (IsWhiteSpace(*p))		// Still using old rules for white space.
				++p;
			else
				break;
		}
	}
	else
	{
		while (*p && IsWhiteSpace(*p))
			++p;
	}

	return p;
}
const char* xmlParser::TiXmlBase::ReadName(const char* p, TiXmlString* name, TiXmlEncoding encoding)
{
	// Oddly, not supported on some comilers,
	//name->clear();
	// So use this:
	*name = "";
	assert(p);

	// Names start with letters or underscores.
	// Of course, in unicode, tinyxml has no idea what a letter *is*. The
	// algorithm is generous.
	//
	// After that, they can be letters, underscores, numbers,
	// hyphens, or colons. (Colons are valid ony for namespaces,
	// but tinyxml can't tell namespaces from names.)
	if (p && *p
		&& (IsAlpha((unsigned char)*p, encoding) || *p == '_'))
	{
		const char* start = p;
		while (p && *p
			&& (IsAlphaNum((unsigned char)*p, encoding)
				|| *p == '_'
				|| *p == '-'
				|| *p == '.'
				|| *p == ':'))
		{
			//(*name) += *p; // expensive
			++p;
		}
		if (p - start > 0) {
			name->assign(start, p - start);
		}
		return p;
	}
	return 0;
}
const char* xmlParser::TiXmlBase::GetEntity(const char* p, char* value, int* length, TiXmlEncoding encoding)
{
	// Presume an entity, and pull it out.
	TiXmlString ent;
	int i;
	*length = 0;

	if (*(p + 1) && *(p + 1) == '#' && *(p + 2))
	{
		unsigned long ucs = 0;
		ptrdiff_t delta = 0;
		unsigned mult = 1;

		if (*(p + 2) == 'x')
		{
			// Hexadecimal.
			if (!*(p + 3)) return 0;

			const char* q = p + 3;
			q = strchr(q, ';');

			if (!q || !*q) return 0;

			delta = q - p;
			--q;

			while (*q != 'x')
			{
				if (*q >= '0' && *q <= '9')
					ucs += mult * (*q - '0');
				else if (*q >= 'a' && *q <= 'f')
					ucs += mult * (*q - 'a' + 10);
				else if (*q >= 'A' && *q <= 'F')
					ucs += mult * (*q - 'A' + 10);
				else
					return 0;
				mult *= 16;
				--q;
			}
		}
		else
		{
			// Decimal.
			if (!*(p + 2)) return 0;

			const char* q = p + 2;
			q = strchr(q, ';');

			if (!q || !*q) return 0;

			delta = q - p;
			--q;

			while (*q != '#')
			{
				if (*q >= '0' && *q <= '9')
					ucs += mult * (*q - '0');
				else
					return 0;
				mult *= 10;
				--q;
			}
		}
		if (encoding == TIXML_ENCODING_UTF8)
		{
			// convert the UCS to UTF-8
			ConvertUTF32ToUTF8(ucs, value, length);
		}
		else
		{
			*value = (char)ucs;
			*length = 1;
		}
		return p + delta + 1;
	}

	// Now try to match it.
	for (i = 0; i < NUM_ENTITY; ++i)
	{
		if (strncmp(entity[i].str, p, entity[i].strLength) == 0)
		{
			assert(strlen(entity[i].str) == entity[i].strLength);
			*value = entity[i].chr;
			*length = 1;
			return (p + entity[i].strLength);
		}
	}

	// So it wasn't an entity, its unrecognized, or something like that.
	*value = *p;	// Don't put back the last one, since we return it!
	//*length = 1;	// Leave unrecognized entities - this doesn't really work.
					// Just writes strange XML.
	return p + 1;
}
bool xmlParser::TiXmlBase::StringEqual(const char* p, const char* tag, bool ignoreCase, TiXmlEncoding encoding)
{
	assert(p);
	assert(tag);
	if (!p || !*p)
	{
		assert(0);
		return false;
	}

	const char* q = p;

	if (ignoreCase)
	{
		while (*q && *tag && ToLower(*q, encoding) == ToLower(*tag, encoding))
		{
			++q;
			++tag;
		}

		if (*tag == 0)
			return true;
	}
	else
	{
		while (*q && *tag && *q == *tag)
		{
			++q;
			++tag;
		}

		if (*tag == 0)		// Have we found the end of the tag, and everything equal?
			return true;
	}
	return false;
}
const char* xmlParser::TiXmlBase::ReadText(const char* p, TiXmlString* text, bool trimWhiteSpace, const char* endTag, bool caseInsensitive, TiXmlEncoding encoding)
{
	*text = "";
	if (!trimWhiteSpace			// certain tags always keep whitespace
		|| !condenseWhiteSpace)	// if true, whitespace is always kept
	{
		// Keep all the white space.
		while (p && *p
			&& !StringEqual(p, endTag, caseInsensitive, encoding)
			)
		{
			int len;
			char cArr[4] = { 0, 0, 0, 0 };
			p = GetChar(p, cArr, &len, encoding);
			text->append(cArr, len);
		}
	}
	else
	{
		bool whitespace = false;

		// Remove leading white space:
		p = SkipWhiteSpace(p, encoding);
		while (p && *p
			&& !StringEqual(p, endTag, caseInsensitive, encoding))
		{
			if (*p == '\r' || *p == '\n')
			{
				whitespace = true;
				++p;
			}
			else if (IsWhiteSpace(*p))
			{
				whitespace = true;
				++p;
			}
			else
			{
				// If we've found whitespace, add it before the
				// new character. Any whitespace just becomes a space.
				if (whitespace)
				{
					(*text) += ' ';
					whitespace = false;
				}
				int len;
				char cArr[4] = { 0, 0, 0, 0 };
				p = GetChar(p, cArr, &len, encoding);
				if (len == 1)
					(*text) += cArr[0];	// more efficient
				else
					text->append(cArr, len);
			}
		}
	}
	if (p && *p)
		p += strlen(endTag);
	return (p && *p) ? p : 0;
}
void xmlParser::TiXmlBase::EncodeString(const TiXmlString& str, TiXmlString* outString)
{
	int i = 0;

	while (i < (int)str.length())
	{
		unsigned char c = (unsigned char)str[i];

		if (c == '&'
			&& i < ((int)str.length() - 2)
			&& str[i + 1] == '#'
			&& str[i + 2] == 'x')
		{
			// Hexadecimal character reference.
			// Pass through unchanged.
			// &#xA9;	-- copyright symbol, for example.
			//
			// The -1 is a bug fix from Rob Laveaux. It keeps
			// an overflow from happening if there is no ';'.
			// There are actually 2 ways to exit this loop -
			// while fails (error case) and break (semicolon found).
			// However, there is no mechanism (currently) for
			// this function to return an error.
			while (i < (int)str.length() - 1)
			{
				outString->append(str.c_str() + i, 1);
				++i;
				if (str[i] == ';')
					break;
			}
		}
		else if (c == '&')
		{
			outString->append(entity[0].str, entity[0].strLength);
			++i;
		}
		else if (c == '<')
		{
			outString->append(entity[1].str, entity[1].strLength);
			++i;
		}
		else if (c == '>')
		{
			outString->append(entity[2].str, entity[2].strLength);
			++i;
		}
		else if (c == '\"')
		{
			outString->append(entity[3].str, entity[3].strLength);
			++i;
		}
		else if (c == '\'')
		{
			outString->append(entity[4].str, entity[4].strLength);
			++i;
		}
		else if (c < 32)
		{
			// Easy pass at non-alpha/numeric/symbol
			// Below 32 is symbolic.
			char buf[32];

#if defined(_snprintf_s)		
			_snprintf_s(buf, sizeof(buf), "&#x%02X;", (unsigned)(c & 0xff));
#else
			sprintf_s(buf, 32, "&#x%02X;", (unsigned)(c & 0xff));
#endif		

			//*ME:	warning C4267: convert 'size_t' to 'int'
			//*ME:	Int-Cast to make compiler happy ...
			outString->append(buf, (int)strlen(buf));
			++i;
		}
		else
		{
			//char realc = (char) c;
			//outString->append( &realc, 1 );
			*outString += (char)c;	// somewhat more efficient function call.
			++i;
		}
	}
}
#pragma endregion

#pragma region(document)
const char* xmlParser::TiXmlDocument::Parse(const char* p, TiXmlParsingData* prevData, TiXmlEncoding encoding)
{
	ClearError();

	// Parse away, at the document level. Since a document
	// contains nothing but other tags, most of what happens
	// here is skipping white space.
	if (!p || !*p)
	{
		SetError(TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN);
		return 0;
	}

	// Note that, for a document, this needs to come
	// before the while space skip, so that parsing
	// starts from the pointer we are given.
	location.Clear();
	if (prevData)
	{
		location.row = prevData->cursor.row;
		location.col = prevData->cursor.col;
	}
	else
	{
		location.row = 0;
		location.col = 0;
	}
	TiXmlParsingData data(p, TabSize(), location.row, location.col);
	location = data.Cursor();

	if (encoding == TIXML_ENCODING_UNKNOWN)
	{
		// Check for the Microsoft UTF-8 lead bytes.
		const unsigned char* pU = (const unsigned char*)p;
		if (*(pU + 0) && *(pU + 0) == TIXML_UTF_LEAD_0
			&& *(pU + 1) && *(pU + 1) == TIXML_UTF_LEAD_1
			&& *(pU + 2) && *(pU + 2) == TIXML_UTF_LEAD_2)
		{
			encoding = TIXML_ENCODING_UTF8;
			useMicrosoftBOM = true;
		}
	}

	p = SkipWhiteSpace(p, encoding);
	if (!p)
	{
		SetError(TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN);
		return 0;
	}

	while (p && *p)
	{
		TiXmlNode* node = Identify(p, encoding);
		if (node)
		{
			p = node->Parse(p, &data, encoding);
			LinkEndChild(node);
		}
		else
		{
			break;
		}

		// Did we get encoding info?
		if (encoding == TIXML_ENCODING_UNKNOWN
			&& node->ToDeclaration())
		{
			TiXmlDeclaration* dec = node->ToDeclaration();
			const char* enc = dec->Encoding();
			assert(enc);

			if (*enc == 0)
				encoding = TIXML_ENCODING_UTF8;
			else if (StringEqual(enc, "UTF-8", true, TIXML_ENCODING_UNKNOWN))
				encoding = TIXML_ENCODING_UTF8;
			else if (StringEqual(enc, "UTF8", true, TIXML_ENCODING_UNKNOWN))
				encoding = TIXML_ENCODING_UTF8;	// incorrect, but be nice
			else
				encoding = TIXML_ENCODING_LEGACY;
		}

		p = SkipWhiteSpace(p, encoding);
	}

	// Was this empty?
	if (!firstChild) {
		SetError(TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, encoding);
		return 0;
	}

	// All is well.
	return p;
}
void xmlParser::TiXmlDocument::SetError(int err, const char* pError, TiXmlParsingData* data, TiXmlEncoding encoding)
{
	// The first error in a chain is more accurate - don't set again!
	if (error)
		return;

	assert(err > 0 && err < TIXML_ERROR_STRING_COUNT);
	error = true;
	errorId = err;
	errorDesc = errorString[errorId];

	errorLocation.Clear();
	if (pError && data)
	{
		data->Stamp(pError, encoding);
		errorLocation = data->Cursor();
	}
}
#pragma endregion

#pragma region(string)
void xmlParser::TiXmlString::reserve(size_type cap)
{
	if (cap > capacity())
	{
		TiXmlString tmp;
		tmp.init(length(), cap);
		memcpy(tmp.start(), data(), length());
		swap(tmp);
	}
}
xmlParser::TiXmlString& xmlParser::TiXmlString::assign(const char* str, size_type len)
{
	size_type cap = capacity();
	if (len > cap || cap > 3 * (len + 8))
	{
		TiXmlString tmp;
		tmp.init(len);
		memcpy(tmp.start(), str, len);
		swap(tmp);
	}
	else
	{
		memmove(start(), str, len);
		set_size(len);
	}
	return *this;
}
xmlParser::TiXmlString& xmlParser::TiXmlString::append(const char* str, size_type len)
{
	size_type newsize = length() + len;
	if (newsize > capacity())
	{
		reserve(newsize + capacity());
	}
	memmove(finish(), str, len);
	set_size(newsize);
	return *this;
}
#pragma endregion

#pragma region(parsing)
void xmlParser::TiXmlParsingData::Stamp(const char* now, TiXmlEncoding encoding)
{
	assert(now);

	// Do nothing if the tabsize is 0.
	if (tabsize < 1)
	{
		return;
	}

	// Get the current row, column.
	int row = cursor.row;
	int col = cursor.col;
	const char* p = stamp;
	assert(p);

	while (p < now)
	{
		// Treat p as unsigned, so we have a happy compiler.
		const unsigned char* pU = (const unsigned char*)p;

		// Code contributed by Fletcher Dunn: (modified by lee)
		switch (*pU) {
		case 0:
			// We *should* never get here, but in case we do, don't
			// advance past the terminating null character, ever
			return;

		case '\r':
			// bump down to the next line
			++row;
			col = 0;
			// Eat the character
			++p;

			// Check for \r\n sequence, and treat this as a single character
			if (*p == '\n') {
				++p;
			}
			break;

		case '\n':
			// bump down to the next line
			++row;
			col = 0;

			// Eat the character
			++p;

			// Check for \n\r sequence, and treat this as a single
			// character.  (Yes, this bizarre thing does occur still
			// on some arcane platforms...)
			if (*p == '\r') {
				++p;
			}
			break;

		case '\t':
			// Eat the character
			++p;

			// Skip to next tab stop
			col = (col / tabsize + 1) * tabsize;
			break;

		case TIXML_UTF_LEAD_0:
			if (encoding == TIXML_ENCODING_UTF8)
			{
				if (*(p + 1) && *(p + 2))
				{
					// In these cases, don't advance the column. These are
					// 0-width spaces.
					if (*(pU + 1) == TIXML_UTF_LEAD_1 && *(pU + 2) == TIXML_UTF_LEAD_2)
						p += 3;
					else if (*(pU + 1) == 0xbfU && *(pU + 2) == 0xbeU)
						p += 3;
					else if (*(pU + 1) == 0xbfU && *(pU + 2) == 0xbfU)
						p += 3;
					else
					{
						p += 3; ++col;
					}	// A normal character.
				}
			}
			else
			{
				++p;
				++col;
			}
			break;

		default:
			if (encoding == TIXML_ENCODING_UTF8)
			{
				// Eat the 1 to 4 byte utf8 character.
				int step = TiXmlBase::utf8ByteTable[*((const unsigned char*)p)];
				if (step == 0)
					step = 1;		// Error case from bad encoding, but handle gracefully.
				p += step;

				// Just advance one column, of course.
				++col;
			}
			else
			{
				++p;
				++col;
			}
			break;
		}
	}
	cursor.row = row;
	cursor.col = col;
	assert(cursor.row >= -1);
	assert(cursor.col >= -1);
	stamp = p;
	assert(stamp);
}
#pragma endregion

#pragma region(operator)
inline bool operator == (const xmlParser::TiXmlString& a, const xmlParser::TiXmlString& b)
{
	return    (a.length() == b.length())				// optimization on some platforms
		&& (strcmp(a.c_str(), b.c_str()) == 0);	// actual compare
}
inline bool operator < (const xmlParser::TiXmlString& a, const xmlParser::TiXmlString& b)
{
	return strcmp(a.c_str(), b.c_str()) < 0;
}

inline bool operator != (const xmlParser::TiXmlString& a, const xmlParser::TiXmlString& b) { return !(a == b); }
inline bool operator >  (const xmlParser::TiXmlString& a, const xmlParser::TiXmlString& b) { return b < a; }
inline bool operator <= (const xmlParser::TiXmlString& a, const xmlParser::TiXmlString& b) { return !(b < a); }
inline bool operator >= (const xmlParser::TiXmlString& a, const xmlParser::TiXmlString& b) { return !(a < b); }

inline bool operator == (const xmlParser::TiXmlString& a, const char* b) { return strcmp(a.c_str(), b) == 0; }
inline bool operator == (const char* a, const xmlParser::TiXmlString& b) { return b == a; }
inline bool operator != (const xmlParser::TiXmlString& a, const char* b) { return !(a == b); }
inline bool operator != (const char* a, const xmlParser::TiXmlString& b) { return !(b == a); }

xmlParser::TiXmlString operator + (const xmlParser::TiXmlString& a, const xmlParser::TiXmlString& b)
{
	xmlParser::TiXmlString tmp;
	tmp.reserve(a.length() + b.length());
	tmp += a;
	tmp += b;
	return tmp;
}
xmlParser::TiXmlString operator + (const xmlParser::TiXmlString& a, const char* b)
{
	xmlParser::TiXmlString tmp;
	xmlParser::TiXmlString::size_type b_len = static_cast<xmlParser::TiXmlString::size_type>(strlen(b));
	tmp.reserve(a.length() + b_len);
	tmp += a;
	tmp.append(b, b_len);
	return tmp;
}
xmlParser::TiXmlString operator + (const char* a, const xmlParser::TiXmlString& b)
{
	xmlParser::TiXmlString tmp;
	xmlParser::TiXmlString::size_type a_len = static_cast<xmlParser::TiXmlString::size_type>(strlen(a));
	tmp.reserve(a_len + b.length());
	tmp.append(a, a_len);
	tmp += b;
	return tmp;
}
#pragma endregion

#pragma region(variable initial)
bool xmlParser::TiXmlBase::condenseWhiteSpace = true;
const xmlParser::TiXmlString::size_type xmlParser::TiXmlString::npos = static_cast<TiXmlString::size_type>(-1);
xmlParser::TiXmlString::Rep xmlParser::TiXmlString::nullrep_ = { 0, 0, { '\0' } };
const int xmlParser::TiXmlBase::utf8ByteTable[256] =
{
	//	0	1	2	3	4	5	6	7	8	9	a	b	c	d	e	f
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x00
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x10
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x20
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x30
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x40
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x50
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x60
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x70	End of ASCII range
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x80 0x80 to 0xc1 invalid
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x90 
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0xa0 
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0xb0 
		1,	1,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	// 0xc0 0xc2 to 0xdf 2 byte
		2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	// 0xd0
		3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	// 0xe0 0xe0 to 0xef 3 byte
		4,	4,	4,	4,	4,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1	// 0xf0 0xf0 to 0xf4 4 byte, 0xf5 and higher invalid
};
xmlParser::TiXmlBase::Entity xmlParser::TiXmlBase::entity[TiXmlBase::NUM_ENTITY] =
{
	{ "&amp;",  5, '&' },
	{ "&lt;",   4, '<' },
	{ "&gt;",   4, '>' },
	{ "&quot;", 6, '\"' },
	{ "&apos;", 6, '\'' }
};
const char* xmlParser::TiXmlBase::errorString[xmlParser::TiXmlBase::TIXML_ERROR_STRING_COUNT] =
{
	"No error",
	"Error",
	"Failed to open file",
	"Error parsing Element.",
	"Failed to read Element name",
	"Error reading Element value.",
	"Error reading Attributes.",
	"Error: empty tag.",
	"Error reading end tag.",
	"Error parsing Unknown.",
	"Error parsing Comment.",
	"Error parsing Declaration.",
	"Error document empty.",
	"Error null (0) or unexpected EOF found in input stream.",
	"Error parsing CDATA.",
	"Error when TiXmlDocument added to document, because TiXmlDocument can only be at the root.",
};
#pragma endregion

#pragma region(CS extern)
FFileParser* CreateParser(char* filePath, int mode)
{
	return new FFileParser(filePath, mode);
}
const char* ParserGetString(FFileParser* file, char* cmd, char* defData)
{
	char* ret = file->GetString(cmd, defData);
	return ret;
}
int ParserGetInt(FFileParser* file, char* cmd, int defData)
{
	int ret = file->GetInt(cmd, defData);
	return ret;
}
double ParserGetDouble(FFileParser* file, char* cmd, double defData)
{
	double ret = file->GetDouble(cmd, defData);
	return ret;
}
void ParserSetString(FFileParser* file, char* cmd, char* value)
{
	file->SetString(cmd, value);
}
void ParserSetInt(FFileParser* file, char* cmd, int value)
{
	file->SetInt(cmd, value);
}
void ParserSetDouble(FFileParser* file, char* cmd, double value)
{
	file->SetDouble(cmd, value);
}
#pragma endregion