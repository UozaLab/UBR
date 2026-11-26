# UBR

UBR is software that allows you to back up, restore, and clone HDDs and SSDs.  
UBR accesses the disk sector information and convert the entire disk into a single file.  
You can choose the file format from VHDX, VHD, or RAW.  
Software for creating a USB flash drive to boot Windows PE named PEMaker is included.  
By booting into Windows PE and running UBR, you can restore a disk containing the OS.  

## Features

* Designed for Windows 7/10/11
* Run on Windows PE/SE
* Support for MBR/GPT
* Backup to VHD, VHDX and RAW(DD) images
* Backup to compressed(lz4) images
* Restore from VHD, VHDX and RAW images
* Restore from compressed(lz4) images
* Restore and clone to a smaller HDD
* Clone disks containing drives encrypted with bitlocker
* Live clone by using VSS snapshots
* Improve performance by skipping non used area during copying(FAT12/16/32, NTFS)
* Support for making a bootable media to run a WinPE
* Portable. No installation needed
* Free for personal and commercial use

## Download Binaries

* [GitHub Releases](https://github.com/UozaLab/UBR/releases)

## Screenshot

![](https://github.com/UozaLab/UBR/wiki/images/ubr_screenshot.png)

## Information

* [Website](https://uozalab.web.fc2.com)

## Compiling

* msys2(mingw32/mingw64)
* wxWidgets-3.2.10
* LZ4 v1.10.0
