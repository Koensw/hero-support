#include "pulp_func.h"
#include "pulp_host.h"

/**
 * Reserve the virtual address space overlapping with the physical
 * address map of pulp using the mmap() syscall with MAP_FIXED and
 * MAP_ANONYMOUS
 *
 * @pulp: pointer to the PulpDev structure
 */
int pulp_reserve_v_addr(PulpDev *pulp)
{
  pulp->reserved_v_addr.size = PULP_SIZE_B;
  pulp->reserved_v_addr.v_addr = mmap((int *)PULP_BASE_ADDR,pulp->reserved_v_addr.size,
                                      PROT_NONE,MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS,-1,0);
  if (pulp->reserved_v_addr.v_addr == MAP_FAILED) {
    printf("MMAP failed to reserve virtual addresses overlapping with physical address map of PULP.\n");
    return EIO;
  }
  else {
    printf("Reserved virtual addresses starting at %p and overlapping with physical address map of PULP. \n",
           pulp->reserved_v_addr.v_addr);
  }
  
  return 0;
}

/**
 * Free the virtual address space overlapping with the physical
 * address map of pulp using the munmap() syscall
 *
 * @pulp: pointer to the PulpDev structure
 */
int pulp_free_v_addr(PulpDev *pulp)
{  
  int status;

  printf("Freeing reserved virtual addresses overlapping with physical address map of PULP.\n");

  status = munmap(pulp->reserved_v_addr.v_addr,pulp->reserved_v_addr.size);
  if (status) {
    printf("MUNMAP failed to free reserved virtual addresses overlapping with physical address map of PULP.\n");
  } 

  return 0;
}

/**
 * Print information about the reserved virtual memory on the host
 */
void pulp_print_v_addr(PulpDev *pulp)
{
  // check the reservation
  printf("\nMemory map of the process:\n");
  printf("# cat /proc/getpid()/maps\n");
  
  char cmd[20];
  sprintf(cmd,"cat /proc/%i/maps",getpid());
  system(cmd);
  
  // check wether the reservation contributes to the kernels overcommit accounting -> Committed_AS
  printf("\nInformation about the system's memory:\n");
  printf("# cat /proc/meminfo\n");
  system("cat /proc/meminfo");

  return;
}

/**
 * Read 32 bits
 *
 * @base_addr : virtual address pointer to base address 
 * @off       : offset
 * @off_type  : type of the offset, 'b' = byte offset, else word offset
 */
int pulp_read32(unsigned *base_addr, unsigned off, char off_type)
{
  if (DEBUG_LEVEL > 3) {
    unsigned *addr;
    if (off_type == 'b') 
      addr = base_addr + (off>>2);
    else
      addr = base_addr + off;
    printf("Reading from %p\n",addr);
  }
  if (off_type == 'b')
    return *(base_addr + (off>>2));
  else
    return *(base_addr + off);
}

/**
 * Write 32 bits
 *
 * @base_addr : virtual address pointer to base address 
 * @off       : offset
 * @off_type  : type of the offset, 'b' = byte offset, else word offset
 */
void pulp_write32(unsigned *base_addr, unsigned off, char off_type, unsigned value)
{
  if (DEBUG_LEVEL > 3) {
    unsigned *addr;
    if (off_type == 'b') 
      addr = base_addr + (off>>2);
    else
      addr = base_addr + off;
    printf("Writing to %p\n",addr);
  }
  if (off_type == 'b')
    *(base_addr + (off>>2)) = value;
  else
    *(base_addr + off) = value;
}

/**
 * Memory map the device to virtual user space using the mmap()
 * syscall
 *
 * @pulp: pointer to the PulpDev structure
 */
int pulp_mmap(PulpDev *pulp)
{
  int offset;

  /*
   *  open the device
   */
  pulp->fd = open("/dev/PULP", O_RDWR | O_SYNC);
  if (pulp->fd < 0) {
    printf("ERROR: Opening failed \n");
    return -ENOENT;
  }

  /*
   *  do the different remappings
   */
  // PULP internals
  // Clusters
  offset = 0; // start of clusters
  pulp->clusters.size = CLUSTERS_SIZE_B;
  
  pulp->clusters.v_addr = mmap(NULL,pulp->clusters.size,
                               PROT_READ | PROT_WRITE,MAP_SHARED,pulp->fd,offset);
  if (pulp->clusters.v_addr == MAP_FAILED) {
    printf("MMAP failed for clusters.\n");
    return -EIO;
  }
  else {
    printf("Clusters mapped to virtual user space at %p.\n",pulp->clusters.v_addr);
  }

  // SOC_PERIPHERALS
  offset = CLUSTERS_SIZE_B; // start of peripherals
  pulp->soc_periph.size = SOC_PERIPHERALS_SIZE_B;
 
  pulp->soc_periph.v_addr = mmap(NULL,pulp->soc_periph.size,
                                 PROT_READ | PROT_WRITE,MAP_SHARED,pulp->fd,offset); 
  if (pulp->soc_periph.v_addr == MAP_FAILED) {
    printf("MMAP failed for SoC peripherals.\n");
    return -EIO;
  }
  else {
    printf("SoC peripherals mapped to virtual user space at %p.\n",pulp->soc_periph.v_addr);
  }

  // Mailbox
  offset = CLUSTERS_SIZE_B + SOC_PERIPHERALS_SIZE_B; // start of mailbox
  pulp->mailbox.size = MAILBOX_SIZE_B;
 
  pulp->mailbox.v_addr = mmap(NULL,pulp->mailbox.size,
                              PROT_READ | PROT_WRITE,MAP_SHARED,pulp->fd,offset); 
  if (pulp->mailbox.v_addr == MAP_FAILED) {
    printf("MMAP failed for Mailbox.\n");
    return -EIO;
  }
  else {
    printf("Mailbox mapped to virtual user space at %p.\n",pulp->mailbox.v_addr);
  }
 
  // L2
  offset = CLUSTERS_SIZE_B + SOC_PERIPHERALS_SIZE_B + MAILBOX_SIZE_B; // start of L2
  pulp->l2_mem.size = L2_MEM_SIZE_B;
 
  pulp->l2_mem.v_addr = mmap(NULL,pulp->l2_mem.size,
                             PROT_READ | PROT_WRITE,MAP_SHARED,pulp->fd,offset);
 
  if (pulp->l2_mem.v_addr == MAP_FAILED) {
    printf("MMAP failed for L2 memory.\n");
    return -EIO;
  }
  else {
    printf("L2 memory mapped to virtual user space at %p.\n",pulp->l2_mem.v_addr);
  }

  // Platform
  // L3
  offset = CLUSTERS_SIZE_B + SOC_PERIPHERALS_SIZE_B + MAILBOX_SIZE_B + L2_MEM_SIZE_B; // start of L3
  pulp->l3_mem.size = L3_MEM_SIZE_B;
    
  pulp->l3_mem.v_addr = mmap(NULL,pulp->l3_mem.size,
                             PROT_READ | PROT_WRITE,MAP_SHARED,pulp->fd,offset);
  if (pulp->l3_mem.v_addr == MAP_FAILED) {
    printf("MMAP failed for shared L3 memory.\n");
    return -EIO;
  }
  else {
    printf("Shared L3 memory mapped to virtual user space at %p.\n",pulp->l3_mem.v_addr);
  }
 
  // PULP external
  // GPIO
  offset = CLUSTERS_SIZE_B + SOC_PERIPHERALS_SIZE_B + MAILBOX_SIZE_B + L2_MEM_SIZE_B
    + L3_MEM_SIZE_B; // start of GPIO
  pulp->gpio.size = H_GPIO_SIZE_B;
    
  pulp->gpio.v_addr = mmap(NULL,pulp->gpio.size,
                           PROT_READ | PROT_WRITE,MAP_SHARED,pulp->fd,offset);
  if (pulp->gpio.v_addr == MAP_FAILED) {
    printf("MMAP failed for shared L3 memory.\n");
    return -EIO;
  }
  else {
    printf("GPIO memory mapped to virtual user space at %p.\n",pulp->gpio.v_addr);
  }

  // CLKING
  offset = CLUSTERS_SIZE_B + SOC_PERIPHERALS_SIZE_B + MAILBOX_SIZE_B + L2_MEM_SIZE_B
    + L3_MEM_SIZE_B + H_GPIO_SIZE_B; // start of Clking
  pulp->clking.size = CLKING_SIZE_B;
    
  pulp->clking.v_addr = mmap(NULL,pulp->clking.size,
                             PROT_READ | PROT_WRITE,MAP_SHARED,pulp->fd,offset);
  if (pulp->clking.v_addr == MAP_FAILED) {
    printf("MMAP failed for shared L3 memory.\n");
    return -EIO;
  }
  else {
    printf("Clock Manager memory mapped to virtual user space at %p.\n",pulp->clking.v_addr);
  }

  // STDOUT
  offset = CLUSTERS_SIZE_B + SOC_PERIPHERALS_SIZE_B + MAILBOX_SIZE_B + L2_MEM_SIZE_B
    + L3_MEM_SIZE_B + H_GPIO_SIZE_B + CLKING_SIZE_B; // start of Stdout
  pulp->stdout.size = STDOUT_SIZE_B;
    
  pulp->stdout.v_addr = mmap(NULL,pulp->stdout.size,
                             PROT_READ | PROT_WRITE,MAP_SHARED,pulp->fd,offset);
  if (pulp->stdout.v_addr == MAP_FAILED) {
    printf("MMAP failed for shared L3 memory.\n");
    return -EIO;
  }
  else {
    printf("Stdout memory mapped to virtual user space at %p.\n",pulp->stdout.v_addr);
  }

  // RAB config
  offset = CLUSTERS_SIZE_B + SOC_PERIPHERALS_SIZE_B + MAILBOX_SIZE_B + L2_MEM_SIZE_B
    + L3_MEM_SIZE_B + H_GPIO_SIZE_B + CLKING_SIZE_B + STDOUT_SIZE_B; // start of RAB config
  pulp->rab_config.size = RAB_CONFIG_SIZE_B;
    
  pulp->rab_config.v_addr = mmap(NULL,pulp->rab_config.size,
                                 PROT_READ | PROT_WRITE,MAP_SHARED,pulp->fd,offset);
  if (pulp->rab_config.v_addr == MAP_FAILED) {
    printf("MMAP failed for shared L3 memory.\n");
    return -EIO;
  }
  else {
    printf("RAB config memory mapped to virtual user space at %p.\n",pulp->rab_config.v_addr);
  }

  // Zynq
  // SLCR
  offset = CLUSTERS_SIZE_B + SOC_PERIPHERALS_SIZE_B + MAILBOX_SIZE_B + L2_MEM_SIZE_B
    + L3_MEM_SIZE_B + H_GPIO_SIZE_B + CLKING_SIZE_B + STDOUT_SIZE_B + RAB_CONFIG_SIZE_B; // start of SLCR
  pulp->slcr.size = SLCR_SIZE_B;
    
  pulp->slcr.v_addr = mmap(NULL,pulp->slcr.size,
                           PROT_READ | PROT_WRITE,MAP_SHARED,pulp->fd,offset);
  if (pulp->slcr.v_addr == MAP_FAILED) {
    printf("MMAP failed for Zynq SLCR.\n");
    return -EIO;
  }
  else {
    printf("Zynq SLCR memory mapped to virtual user space at %p.\n",pulp->slcr.v_addr);
  }

  // MPCore
  offset = CLUSTERS_SIZE_B + SOC_PERIPHERALS_SIZE_B + MAILBOX_SIZE_B + L2_MEM_SIZE_B
    + L3_MEM_SIZE_B + H_GPIO_SIZE_B + CLKING_SIZE_B + STDOUT_SIZE_B + RAB_CONFIG_SIZE_B
    + SLCR_SIZE_B; // start of MPCore
  pulp->mpcore.size = MPCORE_SIZE_B;
    
  pulp->mpcore.v_addr = mmap(NULL,pulp->mpcore.size,
                             PROT_READ | PROT_WRITE,MAP_SHARED,pulp->fd,offset);
  if (pulp->mpcore.v_addr == MAP_FAILED) {
    printf("MMAP failed for shared L3 memory.\n");
    return -EIO;
  }
  else {
    printf("Zynq MPCore memory mapped to virtual user space at %p.\n",pulp->mpcore.v_addr);
  }

  return 0;
}

