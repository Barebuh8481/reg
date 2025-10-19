#include <windows.h>
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>

#pragma comment(lib, "advapi32.lib")

using namespace std;

// Чтение строкового значения из реестра
string ReadRegString(HKEY rootKey, const char* subKey, const char* valueName) {
    HKEY key;
    if (RegOpenKeyExA(rootKey, subKey, 0, KEY_READ, &key) != ERROR_SUCCESS) {
        return "<Ошибка: ключ не найден>";
    }

    char buffer[1024];
    DWORD bufferSize = sizeof(buffer);
    LONG result = RegQueryValueExA(key, valueName, nullptr, nullptr, (LPBYTE)buffer, &bufferSize);
    RegCloseKey(key);

    if (result != ERROR_SUCCESS) {
        return "<Ошибка: значение не найдено>";
    }

    return string(buffer);
}

// Чтение даты установки ОС (значение InstallDate — Unix timestamp в секундах)
string GetInstallDate() {
    HKEY key;
    const char* subKey = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ, &key) != ERROR_SUCCESS) {
        return "<Ошибка: не удалось открыть ключ CurrentVersion>";
    }

    DWORD installTime = 0;
    DWORD size = sizeof(installTime);
    LONG result = RegQueryValueExA(key, "InstallDate", nullptr, nullptr, (LPBYTE)&installTime, &size);
    RegCloseKey(key);

    if (result != ERROR_SUCCESS) {
        return "<Ошибка: InstallDate не найден>";
    }

    // Преобразуем Unix timestamp в FILETIME
    ULONGLONG ull = Int32x32To64(installTime, 10000000ULL) + 116444736000000000ULL;
    FILETIME ft;
    ft.dwLowDateTime = (DWORD)ull;
    ft.dwHighDateTime = (DWORD)(ull >> 32);

    SYSTEMTIME st;
    if (!FileTimeToSystemTime(&ft, &st)) {
        return "<Ошибка: преобразование даты не удалось>";
    }

    ostringstream oss;
    oss << setfill('0')
        << st.wYear << "-"
        << setw(2) << st.wMonth << "-"
        << setw(2) << st.wDay << " "
        << setw(2) << st.wHour << ":"
        << setw(2) << st.wMinute << ":"
        << setw(2) << st.wSecond;
    return oss.str();
}

int main() {
    SetConsoleOutputCP(CP_UTF8); // Поддержка UTF-8 в консоли

    cout << "=== Информация из системного реестра Windows ===\n\n";

    // --- Операционная система ---
    string productName = ReadRegString(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "ProductName");
    string releaseId = ReadRegString(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "ReleaseId");
    string build = ReadRegString(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "CurrentBuild");
    string installDate = GetInstallDate();

    cout << "Операционная система: " << productName << "\n";
    cout << "Версия (Release ID): " << releaseId << "\n";
    cout << "Сборка (Build): " << build << "\n";
    cout << "Дата установки: " << installDate << "\n\n";

    // --- Процессор ---
    string cpu = ReadRegString(HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", "ProcessorNameString");
    cout << "Процессор: " << cpu << "\n\n";

    // --- Материнская плата ---
    string mbManufacturer = ReadRegString(HKEY_LOCAL_MACHINE,
        "SYSTEM\\HardwareConfig\\Current", "SystemManufacturer");
    string mbModel = ReadRegString(HKEY_LOCAL_MACHINE,
        "SYSTEM\\HardwareConfig\\Current", "SystemProductName");

    cout << "Производитель материнской платы: " << mbManufacturer << "\n";
    cout << "Модель материнской платы: " << mbModel << "\n";

    return 0;
}