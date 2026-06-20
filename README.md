# Purpose Statement

The purpose of the PTSource X86 DOS OS Builder is to preserve and study the Microsoft DOS open-source codebase while making it practical and relevant for modern retro-computing, education or research. This project exists to safeguard an important piece of computing history by maintaining compatibility with the original DOS architecture, behavior, and design principles, while carefully and occasionally extending it with optional improvements, fixes or components. 

#  PTSource X86 DOS OS Builder
### Current version 7 Final Released (29/03/2026)
#### [![ko-fi](/images/ko-fi.png)](https://ko-fi.com/ptsource) For updates, change logs or to buy us a coffee.

PTSource X86 DOS OS Builder is a flexible system creation toolkit designed to help you build minimal custom DOS-compatible operating systems for x86 hardware, based on the Microsoft DOS open-source codebase. It allows developers, educators, and retro-computing enthusiasts to assemble tailored DOS systems by selecting, configuring, and integrating core OS components, device drivers, and utilities. The builder supports customization of the kernel, command interpreter, memory managers, and startup configuration, making it possible to create minimal DOS systems. With a focus on clarity, modularity, and compatibility, the PTSource X86 DOS OS Builder is ideal for experimentation, learning operating system internals, preserving legacy software, and developing specialized DOS distributions for real hardware, emulators, or virtual machines.

## Screenshots
<img src="https://raw.githubusercontent.com/ptsource/X86-DOS-OS-Builder/master/images/01.PNG" width="33%"><img src="https://raw.githubusercontent.com/ptsource/X86-DOS-OS-Builder/master/images/02.PNG" width="33%"><img src="https://raw.githubusercontent.com/ptsource/X86-DOS-OS-Builder/master/images/03.PNG" width="33%">
<img src="https://raw.githubusercontent.com/ptsource/X86-DOS-OS-Builder/master/images/04.PNG" width="33%"><img src="https://raw.githubusercontent.com/ptsource/X86-DOS-OS-Builder/master/images/05.PNG" width="33%"><img src="https://raw.githubusercontent.com/ptsource/X86-DOS-OS-Builder/master/images/06.PNG" width="33%">
## Features 

* Full source code under MIT license
* More than 50 standard DOS commands
* Added system applications
* Complete set of building tools
* BOOT Image included

## Important Note
PTSource X86 DOS OS Builder uses the Microsoft DOS open-source codebase wich is based on MS-DOS 4.0, although some fixes to this code were applied, PTSource X86 DOS OS Builder is ment to be used for minimal systems or tailored minimal bootdisks. If you want a multitasking, multiuser professional operating DOS system 100% compatible with MS-DOS, PC-DOS, DR-DOS and with Windows 3.11 enhanced mode support you should consider using [PTSource MiniVMDOS](https://store.ptsource.eu/soft/mvmdos/) 

## Building

PTSource X86 DOS OS Builder builds in any DOS environment, even in DOSBox. A DOSBox portable distribution is provided for that effect.

### Windows
* Download this repository.
* Extract the contents of the repository.
* Start X86 DOS OS Builder.
* Run BUILD and wait.
* A complete distribution will be avaliable in the DISTRO folder.

### Linux
* Download this repository.
* Extract the contents of the repository.
* Copy the folder C->DOSSTRAP to the C Drive of you DOS emulator.
* Run BUILD and wait.
* A complete distribution will be avaliable in the DISTRO folder.

### Real DOS Machine
* Download this repository.
* Extract the contents of the repository.
* Copy the folder C->DOSSTRAP to the C Drive.
* Run BUILD and wait.
* A complete distribution will be avaliable in the DISTRO folder.

## Boot Image Editing

A Boot Image is provided in the repository BOOT.IMG, this image contains the complete distribution in the distro folder and it can be edited to suite your needs. Here is how to do it.

* Open the image with a RAW image mounter (DO NOT USE WINIMAGE YOU MAY DESTROY THE BOOT RECORD).  There are many RAW image mounters avaliable, there is also one from [PTSource here](https://ko-fi.com/s/0d421f99c0).
* Edit the image to suite your needs and unmount the image.

## Applications

PTSource X86 DOS OS Builder uses the Microsoft DOS open-source codebase wich is based on MS-DOS 4.0, despite that almost all not too complex DOS applications will run with no hassle, its up to you to make a test and see. Applications can be added to the image using the above method. If you want a Norton Commander style shell occupying just 13KB of memory (the one used on the preview) you can get [Wee Commander here](https://ko-fi.com/s/8df79a705d). For a continued updated list of applications for DOS go [here](https://store.ptsource.eu/files_16bits).

## MiniVM Booting

If you are a [PTSource MiniVM](https://wiki.ptsource.eu/soft/ptminivm/start) user you need to set the floppies to 1.44 to boot. You can do this by adding --f140 to the command line and then enter the BIOS and change the drives to 1.44.

## Making a CP/M-80 System using ZRUN
This example use case allows you to have a CP/M-80 System using only ALIAS, [ZRUN](https://ko-fi.com/s/14d98aea8d) and [COLOR](https://ko-fi.com/s/edfcdbe2af). The example is in the ZRUNCPM.IMG and it builds under the folder DISTROC.

<img src="https://raw.githubusercontent.com/ptsource/X86-DOS-OS-Builder/master/images/Z1.PNG" width="33%"><img src="https://raw.githubusercontent.com/ptsource/X86-DOS-OS-Builder/master/images/Z2.PNG" width="33%"><img src="https://raw.githubusercontent.com/ptsource/X86-DOS-OS-Builder/master/images/Z3.PNG" width="33%">

### Instructions

*  Open ZRUNCPM.IMG with a RAW image mounter.You can find one from [PTSource here](https://ko-fi.com/s/0d421f99c0).
*  Download ZRUN from [here](https://ko-fi.com/s/14d98aea8d).
*  Download COLOR from [here](https://ko-fi.com/s/edfcdbe2af).
*  Add ZRUN and COLOR to the ZRUNCPM.IMG.
*  Add the CPM programs you like changing the extension from .COM to .CPM.
*  Edit AUTOEXEC.BAT and use ALIAS to prepare your commands.
*  Close the image, boot and issue your ALIAS defined commands.
*  For a continued updated list of applications for CPM go [here](https://store.ptsource.eu/files_8bits).

## Support for LiberDOS
PTSource X86 DOS OS Builder can be used to build a full toolchain for the [LiberDOS Project](https://github.com/libersoft-org/liberdos). To do that you just have to edit 2 files. VERSIONINC.H in the H folder and change the expected_version_major to 5 and VERSIONA.INC in the INC folder and change major_version to 5. After build you should have a complete toolchain for the [LiberDOS Project](https://github.com/libersoft-org/liberdos).

## Homepage

[![Visit homepage.](/images/homepage-here-yellowgreen.svg)](https://wiki.ptsource.eu/soft/x86dososbuilder/start)

<p align="center" width="100%">
    <a href="https://store.ptsource.eu/soft/x86dososbuilder/start" target="_blank"><img src="/images/bannerx86dosbuilder.png" alt="PTSource X86 DOS OS Builder"></a> 
</p>
<p align="center" width="100%">
<a href="https://european-union.europa.eu/"><img src="https://raw.githubusercontent.com/ptsource/X86-DOS-OS-Builder/master/images/madeeu.png"></a><a  href="https://noaimovement.com/"><img src="https://raw.githubusercontent.com/ptsource/X86-DOS-OS-Builder/master/images/noai.png"></a>
</p>