/**
 * Undo the memory mapping of the device to virtual user space using
 * the munmap() syscall
 *
 * @pulp: pointer to the PulpDev structure
 */
int pulp_munmap(PulpDev *pulp)
{
  unsigned status;

  // undo the memory mappings
  printf("Undo the memory mappings.\n");
  status = munmap(pulp->slcr.v_addr,pulp->slcr.size);
  if (status) {
    printf("MUNMAP failed for SLCR.\n");
  } 
  status = munmap(pulp->gpio.v_addr,pulp->gpio.size);
  if (status) {
    printf("MUNMAP failed for GPIO.\n");
  } 
  status = munmap(pulp->rab_config.v_addr,pulp->rab_config.size);
  if (status) {
    printf("MUNMAP failed for RAB config.\n");
  } 
  status = munmap(pulp->l2_mem.v_addr,pulp->l2_mem.size);
  if (status) {
    printf("MUNMAP failed for L2 memory.\n");
  } 
  status = munmap(pulp->soc_periph.v_addr,pulp->soc_periph.size);
  if (status) {
    printf("MUNMAP failed for SoC peripherals\n.");
  }
  status = munmap(pulp->clusters.v_addr,pulp->clusters.size);
  if (status) {
    printf("MUNMAP failed for clusters\n.");
  }
  status = munmap(pulp->l3_mem.v_addr,pulp->l3_mem.size);
  if (status) {
    printf("MUNMAP failed for shared L3 memory\n.");
  }
   
  // close the file descriptor
  printf("Close the file descriptor. \n");
  close(pulp->fd);

  return 0;
}

/**
 * Set the clock frequency of PULP, only do this at startup of PULP!!! 
 *
 * @pulp:         pointer to the PulpDev structure
 * @des_freq_mhz: desired frequency in MHz
 */
int pulp_clking_set_freq(PulpDev *pulp, unsigned des_freq_mhz)
{
  unsigned status;
  int timeout;
  int freq_mhz = des_freq_mhz - (des_freq_mhz % 5);
  if(freq_mhz <= 0)
    freq_mhz = 5;
  else if(freq_mhz >= 200)
    freq_mhz = 200;

  // input clock = 100 MHz
  // default output clock = 50 MHz
  int divclk_divide = 1;
  int clkfbout_mult = 5;
  int clkout0_divide = 500/freq_mhz;
  int clkout0_divide_frac = ((500 % freq_mhz) << 10)/freq_mhz;

  // config DIVCLK_DIVIDE, CLKFBOUT_MULT, CLKFBOUT_FRAC, CLKFBOUT_PHASE
  unsigned value;
  value = 0x04000000 + 0x100*clkfbout_mult + 0x1*divclk_divide;
  pulp_write32(pulp->clking.v_addr,CLKING_CONFIG_REG_0_OFFSET_B,'b',value);
  if (DEBUG_LEVEL > 3)
    printf("CLKING_CONFIG_REG_0: %#x\n",value);

  // config CLKOUT0/1/2: DIVIDE, FRAC, FRAC_EN
  value = 0x00040000 + 0x100*clkout0_divide_frac + 0x1*clkout0_divide;
  //pulp_write32(pulp->clking.v_addr,CLKING_CONFIG_REG_2_OFFSET_B,'b',value);
  pulp_write32(pulp->clking.v_addr,CLKING_CONFIG_REG_5_OFFSET_B,'b',value);
  pulp_write32(pulp->clking.v_addr,CLKING_CONFIG_REG_8_OFFSET_B,'b',value);
  if (DEBUG_LEVEL > 3)
    printf("CLKING_CONFIG_REG_5/8: %#x\n",value);

  // check status
  if ( !(pulp_read32(pulp->clking.v_addr,CLKING_STATUS_REG_OFFSET_B,'b') & 0x1) ) {
    timeout = 10;
    status = 1;
    while ( status && (timeout > 0) ) {
      usleep(10000);
      timeout--;
      status = !(pulp_read32(pulp->clking.v_addr,CLKING_STATUS_REG_OFFSET_B,'b') & 0x1);
    }
    if ( status ) {
      printf("ERROR: Clock manager not locked, cannot start reconfiguration.\n");
      return -EBUSY;
    } 
  }

  // start reconfiguration
  pulp_write32(pulp->clking.v_addr,CLKING_CONFIG_REG_23_OFFSET_B,'b',0x7);
  usleep(1000);
  pulp_write32(pulp->clking.v_addr,CLKING_CONFIG_REG_23_OFFSET_B,'b',0x2);

  // check status
  if ( !(pulp_read32(pulp->clking.v_addr,CLKING_STATUS_REG_OFFSET_B,'b') & 0x1) ) {
    timeout = 10;
    status = 1;
    while ( status && (timeout > 0) ) {
      usleep(10000);
      timeout--;
      status = !(pulp_read32(pulp->clking.v_addr,CLKING_STATUS_REG_OFFSET_B,'b') & 0x1);
    }
    if ( status ) {
      printf("ERROR: Clock manager not locked, clock reconfiguration failed.\n");
      return -EBUSY;
    } 
  }
 
  return freq_mhz;
}

/**
 * Measure the clock frequency of PULP. Can only be executed with the
 * RAB configured to allow accessing the cluster peripherals. To
 * validate the measurement, the ZYNQ_PMM needs to be loaded for
 * access to the ARM clock counter.
 *
 * @pulp:         pointer to the PulpDev structure
 */
int pulp_clking_measure_freq(PulpDev *pulp)
{
  unsigned seconds = 1;
  unsigned limit = (unsigned)((float)(ARM_CLK_FREQ_MHZ*100000*1.61)*seconds);

  unsigned pulp_clk_counter, arm_clk_counter;
  unsigned zynq_pmm;

  volatile unsigned k;
  int mes_freq_mhz;
  
  if( access("/dev/ZYNQ_PMM", F_OK ) != -1 )
    zynq_pmm = 1;
  else
    zynq_pmm = 0;

  // start clock counters
  if (zynq_pmm) {
    // enable clock counter divider (by 64), reset & enable clock counter, PMCR register 
    asm volatile("mcr p15, 0, %0, c9, c12, 0" :: "r"(0xD));
  }
  pulp_write32(pulp->clusters.v_addr,TIMER_STOP_OFFSET_B,'b',0x1);
  pulp_write32(pulp->clusters.v_addr,TIMER_RESET_OFFSET_B,'b',0x1);
  pulp_write32(pulp->clusters.v_addr,TIMER_START_OFFSET_B,'b',0x1);

  // wait but don't sleep
  k = 0;
  while (k<limit) {
    k++;
    k++;
    k++;
    k++;
    k++;
    k++;
    k++;
    k++;
    k++;
    k++;
  }

  // stop and read clock counters
  if (zynq_pmm) {
    // Read the counter value, PMCCNTR register
    asm volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(arm_clk_counter) : );
  }
  pulp_clk_counter = pulp_read32(pulp->clusters.v_addr,TIMER_GET_TIME_LO_OFFSET_B,'b');
 
  if (zynq_pmm) {
    mes_freq_mhz = (int)((float)pulp_clk_counter/((float)(arm_clk_counter*ARM_PMU_CLK_DIV)/ARM_CLK_FREQ_MHZ));
  }
  else {
    mes_freq_mhz = (int)((float)pulp_clk_counter/seconds/1000000);
  }

  return mes_freq_mhz;
}

