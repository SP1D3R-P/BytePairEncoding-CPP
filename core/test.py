import BPE  


m = BPE.BytePairEncoding(500)

m.compile('/home/nandi/Desktop/BytePairEncoding/Corpus/TestData.en')
m.train()
print(ecoded:=m.encode("Hello world!!"))
print(m.decode(ecoded))
# m.displayTable("../Output/output.txt")
# m.displayTable()

# import _core
# m = _core.pyBytePairEncoding(500)

# with open('/home/nandi/Desktop/BytePairEncoding/Corpus/Smalltext.en','r') as fp:
#     m.compile(fp.read(),'D')
# with open('/home/nandi/Desktop/BytePairEncoding/Corpus/TestData.en','r') as fp:
#     m.compile(fp.read(),'C')
# m.train()

# encoded = m.encode("Hello world")
# decoded = m.decode(encoded)
# print(f'{encoded=} {decoded=}')


# m.displayTable("../Output/output.txt")
