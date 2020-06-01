import random, math

if __name__=="__main__":
    import Genetic

class Matrix(object):
    
    def __init__(self,data=False,width=False,height=False):
        if data:
            self.array = data
            self.width = len(data[0])
            self.height = len(data)
        elif width and height:
            self.width = width
            self.height = height
            self.array = []
            for r in range(height):
                row = []
                for c in range(width):
                    row.append(random.uniform(-1,1))
                self.array.append(row)
                
    def __str__(self):
        to_return = "|"
        for r in self.array:
            for c in r:
                to_return += str(c)+","
            to_return = to_return[:-1]
            to_return += "|\n|"
        to_return = to_return[:-1]
        return(to_return)
    
    def __add__(self,other):
        if isinstance(other,int)or isinstance(other,float):       
            array = []
            for r in range(self.height):
                row = []
                for c in range(self.width):
                    row.append(self.array[r][c]+other)
                array.append(row)
        elif isinstance(other,Matrix):
            array = []
            for r in range(self.height):
                row = []
                for c in range(self.width):
                    row.append(self.array[r][c]+other.array[r][c])
                array.append(row)
        return(Matrix(array))
    
    def __radd__(self,other):
        return(self.__add__(other))
    
    def __sub__(self,other):
        array = []
        if isinstance(other,int)or isinstance(other,float):
            for r in range(self.height):
                row = []
                for c in range(self.width):
                    row.append(self.array[r][c]-other)
                array.append(row)
        elif isinstance(other,Matrix):
            for r in range(self.height):
                row = []
                for c in range(self.width):
                    row.append(self.array[r][c]-other.array[r][c])
                array.append(row)
        return(Matrix(array))
    
    def __rsub__(self,other):
        return(self.__sub__(other))
    
    def __mul__(self,other):
        array = []
        if isinstance(other,int)or isinstance(other,float):
            for r in range(self.height):
                row = []
                for c in range(self.width):
                    row.append(self.array[r][c]*other)
                array.append(row)
        elif isinstance(other,Matrix):
            for r in range(self.height):
                row = []
                for c in range(self.width):
                    row.append(self.array[r][c]*other.array[r][c])
                array.append(row)
        return(Matrix(array))
    
    def __rmul__(self,other):
        return(self.__mul__(other))

    def breed(self, other):
        r = random.randrange(0, 10)
        current = True if r>5 else False
        a = []
        for i in range(self.height):
            b = []
            for j in range(self.width):
                r = random.randrange(0, 10)
                current = current if r<8 else not current
                b.append(self.array[i][j] if current else other.array[i][j])
            a.append(b)
        return(Matrix(a))
        

    def mutate(self, rate):
        for r in range(self.height):
            for c in range(self.width):
                mult = 1 if random.random()>rate else random.random()
                mult *= 1 if mult==1 else 1 if random.random()>0.5 else -1
                self.array[r][c] *= mult
    
    def transpose(self):
        a = []
        for c in range(self.width):
            b = []
            for r in range(self.height):
                b.append(self.array[r][c])
            a.append(b)
        return(Matrix(a))
    
    def dot(self,other):
        if self.width==other.height:
            x = []
            for a in range(self.height):
                y = []
                for b in range(other.width):
                    z = 0
                    for c in range(self.width):
                        z += self.array[a][c]*other.array[c][b]
                    y.append(z)
                x.append(y)
            return(Matrix(x))
        else:
            raise Exception("matrix dimensions: "+str(self.width)+","+str(self.height)+" and "+str(other.width)+","+str(other.height)+" do not match.")

    def sigmoid(self,derivative=False):
        array = []
        if derivative:
            for r in range(self.height):
                row = []
                for c in range(self.width):
                    row.append(1/(1+math.exp(-self.array[r][c]))*(1-(1/(1+math.exp(-self.array[r][c])))))
                array.append(row)
        else:
            for r in range(self.height):
                row = []
                for c in range(self.width):
                    if self.array[r][c] >= -500:
                        row.append(1/(1+math.exp(-self.array[r][c])))
                    else:
                        row.append(0)
                array.append(row)
        return(Matrix(array))
    
    def softmax(self):
        array = []
        total = 0
        for r in self.array:
            row = []
            for c in r:
                total += c
            for c in range(self.width):
                row.append(r[c]/total)
            array.append(row)
        return(Matrix(array))

    def one_Hot(self):
        array = []
        for r in self.array:
            highest,index = 0,0
            row = []
            for c in range(len(r)):
                if r[c] > highest:
                    highest,index = r[c],c
            for c in range(len(r)):
                if c != index:
                    row.append(0)
                else:
                    row.append(1)
            array.append(row)
        return(Matrix(array))
    
    def add_Weights(self):
        self.width += 1
        for r in self.array:
            r.append(1)

    def remove_Weights(self):
        self.width -= 1
        for i in range(len(self.array)):
            self.array[i] = self.array[i][::-1]

