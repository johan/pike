/*
**! module Image
**! note
**!	$Id$
*/

struct gif_lzw
{
  int very_dummy;
};

/* returns number of strings written on stack */

int image_gif_lzw_init(struct gif_lzw *lzw,int bits);
int image_gif_lzw_add(struct gif_lzw *lzw,unsigned char *data,int len);
int image_gif_lzw_finish(struct gif_lzw *lzw);
