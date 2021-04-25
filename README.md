# Sistemi Operativi Avanzati - Project
[Progetto di Sistemi Operativi Avanzati 2021](https://francescoquaglia.github.io/TEACHING/AOS/PROJECTS/project-specification-2020-2021.html), prof Francesco Quaglia, Tor Vergata

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
- [x] Refcount of the Obj to perform free only when need
- [x] Different level of verbose in dmes with macro
- [x] No locking in normal read/write operation, only queuing
### Build and load
To build and load the project run:
```
make unload ; make clean ; make -j$(nproc) ; make load ; sudo dmesg --clear 
```
To test the varius scenarius run one of the script in the `test` directory, in other case run the `cmd` script one at time

### Tested platform
For now the system passed all of the 6-th test + unload on:
- [x] Kde Neon 20.04 LTS (Kernel 5.4.xxx) (real machine)
- [x] Xubunto 20.10 LTS (Kernel 5.8.xxx) (virtual machine)
- [ ] (Kernel 3.0.xxx) (virtual machine)