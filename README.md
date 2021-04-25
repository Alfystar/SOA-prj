# Sistemi Operativi Avanzati - Project
[Progetto di Sistemi Operativi Avanzati 2021](https://francescoquaglia.github.io/TEACHING/AOS/PROJECTS/project-specification-2020-2021.html), prof Francesco Quaglia, Tor Vergata

### Project state
- [x] Rooms indexing with AVL tree for key & tag
- [x] Room creat named from key
- [x] Room creat unnamed
- [x] Room check permission
- [x] Room open named from key
- [x] Room remove if empty
- [x] Room wake_up All
- [x] Room send/recive message
- [x] Room recive wakeup after signal

- [x] Super User have alwais permission succes
- [x] Room driver print state

### Build and load
To build and load the project run:
```
make unload ; make clean ; make -j$(nproc) ; make load ; sudo dmesg --clear 
```
To test the varius scenarius run one of the script in the `test` directory, in other case run the `cmd` script one at time