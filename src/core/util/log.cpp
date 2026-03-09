#include "pch.h"
#include <ctime>
#include <Windows.h>
#include <iostream>
#include <shlobj.h>
#include "log.h"
#include "core/mem/memory.h"
#include "core/math/math.h"

static HANDLE hConsoleStream = INVALID_HANDLE_VALUE;
static HANDLE hFileStream = INVALID_HANDLE_VALUE;

#pragma region log_main
bool console::AttachConsole(const wchar_t* wszWindowTitle)
{
	if (::AllocConsole() != TRUE)
		return false;

	if (hConsoleStream = ::CreateFileW(L"CONOUT$", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr); hConsoleStream == INVALID_HANDLE_VALUE)
		return false;

	if (::SetStdHandle(STD_OUTPUT_HANDLE, hConsoleStream) != TRUE)
		return false;

	if (::SetConsoleTitleW(wszWindowTitle) != TRUE)
		return false;

	return true;
}

void console::DetachConsole()
{
	::CloseHandle(hConsoleStream);

	if (::FreeConsole() != TRUE)
		return;

	if (const HWND hConsoleWindow = ::GetConsoleWindow(); hConsoleWindow != nullptr)
		::PostMessageW(hConsoleWindow, WM_CLOSE, 0U, 0L);
}

bool console::OpenFile(const wchar_t* wszFileName)
{
	wchar_t wszFilePath[MAX_PATH];
	wchar_t wszDocumentsPath[MAX_PATH];

	HRESULT result = ::SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, wszDocumentsPath);

	if (!SUCCEEDED(result))
		return false;

	wcscpy_s(wszFilePath, wszDocumentsPath);
	wcscat_s(wszFilePath, L"\\.chudbase\\");
	//when i recode this whole thing i'll make a fileutil for this instead of doing it in here
	::CreateDirectoryW(wszFilePath, nullptr);

	CRT::StringCat(wszFilePath, wszFileName);

	// @todo: append time/date to filename and always keep up to 3 files, otherwise delete with lowest date
	if (hFileStream = ::CreateFileW(wszFilePath, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr); hFileStream == INVALID_HANDLE_VALUE)
		return false;

	::WriteFile(hFileStream, "\xEF\xBB\xBF", 3UL, nullptr, nullptr);

	return true;
}

void console::CloseFile()
{
	::CloseHandle(hFileStream);
}

void console::WriteMessage(const char* szMessage, const std::size_t nMessageLength)
{
#ifdef _DEBUG
	::WriteConsoleA(hConsoleStream, szMessage, nMessageLength, nullptr, nullptr);
#endif

::WriteFile(hFileStream, szMessage, nMessageLength, nullptr, nullptr);

}
#pragma endregion

#pragma region log_stream_control
console::Stream_t::ColorMarker_t console::SetColor(const LogColorFlags_t nColorFlags)
{
	return { nColorFlags };
}

console::Stream_t::PrecisionMarker_t console::SetPrecision(const int iPrecision)
{
	return { iPrecision };
}

console::Stream_t::ModeMarker_t console::AddFlags(const LogModeFlags_t nModeFlags)
{
	return { nModeFlags };
}

console::Stream_t::ModeMarker_t console::RemoveFlags(const LogModeFlags_t nModeFlags)
{
	return { static_cast<LogModeFlags_t>(nModeFlags | LOG_MODE_REMOVE) };
}
#pragma endregion

