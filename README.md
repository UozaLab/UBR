# UBR

UBR is a HDD backup, restore and clone utility.

## Features

* Designed for Windows 7/10/11
* Run on Windows PE/SE
* Support for MBR/GPT
* Backup to VHD, VHDX and RAW(DD) images
* Backup to compressed(lz4) images
* Restore from VHD, VHDX and RAW images
* Restore from compressed(lz4) images
* Restore and clone to a smaller HDD
* Live clone by using VSS snapshots
* Improve performance by skipping non used area during copying(FAT12/16/32, NTFS)
* Support for making a bootable media to run a WinPE
* Portable. No installation needed
* Free for personal and commercial use

## Download Binaries

* [GitHub Releases](https://github.com/UozaLab/UBR/releases)

## Screenshot

![](https://github.com/UozaLab/UBR/wiki/images/ubr_screenshot.png)

## Wiki

See documentation at the [Wiki](https://github.com/UozaLab/UBR/wiki)

## Project Status

This project is in development phase.

Features like partition backup and restore are not implemented yet.


## Compiling

* msys2(mingw32/mingw64)
* wxWidgets-3.2.10
* LZ4 v1.10.0