/**
 * Initialize the memory mapped device 
 *
 * @pulp: pointer to the PulpDev structure
 */
int pulp_init(PulpDev *pulp)
{
  // set fetch enable to 0, set global clk enable, disable reset
  pulp_write32(pulp->gpio.v_addr,0x8,'b',0xC0000000);

  // RAB setup
  // port 0: Host -> PULP
  pulp_rab_req(pulp,L2_MEM_H_BASE_ADDR,L2_MEM_SIZE_B,0x7,0,RAB_MAX_DATE,RAB_MAX_DATE);     // L2
  //pulp_rab_req(pulp,MAILBOX_H_BASE_ADDR,MAILBOX_SIZE_B,0x7,0,RAB_MAX_DATE,RAB_MAX_DATE); // Mailbox, Interface 0
  pulp_rab_req(pulp,MAILBOX_H_BASE_ADDR,MAILBOX_SIZE_B*2,0x7,0,RAB_MAX_DATE,RAB_MAX_DATE); // Mailbox, Interface 0 and Interface 1
  pulp_rab_req(pulp,PULP_H_BASE_ADDR,CLUSTERS_SIZE_B,0x7,0,RAB_MAX_DATE,RAB_MAX_DATE);     // TCDM + Cluster Peripherals
  // port 1: PULP -> Host
  pulp_rab_req(pulp,L3_MEM_BASE_ADDR,L3_MEM_SIZE_B,0x7,1,RAB_MAX_DATE,RAB_MAX_DATE);       // L3 memory (contiguous)
  
  // enable mailbox interrupts
  pulp_write32(pulp->mailbox.v_addr,MAILBOX_IE_OFFSET_B,'b',0x6);
 
  // reset the l3_offset pointer
  pulp->l3_offset = 0;

  return 0;
}

/**
 * Read n_words words from mailbox, can block if the mailbox does not
 * contain enough data.
 *
 * @pulp      : pointer to the PulpDev structure
 * @buffer    : pointer to read buffer
 * @n_words   : number of words to read
 */
int pulp_mailbox_read(PulpDev *pulp, unsigned *buffer, unsigned n_words)
{
  int n_char, n_char_left, ret;
  ret = 1;
  n_char = 0;
  n_char_left = n_words*sizeof(buffer[0]);

  // read n_words words or until error
  while (n_char_left) {
    ret = read(pulp->fd, (char *)&buffer[n_char],n_char_left*sizeof(char));
    if (ret < 0) {
      printf("ERROR: Could not read mailbox.\n");
      return ret;
    }
    n_char += ret;
    n_char_left -= ret;
  }

  return 0;
}

/**
 * Clear interrupt status flag in mailbox. The next write of PULP will
 * again be handled by the PULP driver.
 *
 * @pulp: pointer to the PulpDev structure
 */
void pulp_mailbox_clear_is(PulpDev *pulp)
{
  pulp_write32(pulp->mailbox.v_addr,MAILBOX_IS_OFFSET_B,'b',0x7);
}

/**
 * Request a remapping (one or more RAB slices)
 *
 * @pulp      : pointer to the PulpDev structure
 * @addr_start: (virtual) start address
 * @size_b    : size of the remapping in bytes
 * @prot      : protection flags, one bit each for write, read, and enable
 * @port      : RAB port, 0 = Host->PULP, 1 = PULP->Host
 * @date_exp  : expiration date of the mapping
 * @date_cur  : current date, used to check for suitable slices
 */
int pulp_rab_req(PulpDev *pulp, unsigned addr_start, unsigned size_b, 
                 unsigned char prot, unsigned char port,
                 unsigned char date_exp, unsigned char date_cur)
{
  unsigned request[3];

  // setup the request
  request[0] = 0;
  RAB_SET_PROT(request[0], prot);
  RAB_SET_PORT(request[0], port);
  RAB_SET_DATE_EXP(request[0], date_exp);
  RAB_SET_DATE_CUR(request[0], date_cur);
  request[1] = addr_start;
  request[2] = size_b;
  
  // make the request
  ioctl(pulp->fd,PULP_IOCTL_RAB_REQ,request);
  
  return 0;
} 

/**
 * Free RAB slices
 *
 * @pulp      : pointer to the PulpDev structure
 * @date_cur  : current date, 0 = free all slices
 */
void pulp_rab_free(PulpDev *pulp, unsigned char date_cur) {
  
  // make the request
  ioctl(pulp->fd,PULP_IOCTL_RAB_FREE,(unsigned)date_cur);
  
  return;
} 

/**
 * Request striped remappings
 *
 * @pulp:       pointer to the PulpDev structure
 * @task:       pointer to the TaskDesc structure
 * @data_idxs:  pointer to array marking the elements to pass by reference
 * @n_elements: number of striped data elements
 * @prot      : protection flags, one bit each for write, read, and enable
 * @port      : RAB port, 0 = Host->PULP, 1 = PULP->Host
 */
