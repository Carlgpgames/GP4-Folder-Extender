
#include "pch.h"
#include <windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <malloc.h>

void CreateExampleIniFile(const char* iniFilePath) {
    std::ofstream iniFile(iniFilePath);
    iniFile << "[content]\n";
    iniFile << "folder1=cars\n"; // folders are relative to the gp4 folder path
    iniFile << "folder2=MAPS\\RESOURCES\n"; // with added "maps\resources" and "cars" folders, gp4 first reads and uses ALL files from these local directories, then it uses (possible) additional files from WAD-files
    iniFile << "file1=driver9_1.tex\n";
    iniFile << "file2=driver9_2.tex\n";
    iniFile << "file3=driver10_1.tex\n";
    iniFile << "file4=driver10_2.tex\n";
    iniFile << "file5=driver21_1.tex\n";
    iniFile << "file6=driver21_2.tex\n";
    iniFile << "file7=cockpit_damage.tex\n";
    iniFile << "file8=hi_cockpit_damage.tex\n";
    iniFile << "file9=dial.tex\n";
    iniFile << "file10=inner_dial.tex\n";
    iniFile << "file11=menu.gpm\n";
    iniFile << "file12=gp2001_english.gps\n";
    iniFile << "file13=gp2001_deutsch.gps\n";
    iniFile << "file14=gp2001_espanol.gps\n";
    iniFile << "file15=gp2001_francais.gps\n";
    iniFile << "file16=gp2001_italiano.gps\n";
    iniFile << "file17=29.gpi\n";
    iniFile << "file18=digital.tga\n";
    iniFile << "file19=tvoverlay.tex\n";
    iniFile.close();
    OutputDebugStringA("FolderExtender: Sample FolderContent.ini created\n");
}

void PatchAddress(LPVOID address, BYTE* patch, SIZE_T size) {
    DWORD oldProtect;
    if (VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy(address, patch, size);
        VirtualProtect(address, size, oldProtect, &oldProtect);
        OutputDebugStringA("FolderExtender: Memory patch successful\n");
    }
    else {
        OutputDebugStringA("FolderExtender: Error in VirtualProtect during patch\n");
    }
}

void GetFilesFromFolder(const std::string& folderPath, std::vector<std::string>& files) {
    WIN32_FIND_DATAA findFileData;
    std::string searchPath = folderPath + "\\*";
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        OutputDebugStringA(("FolderExtender: Failed to open folder: " + folderPath + "\n").c_str());
        return;
    }

    do {
        const std::string fileOrFolderName = findFileData.cFileName;

        if (fileOrFolderName == "." || fileOrFolderName == "..") {
            continue;
        }

        std::string fullPath = folderPath + "\\" + fileOrFolderName;

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            // Navigate recursively through subfolders
            OutputDebugStringA(("FolderExtender: Entering folder: " + fullPath + "\n").c_str());
            GetFilesFromFolder(fullPath, files);
        }
        else {
            files.push_back(fileOrFolderName); // filename only
            OutputDebugStringA(("FolderExtender: Adding file from folder: " + fileOrFolderName + "\n").c_str());
        }

    } while (FindNextFileA(hFind, &findFileData) != 0);

    FindClose(hFind);
}


DWORD WINAPI MainThread(LPVOID param) {
    Sleep(5000);

    char iniFilePath[MAX_PATH];
    GetModuleFileNameA(NULL, iniFilePath, MAX_PATH);
    std::string::size_type pos = std::string(iniFilePath).find_last_of("\\/");
    std::string folderPath = std::string(iniFilePath).substr(0, pos);
    std::string iniFile = folderPath + "\\FolderContent.ini";

    if (GetFileAttributesA(iniFile.c_str()) == INVALID_FILE_ATTRIBUTES) {
        CreateExampleIniFile(iniFile.c_str());
    }
    else {
        OutputDebugStringA("FolderExtender: FolderContent.ini exists\n");
    }

    char buffer[256];
    std::vector<std::string> files;
    std::vector<std::string> folders;

    // read folders
    for (int i = 1;; i++) {
        std::ostringstream key;
        key << "folder" << i;
        GetPrivateProfileStringA("content", key.str().c_str(), "", buffer, sizeof(buffer), iniFile.c_str());
        if (strlen(buffer) == 0) {
            break;
        }
        std::string folderPathRelative = folderPath + "\\" + buffer;
        OutputDebugStringA(("FolderExtender: Adding folder to search: " + folderPathRelative + "\n").c_str());
        folders.push_back(folderPathRelative);
    }

    // Read files from the specified folders and subfolders
    for (const auto& folder : folders) {
        OutputDebugStringA(("FolderExtender: Searching in folder: " + folder + "\n").c_str());
        GetFilesFromFolder(folder, files);
    }

    // Optional: Additionally read files explicitly specified in the INI file
    for (int i = 1;; i++) {
        std::ostringstream key;
        key << "file" << i;
        GetPrivateProfileStringA("content", key.str().c_str(), "", buffer, sizeof(buffer), iniFile.c_str());
        if (strlen(buffer) == 0) {
            break;
        }
        OutputDebugStringA(("FolderExtender: Adding file from FolderContent.ini: " + std::string(buffer) + "\n").c_str());
        files.push_back(buffer);
    }

    for (const auto& file : files) {
        OutputDebugStringA(("FolderExtender: File entry: " + file + "\n").c_str());
    }

    if (files.empty()) {
        OutputDebugStringA("FolderExtender: No valid entries found in FolderContent.ini\n");
        return 1;
    }

    std::ostringstream outputString;
    outputString << files.size();
    OutputDebugStringA(("FolderExtender: Number of Files: " + outputString.str() + "\n").c_str());

    char** newTable = new char* [files.size()];
    for (int i = 0; i < files.size(); i++) {
        newTable[i] = _strdup(files[i].c_str());
    }

    outputString.str(std::string());
    outputString << std::hex << std::showbase << newTable;
    OutputDebugStringA(("FolderExtender: Address of new table: " + outputString.str() + "\n").c_str());

    PatchAddress((LPVOID)0x0046B1C1, (BYTE*)&newTable, sizeof(newTable));
    PatchAddress((LPVOID)0x0046AF48, (BYTE*)&newTable, sizeof(newTable));

    void* endLoopAddress = newTable + files.size();

    outputString.str(std::string());
    outputString << std::hex << std::showbase << endLoopAddress;
    OutputDebugStringA(("FolderExtender: Address of end of loop: " + outputString.str() + "\n").c_str());

    PatchAddress((LPVOID)0x0046B1EA, (BYTE*)&endLoopAddress, sizeof(endLoopAddress));
    PatchAddress((LPVOID)0x0046AF75, (BYTE*)&endLoopAddress, sizeof(endLoopAddress));

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}