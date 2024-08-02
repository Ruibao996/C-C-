#include "../inc/simulator.h"
#include "../inc/TLB.h"
#include <assert.h>
#include <stdlib.h>

static inline size_t get_l1_index(addr_t address) {
    return (address >> (L2_BITS + OFFSET_BITS)) & ((1 << L1_BITS) - 1);
}

static inline size_t get_l2_index(addr_t address) {
    return (address >> OFFSET_BITS) & ((1 << L2_BITS) - 1);
}

static inline size_t get_offset(addr_t address) {
    return address & ((1 << OFFSET_BITS) - 1);  // OFFSET_MASK 的实现
}

status_t allocate_page(Process *process, addr_t address,
                       addr_t physical_address) {
  // This is guaranteed by the simulator
  assert(address >> OFFSET_BITS << OFFSET_BITS == address);
  assert(physical_address >> OFFSET_BITS << OFFSET_BITS == physical_address);
  // 1. Check if the process is valid
  if (process == NULL) {
    return ERROR;
  }
  // TODO: Implement me!


    size_t l1_index = get_l1_index(address);
    size_t l2_index = get_l2_index(address);

    if (process->page_table.entries[l1_index].entries == NULL) {
        process->page_table.entries[l1_index].entries = calloc(L2_PAGE_TABLE_SIZE, sizeof(PTE));
        if (process->page_table.entries[l1_index].entries == NULL) {
            return ERROR;
        }
    }

    if (process->page_table.entries[l1_index].entries[l2_index].valid) {
        return ERROR;
    }

    process->page_table.entries[l1_index].entries[l2_index].frame = physical_address >> OFFSET_BITS;
    process->page_table.entries[l1_index].entries[l2_index].valid = 1;
    process->page_table.entries[l1_index].valid_count++;


  return SUCCESS;
}

status_t deallocate_page(Process *process, addr_t address) {
  // This is guaranteed by the simulator
  assert(address >> OFFSET_BITS << OFFSET_BITS == address);
  // 1. Check if the process is valid
  if (process == NULL) {
    return ERROR;
  }
  // TODO: Implement me!

  size_t l1_index = get_l1_index(address);
  size_t l2_index = get_l2_index(address);

  if (!process->page_table.entries[l1_index].entries ||
      !process->page_table.entries[l1_index].entries[l2_index].valid) {
    return ERROR; // Virtual page not allocated
  }

  process->page_table.entries[l1_index].entries[l2_index].valid = 0;
  process->page_table.entries[l1_index].valid_count--;

  //check if we need to free the L2 page table
  if (process->page_table.entries[l1_index].valid_count == 0) {
    free(process->page_table.entries[l1_index].entries);
    process->page_table.entries[l1_index].entries = NULL;
  }

  // Invalidate TLB entry for this address
  remove_TLB(process->pid, address >> OFFSET_BITS);

  return SUCCESS;
}

status_t read_byte(Process *process, addr_t address, byte_t *byte) {
  // 1. Check if the process is valid
  if (process == NULL) {
    return ERROR;
  }
  // TODO: Implement me!

  addr_t physical_address;
  int tlb_result = read_TLB(process->pid, address >> OFFSET_BITS);

  if (tlb_result != -1) {
    physical_address = tlb_result << OFFSET_BITS;
    *byte = main_memory->pages[physical_address >> OFFSET_BITS]
                ->data[get_offset(address)];
    return TLB_HIT;
  }

  size_t l1_index = get_l1_index(address);
  size_t l2_index = get_l2_index(address);

  if (!process->page_table.entries[l1_index].entries ||
      !process->page_table.entries[l1_index].entries[l2_index].valid) {
    return ERROR; // Virtual page not allocated
  }

  physical_address = process->page_table.entries[l1_index].entries[l2_index].frame << OFFSET_BITS;
  *byte = main_memory->pages[physical_address >> OFFSET_BITS]->data[get_offset(address)];
  write_TLB(process->pid, address >> OFFSET_BITS, physical_address >> OFFSET_BITS);

  return SUCCESS;
}

status_t write_byte(Process *process, addr_t address, const byte_t *byte) {
  // 1. Check if the process is valid
  if (process == NULL) {
    return ERROR;
  }
  // TODO: Implement me!

  addr_t physical_address;
  int tlb_result = read_TLB(process->pid, address >> OFFSET_BITS);

  if (tlb_result != -1) {
    physical_address = tlb_result << OFFSET_BITS;
    main_memory->pages[physical_address >> OFFSET_BITS]->data[get_offset(address)] = *byte;
    return TLB_HIT;
  }

  size_t l1_index = get_l1_index(address);
  size_t l2_index = get_l2_index(address);

  if (!process->page_table.entries[l1_index].entries ||
      !process->page_table.entries[l1_index].entries[l2_index].valid) {
    return ERROR; // Virtual page not allocated
  }

  physical_address = process->page_table.entries[l1_index].entries[l2_index].frame << OFFSET_BITS;
  main_memory->pages[physical_address >> OFFSET_BITS]->data[get_offset(address)] = *byte;
  write_TLB(process->pid, address >> OFFSET_BITS, physical_address >> OFFSET_BITS);


  return SUCCESS;
}