console::Stream_t& console::Stream_t::operator()(const ELogLevel nLevel, const char* szFileBlock)
{
	// reset previous flags
	nModeFlags = LOG_MODE_NONE;

	const char* szTypeBlock = nullptr;
	[[maybe_unused]] LogColorFlags_t nTypeColorFlags = CONSOLE_DEFAULT;

	switch (nLevel)
	{
	case LOG_INFO:
		szTypeBlock = "[info] ";
		nTypeColorFlags = CONSOLE_FORE_CYAN;
		break;
	case LOG_WARNING:
		szTypeBlock = "[warning] ";
		nTypeColorFlags = CONSOLE_FORE_YELLOW;
		break;
	case LOG_ERROR:
		szTypeBlock = "[error] ";
		nTypeColorFlags = CONSOLE_FORE_RED;
		break;
	default:
		break;
	}

	const std::time_t time = std::time(nullptr);
	std::tm timePoint;
	localtime_s(&timePoint, &time);

	char szTimeBuffer[32];
	const std::size_t nTimeSize = CRT::TimeToString(szTimeBuffer, sizeof(szTimeBuffer), "\n[%d-%m-%Y %T] ", &timePoint) - bFirstPrint;

#ifdef _DEBUG
	::SetConsoleTextAttribute(hConsoleStream, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	::WriteConsoleA(hConsoleStream, szTimeBuffer + bFirstPrint, nTimeSize, nullptr, nullptr);

	if (szFileBlock != nullptr)
	{
		::SetConsoleTextAttribute(hConsoleStream, FOREGROUND_INTENSITY);
		::WriteConsoleA(hConsoleStream, szFileBlock, CRT::StringLength(szFileBlock), nullptr, nullptr);
	}

	if (szTypeBlock != nullptr)
	{
		::SetConsoleTextAttribute(hConsoleStream, static_cast<WORD>(nTypeColorFlags));
		::WriteConsoleA(hConsoleStream, szTypeBlock, CRT::StringLength(szTypeBlock), nullptr, nullptr);
	}

	::SetConsoleTextAttribute(hConsoleStream, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
	::WriteFile(hFileStream, szTimeBuffer + bFirstPrint, nTimeSize, nullptr, nullptr);

	char szBlockBuffer[MAX_PATH] = { '\0' };
	char* szCurrentBlock = szBlockBuffer;

	if (szFileBlock != nullptr)
		szCurrentBlock = CRT::StringCat(szCurrentBlock, szFileBlock);

	if (szTypeBlock != nullptr)
		szCurrentBlock = CRT::StringCat(szCurrentBlock, szTypeBlock);

	if (szBlockBuffer[0] != '\0')
		::WriteFile(hFileStream, szBlockBuffer, static_cast<DWORD>(szCurrentBlock - szBlockBuffer), nullptr, nullptr);


	bFirstPrint = false;
	return *this;
}

console::Stream_t& console::Stream_t::operator<<(const ColorMarker_t colorMarker)
{
#ifdef _DEBUG
	::SetConsoleTextAttribute(hConsoleStream, static_cast<WORD>(colorMarker.nColorFlags));
#endif
	return *this;
}

console::Stream_t& console::Stream_t::operator<<(const PrecisionMarker_t precisionMarker)
{
	this->iPrecision = precisionMarker.iPrecision;

	return *this;
}

console::Stream_t& console::Stream_t::operator<<(const ModeMarker_t modeMarker)
{
#if defined(_DEBUG) || defined(CS_LOG_FILE)
	CS_ASSERT(nModeFlags == 0U || MATH::IsPowerOfTwo(nModeFlags & LOG_MODE_INT_FORMAT_MASK)); // used conflicting format flags

	if (modeMarker.nModeFlags & LOG_MODE_REMOVE)
		nModeFlags &= ~modeMarker.nModeFlags;
	else
		nModeFlags |= modeMarker.nModeFlags;
#endif
	return *this;
}

console::Stream_t& console::Stream_t::operator<<(const char* szMessage)
{
	WriteMessage(szMessage, CRT::StringLength(szMessage));

	return *this;
}

console::Stream_t& console::Stream_t::operator<<(const wchar_t* wszMessage)
{

	const std::size_t nMessageLength = CRT::StringLengthMultiByte(wszMessage);
	char* szMessage = static_cast<char*>(MEM_STACKALLOC(nMessageLength + 1U));
	CRT::StringUnicodeToMultiByte(szMessage, nMessageLength + 1U, wszMessage);

	WriteMessage(szMessage, nMessageLength);

	MEM_STACKFREE(szMessage);
	return *this;
}

console::Stream_t& console::Stream_t::operator<<(const bool bValue)
{
	const char* szBoolean = ((nModeFlags & LOG_MODE_BOOL_ALPHA) ? (bValue ? "true" : "false") : (bValue ? "1" : "0"));
	const std::size_t nBooleanLength = CRT::StringLength(szBoolean);

	WriteMessage(szBoolean, nBooleanLength);

	return *this;
}
