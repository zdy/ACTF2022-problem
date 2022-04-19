# 英文wp

## Dropper

- difficulty: easy
- flag: `ACTF{dr0pp3r_1s_v3ry_int3r3st1ng_1d7a90a63039831c7fcaa53b766d5b2d!!!!!}`
- description:

The competition question sets a little simple software protection method, can you solve it?

### Writeup

**technology：**

- upx pack + dropper + large number operation + exception handling + virtual function table hook

**Problem solving ideas：**

- Use the upx packer tool to unpack
- Search for related meanings through dropper, find the binary that gets data from the resource area and decrypts it. Learn from： https://github.com/marcusbotacin/Dropper
    - The necessary function‘s cross reference for running the PE file was not found. The guess is that GetProcAddress was used, and the cross reference was used to find the place where the function actually executes.
        - Static analysis to find, or dynamic tracking to find the corresponding data, analyze and decrypt, write a script to decrypt the data, and generate a real executable file.
    - Start to analyze the dropped binary file, dynamically debug and trace, you can trace all the processes of encryption and verification, it is not complicated. here is just an anti-static analysis method, using exception handling to perform virtual function table hook, making static analysis invalid.
        - After entering the flag, set a breakpoint in the text area to obtain the processing logic address. After finding it in IDA, set a breakpoint and trace.
            - To guess how the generated large numbers are stored, dynamically debug to find the address space where the data is stored, and try to convert the hexadecimal to decimal, and you can see that the decimal is only stored in hexadecimal.
        - According to the string of IDA, find the string table of BASE64, and find the real encryption judgment address according to the cross-reference.
    - You can also use dynamic debugging, open the file, run to "flag:", stop the current process, and then attach to the process that dropped, you can dynamically debug the drop process.

### Decrypt Code

```cpp
import base64

res = 834572051814337070469744559761199605121805728622619480039894407167152612470842477813941120780374570205930952883661000998715107231695919001238818879944773516507366865633886966330912156402063735306303966193481658066437563587241718036562480496368592194719092339868512773222711600878782903109949779245500098606570248830570792028831133949440164219842871034275938433
res = res + 57705573952449699620072104055030025886984180500734382250587152417040141679598894
res = res - 71119332457202863671922045224905384620742912949065190274173724688764272313900465
res = res + 55079029772840138145785005601340325789675668817561045403173659223377346727295749
res = res - 14385283226689171523445844388769467232023411467394422980403729848631619308579599
res = res + 80793226935699295824618519685638809874579343342564712419235587177713165502121664
res = res // 7537302706582391238853817483600228733479333152488218477840149847189049516952787
res = res - 17867047589171477574847737912328753108849304549280205992204587760361310317983607
res = res + 55440851777679184418972581091796582321001517732868509947716453414109025036506793
res = res // 11783410410469738048283152171898507679537812634841032055361622989575562121323526
res = res - 64584540291872516627894939590684951703479643371381420434698676192916126802789388

s = ''
while res:
    s += chr(res % 128)
    res = res // 128

print(base64.b64decode(s))
```