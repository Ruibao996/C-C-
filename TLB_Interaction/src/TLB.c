#include "../inc/TLB.h"

#include <stdlib.h>

unsigned global_TLB_init(void) {
  global_tlb = calloc(1, sizeof(TLB));
  return 0;
}

void global_TLB_free(void) {
  if (global_tlb == NULL) {
    return;
  }
  free(global_tlb);
  global_tlb = NULL;
}

unsigned read_TLB(proc_id_t pid, unsigned vpn) {
  if (global_tlb == NULL || global_tlb->pid != pid) {
    return -1;
  }

  for (size_t i = 0; i < TLB_SIZE; i++) {
      if (global_tlb->entries[i].valid && global_tlb->entries[i].vpn == vpn) {
          global_tlb->entries[i].lut = global_tlb->clock++;
          return global_tlb->entries[i].ppn;
      }
  }
  return -1; 


}

void write_TLB(proc_id_t pid, unsigned vpn, unsigned ppn) {
  if (global_tlb == NULL) {
    return;
  }

  if (global_tlb->pid != pid) {
    global_tlb->pid = pid;
    for (size_t i = 0; i < TLB_SIZE; i++) {
      global_tlb->entries[i].valid = 0;
    }
  }

  size_t lru_index = 0;
  uint32_t lru_time = UINT32_MAX;

  for (size_t i = 0; i < TLB_SIZE; i++) {
    if (!global_tlb->entries[i].valid) {
      lru_index = i;
      break;
    }
    if (global_tlb->entries[i].lut < lru_time) {
      lru_time = global_tlb->entries[i].lut;
      lru_index = i;
    }
  }

  global_tlb->entries[lru_index].vpn = vpn;
  global_tlb->entries[lru_index].ppn = ppn;
  global_tlb->entries[lru_index].valid = 1;
  global_tlb->entries[lru_index].lut = global_tlb->clock++;


}

void remove_TLB(proc_id_t pid, unsigned vpn) {
  if (global_tlb == NULL || global_tlb->pid != pid) {
    return;
  }

  for (size_t i = 0; i < TLB_SIZE; i++) {
    if (global_tlb->entries[i].valid && global_tlb->entries[i].vpn == vpn) {
      global_tlb->entries[i].valid = 0;
      break;
    }
  }
}
