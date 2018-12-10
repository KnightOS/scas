start:
	jp main

.org 0x100
main:
	jp end

end:
	halt
	jp end
