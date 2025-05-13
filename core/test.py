import BPE  


m = BPE.BytePairEncoding(500)

m.compile('/home/nandi/Desktop/BytePairEncoding/Corpus/TestData.en')
m.train()
print(ecoded:=m.encode("Hello world!!"))
print(m.decode(ecoded))
print(m.vocab_capacity)
m.vocab_capacity = 1000
print(m.vocab_capacity)
# d = m._toDict()
# print(d)

m.save('../Output/output.json')

