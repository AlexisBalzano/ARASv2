#include "DataManager.h"

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <knownfolders.h>
#elif defined(__APPLE__) || defined(__linux__)
#include <dlfcn.h>
#include <cstdlib>
#endif

DataManager::DataManager()
{
	m_configPath = GetConfigPath();
}

std::filesystem::path DataManager::GetConfigPath()
{
#if defined(_WIN32)
	PWSTR path = nullptr;
	HRESULT hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &path);
	std::filesystem::path documentsPath;
	if (SUCCEEDED(hr)) {
		documentsPath = path;
		CoTaskMemFree(path);
	}
	return documentsPath / "Aras";
#elif defined(__APPLE__) || defined(__linux__)
	const char* homeDir = std::getenv("HOME");
	if (homeDir) {
		return std::filesystem::path(homeDir) / "Documents" / "Aras";
	}
	return std::filesystem::path(); // Return empty path if HOME is not set
#else
	return std::filesystem::path(); // Return an empty path for unsupported platforms
#endif
}