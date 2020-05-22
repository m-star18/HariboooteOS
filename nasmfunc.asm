GLOBAL  io_hlt
GLOBAL  write_mem8

io_hlt:
    HLT
    RET

write_mem8:
    MOV     ECX, DWORD [ESP + 4]
    MOV     AL, BYTE [ESP + 8]

    MOV     BYTE [ECX], AL
    RET