int pulp_rab_req_striped(PulpDev *pulp, TaskDesc *task,
                         unsigned **data_idxs, int n_elements,  
                         unsigned char prot, unsigned char port)
{
  int i,j,k,m;

  /////////////////////////////////////////////////////////////////

  unsigned request[3];
  unsigned n_stripes, n_slices_max, n_fields, offload_id; 

  unsigned addr_start, addr_end, size_b, page_size_b;

  unsigned * max_stripe_size_b; 
  unsigned ** rab_stripes;

  // extracted from accelerator code
  // for ROD
  unsigned R, TILE_HEIGHT;
  unsigned w, h, n_bands, band_height, odd;
  unsigned tx_band_size_in = 0, tx_band_size_in_first = 0, tx_band_size_in_last = 0;
  unsigned tx_band_size_out = 0, tx_band_size_out_last = 0;
  unsigned overlap = 0;

  // for CT
  unsigned width, height, bHeight, nbBands;
  unsigned band_size_1ch = 0, band_size_3ch = 0;
  
  page_size_b = getpagesize();

  max_stripe_size_b = (unsigned *)malloc((size_t)(n_elements*sizeof(unsigned))); 
  rab_stripes = (unsigned **)malloc((size_t)(n_elements*sizeof(unsigned *))); 
  if ( (rab_stripes == NULL) || (max_stripe_size_b == NULL) ) {
    printf("ERROR: Malloc failed.\n");
    return -ENOMEM;
  }

  unsigned tx_band_start = 0;
  size_b = 0;
  offload_id = 0;

  if ( !strcmp(task->name, "profile_rab") ) { // valid for PROFILE_RAB only

    for (i=0;i<n_elements;i++) {
      max_stripe_size_b[i] = MAX_STRIPE_SIZE; 
    }

    n_stripes = (task->data_desc[0].size)/max_stripe_size_b[0];
    if (n_stripes * max_stripe_size_b[0] < task->data_desc[0].size)
      n_stripes++;

  }
  else if ( !strcmp(task->name, "rod") ) { // valid for ROD only

    // max sizes hardcoded
    max_stripe_size_b[0] = 0x1100;
    max_stripe_size_b[1] = 0x1100;
    max_stripe_size_b[2] = 0xe00;  
  
    R = 3;
    TILE_HEIGHT = 10;
    w = *(unsigned *)(task->data_desc[3].ptr);
    h = *(unsigned *)(task->data_desc[4].ptr);

    n_bands = h / TILE_HEIGHT;
    band_height = TILE_HEIGHT;
    odd = h - (n_bands * band_height);
  
    tx_band_size_in       = sizeof(unsigned char)*((band_height + (R << 1)) * w);
    tx_band_size_in_first = sizeof(unsigned char)*((band_height + R) * w);
    tx_band_size_in_last  = sizeof(unsigned char)*((band_height + odd + R ) * w);
    tx_band_size_out      = sizeof(unsigned char)*( band_height * w);
    tx_band_size_out_last = sizeof(unsigned char)*((band_height + odd )* w);
  
    overlap = sizeof(unsigned char)*(R * w);
    n_stripes = n_bands;

  }
  else if ( !strcmp(task->name, "ct") ) { // valid for CT only

    // max sizes hardcoded
    max_stripe_size_b[0] = 0x3000;

    width = *(unsigned *)(task->data_desc[1].ptr);
    height = *(unsigned *)(task->data_desc[2].ptr);
    bHeight = *(unsigned *)(task->data_desc[3].ptr);

    nbBands = (height / bHeight);
    //band_size_1ch = (width*bHeight);
    band_size_3ch = (width*bHeight*3);  
    //printf("buffer size = %#x \n",(band_size_3ch*2+band_size_1ch)*sizeof(unsigned char));

    n_stripes = nbBands;

  }
  else if ( !strcmp(task->name, "jpeg") ) { // valid for JPEG only

    // max sizes hardcoded
    max_stripe_size_b[0] = 0x1000;
    n_stripes = 18;

  }
  else {

    n_stripes = 1;
    printf("ERROR: Unknown task name %s\n",task->name);

  }
  
  i = -1;
  for (k=0; k<task->n_data; k++) {

    // check if element is passed by value or if the this element is not striped over
    if ( (*data_idxs)[k] < 2 )
      continue;
    i++;
    
    if (DEBUG_LEVEL > 2) {
      printf("size_b[%d] = %#x\n",i,task->data_desc[k].size);
      printf("max_stripe_size_b[%d] = %#x\n",i,max_stripe_size_b[i]);
    }
     
    n_slices_max = max_stripe_size_b[i]/page_size_b;
    if (n_slices_max*page_size_b < max_stripe_size_b[i])
      n_slices_max++; // remainder
    n_slices_max++;   // non-aligned

    n_fields = 2*n_slices_max + 1; // addr_start & addr_offset per slice, addr_end  

    rab_stripes[i] = (unsigned *)malloc((size_t)((n_stripes+1)*(n_fields)*sizeof(unsigned)));
    if ( rab_stripes[i] == NULL  ) {
      printf("ERROR: Malloc failed for rab_stripes[%i].\n",i);
      return -ENOMEM;
    }

    // fill in stripe data
    for (j=0; j<(n_stripes+1); j++) {

      // first stripe for output elements, last stripe for input elements
      if ( ((j == 0) && ((*data_idxs)[k] == 3)) || ((j == n_stripes) && ((*data_idxs)[k] == 2)) ) {
        addr_start = 0;
        addr_end = 0;
      }
      else if ( (j == n_stripes) && ((*data_idxs)[k] == 4))  { // last stripe for inout elements
        addr_start = 0xFFFFFFFF;
        addr_end = 0xFFFFFFFF;
      }
      else {
        addr_start = (unsigned)(task->data_desc[k].ptr);

        if ( !strcmp(task->name, "profile_rab") ) {
    
          size_b = max_stripe_size_b[k];
          tx_band_start = j*size_b;

        }
        else if ( !strcmp(task->name, "rod") ) {
    
          if ( (*data_idxs)[k] == 2 ) { // input elements
            if (j == 0) {
              tx_band_start = 0;
              size_b = tx_band_size_in_first;
            }
            else {
              tx_band_start = tx_band_size_in_first + tx_band_size_in * (j-1) - (overlap * (2 + (j-1)*2));
              if (j == n_stripes-1 )      
                size_b = tx_band_size_in_last;
              else
                size_b = tx_band_size_in;
            }
          }
          else {// 3, output elements
            tx_band_start = tx_band_size_out * (j-1);
            if (j == n_stripes ) 
              size_b = tx_band_size_out_last;
            else
              size_b = tx_band_size_out;
          }

        }
        else if ( !strcmp(task->name, "ct") ) {
    
          tx_band_start = j*band_size_3ch;
          size_b = band_size_3ch;

        }
        else if ( !strcmp(task->name, "jpeg") ) {
    
          tx_band_start = j*max_stripe_size_b[0];
          size_b = max_stripe_size_b[0];

        }
        else {
          printf("ERROR: Unknown task name %s\n",task->name);
        }

        addr_start += tx_band_start;
        addr_end = addr_start + size_b;
      }
 
      // write the rab_stripes table
      *(rab_stripes[i] + j*n_fields + 0) = addr_start;
      for (m = 1; m<(n_fields-1); m++)
        *(rab_stripes[i] + j*n_fields + m) = 0;
      *(rab_stripes[i] + j*n_fields + n_fields-1) = addr_end; 
    }

    if (DEBUG_LEVEL > 2) {
      printf("RAB stripe table @ %#x\n",(unsigned)rab_stripes[i]);
      printf("Shared Element %d: \n",k);
      for (j=0; j<(n_stripes+1); j++) {
        if (j>2 && j<(n_stripes+1-3))
          continue;
        printf("%d\t",j);
        for (m=0; m<n_fields; m++) {
          printf("%#x\t",*(rab_stripes[i] + j*n_fields + m));
        }
        printf("\n");
      }
    }
  }

  // set up the request
  request[0] = 0;
  RAB_SET_PROT(request[0], prot);
  RAB_SET_PORT(request[0], port);
  RAB_SET_OFFLOAD_ID(request[0], offload_id);
  RAB_SET_N_ELEM(request[0], n_elements);
  RAB_SET_N_STRIPES(request[0], n_stripes);

  request[1] = (unsigned)max_stripe_size_b; // addr of array holding max stripe sizes
  request[2] = (unsigned)rab_stripes;       // addr of array holding pointers to stripe data
  
  // make the request
  ioctl(pulp->fd,PULP_IOCTL_RAB_REQ_STRIPED,request);

  // free memory
  free(max_stripe_size_b);
  for (i=0; i<n_elements; i++) {
    free(rab_stripes[i]);
  }
  free(rab_stripes);

  return 0;
} 

/**
 * Free striped remappings
 *
 * @pulp      : pointer to the PulpDev structure
 */
void pulp_rab_free_striped(PulpDev *pulp)
{
  
  unsigned offload_id = 0;

  // make the request
  ioctl(pulp->fd,PULP_IOCTL_RAB_FREE_STRIPED,offload_id);
  
  return;
} 

void pulp_rab_mh_enable(PulpDev *pulp)
{
  ioctl(pulp->fd,PULP_IOCTL_RAB_MH_ENA);
  
  return;
}

void pulp_rab_mh_disable(PulpDev *pulp)
{
  ioctl(pulp->fd,PULP_IOCTL_RAB_MH_DIS);

  return;
}

/**
 * Setup a DMA transfer using the Zynq PS DMA engine
 *
 * @pulp      : pointer to the PulpDev structure
 * @addr_l3   : virtual address in host's L3
 * @addr_pulp : physical address in PULP, so far, only L2 tested
 * @size_b    : size in bytes
 * @host_read : 0: Host -> PULP, 1: PULP -> Host (not tested)
 */
int pulp_dma_xfer(PulpDev *pulp,
                  unsigned addr_l3, unsigned addr_pulp, unsigned size_b,
                  unsigned host_read)
{ 
  unsigned request[3];
  
  // check & process arguments
  if (size_b >> 31) {
    printf("ERROR: Requested transfer size too large - cannot encode DMA transfer direction.\n ");
    return -EINVAL;
  }
  else if (host_read) {
    BF_SET(size_b,1,31,1);
  }
  
  // setup the request
  request[0] = addr_l3;
  request[1] = addr_pulp;
  request[2] = size_b;

  // make the request
  ioctl(pulp->fd,PULP_IOCTL_DMAC_XFER,request);

  return 0;
}

/**
 * Setup a DMA transfer using the Zynq PS DMA engine
 *
 * @pulp : pointer to the PulpDev structure
 * @task : pointer to the TaskDesc structure
 */
int pulp_omp_offload_task(PulpDev *pulp, TaskDesc *task) {

  int i,n_idxs;
  unsigned * data_idxs;
 
  /*
   * RAB setup
   */
  data_idxs = (unsigned *)malloc(task->n_data*sizeof(unsigned));
  if ( data_idxs == NULL ) {
    printf("ERROR: Malloc failed for data_idxs.\n");
    return -ENOMEM;
  }
  // only remap addresses belonging to data elements larger than 32 bit
  n_idxs = pulp_offload_get_data_idxs(task, &data_idxs);
  
  // RAB setup
  pulp_offload_rab_setup(pulp, task, &data_idxs, n_idxs);
    
#ifndef PROFILE_RAB
  // Pass data descriptor to PULP
  pulp_offload_pass_desc(pulp, task, &data_idxs);
#endif  

  // free memory
  free(data_idxs);

  // check the virtual addresses
  if (DEBUG_LEVEL > 2) {
    for (i=0;i<task->n_data;i++) {
      printf("ptr_%d %#x\n",i,(unsigned) (task->data_desc[i].ptr));
    }
  }

  // reset sync address
  pulp_write32(pulp->l2_mem.v_addr,0xFFF8,'b',0);

  /*
   * offload
   */
  pulp_load_bin(pulp, task->name);
  pulp_exe_start(pulp);
    
#ifdef PROFILE_RAB
  pulp_write32(pulp->mailbox.v_addr,MAILBOX_WRDATA_OFFSET_B,'b',PULP_START);
#endif

  // poll l2_mem address for finish
  volatile int done;
  done = 0;
  while (!done) {
    done = pulp_read32(pulp->l2_mem.v_addr,0xFFF8,'b');
    usleep(100000);
    //printf("Waiting...\n");
  }
  pulp_write32(pulp->gpio.v_addr,0x8,'b',0xC0000000);
  
  // reset sync address
  pulp_write32(pulp->l2_mem.v_addr,0xFFF8,'b',0);

  // -> dynamic linking here?
  
  // tell PULP where to get the binary from and to start
  // -> write address to mailbox

  // if sync, wait for PULP to finish
  // else configure the driver to listen for the interrupt and which process to wakeup

  return 0;
}

/**
 * Reset PULP
 * @pulp : pointer to the PulpDev structure
 * @full : type of reset: 0 for PULP reset, 1 for entire FPGA
 */
