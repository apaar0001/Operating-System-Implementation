# ELF Loader Program

The ELF Loader Program is designed to load and execute ELF (Executable and Linkable Format) files. It provides a custom handler for handling segmentation faults and dynamically allocates memory for program segments as needed. This README provides an overview of the program and explains the purpose and functionality of each function.

# Auhtor 
Angadjeet Singh(2022071) & Apaar IIITD(2022089)

## Introduction

The ELF Loader Program is a C program that allows you to load and run ELF executable files. It includes a custom handler for handling segmentation faults (SIGSEGV) and allocates memory for program segments as required. The program is designed to dynamically load and execute ELF files and provides detailed information about memory usage and page faults.

## Program Overview

The program consists of several functions, each serving a specific purpose. Here is an overview of the key functions and their functionality:

### `loader_cleanup`

This function is responsible for cleaning up resources used by the program, such as allocated memory and open file descriptors.

### `calculating_fragmentation`

This function calculates fragmentation based on the virtual address, memory size, page offset, and the number of pages allocated.(Only in with-bonus)

### `calculate_fragmentation`

A wrapper function for `calculating_fragmentation` to calculate fragmentation based on provided parameters.

### `check_MapFailed`

This function checks if `mmap` failed and prints an error message along with the error description.

### `print_details`

Prints details such as the number of pages used, page faults, and fragmentation.

### `custom_handler`

This is the custom signal handler for SIGSEGV (segmentation fault). It identifies the segment containing the faulting address, allocates memory for the segment, and copies the content into the allocated memory. It also updates page fault and fragmentation information.

### `check_heapoverflow_ehdr` and `check_heapoverflow_phdr`

These functions check if `malloc` failed for the ELF header and program headers, respectively, to prevent heap overflow.

### `allocate_memory_ehdr` and `allocate_memory_phdr`

These functions allocate memory for the ELF header and program headers, respectively.

### `load_and_run_elf`

This is the main function responsible for opening the ELF file, reading the ELF header and program headers, allocating memory for program segments, executing the `_start` function, and providing program details.

### `check_elf_file`

This function is used to make ensure that we are entering a ELF format file with our ./smartloader.

### `main`

The program's entry point, which parses command-line arguments, loads and runs the specified ELF file, and performs cleanup.

## Usage

The programme can eb run by first going in the folder (with_bonus/without_bonus) and simply running make command and then running ./smartloader sum.

## Contribution

Angadjeet Singh(2022071)-50% -->Worked on the implementation of teh strucuture
Apaar IIITD (2022089)- 50% --> Worked on the logic implementation and error handling




