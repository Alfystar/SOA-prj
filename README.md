# Sistemi Operativi Avanzati - Project
[Progetto di Sistemi Operativi Avanzati 2021](https://francescoquaglia.github.io/TEACHING/AOS/PROJECTS/project-specification-2020-2021.html), prof Francesco Quaglia, Tor Vergata.

[Report del progetto](/00_Doc/SOA-report/main.pdf)

### Project state
Core:
- [x] Room creat named from key
- [x] Room creat unnamed
- [x] Room check permission and allow **ALWAYS** Super User all operation
- [x] Room open named from key
- [x] Room remove room if no waiters at any level
- [x] Room wake_up All waiter in all level in any room
- [x] Room send/recive message
- [x] Room recive wakeup after signal

Driver:
- [x] Room driver print state
- [x] Room driver llseek function
- [x] Room driver multiple reader

Technicism:
- [x] Rooms indexing with AVL tree for key & tag
- [x] **Max Room in the system modify at run time**
- [x] Refcount of the Obj to perform free only when need
- [x] *Different level of verbose in dmes with macro*
- [x] No locking in normal read/write operation, only queuing
### Build and load
To build and load the project the only think you need are the `make` program, all the target are present inside the `Makefile`, and to build, load ,unload and clean need only his.



In particular, to unload, clean, make, reload you can copy and paste this:

```bash
make unload ; make clean ; make -j$(nproc) ; make load 
```
If you also want delete all previus System Message and keep only from now, use:
```bash
make unload ; make clean ; make -j$(nproc) ; sudo dmesg --clear ; make load 
```
### Using and Testing

**After the building** of the `Makefile` you can:

- Test the varius scenarius there are 6° test script in the `test` directory (create by make).

```
test/
├── 1_createDeleteTest.out
├── 2_createDelete_LOAD.out
├── 3_roomExange.out
├── 4_wakeUpTest.out
├── 5_signalWait.out
├── 6_roomExange_signal_LOAD.out
└── 7_roomExange_LOAD.out
```



- Interact with the system with the cmd scritp present in `cmd` directory (create by make).

```
cmd/
├── ctl.out
├── get.out
├── recive.out
└── send.out
```

More over, **after the loading the module into the Kernel** you can:

- See the actual state of the system, after loading is possible use:

```bash
cat /dev/TAG_DataExchange/tbde_stat 
```

- Change the current max-room allow in the system:

```bash
sudo su
echo {New Number} > /sys/kernel/TAG_DataExchange/MAX_ROOM
# Keep in mind at least there are 256 rooms, and if the number of current
# opened room are grate of the propose, the change will be ignore
```

- Found in `/sys/module/TAG_DataExchange` the expose variable of the system (like the syscall number array)

### Tested platform
For now the system passed all of the 6-th test + unload on:
- [x] Kde Neon 20.04 LTS (Kernel 5.4.xxx) (real machine)
- [x] Xubunto 20.10  (Kernel 5.8.xxx) (virtual machine)

- [x] Kubunto 21.04  (Kernel 5.11.xxx) (real machine)