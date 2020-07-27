#include "bootpack.h"

void console_task(struct SHEET *sheet, unsigned int memtotal) {
    struct TIMER *timer;
    struct TASK *task = task_now();

    int i;
    int x, y;
    int fifobuf[128];
    struct CONSOLE cons;
    char cmdline[30];
    cons.sht = sheet;
    cons.cur_x = 8;
    cons.cur_y = 28;
    cons.cur_c = -1;

    *((int *) 0xfec) = (int) & cons;

    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;

    int *fat = (int *) memman_alloc_4k(memman, 4 * 2880);
    file_readfat(fat, (unsigned char *) (ADR_DISKIMG + 0x000200));

    fifo32_init(&task->fifo, 128, fifobuf, task);

    timer = timer_alloc();
    timer_init(timer, &task->fifo, 1);
    timer_settime(timer, 50);

    cons_putchar(&cons, '>', 1);

    for (;;) {
        io_cli();
        if (fifo32_status(&task->fifo) == 0) {
            task_sleep(task);
            io_sti();

        } else {
            i = fifo32_get(&task->fifo);
            io_sti();

            if (i <= 1) {
                if (i != 0) {
                    timer_init(timer, &task->fifo, 0);
                    if (cons.cur_c >= 0)
                        cons.cur_c = COL8_FFFFFF;

                } else {
                    timer_init(timer, &task->fifo, 1);
                    if (cons.cur_c >= 0)
                        cons.cur_c = COL8_000000;
                }
                timer_settime(timer, 50);
            }

            if (i == 2) //cursor on
                cons.cur_c = COL8_FFFFFF;

            if (i == 3) { //cursor off
                boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
                cons.cur_c = -1;
            }

            //キーボード
            if (i >= 256 && i <= 511) {
                if (i == 8 + 256) { //backspace
                    if (cons.cur_x > 16) {
                        cons_putchar(&cons, ' ', 0);
                        cons.cur_x -= 8;
                    }

                } else if (i == 10 + 256) { //enter
                    //カーソルをスペースで消して改行
                    cons_putchar(&cons, ' ', 0);
                    cmdline[cons.cur_x / 8 - 2] = 0;
                    cons_newline(&cons);
                    cons_runcmd(cmdline, &cons, fat, memtotal);

                    //プロンプト表示
                    cons_putchar(&cons, '>', 1);

                } else {
                    if (cons.cur_x < 240) {
                        cmdline[cons.cur_x / 8 - 2] = i - 256;
                        cons_putchar(&cons, i - 256, 1);
                    }
                }
            }
            if (cons.cur_c >= 0)
               boxfill8(sheet->buf, sheet->bxsize, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);

            sheet_refresh(sheet, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
        }
    }
}

void cons_putchar(struct CONSOLE *cons, int chr, char move) {
    char str[2];
    str[0] = chr;
    str[1] = 0;

    //tab
    if (str[0] == 0x09) {
        for (;;) {
            putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
            cons->cur_x += 8;
            if (cons->cur_x == 8 + 240)
                cons_newline(cons);
            if (((cons->cur_x - 8) & 0x1f) == 0) break; //32で割り切れたら
        }
    //LF
    } else if (str[0] == 0x0a)
        cons_newline(cons);
    else if (str[0] == 0x0d);
    else {
        putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, str, 1);
        if (move != 0) { //moveが0のときはカーソルを進めない
            cons->cur_x += 8;
            if (cons->cur_x == 8 + 240)
                cons_newline(cons);
        }
    }
}

void cons_newline(struct CONSOLE *cons) {
    int x, y;
    struct SHEET *sheet = cons->sht;

    if (cons->cur_y < 28 + 112)
        cons->cur_y += 16;

    else { //scroll
        for (y = 28; y < 28 + 112; y++) {
            for (x = 8; x < 8 + 240; x++)
                sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
        }
        for (y = 28 + 112; y < 28 + 128; y++) {
            for (x = 8; x < 8 + 240; x++)
                sheet->buf[x + y * sheet->bxsize] = COL8_000000;
        }
        sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
    }
    cons->cur_x = 8;
}


