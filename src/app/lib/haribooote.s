.arch i486

.extern HariMain

.global __start
__start:
    call HariMain
    lret

.global api_putchar
api_putchar:
    mov $1, %edx
    mov 4(%esp), %al
    int $0x40
    ret

.global api_putstr0
#void api_putstr0(char *s)
api_putstr0:
    push %ebx
    mov $2, %edx
    mov 8(%esp), %ebx
    int $0x40
    pop %ebx
    ret

#void api_end(void)
.global api_end
api_end:
    mov $4, %edx
    int $0x40

#int api_openwin(char *buf, int xsiz, int ysiz, int col_inv, char *title)
.global api_openwin
api_openwin:
    push %edi
    push %esi
    push %ebx
    movl $5, %edx
    movl 16(%esp), %ebx #buf
    movl 20(%esp), %esi #xsiz
    movl 24(%esp), %edi #ysiz
    movl 28(%esp), %eax #col_inv
    movl 32(%esp), %ecx #title
    int $0x40
    pop %ebx
    pop %esi
    pop %edi
    ret
