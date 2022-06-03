
def carry_around_add(a, b):
    c = a + b
    print("m",hex(c))
    return (c & 0xffff) + (c >> 16)


def compute_checksum(message):
    b_str = message
    if len(b_str) % 2 == 1:
        b_str += b'\0'
    checksum = 0
    for i in range(0, len(b_str), 2):        
        w = b_str[i] + (b_str[i+1] << 8)
        print("w ", hex(w), chr(b_str[i]), chr(b_str[i+1]))
        checksum = carry_around_add(checksum, w)
        print("s ", hex(checksum))

    return ~checksum & 0xffff

print(compute_checksum(bytes("files/file.txt", 'utf-8')))
