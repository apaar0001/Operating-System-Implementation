#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include<stdbool.h>

void load_and_run_elf(char** exe);
void loader_cleanup();
void calculate_fragmentation(uintptr_t vaddr, size_t memsz, size_t page_offset, int num_pages);
void custom_handler(int signum, siginfo_t *information, void *context);
bool check_elf_file(const char *filename);