void pulp_reset(PulpDev *pulp, unsigned full)
{
  unsigned slcr_value;

  // FPGA reset control register
  slcr_value = pulp_read32(pulp->slcr.v_addr, SLCR_FPGA_RST_CTRL_OFFSET_B, 'b');
 
  // extract the FPGA_OUT_RST bits
  slcr_value = slcr_value & 0xF;


  if (full) {
    // enable reset
    pulp_write32(pulp->slcr.v_addr, SLCR_FPGA_RST_CTRL_OFFSET_B, 'b', 0xF);

    // wait
    usleep(100000);
    
    // disable reset
    pulp_write32(pulp->slcr.v_addr, SLCR_FPGA_RST_CTRL_OFFSET_B, 'b', slcr_value);

  }
  else {
    // enable reset
    pulp_write32(pulp->slcr.v_addr, SLCR_FPGA_RST_CTRL_OFFSET_B, 'b',
                 slcr_value | (0x1 << SLCR_FPGA_OUT_RST));
    
    // wait
    usleep(100000);
    
    // disable reset
    pulp_write32(pulp->slcr.v_addr, SLCR_FPGA_RST_CTRL_OFFSET_B, 'b', 
                 slcr_value );

    // reset using GPIO register
    pulp_write32(pulp->gpio.v_addr,0x8,'b',0x00000000);
    usleep(100000);
    pulp_write32(pulp->gpio.v_addr,0x8,'b',0xC0000000);
  }
  // temporary fix: global clk enable
  pulp_write32(pulp->gpio.v_addr,0x8,'b',0xC0000000);
}

/**
 * Boot PULP.  
 *
 * @pulp : pointer to the PulpDev structure 
 * @task : pointer to the TaskDesc structure
 */
int pulp_boot(PulpDev *pulp, TaskDesc *task)
{
  int err;

  // load the binary
  err = pulp_load_bin(pulp, task->name);
  if (err) {
    printf("ERROR: Load of PULP binary failed.\n");
    return err;
  }
  
  // start execution
  pulp_exe_start(pulp);

  return 0;
}

/**
 * Load binary to PULP. Not yet uses the Zynq PS DMA engine.
 *
 * @pulp : pointer to the PulpDev structure 
 * @name : pointer to the string containing the name of the 
 *         application to load
 */
int pulp_load_bin(PulpDev *pulp, char *name)
{
  int i;

  // prepare binary reading
  char * bin_name;
  bin_name = (char *)malloc((strlen(name)+4+1)*sizeof(char));
  if (!bin_name) {
    printf("ERROR: Malloc failed for bin_name.\n");
    return -ENOMEM;
  }
  strcpy(bin_name,name);
  strcat(bin_name,".bin");
  
  printf("Loading binary file: %s\n",bin_name);

  // read in binary
  FILE *fp;
  if((fp = fopen(bin_name, "r")) == NULL) {
    printf("ERROR: Could not open PULP binary.\n");
    return -ENOENT;
  }
  int sz, nsz;
  unsigned *bin;
  fseek(fp, 0L, SEEK_END);
  sz = ftell(fp);
  fseek(fp, 0L, SEEK_SET);
  bin = (unsigned *) malloc(sz*sizeof(char));
  if((nsz = fread(bin, sizeof(char), sz, fp)) != sz)
    printf("ERROR: Red only %d bytes in binary.\n", nsz);
  fclose(fp);
  
  // write binary to L2
  for (i=0; i<nsz/4; i++)
    pulp->l2_mem.v_addr[i] = bin[i];

  return 0;
}

/**
 * Starts programm execution on PULP.
 *
 * @pulp : pointer to the PulpDev structure 
 */
void pulp_exe_start(PulpDev *pulp)
{
  printf("Starting program execution.\n");
  pulp_write32(pulp->gpio.v_addr,0x8,'b',0xC0000001);

  return;
}

/**
 * Stops programm execution on PULP.
 *
 * @pulp : pointer to the PulpDev structure 
 */
void pulp_exe_stop(PulpDev *pulp)
{
  printf("Stopping program execution.\n");
  pulp_write32(pulp->gpio.v_addr,0x8,'b',0xC0000000);

  return;
}

/**
 * Polls the GPIO register for the end of computation signal for at
 * most timeous_s seconds.
 *
 * @pulp      : pointer to the PulpDev structure
 * @timeout_s : maximum number of seconds to wait for end of 
 *              computation
 */
int pulp_exe_wait(PulpDev *pulp, int timeout_s)
{
  unsigned status;
  float interval_us = 100000;
  float timeout = 0;

  if ( !(pulp_read32(pulp->gpio.v_addr,0,'b') & 0x1) ) {
    timeout = (float)timeout_s*1000000/interval_us;
    status = 1;
    while ( status && (timeout > 0) ) {
      usleep(interval_us);
      timeout--;
      status = !(pulp_read32(pulp->gpio.v_addr,0,'b') & 0x1);
    }
    if ( status ) {
      printf("ERROR: PULP execution timeout.\n");
      return -ETIME;
    }
  }
 
  return 0;
}

/**
 * Allocate memory in contiguous L3
 *
 * @pulp:   pointer to the PulpDev structure
 * @size_b: size in Bytes of the requested chunk
 * @p_addr: pointer to store the physical address to
 *
 * ATTENTION: This function can only allocate each address once!
 *
 */
unsigned int pulp_l3_malloc(PulpDev *pulp, size_t size_b, unsigned *p_addr)
{
  unsigned int v_addr;

  // round l3_offset to next higher 64-bit word -> required for PULP DMA
  if (pulp->l3_offset & 0x7) {
    pulp->l3_offset = (pulp->l3_offset & 0xFFFFFFF8) + 0x8;
  }

  if ( (pulp->l3_offset + size_b) >= L3_MEM_SIZE_B) {
    printf("ERROR: out of contiguous L3 memory.\n");
    *p_addr = 0;
    return 0;
  }
  
  v_addr = (unsigned int)pulp->l3_mem.v_addr + pulp->l3_offset;
  *p_addr = L3_MEM_BASE_ADDR + pulp->l3_offset;
 
  pulp->l3_offset += size_b;

  if (DEBUG_LEVEL > 2) {
    printf("Host virtual address = %#x \n",v_addr);
    printf("PMCA physical address = %#x \n",*p_addr);
  }
  
  return v_addr;
}

/**
 * Free memory previously allocated in contiguous L3
 *
 * @pulp:   pointer to the PulpDev structure
 * @v_addr: pointer to unsigned containing the virtual address
 * @p_addr: pointer to unsigned containing the physical address
 *
 * ATTENTION: This function does not do anything!
 *
 */
void pulp_l3_free(PulpDev *pulp, unsigned v_addr, unsigned p_addr)
{
  return;
}

/**
 * Find out which shared data elements to pass by reference. 
 *
 * @task :     pointer to the TaskDesc structure
 * @data_idxs: pointer to array marking the elements to pass by reference
 */
int pulp_offload_get_data_idxs(TaskDesc *task, unsigned **data_idxs) {
  
  int i, n_data, n_idxs, size_b;

  n_data = task->n_data;
  n_idxs = 0;
  size_b = sizeof(unsigned);

  for (i=0; i<n_data; i++) {
    if ( task->data_desc[i].size > size_b ) {
      (*data_idxs)[i] = 1;
      n_idxs++;
    }
    else
      (*data_idxs)[i] = 0;
  }

  return n_idxs;
}

/**
 * Try to reorder the shared data elements to minimize the number of
 * calls to the driver. Call the driver to set up RAB slices.
 *
 * @task:      pointer to the TaskDesc structure
 * @data_idxs: pointer to array marking the elements to pass by reference
 * @n_idxs:    number of shared data elements passed by reference
 */
