digits = [chr(x + ord('0')) for x in range(0,10)]
digits.extend([chr(x + ord('a')) for x in range(0,26)])
digits.extend([chr(x + ord('A')) for x in range(0,26)])
nDigits = len(digits)

values = {}
for i in range(nDigits):
    values[digits[i]] = i

def int2hash(a:int) -> str:
    if a < nDigits:
        return digits[a]
    (d, m) = divmod(a, nDigits)
    return int2hash(d) + digits[m]

def hash2int(a:str) -> int:
    val = 0
    for c in a:
        if c not in values:
            raise Exception('Invalid hash character({}) found in {}'.format(bytes(c), bytes(a)))
        val = (val * nDigits) + values[c]
    return val

x = 10000000
y = int2hash(x)
z = hash2int(y)

print(x, y, z, x-z)
