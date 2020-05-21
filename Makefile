OSNAME := haribooote
ASMHEADNAME := asmhead
IPLNAME := ipl10
CNAME := bootpack

.DEFAULT_GOAL : all
.PHONY : all
all : img

${IPLNAME}.bin : ${IPLNAME}.asm
${ASMHEADNAME}.bin : ${ASMHEADNAME}.asm
%.bin : %.asm
	nasm $^ -o $@ -l $*.lst

${CNAME}.hrb : ${CNAME}.c os.lds
	gcc -fno-pie -no-pie -march=i486 -m32 -nostdlib -T os.lds ${CNAME}.c -o ${CNAME}.hrb

${OSNAME}.sys : ${ASMHEADNAME}.bin ${CNAME}.hrb
	cat $^ > $@

${OSNAME}.img : ${IPLNAME}.bin ${OSNAME}.sys
# 1440KBのフロッピーディスクに書き込む
	mformat -f 1440 -C -B ${IPLNAME}.bin -i $@ ::
# OS本体をイメージに書き込む
	mcopy -i $@ ${OSNAME}.sys ::

#===============================================================================
.PHONY : asm
asm :
	make ${IPLNAME}.bin

.PHONY : img
img :
	make ${OSNAME}.img

.PHONY : run
run :
	make img
	qemu-system-i386 -fda ${OSNAME}.img

#===============================================================================
.PHONY : clean
clean :
	@rm *.img *.bin *.sys *.hrb

.PHONY : debug
debug:
	make img
	qemu-system-i386 -fda ${OSNAME}.img -gdb tcp::10000 -S