int pulp_offload_rab_setup(PulpDev *pulp, TaskDesc *task, unsigned **data_idxs, int n_idxs)
{
  int i, j;
  unsigned char prot, port;
  unsigned char date_cur, date_exp;

  unsigned n_data, n_data_int, gap_size, temp;
  unsigned * v_addr_int;
  unsigned * size_int;
  unsigned * order;

  n_data_int = 1;

  // Mark striped data elements
  if ( !strcmp(task->name, "profile_rab") ) { // valid for PROFILE_RAB only
        
    n_data_int = 0;
    for (i=0; i<task->n_data; i++) {
      (*data_idxs)[i] = 2; // striped in
    }
    n_idxs -= task->n_data;

  }
  else if ( !strcmp(task->name, "rod") ) { // valid for ROD only

    (*data_idxs)[0] = 2; // striped in
    (*data_idxs)[1] = 2; // striped in 
    (*data_idxs)[2] = 3; // striped out (shifted)
    n_idxs -= 3;

  }
  else if ( !strcmp(task->name, "ct") ) { // valid for CT only
    
    (*data_idxs)[0] = 2; // striped in
    n_idxs -= 1;

  }
  else if ( !strcmp(task->name, "jpeg") ) { // valid for JPEG only
    
    (*data_idxs)[3] = 4; // striped inout
    n_idxs -= 1;

  }
  else {
    if ( strcmp(task->name, "face_detect") )
      printf("ERROR: Unknown task name %s\n",task->name);
  }

  // !!!!TO DO: check type and set protections!!!
  prot = 0x7; 
  port = 1;   // PULP -> Host
  
  n_data = task->n_data;

  date_cur = (unsigned char)(task->task_id + 1);
  date_exp = (unsigned char)(task->task_id + 3);

  // memory allocation
  v_addr_int = (unsigned *)malloc((size_t)n_idxs*sizeof(unsigned));
  size_int = (unsigned *)malloc((size_t)n_idxs*sizeof(unsigned));
  order = (unsigned *)malloc((size_t)n_idxs*sizeof(unsigned));
  if (!v_addr_int | !size_int | !order) {
    printf("Malloc failed for RAB setup.\n");
    return -ENOMEM;
  }
  j=0;
  for (i=0;i<n_data;i++) {
    if ( (*data_idxs)[i] == 1 ) {
      order[j] = i;
      j++;
    }
  }

  // order the elements - bubble sort
  for (i=n_idxs;i>1;i--) {
    for (j=0;j<i-1;j++) {
      if (task->data_desc[j].ptr > task->data_desc[j+1].ptr) {
        temp = order[j];
        order[j] = order[j+1];
        order[j+1] = temp;
      }
    }
  }
  if (DEBUG_LEVEL > 2) {
    printf("Reordered %d data elements: \n",n_idxs);
    for (i=0;i<n_idxs;i++) {
      printf("%d \t %#x \t %#x \n", order[i],
             (unsigned)task->data_desc[order[i]].ptr,
             (unsigned)task->data_desc[order[i]].size);
    }
  }

  // determine the number of remappings/intervals to request
  v_addr_int[0] = (unsigned)task->data_desc[order[0]].ptr; 
  size_int[0] = (unsigned)task->data_desc[order[0]].size; 
  for (i=1;i<n_idxs;i++) {
    j = order[i];
    gap_size = (unsigned)task->data_desc[j].ptr - (v_addr_int[n_data_int-1]
                                                   + size_int[n_data_int-1]);
    // !!!!TO DO: check protections, check dates!!!
    if ( gap_size > RAB_CONFIG_MAX_GAP_SIZE_B ) { 
      // the gap is too large, create a new mapping
      n_data_int++;
      v_addr_int[n_data_int-1] = (unsigned)task->data_desc[j].ptr;
      size_int[n_data_int-1] = (unsigned)task->data_desc[j].size;
    }
    else {
      // extend the previous mapping
      size_int[n_data_int-1] += (gap_size + task->data_desc[j].size);  
    }
  }

  // set up the RAB
  if (DEBUG_LEVEL > 2) {
    printf("Requesting %d remapping(s):\n",n_data_int);
  }
  for (i=0;i<n_data_int;i++) {
    if (DEBUG_LEVEL > 2) {
      printf("%d \t %#x \t %#x \n",i,v_addr_int[i],size_int[i]);
      usleep(1000000);
    }
    pulp_rab_req(pulp, v_addr_int[i], size_int[i], prot, port, date_exp, date_cur);
  }

  // set up RAB stripes
  //pulp_rab_req_striped(pulp, task, data_idxs, n_idxs, prot, port);
  if ( !strcmp(task->name, "profile_rab") ) {
    pulp_rab_req_striped(pulp, task, data_idxs, task->n_data, prot, port);
  }
  else if ( !strcmp(task->name, "rod") ) {
    pulp_rab_req_striped(pulp, task, data_idxs, 3, prot, port);
  }
  else if ( !strcmp(task->name, "ct") ) {
    pulp_rab_req_striped(pulp, task, data_idxs, 1, prot, port);
  }
  else if ( !strcmp(task->name, "jpeg") ) {
    pulp_rab_req_striped(pulp, task, data_idxs, 1, prot, port);
  }
  else {
    if ( strcmp(task->name, "face_detect") )
      printf("ERROR: Unknown task name %s\n",task->name);
  }

  // free memory
  free(v_addr_int);
  free(size_int);
  free(order);
  
  return 0;
}

/**
 * Pass the descriptors of the shared data elements to PULP.
 * @pulp:      pointer to the PulpDev structure
 * @task:      pointer to the TaskDesc structure
 * @data_idxs: pointer to array marking the elements to pass by reference
 */
int pulp_offload_pass_desc(PulpDev *pulp, TaskDesc *task, unsigned **data_idxs)
{
  int i, timeout, us_delay;
  unsigned status, n_data;

  n_data = task->n_data;

  us_delay = 100;

  if (DEBUG_LEVEL > 2) {
    printf("Mailbox status = %#x.\n",pulp_read32(pulp->mailbox.v_addr, MAILBOX_STATUS_OFFSET_B, 'b'));
  }

  for (i=0;i<n_data;i++) {

    // check if mailbox is full
    if ( pulp_read32(pulp->mailbox.v_addr, MAILBOX_STATUS_OFFSET_B, 'b') & 0x2 ) {
      timeout = 1000;
      status = 1;
      // wait for not full or timeout
      while ( status && (timeout > 0) ) {
        usleep(us_delay);
        timeout--;
        status = (pulp_read32(pulp->mailbox.v_addr, MAILBOX_STATUS_OFFSET_B, 'b') & 0x2);
      }
      if ( status ) {
        printf("ERROR: mailbox timeout.\n");
        return i;
      } 
    }

    // mailbox is ready to receive
    if ( (*data_idxs)[i] ) {
      // pass data element by reference
      pulp_write32(pulp->mailbox.v_addr,MAILBOX_WRDATA_OFFSET_B,'b',
                   (unsigned)(task->data_desc[i].ptr));
      if (DEBUG_LEVEL > 2)
        printf("Element %d: wrote %#x to mailbox.\n",i,(unsigned) (task->data_desc[i].ptr));
    }
    else {
      // pass data element by value
      pulp_write32(pulp->mailbox.v_addr,MAILBOX_WRDATA_OFFSET_B,'b',
                   *(unsigned *)(task->data_desc[i].ptr));
      if (DEBUG_LEVEL > 2)
        printf("Element %d: wrote %#x to mailbox.\n",i,*(unsigned*)(task->data_desc[i].ptr));
    }    
  }
  
  if (DEBUG_LEVEL > 1) {
    printf("Passed %d of %d data elements to PULP.\n",i,n_data);
  }

  return i;
}

/**
 * Get back the shared data elements from PULP that were passed by value.
 *
 * @pulp:      pointer to the PulpDev structure
 * @task:      pointer to the TaskDesc structure
 * @data_idxs: pointer to array marking the elements to pass by reference
 * @n_idxs:    number of shared data elements passed by reference
 */
int pulp_offload_get_desc(PulpDev *pulp, TaskDesc *task, unsigned **data_idxs, int n_idxs)
{
  int i,j, n_data;
  unsigned *buffer;

  j = 0;
  n_data = task->n_data;

  buffer = (unsigned *)malloc((n_data-n_idxs)*sizeof(unsigned));
  if ( buffer == NULL ) {
    printf("ERROR: Malloc failed for buffer.\n");
    return -ENOMEM;
  }
  
  // read from mailbox
  pulp_mailbox_read(pulp, &buffer[0], n_data-n_idxs);
  
  for (i=0; i<n_data; i++) {
    // check if argument has been passed by value
    if ( (*data_idxs)[i] == 0 ) {
      // read from buffer
      *(unsigned *)(task->data_desc[i].ptr) = buffer[j];
      j++;
    }
  }
  
  //for (i=0; i<n_data; i++) {
  //  // check if argument has been passed by value
  //  if ( (*data_idxs)[i] == 0 ) {
  //    // read from mailbox
  //    pulp_mailbox_read(pulp, task->data_desc[i].ptr, 1);
  //    j++;
  //  }
  //}
  
  if (DEBUG_LEVEL > 1) {
    printf("Got back %d of %d data elements from PULP.\n",j,n_data-n_idxs);
  }

#ifndef JPEG // generates error
  free(buffer);
#endif

  return j;
}

/**
 * Offload a new task to PULP, set up RAB slices and pass descriptors
 * to PULP. 
 *
 * @pulp: pointer to the PulpDev structure 
 * @task: pointer to the TaskDesc structure
 */
int pulp_offload_out(PulpDev *pulp, TaskDesc *task)
{
  int err, n_idxs;
  unsigned *data_idxs;
  data_idxs = (unsigned *)malloc(task->n_data*sizeof(unsigned));
  if ( data_idxs == NULL ) {
    printf("ERROR: Malloc failed for data_idxs.\n");
    return -ENOMEM;
  }

  // only remap addresses belonging to data elements larger than 32 bit
  n_idxs = pulp_offload_get_data_idxs(task, &data_idxs);
  
#if (MEM_SHARING != 3)
  // RAB setup
  err = pulp_offload_rab_setup(pulp, task, &data_idxs, n_idxs);
  if (err) {
    printf("ERROR: pulp_offload_rab_setup failed.\n");
    return err;
  }
#endif

  // Pass data descriptor to PULP
  err = pulp_offload_pass_desc(pulp, task, &data_idxs);
  if ( err != task->n_data ) {
    printf("ERROR: pulp_offload_pass_desc failed.\n");
    return err;
  }

  // free memory
  free(data_idxs);

  return 0;
}

/**
 * Finish a task offload, clear RAB slices and get back descriptors
 * passed by value.
 *
 * @pulp: pointer to the PulpDev structure 
 * @task: pointer to the TaskDesc structure
 */
