prog1 = [1.67, 2.14, 2.50, 2.78, 3.00, 3.18, 3.33]
prog2 = [1.89, 2.63, 3.23, 3.68, 4.00, 4.22, 4.35]
prog3 = [1.89, 2.68, 3.39, 4.03, 4.62, 5.15, 5.63]


def calce(prog):
    arr = []
    for n, w in enumerate(prog):
        e = ((1 / w) - (1 / (n + 2))) / (1 - (1 / (n + 2)))
        arr.append(e)
    return arr


e1 = calce(prog1)
e2 = calce(prog2)
e3 = calce(prog3)

print(e1)
print(e2)
print(e3)
