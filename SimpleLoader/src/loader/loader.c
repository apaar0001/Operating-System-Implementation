#include"loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;
void loader_cleanup() {
  if (ehdr != NULL && sizeof(ehdr)!= 0) {
    unsigned int sizeEhdr = ehdr->e_ehsize;
    sizeEhdr = 0;
    ehdr->e_ehsize = sizeEhdr;
    printf("Deallocating the ehdr\n");
    munmap(ehdr, sizeof(ehdr));
    ehdr = NULL;
  }

  if (phdr != NULL && sizeof(phdr) != 0) {
    unsigned int sizePhdr = phdr->p_memsz;
    sizePhdr = 0;
    printf("Deallocating the phdr\n");
    munmap(phdr, sizeof(phdr));
    phdr = NULL;
  }

  if (fd != -1) {
    printf("closing the file\n");
    close(fd);
    fd = -1;
  }
}

// 1. Load entire binary content into the memory from the ELF file.
// 2. Iterate through the PHDR table and find the section of PT_LOAD 
//    type that contains the address of the entrypoint method in fib.c
// 3. Allocate memory of the size "p_memsz" using mmap function 
//    and then copy the segment content
// 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
// 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
// 6. Call the "_start" method and print the value returned from the "_start"
void checkLseekFailure(size_t check)
{
  if(check == -1)
  {
    printf("lseek failed to set seek");
  }
}
void load_and_run_elf(char **exe) {

    fd = open(exe[1], O_RDONLY);//opening the file using the file descryptor;

    unsigned int address = 0;//address to store the entrypoint address for the main function;

    if (fd < 0) {
        printf("Failed to open the File.\n");
        exit(EXIT_FAILURE);
    }

    ehdr = malloc(sizeof(Elf32_Ehdr));//allocating the elf header as was mentioned in online class

    if(ehdr == NULL)
    {
      printf("Malloc failure gave heap overflow!\n");
      exit(EXIT_FAILURE);
    }

    // Read ELF header
    size_t readCheck = read(fd, ehdr, sizeof(Elf32_Ehdr));
    size_t lseekcheck = 0;
    if(readCheck<0)
    {
      printf("read failed\n");
      exit(EXIT_FAILURE);
    }

    int numberOfPhSec = ehdr->e_phnum;
    int sizeOfFirstSec = ehdr->e_phentsize;
    if(numberOfPhSec == 0){
      printf("no program header section in the given elf");
      exit(1);
    }

    if(sizeOfFirstSec==0)
    {
      printf("cannot locate size of first program table header section");
      exit(1);
    }

    phdr = malloc(sizeOfFirstSec* numberOfPhSec);//allocating the program header table as was mentioned in online class

    if(phdr == NULL)
    {
      printf("Malloc failure gave heap overflow!\n");
      exit(EXIT_FAILURE);
    }

    // Read program headers
    off_t startPointOfPHT = ehdr -> e_phoff;

    if(startPointOfPHT == 0)
    {
      printf("Program Header Table not found");
    }

    lseek(fd, startPointOfPHT, SEEK_SET);

    readCheck = read(fd, phdr, sizeOfFirstSec * numberOfPhSec);

    if(readCheck<0)
    {
      printf("read failed\n");
      exit(EXIT_FAILURE);
    }

    int numOtherTypes = 0;
    int index = 1;
    for(index = 0; index < numberOfPhSec; index++) {

        if (phdr[index].p_type == PT_LOAD) {
            //command was specified in the assignment itself 


            void *virtual_mem = mmap((void *)phdr[index].p_vaddr,phdr[index].p_memsz,PROT_READ | PROT_WRITE | PROT_EXEC,MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,0,0);


            if (virtual_mem == MAP_FAILED) {
                perror("mmap failed\n");
                exit(EXIT_FAILURE);
            }
            readCheck = 0;
            int check = 0;
            check = lseek(fd, phdr[index].p_offset, SEEK_SET);
            readCheck = read(fd, virtual_mem, phdr[index].p_filesz);
            checkLseekFailure(check);

            if(readCheck < 0)
            {
              printf("Error reading after mapping to the virtual memory");
              exit(EXIT_FAILURE);
            }
        }
        else
        {
          numOtherTypes++;
          continue;
        }
    }

    int flag = 0;

    if(numOtherTypes + 1 == numberOfPhSec){
      printf("There is no PT_LOAD type program header in the Program Header Table");
    }

    if(ehdr->e_entry<0)
    {
      printf("Address Not found");
      exit(EXIT_FAILURE);
    }
    else{
      address = ehdr->e_entry;
    }
    
    if (address != 0) {
      int (*_start)(void) = (int (*)(void))address;
      int result = _start();

      int fib40 = 102334155;

      if(result != fib40){
        printf("Error calculating the required fibonacci number");
        exit(EXIT_FAILURE);
      }

      printf("User _start return value = %d\n",result);
    } else {
        flag = 1;
        if(flag == 1){
          printf("Loading failed...\n");
        }
    }
}