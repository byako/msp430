import numpy as np
import array

cchar = np.array([[0 for x in xrange(8)] for x in xrange(8)])
ccharIdx = 0
masks = (128,64,32,16,8,4,2,1)
inmasks = (1,2,4,8,16,32,64,128)
f1 = None
f2 = None

def putByte(oldstr):
	global ccharIdx
	global cchar
	global f2

	dec1 = -1
	newstr = ""
	doCalc = False
	i = 0

	# cut the trailing comma
	if oldstr.find(',') != -1:
		dec1 = int(oldstr[0:-1], 16)
	else:
		dec1 = int(oldstr, 16)
		doCalc = True

	# get new bits values
	for mask in masks:
		cchar[ccharIdx][i] = dec1 & mask
		i = i + 1

	# transpose cchar if got last bitarray
	if doCalc:
#		print(cchar)
		cchar = cchar.T
#		print(cchar)
		ccharIdx = 0
		for j in xrange(8):
			dec1 = 0
			i = 0
			for mask in inmasks:
				if cchar[j][i] != 0:
					dec1 = dec1 | mask
				i = i + 1
			f2.write(str('0x%02x' % dec1) + (',\n' if j < 7 else '\n'))
	else:
		ccharIdx = ccharIdx + 1

f1 = open('chars.h','r')
f2 = open('chars2.h','w')
clines = f1.read().split('\n');
for cline in clines:
	if cline.find('0x') == 0:
		print(cline);
		putByte(cline)
	else:
		f2.write(cline)
		f2.write('\n')
