// CSCI3150 Asg 4
// YUAN Lin
// 1155141399
#include "kernel.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int find_free_page(struct Kernel* kernel) {
  int total = KERNEL_SPACE_SIZE / PAGE_SIZE;

  for (int i = 0; i < total; i++) {
    if (kernel->occupied_pages[i] == 0) {
      return i;
    }
  }
  return -1;
}



int proc_create_vm(struct Kernel* kernel, int size) {
  //check free
  int pid = -1;
  for (int i = 0; i < MAX_PROCESS_NUM; i++) {
    if (kernel->running[i] == 0) {
      pid = i; // find the free
      break;
    }
  }

  if (pid == -1 || size > KERNEL_SPACE_SIZE - kernel->allocated_pages * PAGE_SIZE || size > VIRTUAL_SPACE_SIZE) {
    return -1;
  }

  //initialize
  kernel->running[pid] = 1;
  kernel->mm[pid].size = size;
  kernel->mm[pid].page_table = malloc(sizeof(struct PageTable));  
  kernel->mm[pid].page_table->ptes = malloc(size / PAGE_SIZE * sizeof(struct PTE)); 
  kernel->allocated_pages += size / PAGE_SIZE;

  //initialize; set PFN as -1 and present as 0 to indicate unallocate
  for (int i = 0; i < size / PAGE_SIZE; i++) {
    kernel->mm[pid].page_table->ptes[i].PFN = -1;
    kernel->mm[pid].page_table->ptes[i].present = 0;
  }

  return pid;
}



int vm_read(struct Kernel* kernel, int pid, char* addr, int size, char* buf) {
  
  int offset = addr - (char*)0;

  // check if the reading range is out of bound
  if (offset + size > kernel->mm[pid].size) {
    return -1;
  }

  // else, map page if possible
  for (int i = offset / PAGE_SIZE; i < (offset + size + PAGE_SIZE - 1) / PAGE_SIZE; i++) {
    if (kernel->mm[pid].page_table->ptes[i].present == 0) { //no map, then need map
      int free_page = find_free_page(kernel);
      if (free_page == -1) {
        return -1;
      }

      kernel->mm[pid].page_table->ptes[i].PFN = free_page;
      kernel->mm[pid].page_table->ptes[i].present = 1;
      kernel->occupied_pages[free_page] = 1;
    }

    // copy data from kernel space to buffer
    memcpy(buf, kernel->space + kernel->mm[pid].page_table->ptes[i].PFN * PAGE_SIZE, PAGE_SIZE);
    
  }

  return 0;
}





int vm_write(struct Kernel* kernel, int pid, char* addr, int size, char* buf) {
  // convert address to offset
  int offset = addr - (char*)0;

  // check if out of bound
  if (offset + size > kernel->mm[pid].size) {
    return -1;
  }

  
  for (int i = offset / PAGE_SIZE; i < (offset + size + PAGE_SIZE - 1) / PAGE_SIZE; i++) {
    if (kernel->mm[pid].page_table->ptes[i].present == 0) { //if not mapped yet, need to map first 
      int free_page = find_free_page(kernel);
      if (free_page == -1) {
        return -1;
      }

      kernel->mm[pid].page_table->ptes[i].PFN = free_page;
      kernel->mm[pid].page_table->ptes[i].present = 1;
      kernel->occupied_pages[free_page] = 1;
    }

    // copy data 
    memcpy(kernel->space + kernel->mm[pid].page_table->ptes[i].PFN * PAGE_SIZE, buf, PAGE_SIZE);
    
  }

  return 0;
}


int proc_exit_vm(struct Kernel* kernel, int pid) {
  // check if it is not a valid running process
  if (kernel->running[pid] == 0) {
    return -1;
  }

  // deal with each active pages
  for (int i = 0; i < kernel->mm[pid].size / PAGE_SIZE; i++) {
    
    if (kernel->mm[pid].page_table->ptes[i].present) {
      // if page is occupied, then make occupied as 0
      kernel->occupied_pages[kernel->mm[pid].page_table->ptes[i].PFN] = 0;
    }
  }

  // mark progcess as not running
  kernel->running[pid] = 0;

  // free allocated space
  free(kernel->mm[pid].page_table->ptes);
  free(kernel->mm[pid].page_table);

  // avoid dangling pointer
  kernel->mm[pid].page_table = NULL;

  // update # of allocated pages
  kernel->allocated_pages -= kernel->mm[pid].size / PAGE_SIZE;

  return 0;
}