void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal) {
    if (_strcmp(cmdline, "mem") == 0)
        cmd_mem(cons, memtotal);

    else if (_strcmp(cmdline, "cls") == 0)
        cmd_cls(cons);

    else if (_strcmp(cmdline, "dir") == 0)
        cmd_dir(cons);

    else if (_strncmp(cmdline, "type ", 5) == 0)
        cmd_type(cons, fat, cmdline);

    else if (cmdline[0] != 0) {
        if (cmd_app(cons, fat, cmdline) == 0)
            cons_putstr0(cons, "Bad Command.\n\n");
    }
}

void cmd_mem(struct CONSOLE *cons, unsigned int memtotal){
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    char str[60];

    _sprintf(str, "total  %dMB\nfree   %dKB\n\n", memtotal / (1024 * 1024),  memman_total(memman) / 1024);
    cons_putstr0(cons, str);
}

void cmd_cls(struct CONSOLE *cons) {
    int x,y;
    struct SHEET *sheet = cons->sht;

    for (y = 28; y < 28 + 128; y++) {
        for (x = 8; x < 8 + 240; x++)
            sheet->buf[x + y * sheet->bxsize] = COL8_000000;
    }
    sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
    cons->cur_y = 28;
}

void cmd_dir(struct CONSOLE *cons) {
    struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);
    int i, j;
    char str[30];

   for (i = 0; i < 224; i++) {
        if (finfo[i].name[0] == 0x00) break;
        if (finfo[i].name[0] != 0xa5) {
            if ((finfo[i].type & 0x18) == 0) {
                _sprintf(str, "filename.ext    %7d", finfo[i].size);
                for (j = 0; j < 8; j++)
                    str[j] = finfo[i].name[j];

                str[9] = finfo[i].ext[0];
                str[10] = finfo[i].ext[1];
                str[11] = finfo[i].ext[2];

                cons_putstr0(cons, str);
                cons_newline(cons);
            }
        }
    }
    cons_newline(cons);
}

void cmd_type(struct CONSOLE *cons, int *fat, char *cmdline) {
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct FILEINFO *finfo = file_search(cmdline + 5, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);

    char *p;
    int i;

    //ファイルが見つかった
    if (finfo != 0) {
        p = (char *) memman_alloc_4k(memman, finfo->size);
        file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
        cons_putstr1(cons, p, finfo->size);
        memman_free_4k(memman, (int) p, finfo->size);

    } else
        cons_putstr0(cons, "File not found.\n");

    cons_newline(cons);
}

int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline) {
    int i;
    char *p;
    char *q;
    char name[18];

    char str[128];

    int segsiz;
    int esp;
    int datsiz;
    int dathrb;

    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
    struct FILEINFO *finfo;
    struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
    struct TASK *task = task_now();

    for (i = 0; i < 13; i++) {
        if (cmdline[i] <= ' ') break;
        name[i] = cmdline[i];
    }

    name[i] = 0;

    //ファイルを探す
    finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
    if (finfo == 0 && name[i - 1] != '.') {
        //拡張子付きでも探す
        name[i] = '.';
        name[i + 1] = 'H';
        name[i + 2] = 'R';
        name[i + 3] = 'B';
        name[i + 4] = 0;
        finfo = file_search(name, (struct FILEINFO *) (ADR_DISKIMG + 0x002600), 224);
    }

    if (finfo != 0) {
        p = (char *) memman_alloc_4k(memman, finfo->size);
        *((int *) 0xfe8) = (int) p;
        file_loadfile(finfo->clustno, finfo->size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));

        //シグネチャのチェックと、mainを呼び出すように書き換え
        if (finfo->size >= 36 && _strncmp(p + 4, "Hari", 4) == 0 && *p == 0x00) {

            //ヘッダから読み取る
            segsiz = *((int *) (p + 0x0000)); //データセグメントの大きさ
            esp = *((int *) (p + 0x000c)); //初期esp, データ転送先の初期位置
            datsiz = *((int *) (p + 0x0010)); //データセクションからデータセグメントにコピーする大きさ
            dathrb = *((int *) (p + 0x0014)); //hrbファイル内のデータセクションの位置

            _sprintf(str, "[debug] segsiz=0x%X, esp=0x%X\n[debug] datsiz=0x%x, dathrb=0x%x\n", segsiz, esp, datsiz, dathrb);
            cons_putstr0(cons, str);

            //データセグメントのサイズに基づいてメモリ確保
            q = (char *) memman_alloc_4k(memman, segsiz);

            //データセグメントを覚えておく(システムコールされたときにアプリケーションのデータのアクセスするのに必要)
            *((int *) 0xfe8) = (int) q;

            //0x60を足すのは、アプリのセグメントであるとあつかうため
            //コードセグメント
            set_segmdesc(gdt + 1003, finfo->size - 1, (int) p, AR_CODE32_ER + 0x60);
            //データセグメント
            set_segmdesc(gdt + 1004,  segsiz - 1, (int) q, AR_DATA32_RW + 0x60);

            //データセクションからデータセグメントにコピー
            for (i = 0; i < datsiz; i++)
                q[esp + i] = p[dathrb + i];

            //権限による制御を使う場合は、TSSにOS用のセグメントと、ESPを登録する必要がある(P438)
            //0x1bから始めるのは、その位置(実際にはヘッダ内)に、mainへのjmp命令が埋め込まれてるから
            start_app(0x1b, 1003 * 8, esp, 1004 * 8, &(task->tss.esp0));
            memman_free_4k(memman, (int) q, segsiz);

        } else
            cons_putstr0(cons, ".hrb file format error. \n");

        memman_free_4k(memman, (int) p, finfo->size);
        cons_newline(cons);

        return 1;
    }
    return 0;
}

