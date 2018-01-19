/* Copyright (C) 2017 ETH Zurich, University of Bologna
 * All rights reserved.
 *
 * This code is under development and not yet released to the public.
 * Until it is released, the code is under the copyright of ETH Zurich and
 * the University of Bologna, and may contain confidential and/or unpublished 
 * work. Any reuse/redistribution is strictly forbidden without written
 * permission from ETH Zurich.
 *
 * Bug fixes and contributions will eventually be released under the
 * SolderPad open hardware license in the context of the PULP platform
 * (http://www.pulp-platform.org), under the copyright of ETH Zurich and the
 * University of Bologna.
 */
#ifndef _PULP_MEM_H_
#define _PULP_MEM_H_

#include <linux/module.h>    /* Needed by all modules */
#include <linux/kernel.h>    /* KERN_ALERT, container_of */
#include <linux/slab.h>      /* kmalloc */
#include <linux/mm.h>        /* vm_area_struct struct, page struct, PAGE_SHIFT, page_to_phys */
#include <linux/highmem.h>   /* kmap, kunmap */

#include "pulp_module.h"

#include "pulp_host.h"

// funtions
void pulp_mem_cache_flush(struct page * page, unsigned offset_start, unsigned offset_end);
void pulp_mem_cache_inv(struct page * page, unsigned offset_start, unsigned offset_end);
unsigned  pulp_mem_get_num_pages(unsigned addr_start, unsigned size_b);
int pulp_mem_get_user_pages(struct page *** pages, unsigned addr_start, unsigned n_pages, unsigned write);
int pulp_mem_map_sg(unsigned ** addr_start_vec, unsigned ** addr_end_vec, unsigned long ** addr_offset_vec,
                    unsigned ** page_start_idxs, unsigned ** page_end_idxs, 
                    struct page *** pages, unsigned n_pages, 
                    unsigned addr_start, unsigned addr_end);
int pulp_mem_check_num_sg( struct page *** pages, unsigned n_pages);
int pulp_mem_l2_get_entries(unsigned ** virtual_page_vec, unsigned ** phy_page_vec,
                            struct page *** pages, unsigned n_pages, unsigned addr_start);

#endif/*_PULP_MEM_H_*/
