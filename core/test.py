import _core

m = _core.pyBytePairEncoding(500)

with open('/home/nandi/Desktop/BytePairEncoding/Corpus/Smalltext.en','r') as fp:
    m.compile(fp.read().lower(),'D')
with open('/home/nandi/Desktop/BytePairEncoding/Corpus/TestData.en','r') as fp:
    m.compile(fp.read().lower(),'C')
m.train()

encoded = m.encode("Hello world")
decoded = m.decode(encoded)
print(f'{encoded=} {decoded=}')


m.displayTable("../Output/output.txt")
