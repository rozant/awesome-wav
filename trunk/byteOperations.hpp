/****************************************************************/
/* function: setBit												*/
/* purpose: sets the bit at a specific position					*/
/* args: BYTE&, const char, const bool&							*/
/****************************************************************/
void setBit(BYTE &b, const char index, const bool &torf) {
	BYTE bitMask = 1;
	if (torf) { // Set bit to 1
		bitMask <<= index;
		b |= bitMask; 
	} else { // Set bit to 0
		bitMask <<= index;
		b &= ~bitMask;
	}
}

/****************************************************************/
/* function: getBit												*/
/* purpose: gets the bit at a specific position					*/
/* args: const BYTE&, const char								*/
/* returns: bool												*/
/*		1 = bit is a 1											*/
/*		0 = bit is a 0											*/
/****************************************************************/
bool getBit(const BYTE &b, const char index) {
	BYTE bitMask = 1;
	bitMask <<= index;
	if (bitMask & b)
		return true;
	return false;
}

/****************************************************************/
/* function: bytencmp											*/
/* purpose: compares two bytes									*/
/* args: const BYTE *, const BYTE *, size_t						*/
/* returns: int													*/
/****************************************************************/
int bytencmp(const BYTE* b1, const BYTE* b2, size_t n) {
	while(n--)
		if(*b1++!=*b2++)
			return *(BYTE*)(b1 - 1) - *(BYTE*)(b2 - 1);
	return 0;
}
