class LogDescriptor:
    def __init__(self, initial_val):
        self._val = initial_val
    def __get__(*args):
        print(args)
        return self._val
    def __set__(*args):
        print(args)
        self._val += 1

a = LogDescriptor(2)
print(a)
a = 3
print(a)
