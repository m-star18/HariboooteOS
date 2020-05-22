void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);

#define COL8_000000	0
#define COL8_FF0000	1
#define COL8_00FF00	2
#define COL8_FFFF00	3
#define COL8_0000FF	4
#define COL8_FF00FF	5
#define COL8_00FFFF	6
#define COL8_FFFFFF	7
#define COL8_C6C6C6	8
#define COL8_840000	9
#define COL8_008400	10
#define COL8_848400	11
#define COL8_000084	12
#define COL8_840084	13
#define COL8_008484	14
#define COL8_848484	15

void HariMain(void) {
  char *p;  // BYTE [...]用番地
  p = (char *) 0xa0000;

  boxfill8(p, 320, COL8_FF0000,  20, 20, 120, 120);
  boxfill8(p, 320, COL8_00FF00,  70, 50, 170, 150);
  boxfill8(p, 320, COL8_0000FF, 120, 80, 220, 180);

  for(;;)
    io_hlt();
}

void init_palette(void) {
  static unsigned char table_rgb[16 * 3] = {
    0x00, 0x00, 0x00,   // 000000 : 0 : 黒
    0xff, 0x00, 0x00,   // ff0000 : 1 : 明るい赤
    0x00, 0xff, 0x00,   // 00ff00 : 2 : 明るい緑
    0xff, 0xff, 0x00,   // ffff00 : 3 : 黄色
    0x00, 0x00, 0xff,   // 0000ff : 4 : 明るい青
    0xff, 0x00, 0xff,   // ff00ff : 5 : 明るい紫
    0x00, 0xff, 0xff,   // 00ffff : 6 : 明るい水色
    0xff, 0xff, 0xff,   // ffffff : 7 : 白
    0xc6, 0xc6, 0xc6,   // c6c6c6 : 8 : 明るい灰色
    0x84, 0x00, 0x00,   // 840000 : 9 : 暗い赤
    0x00, 0x84, 0x00,   // 008400 : 10: 暗い緑
    0x84, 0x84, 0x00,   // 848400 : 11: 暗い黄色
    0x00, 0x00, 0x84,   // 000084 : 12: 暗い青
    0x84, 0x00, 0x84,   // 840084 : 13: 暗い紫
    0x00, 0x84, 0x84,   // 008484 : 14: 暗い水色
    0x84, 0x84, 0x84,   // 848484 : 15: 暗い灰色
  };
  set_palette(0, 15, table_rgb);
    // static char 命令は、データにしか使えないけどDB命令担当
}

void set_palette(int start, int end, unsigned char *rgb) {
  int i, eflags;
  eflags = io_load_eflags();    // 割り込み許可フラグの値を記録
  io_cli();                     // 許可フラグを0にして割り込みを禁止する
  io_out8(0x03c8, start);
  for (i = start; i <= end; i++) {
    io_out8(0x03c9, rgb[0] / 4);
    io_out8(0x03c9, rgb[1] / 4);
    io_out8(0x03c9, rgb[2] / 4);
    rgb += 3;
  }
  io_store_eflags(eflags);    // 割り込み許可フラグを元にもどす
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1) {
  for (int y = y0; y <= y1; y++){
    for (int x = x0; x <= x1; x++)
      vram[y * xsize + x] = c;
  }
}
