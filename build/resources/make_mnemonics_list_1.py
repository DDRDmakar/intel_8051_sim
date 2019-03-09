
import numpy as np

f = open("mnemo_names.txt", "r")
for l, i in zip(f, range(256)):
	print('\t\t"%02X": "%s",' % (i, l.split("\n")[0]))