void cons_putstr0(struct CONSOLE *cons, char *str) {
    for (; *str != 0; str++)
        cons_putchar(cons, *str, 1);
}

void cons_putstr1(struct CONSOLE *cons, char *str, int l) {
    for (int i = 0; i < l; i++)
        cons_putchar(cons, str[i], 1);
}

int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax) {
    int ds_base = *((int *) 0xfe8);
    struct TASK *task = task_now();
    struct CONSOLE *cons = (struct CONSOLE *) *((int *)0xfec);
    struct SHTCTL *shtctl = (struct SHTCTL *) *((int *)0xfe4);
    struct SHEET *sht;

    int *reg = &eax + 1; //eaxの次の番地
    //asm_hrb_apiでこの関数はcallされ、call前にpsuhaを2回やっている
    //ここでは引数のeaxの次の番地(=1回目のpushadのedi)のアドレスを参照させる
    //1回目のpushはレジスタを保存するためのpushなので、eaxにpopされるであろうreg[7]に値をセットするとasm_hrb_apiの戻り値になる（無理やり）

    if (edx == 1)
        cons_putchar(cons, eax & 0x000000ff, 1);

    else if (edx == 2)
        cons_putstr0(cons, (char *) ebx + ds_base);

    else if (edx == 3)
        cons_putstr1(cons, (char *) ebx + ds_base, ecx);

    else if (edx == 4)
        return &(task->tss.esp0);

    else if (edx == 5) {
        /*
        ebx buf
        esi xsiz
        edi ysiz
        eax col_in
        ecx title
        */
        sht = sheet_alloc(shtctl);
        sheet_setbuf(sht, (char *) ebx + ds_base, esi, edi, eax);
        make_window8((char *) ebx + ds_base, esi, edi, (char *) ecx + ds_base, 0);
        sheet_slide(sht, 100, 50);
        sheet_updown(sht, 3);
        reg[7] = (int) sht;
    }

    return 0;
}

int *inthandler0d(int *esp) {
    struct TASK *task = task_now();
    struct CONSOLE *cons = (struct CONSOLE *) *((int *) 0xfec);
    char str[30];

    cons_putstr0(cons, "\nINT 0D : \n General Protected Exception.\n");
    _sprintf(str, "EIP = %08X\n", esp[11]);
    cons_putstr0(cons, str);
    //異常終了
    return &(task->tss.esp0);
}

int *inthandler0c(int *esp) {
    struct TASK *task = task_now();
    struct CONSOLE *cons = (struct CONSOLE *) *((int *) 0xfec);
    char str[30];

    cons_putstr0(cons, "\nINT 0C : \n Stack Exception.\n");
    _sprintf(str, "EIP = %08X\n", esp[11]);
    cons_putstr0(cons, str);
    //異常終了
    return &(task->tss.esp0);
}