int pulp_offload_in(PulpDev *pulp, TaskDesc *task)
{
  unsigned char date_cur;
  int err, n_idxs;
  unsigned *data_idxs;
  data_idxs = (unsigned *)malloc(task->n_data*sizeof(unsigned));
  if ( data_idxs == NULL ) {
    printf("ERROR: Malloc failed for data_idxs.\n");
    return -ENOMEM;
  }

  // read back data elements with sizes up to 32 bit from mailbox
  n_idxs = pulp_offload_get_data_idxs(task, &data_idxs);
 
#if (MEM_SHARING != 3) 
  // free RAB slices
  date_cur = (unsigned char)(task->task_id + 4);
  pulp_rab_free(pulp, date_cur);

  if ( !strcmp(task->name, "profile_rab") || !strcmp(task->name, "rod")
       || !strcmp(task->name, "ct") || !strcmp(task->name, "jpeg") ) {
    // free striped RAB slices
    pulp_rab_free_striped(pulp);   
  }
#endif

  // fetch values of data elements passed by value
  err = pulp_offload_get_desc(pulp, task, &data_idxs, n_idxs);
  if ( err != (task->n_data - n_idxs) ) {
    printf("ERROR: pulp_offload_get_desc failed.\n");
    return err;
  }

  // free memory
  free(data_idxs);

  return 0;
}

/**
 * Start offload execution on PULP.
 *
 * @pulp: pointer to the PulpDev structure 
 * @task: pointer to the TaskDesc structure
 */
int pulp_offload_start(PulpDev *pulp, TaskDesc *task)
{
  unsigned status;

  if (DEBUG_LEVEL > 2) {
    printf("Mailbox status = %#x.\n",pulp_read32(pulp->mailbox.v_addr, MAILBOX_STATUS_OFFSET_B, 'b'));
  }
  
  // read status
  pulp_mailbox_read(pulp, &status, 1);
  if ( status != PULP_READY ) {
    printf("ERROR: PULP status not ready. PULP status = %#x.\n",status);
    return -EBUSY;
  }

  // check if mailbox is full
  if ( pulp_read32(pulp->mailbox.v_addr, MAILBOX_STATUS_OFFSET_B, 'b') & 0x2 ) {
    printf("ERROR: PULP mailbox full.\n");
    return -EXFULL;
  } 
  
  // start execution
  pulp_write32(pulp->mailbox.v_addr, MAILBOX_WRDATA_OFFSET_B, 'b', PULP_START);

  return 0;
}

/**
 * Wait for an offloaded task to finish on PULP.
 *
 * @pulp: pointer to the PulpDev structure 
 * @task: pointer to the TaskDesc structure
 */
int pulp_offload_wait(PulpDev *pulp, TaskDesc *task)
{
  unsigned status;

  // check if sync = 0x1
  status = 0;
  while ( status != PULP_DONE ) {
    status = pulp_read32(pulp->l2_mem.v_addr, SYNC_OFFSET_B,'b');
  }

  // read status
  pulp_mailbox_read(pulp, &status, 1);
  if ( status != PULP_DONE ) {
    printf("ERROR: PULP status not done. PULP status = %#x.\n",status);
    return -EBUSY;
  }

  return 0;
}

//
int pulp_offload_out_contiguous(PulpDev *pulp, TaskDesc *task, TaskDesc **ftask)
{
  // similar to pulp_offload_out() but without RAB setup
  int err;
  unsigned *data_idxs;
  data_idxs = (unsigned *)malloc(task->n_data*sizeof(unsigned));
  if ( data_idxs == NULL ) {
    printf("ERROR: Malloc failed for data_idxs.\n");
    return -ENOMEM;
  }

  // only remap addresses belonging to data elements larger than 32 bit  
  pulp_offload_get_data_idxs(task, &data_idxs);

  int i;
  int data_size, data_ptr, data_type;

  *ftask = (TaskDesc *)malloc(sizeof(TaskDesc));
  if ( *ftask == NULL ) {
    printf("ERROR: Malloc failed for ftask.\n");
    return -ENOMEM;
  }

  (*ftask)->task_id = task->task_id;
  (*ftask)->name = task->name;
  (*ftask)->n_clusters = task->n_clusters;
  (*ftask)->n_data = task->n_data;

  if ( !strcmp(task->name, "rod") ) {
#ifdef PROFILE   
    (*ftask)->data_desc  = (DataDesc *)malloc(9*sizeof(DataDesc));
#else
    (*ftask)->data_desc  = (DataDesc *)malloc(6*sizeof(DataDesc));
#endif // PROFILE    
  }
  else if ( !strcmp(task->name, "ct") ) {
#ifdef PROFILE   
    (*ftask)->data_desc  = (DataDesc *)malloc(8*sizeof(DataDesc));
#else
    (*ftask)->data_desc  = (DataDesc *)malloc(6*sizeof(DataDesc));
#endif // PROFILE    
  }
  else if ( !strcmp(task->name, "jpeg") ) {
#ifdef PROFILE   
    (*ftask)->data_desc  = (DataDesc *)malloc(10*sizeof(DataDesc));
#else
    (*ftask)->data_desc  = (DataDesc *)malloc(7*sizeof(DataDesc));
#endif // PROFILE    
  }
  else {
    printf("ERROR: Unknown task name %s\n",task->name);
  }

  if ( ((*ftask)->data_desc) == NULL ) {
    printf("ERROR: Malloc failed for data_desc.\n");
    return -ENOMEM;
  }
 
  // memory allocation in contiguous L3 memory for IN and OUT
  // copy to contiguous L3 memory for IN 
  for (i = 0; i < (*ftask)->n_data; i++) {
    
    data_size = task->data_desc[i].size;
    data_ptr  = (int)task->data_desc[i].ptr;

    // data element to pass by reference, allocate contiguous L3 memory
    if (data_idxs[i] > 0) {
                  
      data_type = task->data_desc[i].type;
     
      // we are going to abuse type to store the virtual host address and
      // ptr to store the physical address in contiguous L3 used by PULP

      switch(data_type) {
      case 0:
        //INOUT
        (*ftask)->data_desc[i].type = (int)pulp_l3_malloc(pulp, data_size,
                                                          (unsigned *)&((*ftask)->data_desc[i].ptr));
        memcpy((void *)(*ftask)->data_desc[i].type, (void *)data_ptr ,data_size);
        break;
                    
      case 1:
        //IN
        (*ftask)->data_desc[i].type = (int)pulp_l3_malloc(pulp, data_size,
                                                          (unsigned *)&((*ftask)->data_desc[i].ptr));
        memcpy((void *)(*ftask)->data_desc[i].type, (void *)data_ptr ,data_size);
        break;
                    
      case 2:
        //OUT
        (*ftask)->data_desc[i].type = (int)pulp_l3_malloc(pulp, data_size,
                                                          (unsigned *)&((*ftask)->data_desc[i].ptr));
        break;
                    
      default:
        //NONE
        break;
      }
    }
    
    // data element to pass by value (just copy descriptor)
    else  {
      (*ftask)->data_desc[i].ptr = (void *)data_ptr;
    }
    (*ftask)->data_desc[i].size = data_size;
  }

  // Pass data descriptor to PULP
  err = pulp_offload_pass_desc(pulp, *ftask, &data_idxs);
  if ( err != task->n_data ) {
    printf("ERROR: pulp_offload_pass_desc failed.\n");
    return err;
  }

  // free memory
  free(data_idxs);
  
  return 0;
}

int pulp_offload_in_contiguous(PulpDev *pulp, TaskDesc *task, TaskDesc **ftask)
{
  int i;

  // similar to pulp_offload_in() but without RAB stuff 
  int err, n_idxs;
  unsigned *data_idxs;
  data_idxs = (unsigned *)malloc(task->n_data*sizeof(unsigned));
  if ( data_idxs == NULL ) {
    printf("ERROR: Malloc failed for data_idxs.\n");
    return -ENOMEM;
  }

  // read back data elements with sizes up to 32 bit from mailbox
  n_idxs = pulp_offload_get_data_idxs(task, &data_idxs);

  // fetch values of data elements passed by value
  err = pulp_offload_get_desc(pulp, task, &data_idxs, n_idxs);
  if ( err != (task->n_data - n_idxs) ) {
    printf("ERROR: pulp_offload_get_desc failed.\n");
    return err;
  }

  // copy back result to virtual paged memory
  for (i = 0; i < (*ftask)->n_data; i++) {

    //if not passed by value 
    if (data_idxs[i] > 0) {
  
      // we are abusing type of ftask->dta_desc  to store the virtual host address and
      // ptr to store the physical address in contiguous L3 used by PULP

      switch(task->data_desc[i].type) {
      case 0:
        //INOUT
        memcpy((void *)task->data_desc[i].ptr, (void *)(*ftask)->data_desc[i].type, task->data_desc[i].size);
        pulp_l3_free(pulp, (unsigned)(*ftask)->data_desc[i].type, (unsigned)(*ftask)->data_desc[i].ptr);
        break;
                    
      case 1:
        //IN                    
        pulp_l3_free(pulp, (unsigned)(*ftask)->data_desc[i].type, (unsigned)(*ftask)->data_desc[i].ptr);
        break;

      case 2:
        //OUT
        memcpy((void *)task->data_desc[i].ptr, (void *)(*ftask)->data_desc[i].type, task->data_desc[i].size);
        pulp_l3_free(pulp, (unsigned)(*ftask)->data_desc[i].type, (unsigned)(*ftask)->data_desc[i].ptr);
        break;
                    
      default:
        //NONE
        break;
      }
    }
  }

  // free memory
  free((*ftask)->data_desc);
  free(*ftask);
  free(data_idxs);

  return 0;
}

