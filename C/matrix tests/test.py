from Matrix import Matrix as oMatrix
from NNAPI import Matrix


def mp(p):
    for i in p.array:
        print(i)
    print()

l1 = [[1.0,1.0,1.0],[2.0,2.0,2.0],[3.0,3.0,3.0]]
l2 = [[1.0,2.0,3.0],[4.0,5.0,6.0],[7.0,8.0,9.0]]

m = Matrix(l1)
om = oMatrix(l1)

m.add_weights()
om.add_Weights()

mp(m)
mp(om)
