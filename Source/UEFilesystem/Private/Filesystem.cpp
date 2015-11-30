#include "UEFilesystemPrivatePCH.h"
#include "Filesystem.h"

#if defined _MSC_VER && _MSC_VER < 1900
#error old compiler version
#endif

#pragma warning(default : 4302 4706 4800)

DEFINE_LOG_CATEGORY(Filesystem)

namespace fs = std::tr2::sys;

namespace
{
	template<typename Char = char>
	inline typename std::enable_if<std::is_same<Char, TCHAR>::value, const Char *>::type Str_char_2_TCHAR(const char *str)
	{
		return str;
	}

	template<typename Char>
	class c_str
	{
		std::basic_string<Char> str;

	public:
		c_str(decltype(str) && str) noexcept : str(std::move(str)) {}
		operator const Char *() const noexcept{ return str.c_str(); }
	};

	template<typename Char = char>
	typename std::enable_if<!std::is_same<Char, TCHAR>::value, c_str<TCHAR>>::type Str_char_2_TCHAR(const char *str)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<TCHAR>> converter;
		return converter.from_bytes(str);
	}
}

// Sets default values
AFilesystem::AFilesystem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

#define FAIL_CREATE_DIR_MSG_PREFIX TEXT("Fail to create directory \"%s\"")
#define FAIL_REMOVE_DIR_MSG_PREFIX TEXT("Fail to remove directory \"%s\"")

void AFilesystem::CreateDirectory(const FString &path) const
{
	try
	{
		if (fs::create_directories(fs::path(*path)))
			UE_LOG(Filesystem, Log, TEXT("Directory \"%s\" created successfully."), *path)
		else
			UE_LOG(Filesystem, Error, FAIL_CREATE_DIR_MSG_PREFIX TEXT("."), *path)
	}
	catch (const std::exception &error)
	{
		UE_LOG(Filesystem, Error, FAIL_CREATE_DIR_MSG_PREFIX TEXT(": %s."), *path, (const TCHAR *)Str_char_2_TCHAR(error.what()))
	}
}

void AFilesystem::Remove(const FString &srcPath, bool force) const
{
	try
	{
		const fs::path path(*srcPath);
		if (force)
		{
			const auto count = fs::remove_all(path);
			UE_LOG(Filesystem, Log, TEXT("Directory \"%s\" with all it's content has been successfully removed (%d items)."), *srcPath, count)
		}
		else if (fs::remove(path))
			UE_LOG(Filesystem, Log, TEXT("Directory \"%s\" has been successfully removed."), *srcPath)
		else
			UE_LOG(Filesystem, Error, FAIL_REMOVE_DIR_MSG_PREFIX TEXT("."), *srcPath)
	}
	catch (const std::exception &error)
	{
		UE_LOG(Filesystem, Error, FAIL_REMOVE_DIR_MSG_PREFIX TEXT(": %s."), *srcPath, (const TCHAR *)Str_char_2_TCHAR(error.what()))
	}
}

void AFilesystem::Rename(const FString &oldPath, const FString &newPath) const
{
	try
	{
		fs::rename(fs::path(*oldPath), fs::path(*newPath));
		UE_LOG(Filesystem, Log, TEXT("Directory \"%s\" has been successfully renamed to \"%s\"."), *oldPath, *newPath)
	}
	catch (const std::exception &error)
	{
		UE_LOG(Filesystem, Error, TEXT("Fail to rename directory \"%s\" to \"%s\": %s."), *oldPath, *newPath, (const TCHAR *)Str_char_2_TCHAR(error.what()))
	}
}

FString AFilesystem::CurrentPath() const
{
	try
	{
		return fs::current_path().string().c_str();
	}
	catch (const std::exception &error)
	{
		UE_LOG(Filesystem, Error, TEXT("Fail to get current path: %s."), (const TCHAR *)Str_char_2_TCHAR(error.what()))
		return "";
	}
}

FString AFilesystem::GamePath() const
{
	TCHAR path[MAX_PATH];
	if (!GetModuleFileName(GetModuleHandle(NULL), path, sizeof path))
		UE_LOG(Filesystem, Error, TEXT("Fail to get game path."))
	return path;
}

FString AFilesystem::GameDir(bool forceAbsolute) const
{
	if (forceAbsolute)
	{
		try
		{
			return fs::canonical(*GameDir(false)).string().c_str();
		}
		catch (const std::exception &error)
		{
			UE_LOG(Filesystem, Error, TEXT("Fail to get absolute game dir: %s. Keeping unmodified (possibly relative)."), (const TCHAR *)Str_char_2_TCHAR(error.what()))
			return GameDir(false);
		}
	}
	else
		return FPaths::GameDir();
}

void AFilesystem::Print(const FString &path) const
{
	const int status = (const int)ShellExecute(0, TEXT("print"), (TEXT('"') + std::basic_string<TCHAR>(*path) + TEXT('"')).c_str(), NULL, NULL, SW_SHOWNORMAL);
	if (status > 32)
		UE_LOG(Filesystem, Log, TEXT("Printing file \"%s\"..."), *path)
	else
	{
		const TCHAR *msg = [status]
		{
			switch (status)
			{
			case 0:							return TEXT("OS is out of memory or resources");
			case ERROR_FILE_NOT_FOUND:		return TEXT("file not found");
			case ERROR_PATH_NOT_FOUND:		return TEXT("path not found");
			case ERROR_BAD_FORMAT:			return TEXT("bad format");
			case SE_ERR_ACCESSDENIED:		return TEXT("access denied");
			case SE_ERR_ASSOCINCOMPLETE:	return TEXT("association is incomplete or invalid");
			case SE_ERR_DDEBUSY:			return TEXT("DDE transaction busy");
			case SE_ERR_DDEFAIL:			return TEXT("DDE transaction failed");
			case SE_ERR_DDETIMEOUT:			return TEXT("DDE transaction timed out");
			case SE_ERR_DLLNOTFOUND:		return TEXT("DLL not found");
			case SE_ERR_NOASSOC:			return TEXT("no association (file is not printable)");
			case SE_ERR_OOM:				return TEXT("out of memory");
			case SE_ERR_SHARE:				return TEXT("share violation");
			default:						return TEXT("unknown error");
			}
		}();
		UE_LOG(Filesystem, Error, TEXT("Fail to print file \"%s\": %s."), *path, msg)
	}
}