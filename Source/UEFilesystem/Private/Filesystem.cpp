#include "UEFilesystemPrivatePCH.h"
#include "Filesystem.h"

#if defined _MSC_VER && _MSC_VER < 1900
#define noexcept
#endif

#pragma warning(default : 4706 4800)

DEFINE_LOG_CATEGORY(Filesystem)

namespace filesystem = std::tr2::sys;

namespace
{
	template<typename Char>
	struct PathTraits;

	template<>
	struct PathTraits<char>
	{
		typedef filesystem::path_traits type;
	};

	template<>
	struct PathTraits<wchar_t>
	{
		typedef filesystem::wpath_traits type;
	};

	typedef filesystem::basic_path<std::basic_string<TCHAR>, PathTraits<TCHAR>::type> Path;

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
		if (filesystem::create_directories(Path(*path)))
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
		const Path path(*srcPath);
		if (force)
		{
			const auto count = filesystem::remove_all(path);
			UE_LOG(Filesystem, Log, TEXT("Directory \"%s\" with all it's content has been successfully removed (%d items)."), *srcPath, count)
		}
		else if (filesystem::remove(path))
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
		filesystem::rename(Path(*oldPath), Path(*newPath));
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
		return filesystem::current_path<Path>().string().c_str();
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
			return filesystem::system_complete(Path(*GameDir(false))).string().c_str();
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