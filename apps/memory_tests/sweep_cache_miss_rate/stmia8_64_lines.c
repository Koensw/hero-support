asm volatile ("ldr r0, [%0]" : : "r" (&address) : "r4","r5","r6","r7","r8","r9","r10","r11");
asm volatile ("mov r4, #0xFF" : : );
asm volatile ("mov r5, #0xFF" : : );
asm volatile ("mov r6, #0xFF" : : );
asm volatile ("mov r7, #0xFF" : : );
asm volatile ("mov r8, #0xFF" : : );
asm volatile ("mov r9, #0xFF" : : );
asm volatile ("mov r10, #0xFF" : : );
asm volatile ("mov r11, #0xFF" : : );
/*
 * 64 lines - increment address by 32 byte (one cache line) after the access
 */
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
asm volatile ("stmia r0!, {r4-r11}" : : );
// \_64
asm volatile ("str r0, [%0]" :  : "r" (&address) );