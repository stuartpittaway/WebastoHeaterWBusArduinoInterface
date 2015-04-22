

char* PrintHexByte(char *str, unsigned int d);
char* hexdump(char *str, unsigned char *d, int l, bool appendNewLine);
char *i2str_zeropad(int i, char *buf);
char *i2str(int i, char *buf);

inline void WORDSWAP(unsigned char *out, unsigned char *in);
short twobyte2word(unsigned char *in);


#define BYTE2TEMP(x)	  ((unsigned char)((x)-50))
#define WORD2HOUR(w)	  twobyte2word(w)
//#define WORD2VOLT_TEXT(t, w) shortToMili(t, twobyte2word(w))

