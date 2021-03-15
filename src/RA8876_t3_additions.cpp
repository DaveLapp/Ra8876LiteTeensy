#include "RA8876_t3.h"

void RA8876_t3::drawText(const char* text) {
  if (text == nullptr || text[0] == 0) {
    return;
  }

  int16_t orig_x = _cursorX;

  const char* currentchar = text;
  const char* nextline;

  if (getTextAlign() & ALIGN_BOTTOM) {
    _cursorY -= measureTextHeight(text);
  } else if (getTextAlign() & ALIGN_VCENTER) {
    _cursorY -= measureTextHeight(text)/2;
  }

  do {
    nextline = strchr(currentchar, '\n');
    int linelen;
    if (nextline) {
      nextline++;
      linelen = nextline - currentchar;
    } else {
      linelen = strlen(currentchar);
    }

    if (getTextAlign() & ALIGN_CENTER) {
      _cursorX = orig_x - measureTextWidth(currentchar, linelen)/2;
    } else if (getTextAlign() & ALIGN_RIGHT) {
      _cursorX = orig_x - measureTextWidth(currentchar, linelen);
    } else {
      // default to left alignment
      _cursorX = orig_x;
    }

    for (int i = 0; i < linelen; i++) {
      // the write() call handles increasing _cursorY when it sees a '\n'
      write(*currentchar);
      currentchar++;
    }

  } while (nextline);
}


void RA8876_t3::drawBitmap(int16_t x, int16_t y,
			      const uint8_t *bitmap, int16_t w, int16_t h,
			      uint16_t color) {

  int16_t i, j, byteWidth = (w + 7) / 8;

  for(j=0; j<h; j++) {
    for(i=0; i<w; i++ ) {
      if(pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7))) {
	drawPixel(x+i, y+j, color);
      }
    }
  }
}

void RA8876_t3::drawBitmap(int16_t x, int16_t y,
			      const uint8_t *bitmap, int16_t w, int16_t h,
			      uint16_t color, uint16_t bgcolor) {

  int16_t i, j, byteWidth = (w + 7) / 8;
// todo: optimize this with a single transaction
  for(j=0; j<h; j++) {
    for(i=0; i<w; i++ ) {
      if(pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7))) {
		  	drawPixel(x+i, y+j, color);
      } else {
      	drawPixel(x+i, y+j, bgcolor);
      }
    }
  }
}

void RA8876_t3::setScroll(uint16_t offset) {
  // TODO
}

void RA8876_t3::measureChar(unsigned char c, uint16_t* w, uint16_t* h) {
	if (c == 0xa0) {
	  c = ' ';
	}

	if (font) {
		*h = font->cap_height;
		*w = 0;

		uint32_t bitoffset;
		const uint8_t *data;

		if (c >= font->index1_first && c <= font->index1_last) {
			bitoffset = c - font->index1_first;
			bitoffset *= font->bits_index;
		} else if (c >= font->index2_first && c <= font->index2_last) {
			bitoffset = c - font->index2_first + font->index1_last - font->index1_first + 1;
			bitoffset *= font->bits_index;
		} else if (font->unicode) {
			return; // TODO: implement sparse unicode
		} else {
			return;
		}

		data = font->data + fetchbits_unsigned(font->index, bitoffset, font->bits_index);

		uint32_t encoding = fetchbits_unsigned(data, 0, 3);

		if (encoding != 0) return;

		//uint32_t width =
		fetchbits_unsigned(data, 3, font->bits_width);
		bitoffset = font->bits_width + 3;

		//uint32_t height =
		fetchbits_unsigned(data, bitoffset, font->bits_height);
		bitoffset += font->bits_height;

		//int32_t xoffset =
		fetchbits_signed(data, bitoffset, font->bits_xoffset);
		bitoffset += font->bits_xoffset;

		//int32_t yoffset =
		fetchbits_signed(data, bitoffset, font->bits_yoffset);
		bitoffset += font->bits_yoffset;

		uint32_t delta = fetchbits_unsigned(data, bitoffset, font->bits_delta);
		*w = delta;
	} else {
		*w = 6 * textsize;
		*h = 8 * textsize;
	}

}

uint16_t RA8876_t3::measureTextWidth(const char* text, int num) {
  uint16_t maxH = 0;
  uint16_t currH = 0;
  uint16_t n = num;

  if (n == 0) {
    n = strlen(text);
  };

  for (int i = 0; i < n; i++) {
    if (text[i] == '\n') {
      if (currH > maxH)
        maxH = currH;
      currH = 0;
    } else {
      uint16_t h, w;
      measureChar(text[i], &w, &h);
      currH += w;
    }
  }
  uint16_t h = maxH > currH ? maxH : currH;
  return h;
}

uint16_t RA8876_t3::measureTextHeight(const char* text, int num) {
  int lines = 1;
  uint16_t n = num;
  if (n == 0) {
    n = strlen(text);
  };
  for (int i = 0; i < n; i++) {
    if (text[i] == '\n') {
      lines++;
    }
  }
  return ((lines-1) * fontLineSpace() + fontCapHeight());
}

void RA8876_t3::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t* colors) {
  putPicture( x,  y, 1, h, (const unsigned char *)colors);
/*  // TODO - optimize?
  for (uint16_t i = 0; i < h; i++) {
    drawPixel(x,y+i,colors[i]);
  }
*/
}

void RA8876_t3::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t* colors) {
  putPicture(x,y,w,1,(const unsigned char*)colors);
/*  // TODO - optimize?
  for (uint16_t i = 0; i < w; i++) {
    drawPixel(x+i,y,colors[i]);
  }
*/
}
