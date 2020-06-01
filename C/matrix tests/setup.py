from distutils.core import setup, Extension

def main():
    setup(name="NNAPI",
          version="1.0.0",
          description="Python interface for my neural net API",
          author="Robert Graham Keeling",
          author_email="ro8b12@googlemail.com",
          ext_modules=[Extension("NNAPI", ["/Users/user/Desktop/C/C/TestC.c", "/Users/user/Desktop/Matrix/Matrix/matrix.c"])])

print("T")
main()
print("TEST")

