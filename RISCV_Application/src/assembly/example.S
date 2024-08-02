.data 
 string1: .byte 'L','O','A','D','I','N','G'
.section .text 

.globl draw
.type draw, @function
draw:
 li a0,1
 li a1,1
 li a2,159
 li a3,79
 li a4,65535
 addi sp,sp,-4
 sw ra,0(sp)
 jal LCD_DrawRectangle
 lw ra,0(sp)
 addi sp,sp,4
 
 # draw the string

 li a0, 5
 li a1, 5
 la a2, string1
 li a3, 65535
 addi sp,sp,-4
 sw ra,0(sp)
 jal LCD_ShowString
 lw ra,0(sp)
 addi sp,sp,4

 li a0, 5
 li a1, 40
 li a2, 90072
 addi sp, sp, -4
 sw ra, 0(sp)
 jal LCD_DrawPoint_big
 lw ra, 0(sp)
 addi sp, sp, 4

 li a0, 1000
 addi sp, sp, -4
 sw ra, 0(sp)
 jal delay_1ms
 lw ra, 0(sp)
 addi sp, sp, 4

