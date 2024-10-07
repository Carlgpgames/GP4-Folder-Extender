#include <windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <malloc.h>
#include "GP4MemLib/GP4MemLib.h"
#include "IniLib/IniLib.h"

using namespace GP4MemLib;
using namespace IniLib;

void CreateExampleIniFile(const char* iniFilePath) {
    IniFile iniFile;

    iniFile["Folders"]["Folder1"] = "cars";
    iniFile["Folders"]["Folder2"] = "MAPS\\RESOURCES";

    iniFile["Files"]["File1"] = "driver9_1.tex";
    iniFile["Files"]["File2"] = "driver9_2.tex";
    iniFile["Files"]["File3"] = "driver10_1.tex";
    iniFile["Files"]["File4"] = "driver10_2.tex";
    iniFile["Files"]["File5"] = "driver21_1.tex";
    iniFile["Files"]["File6"] = "driver21_2.tex";
    iniFile["Files"]["File7"] = "cockpit_damage.tex";
    iniFile["Files"]["File8"] = "hi_cockpit_damage.tex";
    iniFile["Files"]["File9"] = "dial.tex";
    iniFile["Files"]["File10"] = "inner_dial.tex";
    iniFile["Files"]["File11"] = "menu.gpm";
    iniFile["Files"]["File12"] = "gp2001_english.gps";
    iniFile["Files"]["File13"] = "gp2001_deutsch.gps";
    iniFile["Files"]["File14"] = "gp2001_espanol.gps";
    iniFile["Files"]["File15"] = "gp2001_francais.gps";
    iniFile["Files"]["File16"] = "gp2001_italiano.gps";
    iniFile["Files"]["File17"] = "29.gpi";
    iniFile["Files"]["File18"] = "digital.tga";
    iniFile["Files"]["File19"] = "tvoverlay.tex";

    iniFile.save(iniFilePath);

    OutputDebugStringA("FolderExtender: Sample FolderContent.ini created\n");
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
    //Sleep(5000);

    char iniFilePath[MAX_PATH];
    GetModuleFileNameA(NULL, iniFilePath, MAX_PATH);
    std::string::size_type pos = std::string(iniFilePath).find_last_of("\\/");
    std::string folderPath = std::string(iniFilePath).substr(0, pos);
    std::string iniFileFQN = folderPath + "\\FolderContent.ini";

    IniLib::IniFile iniFile;

    iniFile.load(iniFileFQN);

    if (GetFileAttributesA(iniFileFQN.c_str()) == INVALID_FILE_ATTRIBUTES) {
        CreateExampleIniFile(iniFileFQN.c_str());
    }
    else {
        OutputDebugStringA("FolderExtender: FolderContent.ini exists\n");
    }

    std::vector<std::string> files;
    std::vector<std::string> folders;

    // read folders
    for (size_t i = 1; i <= iniFile["Folders"].keyCount(); i++)
    {
        std::ostringstream key;
        key << "Folder" << i;

        std::string folderPathRelative = folderPath + "\\" + iniFile["Folders"][key.str()].getAs<std::string>();
        OutputDebugStringA(("FolderExtender: Adding folder to search: " + folderPathRelative + "\n").c_str());
        folders.push_back(folderPathRelative);
    }

    // Read files from the specified folders and subfolders
    for (const auto& folder : folders) {
        OutputDebugStringA(("FolderExtender: Searching in folder: " + folder + "\n").c_str());
        GetFilesFromFolder(folder, files);
    }

    // Optional: Additionally read files explicitly specified in the INI file
    for (size_t i = 1; i <= iniFile["Files"].keyCount(); i++)
    {
        std::ostringstream key;
        key << "File" << i;

        std::string fileName = iniFile["Files"][key.str()].getAs<std::string>();

        OutputDebugStringA(("FolderExtender: Adding file from FolderContent.ini: " + fileName + "\n").c_str());
        files.push_back(fileName);
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

    char** newTable = new char* [files.size()*MAX_PATH];
    for (size_t i = 0; i < files.size(); i++) {
        newTable[i] = _strdup(files[i].c_str());
    }

    outputString.str(std::string());
    outputString << std::hex << std::showbase << newTable;
    OutputDebugStringA(("FolderExtender: Address of new table: " + outputString.str() + "\n").c_str());

    MemUtils::patchAddress((LPVOID)0x0046B1C1, (BYTE*)&newTable, sizeof(newTable));
    MemUtils::patchAddress((LPVOID)0x0046AF48, (BYTE*)&newTable, sizeof(newTable));

    void* endLoopAddress = newTable + files.size();

    outputString.str(std::string());
    outputString << std::hex << std::showbase << endLoopAddress;
    OutputDebugStringA(("FolderExtender: Address of end of loop: " + outputString.str() + "\n").c_str());

    MemUtils::patchAddress((LPVOID)0x0046B1EA, (BYTE*)&endLoopAddress, sizeof(endLoopAddress));
    MemUtils::patchAddress((LPVOID)0x0046AF75, (BYTE*)&endLoopAddress, sizeof(endLoopAddress));

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