/****************************************************************************************/
int pulp_rab_req_striped_mchan_img(PulpDev *pulp, unsigned char prot, unsigned char port,
                                   unsigned p_height, unsigned p_width, unsigned i_step,
                                   unsigned n_channels, unsigned char **channels,
                                   unsigned *s_height)
{
  int i,j,k;

  unsigned request[3];
  unsigned stripe_size_b, stripe_height, n_stripes_per_channel;
  unsigned n_stripes, n_slices_max, n_fields, offload_id;

  unsigned addr_start, addr_end, page_size_b;

  unsigned * max_stripe_size_b;
  unsigned ** rab_stripes;

  page_size_b = getpagesize();
  offload_id = 0;

  // compute max stripe height
  stripe_size_b = (RAB_N_SLICES/2-1)*page_size_b;
  stripe_height = stripe_size_b / i_step;    
  
  n_stripes_per_channel = p_height / stripe_height;
  if (p_height % stripe_height)
    n_stripes_per_channel++;

  // compute effective stripe height
  stripe_height = p_height / n_stripes_per_channel;
  *s_height = stripe_height;
  stripe_size_b = stripe_height * i_step;
     
  if (DEBUG_LEVEL > 2) {
    printf("n_stripes_per_channel = %d\n", n_stripes_per_channel);
    printf("stripe_size_b = %#x\n", stripe_size_b);
    printf("s_height = %d\n", stripe_height);
    printf("page_size_b = %#x\n",page_size_b);
  }

  // generate the rab_stripes table
  n_stripes = n_stripes_per_channel * n_channels;

  max_stripe_size_b = &stripe_size_b;
  rab_stripes = (unsigned **)malloc((size_t)(sizeof(unsigned *)));
  if (rab_stripes == NULL) {
    printf("ERROR: Malloc failed for rab_stripes.\n");
    return -ENOMEM;
  }
  
  n_slices_max = *max_stripe_size_b/page_size_b;
  if (n_slices_max*page_size_b < *max_stripe_size_b)
    n_slices_max++; // remainder
  n_slices_max++;   // non-aligned

  n_fields = 2*n_slices_max + 1; // addr_start & addr_offset per slice, addr_end

  *rab_stripes = (unsigned *)malloc((size_t)((n_stripes+1)*(n_fields)*sizeof(unsigned)));
  if (*rab_stripes == NULL) {
    printf("ERROR: Malloc failed for *rab_stripes.\n");
    return -ENOMEM;
  }
  memset(*rab_stripes, 0, (n_stripes+1)*(n_fields)*sizeof(unsigned));
  
  for (i=0; i<n_channels; i++) {
    for (j=0; j<n_stripes_per_channel; j++) {
      // align to words
      addr_start = ((unsigned)channels[i] + stripe_size_b*j);
      BF_SET(addr_start,0,0,4);
      addr_end   = ((unsigned)channels[i] + stripe_size_b*(j+1));
      BF_SET(addr_end,0,0,4);
      addr_end += 0x4;
      *(*rab_stripes + i*n_stripes_per_channel*n_fields + j*n_fields + 0) = addr_start; 
      for (k=1; k<(n_fields-1); k++) {
        *(*rab_stripes + i*n_stripes_per_channel*n_fields + j*n_fields + k) = 0;
      }
      *(*rab_stripes + i*n_stripes_per_channel*n_fields + j*n_fields + n_fields-1) = addr_end;
    }
  }

  if (DEBUG_LEVEL > 2) {
    printf("RAB stripe table @ %#x\n",(unsigned)rab_stripes);
    for (i=0; i<(n_stripes+1); i++) {
      if (i>2 && i<(n_stripes+1-3))
        continue;
      printf("%d\t",i);
      for (j=0; j<n_fields; j++) {
        printf("%#x\t",*(*rab_stripes + i*n_fields + j));
      }
      printf("\n");
    }
  }

  // write the img to a file
  //#define PATCH2FILE
  //#define IMG2FILE
#if defined(IMG2FILE) || defined(PATCH2FILE) 
  
  FILE *fp;
  
  // open the file  
  if((fp = fopen("img.h", "a")) == NULL) {
    printf("ERROR: Could not open img.h.\n");
    return -ENOENT;
  }

  printf("p_width  = %d\n",p_width);
  printf("p_height = %d\n",p_height);
  printf("i_step   = %d\n",i_step);
 
#if defined(PATCH2FILE)

  //write start
  printf(fp, "unsigned char img[%d] = {\n",p_width * p_height * n_channels);

  for (i=0; i<n_channels; i++) {
    fprintf(fp,"// Channel %d: \n",i);
    for (j=0; j<p_height; j++) {
      for (k=0; k<p_width; k++) {
        fprintf(fp,"\t%#x,",(unsigned) *(channels[i]+j*i_step+k));
      }
      fprintf(fp,"\n");
    }
  }

#elif defined(IMG2FILE)

  unsigned i_height, i_width;
  i_height = 21;
  i_width = 37;

  // write start
  fprintf(fp, "unsigned char img[%d] = {\n",i_height * i_step * n_channels);

  for (i=0; i<n_channels; i++) {
    //if ((i == 0) || (i == 16)) {
    fprintf(fp,"// Channel %d: \n",i);
    for (j=0; j<i_height; j++) {
      for (k=0; k<i_step; k++) {
        fprintf(fp,"\t%#x,",(unsigned) *(channels[i]+j*i_step+k));
      }
      fprintf(fp,"\n");
    }
    //}
  }

  // write end
  fprintf(fp, "};\n\n");
#endif  

  fclose(fp);
#endif

  // set up the request
  request[0] = 0;
  RAB_SET_PROT(request[0], prot);
  RAB_SET_PORT(request[0], port);
  RAB_SET_OFFLOAD_ID(request[0], offload_id);
  RAB_SET_N_ELEM(request[0], 1);
  RAB_SET_N_STRIPES(request[0], n_stripes);

  request[1] = (unsigned)max_stripe_size_b; // addr of pointer to max stripe size
  request[2] = (unsigned)rab_stripes;       // addr of pointer to stripe data

  // make the request
  ioctl(pulp->fd,PULP_IOCTL_RAB_REQ_STRIPED,request);

  // free memory
  free(*rab_stripes);
  free(rab_stripes);

  return 0;
}

/****************************************************************************************/

// qprintf stuff
#define ANSI_RESET   "\x1b[0m"

#define ANSI_RED     "\x1b[31m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_BLUE    "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN    "\x1b[36m"

#define ANSI_BRED     "\x1b[31;1m"
#define ANSI_BGREEN   "\x1b[32;1m"
#define ANSI_BYELLOW  "\x1b[33;1m"
#define ANSI_BBLUE    "\x1b[34;1m"
#define ANSI_BMAGENTA "\x1b[35;1m"
#define ANSI_BCYAN    "\x1b[36;1m"

#define PULP_PRINTF(...) printf("[" ANSI_BRED "PULP" ANSI_RESET "] " __VA_ARGS__)
#define HOST_PRINTF(...) printf("[" ANSI_BGREEN "HOST" ANSI_RESET "] " __VA_ARGS__)

// from http://creativeandcritical.net/str-replace-c/
static char *replace_str2(const char *str, const char *old, const char *new) {
  char *ret, *r;
  const char *p, *q;
  size_t oldlen = strlen(old);
  size_t count, retlen, newlen = strlen(new);
  int samesize = (oldlen == newlen);

  if (!samesize) {
    for (count = 0, p = str; (q = strstr(p, old)) != NULL; p = q + oldlen)
      count++;
    /* This is undefined if p - str > PTRDIFF_MAX */
    retlen = p - str + strlen(p) + count * (newlen - oldlen);
  } else
    retlen = strlen(str);

  if ((ret = malloc(retlen + 1)) == NULL)
    return NULL;

  r = ret, p = str;
  while (1) {
    /* If the old and new strings are different lengths - in other
     * words we have already iterated through with strstr above,
     * and thus we know how many times we need to call it - then we
     * can avoid the final (potentially lengthy) call to strstr,
     * which we already know is going to return NULL, by
     * decrementing and checking count.
     */
    if (!samesize && !count--)
      break;
    /* Otherwise i.e. when the old and new strings are the same
     * length, and we don't know how many times to call strstr,
     * we must check for a NULL return here (we check it in any
     * event, to avoid further conditions, and because there's
     * no harm done with the check even when the old and new
     * strings are different lengths).
     */
    if ((q = strstr(p, old)) == NULL)
      break;
    /* This is undefined if q - p > PTRDIFF_MAX */
    //ptrdiff_t l = q - p;
    unsigned l = q - p;
    memcpy(r, p, l);
    r += l;
    memcpy(r, new, newlen);
    r += newlen;
    p = q + oldlen;
  }
  strcpy(r, p);

  return ret;
}

void pulp_stdout_print(PulpDev *pulp, unsigned pe)
{
  PULP_PRINTF("PE %d\n",pe);

  char *string_ptr = (char *) (pulp->stdout.v_addr + STDOUT_PE_SIZE_B*pe);
  char *pulp_str = replace_str2((const char *) string_ptr, 
                                "\n", "\n[" ANSI_BRED "PULP" ANSI_RESET "] ");
  PULP_PRINTF("%s\n", pulp_str);
  fflush(stdout);
}

void pulp_stdout_clear(PulpDev *pulp, unsigned pe)
{
  int i;
  for (i=0; i<(STDOUT_PE_SIZE_B/4); i++) {
    pulp_write32(pulp->stdout.v_addr,STDOUT_PE_SIZE_B*pe+i*4,'b',0);    
  }
  for (i=0; i<STDOUT_SIZE_B/4; i++) {
    pulp_write32(pulp->stdout.v_addr,i,'w',0);    
  }
}
