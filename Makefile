OSNAME := haribooote
FILENAME := ipl10

.DEFAULT_GOAL : all
.PHONY : all
all : img

${FILENAME}.bin : ${FILENAME}.asm
${OSNAME}.sys : ${OSNAME}.asm

%.bin : %.asm
	nasm $^ -o $@ -l $*.lst

%.sys : %.asm
	nasm $^ -o $@ -l $*.lst

${OSNAME}.img : ${FILENAME}.bin ${OSNAME}.sys
# mtools
#  - [Mtools - Wikipedia](https://en.wikipedia.org/wiki/Mtools)
#  - [2.2 Drive letters - Mtools 4.0.23](https://www.gnu.org/software/mtools/manual/mtools.html#drive-letters)
#  - [mtoolsの使い方が知りたい - ITmedia エンタープライズ](http://www.itmedia.co.jp/help/tips/linux/l0317.html)
#
# 1440KBのフロッピーディスクに書き込む
	mformat -f 1440 -C -B ${FILENAME}.bin -i $@ ::
# OS本体をイメージに書き込む
	mcopy -i $@ ${OSNAME}.sys ::


.PHONY : asm
asm :
	make ${FILENAME}.bin

.PHONY : img
img :
	make ${OSNAME}.img

.PHONY : run
run :
	make img
	qemu-system-i386 -fda ${OSNAME}.img

.PHONY : clean
clean :
# lstは残しておいてもいいと思うのでcleanに入れていない
	@rm *.img *.bin
