# femto

> Fully Equipped Minimal Text editOr

[![Release version](https://img.shields.io/github/v/release/makuke1234/femto?display_name=release&include_prereleases)](https://github.com/makuke1234/femto/releases/latest)
[![Total downloads](https://img.shields.io/github/downloads/makuke1234/femto/total)](https://github.com/makuke1234/femto/releases)
![C version](https://img.shields.io/badge/version-C99-blue.svg)

It is a bigger more feature-rich version of my other console text editor [atto](https://github.com/makuke1234/atto).
It is also written in vanilla C + Win32 Console API - it's designed to be used solely in Microsoft Windows.
It has been compiled with MinGW GCC.

Currently, my other tool [slocWin32](https://github.com/makuke1234/slocWin32) reports 1665 lines of code for this editor:

![SLOC](./images/sloc.png)


# Obtaining

32-bit (x86) & 64-bit (x64) Windows binaries can be obtained [here](https://github.com/makuke1234/femto/releases).


# Features

Features inherited from [femto](https://github.com/makuke1234/atto):
- [x] file must be given as a command-line argument, 'raw editing'/'saving later to a file' is impossible for a reason
- [x] all saved files use CRLF line-ending format by default, LF and CR are also supported with version 1.8
- [x] femto editor utilizes the whole command prompt window, window is as big as your console currently is
- [x] the last line of the window is dedicated to status, for example showing success or failure when an attempt to save the file has been made
- [x] the following keyboard shortcuts:
    | Key                            | Action                                   |
    | ------------------------------ | ---------------------------------------- |
    | <kbd>ESC</kbd>                 | Closes the editor                        |
    | <kbd>Ctrl+S</kbd>              | Tries to save the current open file      |
    | <kbd>Ctrl+R</kbd>              | Tries to reload contents of current file |
    | <kbd>Ctrl+E</kbd> <kbd>F</kbd> | Switch to CRLF EOL sequence              |
    | <kbd>Ctrl+E</kbd> <kbd>L</kbd> | Switch to LF EOL sequence                |
    | <kbd>Ctrl+E</kbd> <kbd>C</kbd> | Switch to CR EOL sequence                |
- [x] 2 ways to start the program:
    | Syntax          | Action                                                                                                   |
    | --------------- | -------------------------------------------------------------------------------------------------------- |
    | `femto`          | Shows help<br>![help image](./images/help.PNG)                                                          |
    | `femto` \[file\] | Starts editor with the specified file,<br>does not have to exist<br>*where \[file\] is the file's name* |
New features:
- [ ] none yet.


# Screenshots

## Demonstrating unicode support, file test5.txt is open with LF EOL sequences
![Unicode Support](./images/unicodeSupport.png)

## Last key shown on status bar with respective repeat count
![Key frequency](./images/keyFreq.png)

## Saving was successful, file size is now 48 bytes
![!Saving success](./images/savingSuc.png)


# Changelog

* 0.1
	* Initial release, basically a copy of [femto](https://github.com/makuke1234/atto)


# License

This project uses the MIT license.
