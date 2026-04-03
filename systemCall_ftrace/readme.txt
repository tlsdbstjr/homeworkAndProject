혹시 몰라서 커널 모듈을 위한 파일 뿐만 아니라 커널 내부 파일 중 건드린 파일도 같이 넣어서 보내드립니다.
linux-4.19.67/arch/x86/entry/syscalls/syscall_64.tbl
linux-4.19.67/include/linux/syscalls.h
linux-4.19.67/Makefile
이 세 파일을 각자 디렉토리에 맞게 넣어놨습니다.

ftrace폴더 내부에는 다 새로 만든 파일만 있으며, make하시면 모듈이 만들어집니다.
과제 요구사항으로 제출해야 하는 파일은 모두 ftrace 폴더 안에 있습니다.
