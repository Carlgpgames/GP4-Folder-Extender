# FolderExtender

FolderExtender is a C++-based DLL used in Geoff Crammond's Grand Prix 4 (version 1.02) to dynamically load folders and files into the game's memory. This allows the game to first read all files from local directories before potentially loading additional files from WAD files. It even allows you to run the game without WAD-files (like cars.wad or anim.wad). 

## Features

- Creates a sample INI file (`FolderContent.ini`) if it doesn't already exist.
- Reads directories and files from `FolderContent.ini` and loads them into the game's memory.
- Patches the game's memory to utilize the loaded files.
- Supports recursive file searching within specified folders.

## INI File Structure

The `FolderContent.ini` file follows this format:

```ini
[content]
folder1=cars
folder2=MAPS\RESOURCES
file1=driver9_1.tex
file2=driver9_2.tex
file3=driver10_1.tex
...
folder1, folder2, etc.: These are folder paths relative to the GP4 game directory. The program searches these folders recursively for files.
file1, file2, etc.: Specific files to be loaded, independent of the folders specified.
```

## Installation
Copy the DLL to the GP4 game directory.
Ensure that the FolderContent.ini file exists in the same directory as the DLL, or the DLL will create a sample INI file automatically.

## Usage
Inject the DLL into the GP4 process with a tool such as "GP4 Memory Access", "Cheat Engine" or similar. The DLL is automatically triggered when the GP4 game is loaded. It looks for the FolderContent.ini file to load the specified folders and files. If the INI file doesn't exist, a sample INI file will be generated.

## Debugging
The DLL outputs debug messages using OutputDebugStringA. These can be viewed in the debug output of GPxPatch or tools like DebugView from Microsoft.

## Authors
- Carl_gpgames
- Diego "Öggo" Noriega
