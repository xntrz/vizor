bits 64
default rel

%include "win32n.inc"


extern GetSystemInfo
extern GetProcAddress
extern VirtualProtect


%define align_u(value, align)   ((value + (align - 1)) & ~(align - 1))
%define align_d(value, align)   ((value & ~(align - 1))
%define shadow_space            32
%define stack_align             16


%define fnvar(bytes)            (rbp - shadow_space - bytes)


%macro  fnin 0-1
        push    rbp
        mov     rbp, rsp
        sub     rsp, (shadow_space + align_u(%1, stack_align))
        push    rbx
%endmacro


%macro  fnout 0
        pop     rbx
        leave
%endmacro


%define ENCODE(byte, key0, key1) ((byte ^ key0) + key1)
%define DECODE(byte, key0, key1) ((byte - key1) ^ key0)


%macro idatest0 0
    db 0xEB
    db 0xFF
    db 0xC0
    db 0x48
%endmacro


;
;   На этапе компиляции кодирует строку
;       %1 - Строковой литерал необходимый спрятать
;
;   На этапе рантайма декодирует строку и возвращает:
;       RAX - указатель на строчку
;
%macro  strp 1
        idatest0
        call %%lbl_strend                   ; переходим к метке конца строки
                                            ; сохраняя в стек указатель на блок строки                                            
%%lbl_strbegin:
        %strlen lenx %1
        %assign encode_key ENCODE(lenx, lenx * 2, lenx)
        db encode_key                       ; записываем закодированный размер строки
        %strlen len %1
        %assign i 1
        %rep len
            %substr char %1 i
            db ENCODE(char, encode_key, len + i)   ; записываем закодированный символ строки
        %assign i i+1
        %endrep
        db ENCODE(lenx, encode_key, len)
%%lbl_strend:
        ; Извлекаем указатель на блок строки
        pop rax

        ; Проверяем не декодирована ли строка
        ; Если нет, то продолжаем
        ; В противном случае плывем на выход
        xor rbx, rbx
        mov bl, [rax]        
        test bl, bl
        jz %%lbl_already_decoded

        ; Записываем в размер строки 0 как флаг-индикатор того что строка декодирована
        xor rcx, rcx
        mov [rax], cl
        inc rax

        ; Сохраняем закодированую длину
        mov r8, rbx
    
        ; Декодируем длину строки
        sub rbx, lenx    
        mov rcx, lenx * 2
        xor rbx, rcx

        ; Сохраняем декодированую длину
        mov r9, rbx

        ; Устанавливаем счетчик цикла
        mov rcx, rbx
%%lbl_loop:
        ;
        ; Декодируем строку
        ; rax - String pointer
        ; bl - String character
        ; r8 - Xor key
        ; r9 - String len
        ; rcx - i
        ;
        ; Псевдо код:
        ; do
        ; {
        ;   bl = *rax;
        ;   key1 = ((r9 - i) + r9 + 1);
        ;   key0 = r8;
        ;   *rax = (bl - key1) ^ key0;
        ;   ++rax;
        ; } while (--i);
        ;
        mov bl, [rax]

        mov rdx, r9
        sub rdx, rcx
        add rdx, r9
        inc rdx

        sub rbx, rdx
        xor rbx, r8

        mov [rax], bl

        inc rax
        loop %%lbl_loop

        ; Записываем '\0' в конец строчки и выходим из цикла
        mov rcx, 0
        mov [rax], cl
        sub rax, r9
        jmp %%lbl_decoded
        idatest0

%%lbl_already_decoded:
        inc rax

%%lbl_decoded:
%endmacro


;
;   На этапе компиляции производит операцию strp с каждой строчкой и создает jmp-таблицу
;       %1, ... - строка или несколько строк, которые необходимо спрятать
;
;   На этапе рантайма получает номер строки, декодирует её, и возвращает указатель на неё
;   Аргументы
;       RCX - Номер строки
;   Возвращает
;       RAX - Указатель на строку
;
%macro strpsw 1-*
    ; rax = nullptr
    xor rax, rax

    ; if (rcx >= %0) goto exit
    cmp rcx, %0
    jge %%lbl_exit

    ; switch(rcx)
    jmp [%%jmptable + (8 * rcx)]

    %assign j 1     ; TODO если объявить переменную как i, то она перезаписывается вызовом макроса strp ???
    %rep %0
        %%lbl%+j:
            strp %1
            jmp %%lbl_exit
        %rotate 1
    %assign j j+1
    %endrep

%%jmptable:
    %assign i 1
    %rep %0
        dq %%lbl%+i
    %assign i i+1
    %endrep

%%lbl_exit:
%endmacro


section .data
    PAGE_EXECUTE_READWRITE equ 0x40


section .bss
    PageSize:       resd 1
    PageProtect:    resd 1
    SystemInfo:     resb SYSTEM_INFO_size


section .text
;
;   bool PmChangeProtect(uint64 NewProtect)
;   
;   Аргументы
;       RCX - маска значений защиты страницы
;   Возвращает
;       RAX - 1 случае успеха, 0 при не удаче
;       RBX - маска значений прошлой защиты страницы
;
align 16
PmChangeProtect:
    fnin 8    
    lea r9, [fnvar(8)]          ; lpflOldProtect
    mov r8, rcx                 ; flNewProtect
    mov edx, [PageSize]         ; dwSize
    mov rcx, PmRequestString    ; lpAddress
    call VirtualProtect
    mov rbx, [fnvar(8)]
    fnout
    ret


;
;   bool PmInitialize(void)
;
;   Возвращает
;       RAX - 1 в случае успеха, 0 при не удаче
;   Смещение переменных:
;       [8] - старое значение защиты функции VirtualProtectEx
;
align 16
global PmInitialize
PmInitialize:
    fnin

    lea rcx, [SystemInfo]   ; lpSystemInfo
    call GetSystemInfo
    
    ; Выравниваем размер функции
    mov eax, [SystemInfo + SYSTEM_INFO.dwPageSize]
    dec eax
    mov ecx, eax
    add ecx, (lbl_PmRequestStringEnd - lbl_PmRequestStringBegin)
    not eax
    and eax, ecx
    mov [PageSize], eax

    ; Меняем защиту страницы функции
    mov rcx, PAGE_EXECUTE_READWRITE
    call PmChangeProtect

    ; Проверяем результат
    test rax, rax
    jz .lbl_failure

    ; Результат успешный, сохраняем прошлую защиту и выходим
    mov [PageProtect], rbx
    mov rax, 1
    jmp .lbl_success

.lbl_failure:
    ; Результат неудачный, выходим
    mov rax, 0

.lbl_success:
    fnout
    ret


;
;   void PmTerminate(void)
;
align 16
global PmTerminate
PmTerminate:    
    ; Меняем защиту страницы функции на оригинальную
    mov rcx, [PageProtect]
    call PmChangeProtect    
    ret


;
;   char* PmRequestString(int64 No)
;
;   Аргументы
;       RCX - номер строчки
;   Возвращает
;       RAX - указатель на декодированную строку
;
align 16
global PmRequestString
PmRequestString:
lbl_PmRequestStringBegin:
    fnin
    strpsw  "RelicCoH2.exe",        \
            "Company of Heroes 2",  \
            "User32",               \
            "FindWindowA",          \
            "Disable FOW",          \
            "Disable chat"
    fnout
    ret
lbl_PmRequestStringEnd: