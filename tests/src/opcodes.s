; Taken from: https://github.com/Clownacy/clownassembler

.group_0000:
	; Group: 0b0000 -> Bit Manipulation/MOVEP/Immediate
	ori.b		#0xff, %ccr
	ori.w		#0xffff, %sr
	ori.b		#0xff, %d4
	ori.b		#0xff, (%a2)
	ori.b		#0xff, (%a2)+
	ori.b		#0xff, -(%a2)
	ori.b		#0xff, 0x7fff(%a2)
	ori.b		#0xff, 0x7f(%a2,%d4.w)
	ori.b		#0xff, (0xffffffff).w
	ori.b		#0xff, (0xffffffff).l
	ori.w		#0xffff, %d4
	ori.w		#0xffff, (%a2)
	ori.w		#0xffff, (%a2)+
	ori.w		#0xffff, -(%a2)
	ori.w		#0xffff, 0x7fff(%a2)
	ori.w		#0xffff, 0x7f(%a2,%d4.w)
	ori.w		#0xffff, (0xffffffff).w
	ori.w		#0xffff, (0xffffffff).l
	ori.l		#0xffffffff, %d4
	ori.l		#0xffffffff, (%a2)
	ori.l		#0xffffffff, (%a2)+
	ori.l		#0xffffffff, -(%a2)
	ori.l		#0xffffffff, 0x7fff(%a2)
	ori.l		#0xffffffff, 0x7f(%a2,%d4.w)
	ori.l		#0xffffffff, (0xffffffff).w
	ori.l		#0xffffffff, (0xffffffff).l

	andi.b		#0xff, %ccr
	andi.w		#0xffff, %sr
	andi.b		#0xff, %d4
	andi.b		#0xff, (%a2)
	andi.b		#0xff, (%a2)+
	andi.b		#0xff, -(%a2)
	andi.b		#0xff, 0x7fff(%a2)
	andi.b		#0xff, 0x7f(%a2,%d4.w)
	andi.b		#0xff, (0xffffffff).w
	andi.b		#0xff, (0xffffffff).l
	andi.w		#0xffff, %d4
	andi.w		#0xffff, (%a2)
	andi.w		#0xffff, (%a2)+
	andi.w		#0xffff, -(%a2)
	andi.w		#0xffff, 0x7fff(%a2)
	andi.w		#0xffff, 0x7f(%a2,%d4.w)
	andi.w		#0xffff, (0xffffffff).w
	andi.w		#0xffff, (0xffffffff).l
	andi.l		#0xffffffff, %d4
	andi.l		#0xffffffff, (%a2)
	andi.l		#0xffffffff, (%a2)+
	andi.l		#0xffffffff, -(%a2)
	andi.l		#0xffffffff, 0x7fff(%a2)
	andi.l		#0xffffffff, 0x7f(%a2,%d4.w)
	andi.l		#0xffffffff, (0xffffffff).w
	andi.l		#0xffffffff, (0xffffffff).l

	subi.b		#0xff, %d4
	subi.b		#0xff, (%a2)
	subi.b		#0xff, (%a2)+
	subi.b		#0xff, -(%a2)
	subi.b		#0xff, 0x7fff(%a2)
	subi.b		#0xff, 0x7f(%a2,%d4.w)
	subi.b		#0xff, (0xffffffff).w
	subi.b		#0xff, (0xffffffff).l
	subi.w		#0xffff, %d4
	subi.w		#0xffff, (%a2)
	subi.w		#0xffff, (%a2)+
	subi.w		#0xffff, -(%a2)
	subi.w		#0xffff, 0x7fff(%a2)
	subi.w		#0xffff, 0x7f(%a2,%d4.w)
	subi.w		#0xffff, (0xffffffff).w
	subi.w		#0xffff, (0xffffffff).l
	subi.l		#0xffffffff, %d4
	subi.l		#0xffffffff, (%a2)
	subi.l		#0xffffffff, (%a2)+
	subi.l		#0xffffffff, -(%a2)
	subi.l		#0xffffffff, 0x7fff(%a2)
	subi.l		#0xffffffff, 0x7f(%a2,%d4.w)
	subi.l		#0xffffffff, (0xffffffff).w
	subi.l		#0xffffffff, (0xffffffff).l

	eori.b		#0xff, %ccr
	eori.w		#0xffff, %sr
	eori.b		#0xff, %d4
	eori.b		#0xff, (%a2)
	eori.b		#0xff, (%a2)+
	eori.b		#0xff, -(%a2)
	eori.b		#0xff, 0x7fff(%a2)
	eori.b		#0xff, 0x7f(%a2,%d4.w)
	eori.b		#0xff, (0xffffffff).w
	eori.b		#0xff, (0xffffffff).l
	eori.w		#0xffff, %d4
	eori.w		#0xffff, (%a2)
	eori.w		#0xffff, (%a2)+
	eori.w		#0xffff, -(%a2)
	eori.w		#0xffff, 0x7fff(%a2)
	eori.w		#0xffff, 0x7f(%a2,%d4.w)
	eori.w		#0xffff, (0xffffffff).w
	eori.w		#0xffff, (0xffffffff).l
	eori.l		#0xffffffff, %d4
	eori.l		#0xffffffff, (%a2)
	eori.l		#0xffffffff, (%a2)+
	eori.l		#0xffffffff, -(%a2)
	eori.l		#0xffffffff, 0x7fff(%a2)
	eori.l		#0xffffffff, 0x7f(%a2,%d4.w)
	eori.l		#0xffffffff, (0xffffffff).w
	eori.l		#0xffffffff, (0xffffffff).l

	cmpi.b		#0xff, %d4
	cmpi.b		#0xff, (%a2)
	cmpi.b		#0xff, (%a2)+
	cmpi.b		#0xff, -(%a2)
	cmpi.b		#0xff, 0x7fff(%a2)
	cmpi.b		#0xff, 0x7f(%a2,%d4.w)
	cmpi.b		#0xff, (0xffffffff).w
	cmpi.b		#0xff, (0xffffffff).l
	cmpi.w		#0xffff, %d4
	cmpi.w		#0xffff, (%a2)
	cmpi.w		#0xffff, (%a2)+
	cmpi.w		#0xffff, -(%a2)
	cmpi.w		#0xffff, 0x7fff(%a2)
	cmpi.w		#0xffff, 0x7f(%a2,%d4.w)
	cmpi.w		#0xffff, (0xffffffff).w
	cmpi.w		#0xffff, (0xffffffff).l
	cmpi.l		#0xffffffff, %d4
	cmpi.l		#0xffffffff, (%a2)
	cmpi.l		#0xffffffff, (%a2)+
	cmpi.l		#0xffffffff, -(%a2)
	cmpi.l		#0xffffffff, 0x7fff(%a2)
	cmpi.l		#0xffffffff, 0x7f(%a2,%d4.w)
	cmpi.l		#0xffffffff, (0xffffffff).w
	cmpi.l		#0xffffffff, (0xffffffff).l

	btst.l		%d4, %d2
	btst.b		%d4, (%a2)
	btst.b		%d4, (%a2)+
	btst.b		%d4, -(%a2)
	btst.b		%d4, 0x7fff(%a2)
	btst.b		%d4, 0x7f(%a2,%d4.w)
	btst.b		%d4, (0xffffffff).w
	btst.b		%d4, (0xffffffff).l
	btst.b		%d4, #0x55
	btst.b		%d4, *(%pc)
	btst.b		%d4, *(%pc,%d5.w)
	btst.l		#0, %d4
	btst.b		#0, (%a2)
	btst.b		#0, (%a2)+
	btst.b		#0, -(%a2)
	btst.b		#0, 0x7fff(%a2)
	btst.b		#0, 0x7f(%a2,%d4.w)
	btst.b		#0, (0xffffffff).w
	btst.b		#0, (0xffffffff).l
	btst.b		#0, *(%pc)
	btst.b		#0, *(%pc,%d5.w)

	bchg.l		%d4, %d2
	bchg.b		%d4, (%a2)
	bchg.b		%d4, (%a2)+
	bchg.b		%d4, -(%a2)
	bchg.b		%d4, 0x7fff(%a2)
	bchg.b		%d4, 0x7f(%a2,%d4.w)
	bchg.b		%d4, (0xffffffff).w
	bchg.b		%d4, (0xffffffff).l
	bchg.l		#0, %d4
	bchg.b		#0, (%a2)
	bchg.b		#0, (%a2)+
	bchg.b		#0, -(%a2)
	bchg.b		#0, 0x7fff(%a2)
	bchg.b		#0, 0x7f(%a2,%d4.w)
	bchg.b		#0, (0xffffffff).w
	bchg.b		#0, (0xffffffff).l

	bclr.l		%d4, %d2
	bclr.b		%d4, (%a2)
	bclr.b		%d4, (%a2)+
	bclr.b		%d4, -(%a2)
	bclr.b		%d4, 0x7fff(%a2)
	bclr.b		%d4, 0x7f(%a2,%d4.w)
	bclr.b		%d4, (0xffffffff).w
	bclr.b		%d4, (0xffffffff).l
	bclr.l		#0, %d4
	bclr.b		#0, (%a2)
	bclr.b		#0, (%a2)+
	bclr.b		#0, -(%a2)
	bclr.b		#0, 0x7fff(%a2)
	bclr.b		#0, 0x7f(%a2,%d4.w)
	bclr.b		#0, (0xffffffff).w
	bclr.b		#0, (0xffffffff).l

	bset.l		%d4, %d2
	bset.b		%d4, (%a2)
	bset.b		%d4, (%a2)+
	bset.b		%d4, -(%a2)
	bset.b		%d4, 0x7fff(%a2)
	bset.b		%d4, 0x7f(%a2,%d4.w)
	bset.b		%d4, (0xffffffff).w
	bset.b		%d4, (0xffffffff).l
	bset.l		#0, %d4
	bset.b		#0, (%a2)
	bset.b		#0, (%a2)+
	bset.b		#0, -(%a2)
	bset.b		#0, 0x7fff(%a2)
	bset.b		#0, 0x7f(%a2,%d4.w)
	bset.b		#0, (0xffffffff).w
	bset.b		#0, (0xffffffff).l

	movep.w		0x7fff(%a2), %d4
	movep.w		%d4, 0x7fff(%a2)
	movep.l		0x7fff(%a2), %d4
	movep.l		%d4, 0x7fff(%a2)

	moves.b		%d4, (%a2)						; M68010+
	moves.b		%d4, (%a2)+
	moves.b		%d4, -(%a2)
	moves.b		%d4, 0x7fff(%a2)
	moves.b		%d4, 0x7f(%a2,%d4.w)
	moves.b		%d4, (0xffffffff).w
	moves.b		%d4, (0xffffffff).l
	moves.w		%d4, (%a2)
	moves.w		%d4, (%a2)+
	moves.w		%d4, -(%a2)
	moves.w		%d4, 0x7fff(%a2)
	moves.w		%d4, 0x7f(%a2,%d4.w)
	moves.w		%d4, (0xffffffff).w
	moves.w		%d4, (0xffffffff).l
	moves.l		%d4, (%a2)
	moves.l		%d4, (%a2)+
	moves.l		%d4, -(%a2)
	moves.l		%d4, 0x7fff(%a2)
	moves.l		%d4, 0x7f(%a2,%d4.w)
	moves.l		%d4, (0xffffffff).w
	moves.l		%d4, (0xffffffff).l
	moves.b		%a2, (%a2)						; M68010+
	moves.b		%a2, (%a2)+
	moves.b		%a2, -(%a2)
	moves.b		%a2, 0x7fff(%a2)
	moves.b		%a2, 0x7f(%a2,%d4.w)
	moves.b		%a2, (0xffffffff).w
	moves.b		%a2, (0xffffffff).l
	moves.w		%a2, (%a2)
	moves.w		%a2, (%a2)+
	moves.w		%a2, -(%a2)
	moves.w		%a2, 0x7fff(%a2)
	moves.w		%a2, 0x7f(%a2,%d4.w)
	moves.w		%a2, (0xffffffff).w
	moves.w		%a2, (0xffffffff).l
	moves.l		%a2, (%a2)
	moves.l		%a2, (%a2)+
	moves.l		%a2, -(%a2)
	moves.l		%a2, 0x7fff(%a2)
	moves.l		%a2, 0x7f(%a2,%d4.w)
	moves.l		%a2, (0xffffffff).w
	moves.l		%a2, (0xffffffff).l
	moves.b		(%a2), %d4						; M68010+
	moves.b		(%a2)+, %d4
	moves.b		-(%a2), %d4
	moves.b		0x7fff(%a2), %d4
	moves.b		0x7f(%a2,%d4.w), %d4
	moves.b		(0xffffffff).w, %d4
	moves.b		(0xffffffff).l, %d4
	moves.w		(%a2), %d4
	moves.w		(%a2)+, %d4
	moves.w		-(%a2), %d4
	moves.w		0x7fff(%a2), %d4
	moves.w		0x7f(%a2,%d4.w), %d4
	moves.w		(0xffffffff).w, %d4
	moves.w		(0xffffffff).l, %d4
	moves.l		(%a2), %d4
	moves.l		(%a2)+, %d4
	moves.l		-(%a2), %d4
	moves.l		0x7fff(%a2), %d4
	moves.l		0x7f(%a2,%d4.w), %d4
	moves.l		(0xffffffff).w, %d4
	moves.l		(0xffffffff).l, %d4
	moves.b		(%a2), %a2						; M68010+
	moves.b		(%a2)+, %a2
	moves.b		-(%a2), %a2
	moves.b		0x7fff(%a2), %a2
	moves.b		0x7f(%a2,%d4.w), %a2
	moves.b		(0xffffffff).w, %a2
	moves.b		(0xffffffff).l, %a2
	moves.w		(%a2), %a2
	moves.w		(%a2)+, %a2
	moves.w		-(%a2), %a2
	moves.w		0x7fff(%a2), %a2
	moves.w		0x7f(%a2,%d4.w), %a2
	moves.w		(0xffffffff).w, %a2
	moves.w		(0xffffffff).l, %a2
	moves.l		(%a2), %a2
	moves.l		(%a2)+, %a2
	moves.l		-(%a2), %a2
	moves.l		0x7fff(%a2), %a2
	moves.l		0x7f(%a2,%d4.w), %a2
	moves.l		(0xffffffff).w, %a2
	moves.l		(0xffffffff).l, %a2


.group_00ss:
	; Group: 0b00ss -> MOVEA/MOVE
	movea.w		%d4, %a2
	movea.w		%a2, %a4
	movea.w		(%a2), %a2
	movea.w		(%a2)+, %a2
	movea.w		-(%a2), %a2
	movea.w		0x7fff(%a2), %a2
	movea.w		0x7f(%a2,%d4.w), %a2
	movea.w		(0xffffffff).w, %a2
	movea.w		(0xffffffff).l, %a2
	movea.w		*(%pc), %a2
	movea.w		*(%pc,%d4.w), %a2
	movea.w		#0xffff, %a2
	movea.l		%d4, %a2
	movea.l		%a2, %a4
	movea.l		(%a2), %a2
	movea.l		(%a2)+, %a2
	movea.l		-(%a2), %a2
	movea.l		0x7fff(%a2), %a2
	movea.l		0x7f(%a2,%d4.w), %a2
	movea.l		(0xffffffff).w, %a2
	movea.l		(0xffffffff).l, %a2
	movea.l		*(%pc), %a2
	movea.l		*(%pc,%d4.w), %a2
	movea.l		#0xffffffff, %a2

	move.b		%d4, %d2
	move.b		%d4, (%a2)
	move.b		%d4, (%a2)+
	move.b		%d4, -(%a2)
	move.b		%d4, 0x7fff(%a2)
	move.b		%d4, 0x7f(%a2,%d4.w)
	move.b		%d4, (0xffffffff).w
	move.b		%d4, (0xffffffff).l
	move.b		(%a2), %d4
	move.b		(%a2), (%a2)
	move.b		(%a2), (%a2)+
	move.b		(%a2), -(%a2)
	move.b		(%a2), 0x7fff(%a2)
	move.b		(%a2), 0x7f(%a2,%d4.w)
	move.b		(%a2), (0xffffffff).w
	move.b		(%a2), (0xffffffff).l
	move.b		(%a2)+, %d4
	move.b		(%a2)+, (%a2)
	move.b		(%a2)+, (%a2)+
	move.b		(%a2)+, -(%a2)
	move.b		(%a2)+, 0x7fff(%a2)
	move.b		(%a2)+, 0x7f(%a2,%d4.w)
	move.b		(%a2)+, (0xffffffff).w
	move.b		(%a2)+, (0xffffffff).l
	move.b		-(%a2), %d4
	move.b		-(%a2), (%a2)
	move.b		-(%a2), (%a2)+
	move.b		-(%a2), -(%a2)
	move.b		-(%a2), 0x7fff(%a2)
	move.b		-(%a2), 0x7f(%a2,%d4.w)
	move.b		-(%a2), (0xffffffff).w
	move.b		-(%a2), (0xffffffff).l
	move.b		0x7fff(%a2), %d4
	move.b		0x7fff(%a2), (%a2)
	move.b		0x7fff(%a2), (%a2)+
	move.b		0x7fff(%a2), -(%a2)
	move.b		0x7fff(%a2), 0x7fff(%a2)
	move.b		0x7fff(%a2), 0x7f(%a2,%d4.w)
	move.b		0x7fff(%a2), (0xffffffff).w
	move.b		0x7fff(%a2), (0xffffffff).l
	move.b		0x7f(%a2,%d4.w), %d4
	move.b		0x7f(%a2,%d4.w), (%a2)
	move.b		0x7f(%a2,%d4.w), (%a2)+
	move.b		0x7f(%a2,%d4.w), -(%a2)
	move.b		0x7f(%a2,%d4.w), 0x7fff(%a2)
	move.b		0x7f(%a2,%d4.w), 0x7f(%a2,%d4.w)
	move.b		0x7f(%a2,%d4.w), (0xffffffff).w
	move.b		0x7f(%a2,%d4.w), (0xffffffff).l
	move.b		(0xffffffff).w, %d4
	move.b		(0xffffffff).w, (%a2)
	move.b		(0xffffffff).w, (%a2)+
	move.b		(0xffffffff).w, -(%a2)
	move.b		(0xffffffff).w, 0x7fff(%a2)
	move.b		(0xffffffff).w, 0x7f(%a2,%d4.w)
	move.b		(0xffffffff).w, (0xffffffff).w
	move.b		(0xffffffff).w, (0xffffffff).l
	move.b		(0xffffffff).l, %d4
	move.b		(0xffffffff).l, (%a2)
	move.b		(0xffffffff).l, (%a2)+
	move.b		(0xffffffff).l, -(%a2)
	move.b		(0xffffffff).l, 0x7fff(%a2)
	move.b		(0xffffffff).l, 0x7f(%a2,%d4.w)
	move.b		(0xffffffff).l, (0xffffffff).w
	move.b		(0xffffffff).l, (0xffffffff).l
	move.b		*(%pc), %d4
	move.b		*(%pc), (%a2)
	move.b		*(%pc), (%a2)+
	move.b		*(%pc), -(%a2)
	move.b		*(%pc), 0x7fff(%a2)
	move.b		*(%pc), 0x7f(%a2,%d4.w)
	move.b		*(%pc), (0xffffffff).w
	move.b		*(%pc), (0xffffffff).l
	move.b		*(%pc,%d4.w), %d4
	move.b		*(%pc,%d4.w), (%a2)
	move.b		*(%pc,%d4.w), (%a2)+
	move.b		*(%pc,%d4.w), -(%a2)
	move.b		*(%pc,%d4.w), 0x7fff(%a2)
	move.b		*(%pc,%d4.w), 0x7f(%a2,%d4.w)
	move.b		*(%pc,%d4.w), (0xffffffff).w
	move.b		*(%pc,%d4.w), (0xffffffff).l
	move.b		#0xff, %d4
	move.b		#0xff, (%a2)
	move.b		#0xff, (%a2)+
	move.b		#0xff, -(%a2)
	move.b		#0xff, 0x7fff(%a2)
	move.b		#0xff, 0x7f(%a2,%d4.w)
	move.b		#0xff, (0xffffffff).w
	move.b		#0xff, (0xffffffff).l
	move.w		%d4, %d2
	move.w		%d4, (%a2)
	move.w		%d4, (%a2)+
	move.w		%d4, -(%a2)
	move.w		%d4, 0x7fff(%a2)
	move.w		%d4, 0x7f(%a2,%d4.w)
	move.w		%d4, (0xffffffff).w
	move.w		%d4, (0xffffffff).l
	move.w		%a2, %d2
	move.w		%a2, (%a2)
	move.w		%a2, (%a2)+
	move.w		%a2, -(%a2)
	move.w		%a2, 0x7fff(%a2)
	move.w		%a2, 0x7f(%a2,%d4.w)
	move.w		%a2, (0xffffffff).w
	move.w		%a2, (0xffffffff).l
	move.w		(%a2), %d4
	move.w		(%a2), (%a2)
	move.w		(%a2), (%a2)+
	move.w		(%a2), -(%a2)
	move.w		(%a2), 0x7fff(%a2)
	move.w		(%a2), 0x7f(%a2,%d4.w)
	move.w		(%a2), (0xffffffff).w
	move.w		(%a2), (0xffffffff).l
	move.w		(%a2)+, %d4
	move.w		(%a2)+, (%a2)
	move.w		(%a2)+, (%a2)+
	move.w		(%a2)+, -(%a2)
	move.w		(%a2)+, 0x7fff(%a2)
	move.w		(%a2)+, 0x7f(%a2,%d4.w)
	move.w		(%a2)+, (0xffffffff).w
	move.w		(%a2)+, (0xffffffff).l
	move.w		-(%a2), %d4
	move.w		-(%a2), (%a2)
	move.w		-(%a2), (%a2)+
	move.w		-(%a2), -(%a2)
	move.w		-(%a2), 0x7fff(%a2)
	move.w		-(%a2), 0x7f(%a2,%d4.w)
	move.w		-(%a2), (0xffffffff).w
	move.w		-(%a2), (0xffffffff).l
	move.w		0x7fff(%a2), %d4
	move.w		0x7fff(%a2), (%a2)
	move.w		0x7fff(%a2), (%a2)+
	move.w		0x7fff(%a2), -(%a2)
	move.w		0x7fff(%a2), 0x7fff(%a2)
	move.w		0x7fff(%a2), 0x7f(%a2,%d4.w)
	move.w		0x7fff(%a2), (0xffffffff).w
	move.w		0x7fff(%a2), (0xffffffff).l
	move.w		0x7f(%a2,%d4.w), %d4
	move.w		0x7f(%a2,%d4.w), (%a2)
	move.w		0x7f(%a2,%d4.w), (%a2)+
	move.w		0x7f(%a2,%d4.w), -(%a2)
	move.w		0x7f(%a2,%d4.w), 0x7fff(%a2)
	move.w		0x7f(%a2,%d4.w), 0x7f(%a2,%d4.w)
	move.w		0x7f(%a2,%d4.w), (0xffffffff).w
	move.w		0x7f(%a2,%d4.w), (0xffffffff).l
	move.w		(0xffffffff).w, %d4
	move.w		(0xffffffff).w, (%a2)
	move.w		(0xffffffff).w, (%a2)+
	move.w		(0xffffffff).w, -(%a2)
	move.w		(0xffffffff).w, 0x7fff(%a2)
	move.w		(0xffffffff).w, 0x7f(%a2,%d4.w)
	move.w		(0xffffffff).w, (0xffffffff).w
	move.w		(0xffffffff).w, (0xffffffff).l
	move.w		(0xffffffff).l, %d4
	move.w		(0xffffffff).l, (%a2)
	move.w		(0xffffffff).l, (%a2)+
	move.w		(0xffffffff).l, -(%a2)
	move.w		(0xffffffff).l, 0x7fff(%a2)
	move.w		(0xffffffff).l, 0x7f(%a2,%d4.w)
	move.w		(0xffffffff).l, (0xffffffff).w
	move.w		(0xffffffff).l, (0xffffffff).l
	move.w		*(%pc), %d4
	move.w		*(%pc), (%a2)
	move.w		*(%pc), (%a2)+
	move.w		*(%pc), -(%a2)
	move.w		*(%pc), 0x7fff(%a2)
	move.w		*(%pc), 0x7f(%a2,%d4.w)
	move.w		*(%pc), (0xffffffff).w
	move.w		*(%pc), (0xffffffff).l
	move.w		*(%pc,%d4.w), %d4
	move.w		*(%pc,%d4.w), (%a2)
	move.w		*(%pc,%d4.w), (%a2)+
	move.w		*(%pc,%d4.w), -(%a2)
	move.w		*(%pc,%d4.w), 0x7fff(%a2)
	move.w		*(%pc,%d4.w), 0x7f(%a2,%d4.w)
	move.w		*(%pc,%d4.w), (0xffffffff).w
	move.w		*(%pc,%d4.w), (0xffffffff).l
	move.w		#0xffff, %d4
	move.w		#0xffff, (%a2)
	move.w		#0xffff, (%a2)+
	move.w		#0xffff, -(%a2)
	move.w		#0xffff, 0x7fff(%a2)
	move.w		#0xffff, 0x7f(%a2,%d4.w)
	move.w		#0xffff, (0xffffffff).w
	move.w		#0xffff, (0xffffffff).l
	move.l		%d4, %d2
	move.l		%d4, (%a2)
	move.l		%d4, (%a2)+
	move.l		%d4, -(%a2)
	move.l		%d4, 0x7fff(%a2)
	move.l		%d4, 0x7f(%a2,%d4.w)
	move.l		%d4, (0xffffffff).w
	move.l		%d4, (0xffffffff).l
	move.l		%a2, %d2
	move.l		%a2, (%a2)
	move.l		%a2, (%a2)+
	move.l		%a2, -(%a2)
	move.l		%a2, 0x7fff(%a2)
	move.l		%a2, 0x7f(%a2,%d4.w)
	move.l		%a2, (0xffffffff).w
	move.l		%a2, (0xffffffff).l
	move.l		(%a2), %d4
	move.l		(%a2), (%a2)
	move.l		(%a2), (%a2)+
	move.l		(%a2), -(%a2)
	move.l		(%a2), 0x7fff(%a2)
	move.l		(%a2), 0x7f(%a2,%d4.w)
	move.l		(%a2), (0xffffffff).w
	move.l		(%a2), (0xffffffff).l
	move.l		(%a2)+, %d4
	move.l		(%a2)+, (%a2)
	move.l		(%a2)+, (%a2)+
	move.l		(%a2)+, -(%a2)
	move.l		(%a2)+, 0x7fff(%a2)
	move.l		(%a2)+, 0x7f(%a2,%d4.w)
	move.l		(%a2)+, (0xffffffff).w
	move.l		(%a2)+, (0xffffffff).l
	move.l		-(%a2), %d4
	move.l		-(%a2), (%a2)
	move.l		-(%a2), (%a2)+
	move.l		-(%a2), -(%a2)
	move.l		-(%a2), 0x7fff(%a2)
	move.l		-(%a2), 0x7f(%a2,%d4.w)
	move.l		-(%a2), (0xffffffff).w
	move.l		-(%a2), (0xffffffff).l
	move.l		0x7fff(%a2), %d4
	move.l		0x7fff(%a2), (%a2)
	move.l		0x7fff(%a2), (%a2)+
	move.l		0x7fff(%a2), -(%a2)
	move.l		0x7fff(%a2), 0x7fff(%a2)
	move.l		0x7fff(%a2), 0x7f(%a2,%d4.w)
	move.l		0x7fff(%a2), (0xffffffff).w
	move.l		0x7fff(%a2), (0xffffffff).l
	move.l		0x7f(%a2,%d4.w), %d4
	move.l		0x7f(%a2,%d4.w), (%a2)
	move.l		0x7f(%a2,%d4.w), (%a2)+
	move.l		0x7f(%a2,%d4.w), -(%a2)
	move.l		0x7f(%a2,%d4.w), 0x7fff(%a2)
	move.l		0x7f(%a2,%d4.w), 0x7f(%a2,%d4.w)
	move.l		0x7f(%a2,%d4.w), (0xffffffff).w
	move.l		0x7f(%a2,%d4.w), (0xffffffff).l
	move.l		(0xffffffff).w, %d4
	move.l		(0xffffffff).w, (%a2)
	move.l		(0xffffffff).w, (%a2)+
	move.l		(0xffffffff).w, -(%a2)
	move.l		(0xffffffff).w, 0x7fff(%a2)
	move.l		(0xffffffff).w, 0x7f(%a2,%d4.w)
	move.l		(0xffffffff).w, (0xffffffff).w
	move.l		(0xffffffff).w, (0xffffffff).l
	move.l		(0xffffffff).l, %d4
	move.l		(0xffffffff).l, (%a2)
	move.l		(0xffffffff).l, (%a2)+
	move.l		(0xffffffff).l, -(%a2)
	move.l		(0xffffffff).l, 0x7fff(%a2)
	move.l		(0xffffffff).l, 0x7f(%a2,%d4.w)
	move.l		(0xffffffff).l, (0xffffffff).w
	move.l		(0xffffffff).l, (0xffffffff).l
	move.l		*(%pc), %d4
	move.l		*(%pc), (%a2)
	move.l		*(%pc), (%a2)+
	move.l		*(%pc), -(%a2)
	move.l		*(%pc), 0x7fff(%a2)
	move.l		*(%pc), 0x7f(%a2,%d4.w)
	move.l		*(%pc), (0xffffffff).w
	move.l		*(%pc), (0xffffffff).l
	move.l		*(%pc,%d4.w), %d4
	move.l		*(%pc,%d4.w), (%a2)
	move.l		*(%pc,%d4.w), (%a2)+
	move.l		*(%pc,%d4.w), -(%a2)
	move.l		*(%pc,%d4.w), 0x7fff(%a2)
	move.l		*(%pc,%d4.w), 0x7f(%a2,%d4.w)
	move.l		*(%pc,%d4.w), (0xffffffff).w
	move.l		*(%pc,%d4.w), (0xffffffff).l
	move.l		#0xffffffff, %d4
	move.l		#0xffffffff, (%a2)
	move.l		#0xffffffff, (%a2)+
	move.l		#0xffffffff, -(%a2)
	move.l		#0xffffffff, 0x7fff(%a2)
	move.l		#0xffffffff, 0x7f(%a2,%d4.w)
	move.l		#0xffffffff, (0xffffffff).w
	move.l		#0xffffffff, (0xffffffff).l


.group_0100:
	; Group: 0b0100 -> Miscellaneous
	move.w		%sr, %d4
	move.w		%sr, (%a2)
	move.w		%sr, (%a2)+
	move.w		%sr, -(%a2)
	move.w		%sr, 0x7fff(%a2)
	move.w		%sr, 0x7f(%a2,%d4.w)
	move.w		%sr, (0xffffffff).w
	move.w		%sr, (0xffffffff).l
	move.w		%ccr, %d4
	move.w		%ccr, (%a2)
	move.w		%ccr, (%a2)+
	move.w		%ccr, -(%a2)
	move.w		%ccr, 0x7fff(%a2)
	move.w		%ccr, 0x7f(%a2,%d4.w)
	move.w		%ccr, (0xffffffff).w
	move.w		%ccr, (0xffffffff).l
	move.w		%d4, %ccr
	move.w		(%a2), %ccr
	move.w		(%a2)+, %ccr
	move.w		-(%a2), %ccr
	move.w		0x7fff(%a2), %ccr
	move.w		0x7f(%a2,%d4.w), %ccr
	move.w		(0xffffffff).w, %ccr
	move.w		(0xffffffff).l, %ccr
	move.w		%d4, %sr
	move.w		(%a2), %sr
	move.w		(%a2)+, %sr
	move.w		-(%a2), %sr
	move.w		0x7fff(%a2), %sr
	move.w		0x7f(%a2,%d4.w), %sr
	move.w		(0xffffffff).w, %sr
	move.w		(0xffffffff).l, %sr

	bkpt		#7								; M68010+

	negx.b		%d4
	negx.b		(%a2)
	negx.b		(%a2)+
	negx.b		-(%a2)
	negx.b		0x7fff(%a2)
	negx.b		0x7f(%a2,%d4.w)
	negx.b		(0xffffffff).w
	negx.b		(0xffffffff).l
	negx.w		%d4
	negx.w		(%a2)
	negx.w		(%a2)+
	negx.w		-(%a2)
	negx.w		0x7fff(%a2)
	negx.w		0x7f(%a2,%d4.w)
	negx.w		(0xffffffff).w
	negx.w		(0xffffffff).l
	negx.l		%d4
	negx.l		(%a2)
	negx.l		(%a2)+
	negx.l		-(%a2)
	negx.l		0x7fff(%a2)
	negx.l		0x7f(%a2,%d4.w)
	negx.l		(0xffffffff).w
	negx.l		(0xffffffff).l

	clr.b		%d4
	clr.b		(%a2)
	clr.b		(%a2)+
	clr.b		-(%a2)
	clr.b		0x7fff(%a2)
	clr.b		0x7f(%a2,%d4.w)
	clr.b		(0xffffffff).w
	clr.b		(0xffffffff).l
	clr.w		%d4
	clr.w		(%a2)
	clr.w		(%a2)+
	clr.w		-(%a2)
	clr.w		0x7fff(%a2)
	clr.w		0x7f(%a2,%d4.w)
	clr.w		(0xffffffff).w
	clr.w		(0xffffffff).l
	clr.l		%d4
	clr.l		(%a2)
	clr.l		(%a2)+
	clr.l		-(%a2)
	clr.l		0x7fff(%a2)
	clr.l		0x7f(%a2,%d4.w)
	clr.l		(0xffffffff).w
	clr.l		(0xffffffff).l

	neg.b		%d4
	neg.b		(%a2)
	neg.b		(%a2)+
	neg.b		-(%a2)
	neg.b		0x7fff(%a2)
	neg.b		0x7f(%a2,%d4.w)
	neg.b		(0xffffffff).w
	neg.b		(0xffffffff).l
	neg.w		%d4
	neg.w		(%a2)
	neg.w		(%a2)+
	neg.w		-(%a2)
	neg.w		0x7fff(%a2)
	neg.w		0x7f(%a2,%d4.w)
	neg.w		(0xffffffff).w
	neg.w		(0xffffffff).l
	neg.l		%d4
	neg.l		(%a2)
	neg.l		(%a2)+
	neg.l		-(%a2)
	neg.l		0x7fff(%a2)
	neg.l		0x7f(%a2,%d4.w)
	neg.l		(0xffffffff).w
	neg.l		(0xffffffff).l

	not.b		%d4
	not.b		(%a2)
	not.b		(%a2)+
	not.b		-(%a2)
	not.b		0x7fff(%a2)
	not.b		0x7f(%a2,%d4.w)
	not.b		(0xffffffff).w
	not.b		(0xffffffff).l
	not.w		%d4
	not.w		(%a2)
	not.w		(%a2)+
	not.w		-(%a2)
	not.w		0x7fff(%a2)
	not.w		0x7f(%a2,%d4.w)
	not.w		(0xffffffff).w
	not.w		(0xffffffff).l
	not.l		%d4
	not.l		(%a2)
	not.l		(%a2)+
	not.l		-(%a2)
	not.l		0x7fff(%a2)
	not.l		0x7f(%a2,%d4.w)
	not.l		(0xffffffff).w
	not.l		(0xffffffff).l

	ext.w		%d4
	ext.l		%d4

	nbcd.b		%d4
	nbcd.b		(%a2)
	nbcd.b		(%a2)+
	nbcd.b		-(%a2)
	nbcd.b		0x7fff(%a2)
	nbcd.b		0x7f(%a2,%d4.w)
	nbcd.b		(0xffffffff).w
	nbcd.b		(0xffffffff).l

	swap		%d4

	pea.l		(%a2)
	pea.l		0x7fff(%a2)
	pea.l		0x7f(%a2,%d4.w)
	pea.l		(0xffffffff).w
	pea.l		(0xffffffff).l
	pea.l		*(%pc)
	pea.l		*(%pc,%d4.w)

	illegal

	tas.b		%d4
	tas.b		(%a2)
	tas.b		(%a2)+
	tas.b		-(%a2)
	tas.b		0x7fff(%a2)
	tas.b		0x7f(%a2,%d4.w)
	tas.b		(0xffffffff).w
	tas.b		(0xffffffff).l

	tst.b		%d4
	tst.b		(%a2)
	tst.b		(%a2)+
	tst.b		-(%a2)
	tst.b		0x7fff(%a2)
	tst.b		0x7f(%a2,%d4.w)
	tst.b		(0xffffffff).w
	tst.b		(0xffffffff).l
	tst.w		%d4
	tst.w		(%a2)
	tst.w		(%a2)+
	tst.w		-(%a2)
	tst.w		0x7fff(%a2)
	tst.w		0x7f(%a2,%d4.w)
	tst.w		(0xffffffff).w
	tst.w		(0xffffffff).l
	tst.l		%d4
	tst.l		(%a2)
	tst.l		(%a2)+
	tst.l		-(%a2)
	tst.l		0x7fff(%a2)
	tst.l		0x7f(%a2,%d4.w)
	tst.l		(0xffffffff).w
	tst.l		(0xffffffff).l

	trap		#7

	link		%a2, #0x7fff

	unlk		%a2

	move.l		%usp, %a2
	move.l		%a2, %usp

	reset

	nop

	stop		#0x4242

	rte

	rtd			#0x7fff							; M68010+

	rts

	trapv

	rtr

	movec.l		%sfc, %d4						; M68010+
	movec.l		%dfc, %d4
	movec.l		%usp, %d4
	movec.l		%vbr, %d4
	movec.l		%d4, %sfc
	movec.l		%d4, %dfc
	movec.l		%d4, %usp
	movec.l		%d4, %vbr
	movec.l		%sfc, %a2
	movec.l		%dfc, %a2
	movec.l		%usp, %a2
	movec.l		%vbr, %a2
	movec.l		%a2, %sfc
	movec.l		%a2, %dfc
	movec.l		%a2, %usp
	movec.l		%a2, %vbr

	jsr			(%a2)
	jsr			0x7fff(%a2)
	jsr			0x7f(%a2,%d4.w)
	jsr			(0xffffffff).w
	jsr			(0xffffffff).l
	jsr			*(%pc)
	jsr			*(%pc,%d4.w)

	jmp			(%a2)
	jmp			0x7fff(%a2)
	jmp			0x7f(%a2,%d4.w)
	jmp			(0xffffffff).w
	jmp			(0xffffffff).l
	jmp			*(%pc)
	jmp			*(%pc,%d4.w)

	movem.w		%d4-%a2, (%a2)
	movem.w		%d4-%a2, -(%a2)
	movem.w		%d4-%a2, 0x7fff(%a2)
	movem.w		%d4-%a2, 0x7f(%a2,%d4.w)
	movem.w		%d4-%a2, (0xffffffff).w
	movem.w		%d4-%a2, (0xffffffff).l
	movem.w		(%a2), %d4-%a2
	movem.w		(%a2)+, %d4-%a2
	movem.w		0x7fff(%a2), %d4-%a2
	movem.w		0x7f(%a2,%d4.w), %d4-%a2
	movem.w		(0xffffffff).w, %d4-%a2
	movem.w		(0xffffffff).l, %d4-%a2
	movem.w		*(%pc), %d4-%a2
	movem.w		*(%pc,%d4.w), %d4-%a2
	movem.l		%d4-%a2, (%a2)
	movem.l		%d4-%a2, -(%a2)
	movem.l		%d4-%a2, 0x7fff(%a2)
	movem.l		%d4-%a2, 0x7f(%a2,%d4.w)
	movem.l		%d4-%a2, (0xffffffff).w
	movem.l		%d4-%a2, (0xffffffff).l
	movem.l		(%a2), %d4-%a2
	movem.l		(%a2)+, %d4-%a2
	movem.l		0x7fff(%a2), %d4-%a2
	movem.l		0x7f(%a2,%d4.w), %d4-%a2
	movem.l		(0xffffffff).w, %d4-%a2
	movem.l		(0xffffffff).l, %d4-%a2
	movem.l		*(%pc), %d4-%a2
	movem.l		*(%pc,%d4.w), %d4-%a2

	lea.l		(%a2), %a2
	lea.l		0x7fff(%a2), %a2
	lea.l		(0xffffffff).w, %a2
	lea.l		(0xffffffff).l, %a2
	lea.l		*(%pc), %a2
	lea.l		*(%pc,%d4.w), %a2

	chk.w		%d4, %d2
	chk.w		(%a2), %d4
	chk.w		(%a2)+, %d4
	chk.w		-(%a2), %d4
	chk.w		0x7fff(%a2), %d4
	chk.w		0x7f(%a2,%d4.w), %d4
	chk.w		(0xffffffff).w, %d4
	chk.w		(0xffffffff).l, %d4
	chk.w		*(%pc), %d4
	chk.w		*(%pc,%d4.w), %d4
	chk.w		#0xffff, %d4

.group_0101:
	; Group: 0b0101 -> ADDQ/SUBQ/Scc/DBcc
	addq.b		#7, %d4
	addq.b		#7, (%a2)
	addq.b		#7, (%a2)+
	addq.b		#7, -(%a2)
	addq.b		#7, 0x7fff(%a2)
	addq.b		#7, 0x7f(%a2,%d4.w)
	addq.b		#7, (0xffffffff).w
	addq.b		#7, (0xffffffff).l
	addq.w		#7, %d4
	addq.w		#7, (%a2)
	addq.w		#7, (%a2)+
	addq.w		#7, -(%a2)
	addq.w		#7, 0x7fff(%a2)
	addq.w		#7, 0x7f(%a2,%d4.w)
	addq.w		#7, (0xffffffff).w
	addq.w		#7, (0xffffffff).l
	addq.l		#7, %d4
	addq.l		#7, (%a2)
	addq.l		#7, (%a2)+
	addq.l		#7, -(%a2)
	addq.l		#7, 0x7fff(%a2)
	addq.l		#7, 0x7f(%a2,%d4.w)
	addq.l		#7, (0xffffffff).w
	addq.l		#7, (0xffffffff).l

	subq.b		#7, %d4
	subq.b		#7, (%a2)
	subq.b		#7, (%a2)+
	subq.b		#7, -(%a2)
	subq.b		#7, 0x7fff(%a2)
	subq.b		#7, 0x7f(%a2,%d4.w)
	subq.b		#7, (0xffffffff).w
	subq.b		#7, (0xffffffff).l
	subq.w		#7, %d4
	subq.w		#7, (%a2)
	subq.w		#7, (%a2)+
	subq.w		#7, -(%a2)
	subq.w		#7, 0x7fff(%a2)
	subq.w		#7, 0x7f(%a2,%d4.w)
	subq.w		#7, (0xffffffff).w
	subq.w		#7, (0xffffffff).l
	subq.l		#7, %d4
	subq.l		#7, (%a2)
	subq.l		#7, (%a2)+
	subq.l		#7, -(%a2)
	subq.l		#7, 0x7fff(%a2)
	subq.l		#7, 0x7f(%a2,%d4.w)
	subq.l		#7, (0xffffffff).w
	subq.l		#7, (0xffffffff).l

	st			%d4
	st			(%a2)
	st			(%a2)+
	st			-(%a2)
	st			0x7fff(%a2)
	st			0x7f(%a2,%d4.w)
	st			(0xffffffff).w
	st			(0xffffffff).l

	sf			%d4
	sf			(%a2)
	sf			(%a2)+
	sf			-(%a2)
	sf			0x7fff(%a2)
	sf			0x7f(%a2,%d4.w)
	sf			(0xffffffff).w
	sf			(0xffffffff).l

	shi			%d4
	shi			(%a2)
	shi			(%a2)+
	shi			-(%a2)
	shi			0x7fff(%a2)
	shi			0x7f(%a2,%d4.w)
	shi			(0xffffffff).w
	shi			(0xffffffff).l

	sls			%d4
	sls			(%a2)
	sls			(%a2)+
	sls			-(%a2)
	sls			0x7fff(%a2)
	sls			0x7f(%a2,%d4.w)
	sls			(0xffffffff).w
	sls			(0xffffffff).l

	scc			%d4
	scc			(%a2)
	scc			(%a2)+
	scc			-(%a2)
	scc			0x7fff(%a2)
	scc			0x7f(%a2,%d4.w)
	scc			(0xffffffff).w
	scc			(0xffffffff).l

	scs			%d4
	scs			(%a2)
	scs			(%a2)+
	scs			-(%a2)
	scs			0x7fff(%a2)
	scs			0x7f(%a2,%d4.w)
	scs			(0xffffffff).w
	scs			(0xffffffff).l

	sne			%d4
	sne			(%a2)
	sne			(%a2)+
	sne			-(%a2)
	sne			0x7fff(%a2)
	sne			0x7f(%a2,%d4.w)
	sne			(0xffffffff).w
	sne			(0xffffffff).l

	seq			%d4
	seq			(%a2)
	seq			(%a2)+
	seq			-(%a2)
	seq			0x7fff(%a2)
	seq			0x7f(%a2,%d4.w)
	seq			(0xffffffff).w
	seq			(0xffffffff).l

	svc			%d4
	svc			(%a2)
	svc			(%a2)+
	svc			-(%a2)
	svc			0x7fff(%a2)
	svc			0x7f(%a2,%d4.w)
	svc			(0xffffffff).w
	svc			(0xffffffff).l

	svs			%d4
	svs			(%a2)
	svs			(%a2)+
	svs			-(%a2)
	svs			0x7fff(%a2)
	svs			0x7f(%a2,%d4.w)
	svs			(0xffffffff).w
	svs			(0xffffffff).l

	spl			%d4
	spl			(%a2)
	spl			(%a2)+
	spl			-(%a2)
	spl			0x7fff(%a2)
	spl			0x7f(%a2,%d4.w)
	spl			(0xffffffff).w
	spl			(0xffffffff).l

	smi			%d4
	smi			(%a2)
	smi			(%a2)+
	smi			-(%a2)
	smi			0x7fff(%a2)
	smi			0x7f(%a2,%d4.w)
	smi			(0xffffffff).w
	smi			(0xffffffff).l

	sge			%d4
	sge			(%a2)
	sge			(%a2)+
	sge			-(%a2)
	sge			0x7fff(%a2)
	sge			0x7f(%a2,%d4.w)
	sge			(0xffffffff).w
	sge			(0xffffffff).l

	slt			%d4
	slt			(%a2)
	slt			(%a2)+
	slt			-(%a2)
	slt			0x7fff(%a2)
	slt			0x7f(%a2,%d4.w)
	slt			(0xffffffff).w
	slt			(0xffffffff).l

	sgt			%d4
	sgt			(%a2)
	sgt			(%a2)+
	sgt			-(%a2)
	sgt			0x7fff(%a2)
	sgt			0x7f(%a2,%d4.w)
	sgt			(0xffffffff).w
	sgt			(0xffffffff).l

	sle			%d4
	sle			(%a2)
	sle			(%a2)+
	sle			-(%a2)
	sle			0x7fff(%a2)
	sle			0x7f(%a2,%d4.w)
	sle			(0xffffffff).w
	sle			(0xffffffff).l

	dbt			%d4, *
	dbf			%d4, *
	dbhi		%d4, *
	dbls		%d4, *
	dbcc		%d4, *
	dbcs		%d4, *
	dbne		%d4, *
	dbeq		%d4, *
	dbvc		%d4, *
	dbvs		%d4, *
	dbpl		%d4, *
	dbmi		%d4, *
	dbge		%d4, *
	dblt		%d4, *
	dbgt		%d4, *
	dble		%d4, *

.group_0110:
	; Group: 0b0110 -> Bcc/BSR/BRA
	bra.b		*
	bra.w		*

	bsr.b		*
	bsr.w		*

	bhi.b		*
	bls.b		*
	bcc.b		*
	bcs.b		*
	bne.b		*
	beq.b		*
	bvc.b		*
	bvs.b		*
	bpl.b		*
	bmi.b		*
	bge.b		*
	blt.b		*
	bgt.b		*
	ble.b		*
	bhi.w		*
	bls.w		*
	bcc.w		*
	bcs.w		*
	bne.w		*
	beq.w		*
	bvc.w		*
	bvs.w		*
	bpl.w		*
	bmi.w		*
	bge.w		*
	blt.w		*
	bgt.w		*
	ble.w		*

.group_0111:
	; Group: 0b0111 -> MOVEQ

	moveq		#0x7f, %d4

.group_1000:
	; Group: 0b1000 -> OR/DIV/SBCD

	divu.w		%d4, %d2
	divu.w		(%a2), %d4
	divu.w		(%a2)+, %d4
	divu.w		-(%a2), %d4
	divu.w		0x7fff(%a2), %d4
	divu.w		0x7f(%a2,%d4.w), %d4
	divu.w		(0xffffffff).w, %d4
	divu.w		(0xffffffff).l, %d4
	divu.w		*(%pc), %d4
	divu.w		*(%pc,%d4.w), %d4
	divu.w		#0xffff, %d4

	divs.w		%d4, %d2
	divs.w		(%a2), %d4
	divs.w		(%a2)+, %d4
	divs.w		-(%a2), %d4
	divs.w		0x7fff(%a2), %d4
	divs.w		0x7f(%a2,%d4.w), %d4
	divs.w		(0xffffffff).w, %d4
	divs.w		(0xffffffff).l, %d4
	divs.w		*(%pc), %d4
	divs.w		*(%pc,%d4.w), %d4
	divs.w		#0xffff, %d4

	sbcd.b		%d4, %d2
	sbcd.b		-(%a4), -(%a2)

	or.b		%d4, (%a2)
	or.b		%d4, (%a2)+
	or.b		%d4, -(%a2)
	or.b		%d4, (0xffffffff).w
	or.b		%d4, (0xffffffff).l
	or.b		%d4, %d2
	or.b		(%a2), %d4
	or.b		(%a2)+, %d4
	or.b		-(%a2), %d4
	or.b		(0xffffffff).w, %d4
	or.b		(0xffffffff).l, %d4
	or.b		0x7fff(%a2), %d4
	or.b		0x7f(%a2,%d4.w), %d4
	or.b		*(%pc), %d4
	or.b		*(%pc,%d4.w), %d4
	or.b		#0xff, %d4
	or.w		%d4, (%a2)
	or.w		%d4, (%a2)+
	or.w		%d4, -(%a2)
	or.w		%d4, (0xffffffff).w
	or.w		%d4, (0xffffffff).l
	or.w		%d4, %d2
	or.w		(%a2), %d4
	or.w		(%a2)+, %d4
	or.w		-(%a2), %d4
	or.w		(0xffffffff).w, %d4
	or.w		(0xffffffff).l, %d4
	or.w		0x7fff(%a2), %d4
	or.w		0x7f(%a2,%d4.w), %d4
	or.w		*(%pc), %d4
	or.w		*(%pc,%d4.w), %d4
	or.w		#0xffff, %d4
	or.l		%d4, (%a2)
	or.l		%d4, (%a2)+
	or.l		%d4, -(%a2)
	or.l		%d4, (0xffffffff).w
	or.l		%d4, (0xffffffff).l
	or.l		%d4, %d2
	or.l		(%a2), %d4
	or.l		(%a2)+, %d4
	or.l		-(%a2), %d4
	or.l		(0xffffffff).w, %d4
	or.l		(0xffffffff).l, %d4
	or.l		0x7fff(%a2), %d4
	or.l		0x7f(%a2,%d4.w), %d4
	or.l		*(%pc), %d4
	or.l		*(%pc,%d4.w), %d4
	or.l		#0xffffffff, %d4

.group_1001:
	; Group: 0b1001 -> SUB/SUBX
	sub.b		%d4, (%a2)
	sub.b		%d4, (%a2)+
	sub.b		%d4, -(%a2)
	sub.b		%d4, 0x7fff(%a2)
	sub.b		%d4, 0x7f(%a2,%d4.w)
	sub.b		%d4, (0xffffffff).w
	sub.b		%d4, (0xffffffff).l
	sub.b		%d4, %d2
	sub.b		(%a2), %d4
	sub.b		(%a2)+, %d4
	sub.b		-(%a2), %d4
	sub.b		0x7fff(%a2), %d4
	sub.b		0x7f(%a2,%d4.w), %d4
	sub.b		(0xffffffff).w, %d4
	sub.b		(0xffffffff).l, %d4
	sub.b		*(%pc), %d4
	sub.b		*(%pc,%d4.w), %d4
	sub.w		%d4, (%a2)
	sub.w		%d4, (%a2)+
	sub.w		%d4, -(%a2)
	sub.w		%d4, 0x7fff(%a2)
	sub.w		%d4, 0x7f(%a2,%d4.w)
	sub.w		%d4, (0xffffffff).w
	sub.w		%d4, (0xffffffff).l
	sub.w		%d4, %d2
	sub.w		(%a2), %d4
	sub.w		(%a2)+, %d4
	sub.w		-(%a2), %d4
	sub.w		0x7fff(%a2), %d4
	sub.w		0x7f(%a2,%d4.w), %d4
	sub.w		(0xffffffff).w, %d4
	sub.w		(0xffffffff).l, %d4
	sub.w		*(%pc), %d4
	sub.w		*(%pc,%d4.w), %d4
	sub.l		%d4, (%a2)
	sub.l		%d4, (%a2)+
	sub.l		%d4, -(%a2)
	sub.l		%d4, 0x7fff(%a2)
	sub.l		%d4, 0x7f(%a2,%d4.w)
	sub.l		%d4, (0xffffffff).w
	sub.l		%d4, (0xffffffff).l
	sub.l		%d4, %d2
	sub.l		(%a2), %d4
	sub.l		(%a2)+, %d4
	sub.l		-(%a2), %d4
	sub.l		0x7fff(%a2), %d4
	sub.l		0x7f(%a2,%d4.w), %d4
	sub.l		(0xffffffff).w, %d4
	sub.l		(0xffffffff).l, %d4
	sub.l		*(%pc), %d4
	sub.l		*(%pc,%d4.w), %d4

	subx.b		%d4, %d4
	subx.b		-(%a4), -(%a2)
	subx.w		%d4, %d4
	subx.w		-(%a4), -(%a2)
	subx.l		%d4, %d4
	subx.l		-(%a4), -(%a2)

	suba.w		%d4, %a2
	suba.w		%a4, %a2
	suba.w		(%a4), %a2
	suba.w		(%a4)+, %a2
	suba.w		-(%a4), %a2
	suba.w		0x7fff(%a4), %a2
	suba.w		0x7f(%a4,%d4.w), %a2
	suba.w		(0xffffffff).w, %a2
	suba.w		(0xffffffff).l, %a2
	suba.w		#0xffff, %a2
	suba.w		*(%pc), %a2
	suba.w		*(%pc,%d4.w), %a2
	suba.l		%d4, %a2
	suba.l		%a4, %a2
	suba.l		(%a4), %a2
	suba.l		(%a4)+, %a2
	suba.l		-(%a4), %a2
	suba.l		0x7fff(%a4), %a2
	suba.l		0x7f(%a4,%d4.w), %a2
	suba.l		(0xffffffff).w, %a2
	suba.l		(0xffffffff).l, %a2
	suba.l		#0xffff, %a2
	suba.l		*(%pc), %a2
	suba.l		*(%pc,%d4.w), %a2


.group_1011:
	; Group: 0b1011 -> CMP/EOR
	eor.b		%d4, %d2
	eor.b		%d4, (%a2)
	eor.b		%d4, (%a2)+
	eor.b		%d4, -(%a2)
	eor.b		%d4, 0x7fff(%a2)
	eor.b		%d4, 0x7f(%a2,%d4.w)
	eor.b		%d4, (0xffffffff).w
	eor.b		%d4, (0xffffffff).l
	eor.w		%d4, %d2
	eor.w		%d4, (%a2)
	eor.w		%d4, (%a2)+
	eor.w		%d4, -(%a2)
	eor.w		%d4, 0x7fff(%a2)
	eor.w		%d4, 0x7f(%a2,%d4.w)
	eor.w		%d4, (0xffffffff).w
	eor.w		%d4, (0xffffffff).l
	eor.l		%d4, %d2
	eor.l		%d4, (%a2)
	eor.l		%d4, (%a2)+
	eor.l		%d4, -(%a2)
	eor.l		%d4, 0x7fff(%a2)
	eor.l		%d4, 0x7f(%a2,%d4.w)
	eor.l		%d4, (0xffffffff).w
	eor.l		%d4, (0xffffffff).l

	cmpm.b		(%a4)+, (%a2)+
	cmpm.w		(%a4)+, (%a2)+
	cmpm.l		(%a4)+, (%a2)+

	cmp.b		%d4, %d4
	cmp.b		(%a2), %d4
	cmp.b		(%a2)+, %d4
	cmp.b		-(%a2), %d4
	cmp.b		0x7fff(%a2), %d4
	cmp.b		0x7f(%a2,%d4.w), %d4
	cmp.b		(0xffffffff).w, %d4
	cmp.b		(0xffffffff).l, %d4
	cmp.b		*(%pc), %d4
	cmp.b		*(%pc,%d4.w), %d4
	cmp.b		#0xff, %d4
	cmp.w		%d4, %d4
	cmp.w		(%a2), %d4
	cmp.w		(%a2)+, %d4
	cmp.w		-(%a2), %d4
	cmp.w		0x7fff(%a2), %d4
	cmp.w		0x7f(%a2,%d4.w), %d4
	cmp.w		(0xffffffff).w, %d4
	cmp.w		(0xffffffff).l, %d4
	cmp.w		*(%pc), %d4
	cmp.w		*(%pc,%d4.w), %d4
	cmp.w		#0xffff, %d4
	cmp.l		%d4, %d4
	cmp.l		(%a2), %d4
	cmp.l		(%a2)+, %d4
	cmp.l		-(%a2), %d4
	cmp.l		0x7fff(%a2), %d4
	cmp.l		0x7f(%a2,%d4.w), %d4
	cmp.l		(0xffffffff).w, %d4
	cmp.l		(0xffffffff).l, %d4
	cmp.l		*(%pc), %d4
	cmp.l		*(%pc,%d4.w), %d4
	cmp.l		#0xffffffff, %d4

	cmpa.w		%d4, %a2
	cmpa.w		%a4, %a2
	cmpa.w		(%a4), %a2
	cmpa.w		(%a4)+, %a2
	cmpa.w		-(%a4), %a2
	cmpa.w		0x7fff(%a4), %a2
	cmpa.w		0x7f(%a4,%d4.w), %a2
	cmpa.w		(0xffffffff).w, %a2
	cmpa.w		(0xffffffff).l, %a2
	cmpa.w		*(%pc), %a2
	cmpa.w		*(%pc,%d4.w), %a2
	cmpa.w		#0xffff, %a2
	cmpa.l		%d4, %a2
	cmpa.l		%a4, %a2
	cmpa.l		(%a4), %a2
	cmpa.l		(%a4)+, %a2
	cmpa.l		-(%a4), %a2
	cmpa.l		0x7fff(%a4), %a2
	cmpa.l		0x7f(%a4,%d4.w), %a2
	cmpa.l		(0xffffffff).w, %a2
	cmpa.l		(0xffffffff).l, %a2
	cmpa.l		*(%pc), %a2
	cmpa.l		*(%pc,%d4.w), %a2
	cmpa.l		#0xffff, %a2


.group_1100:
	; Group: 0b1100 -> AND/MUL/ABCD/EXG
	mulu.w		%d4, %d2
	mulu.w		(%a2), %d4
	mulu.w		(%a2)+, %d4
	mulu.w		-(%a2), %d4
	mulu.w		0x7fff(%a2), %d4
	mulu.w		0x7f(%a2,%d4.w), %d4
	mulu.w		(0xffffffff).w, %d4
	mulu.w		(0xffffffff).l, %d4
	mulu.w		*(%pc), %d4
	mulu.w		*(%pc,%d4.w), %d4
	mulu.w		#0xffff, %d4

	muls.w		%d4, %d2
	muls.w		(%a2), %d4
	muls.w		(%a2)+, %d4
	muls.w		-(%a2), %d4
	muls.w		0x7fff(%a2), %d4
	muls.w		0x7f(%a2,%d4.w), %d4
	muls.w		(0xffffffff).w, %d4
	muls.w		(0xffffffff).l, %d4
	muls.w		*(%pc), %d4
	muls.w		*(%pc,%d4.w), %d4
	muls.w		#0xffff, %d4

	abcd.b		%d4, %d2
	abcd.b		-(%a4), -(%a2)

	exg.l		%d4, %d2
	exg.l		%a4, %a2
	exg.l		%d4, %a2
	exg.l		%a4, %d2

	and.b		%d4, (%a2)
	and.b		%d4, (%a2)+
	and.b		%d4, -(%a2)
	and.b		%d4, (0xffffffff).w
	and.b		%d4, (0xffffffff).l
	and.b		%d4, %d2
	and.b		(%a2), %d4
	and.b		(%a2)+, %d4
	and.b		-(%a2), %d4
	and.b		(0xffffffff).w, %d4
	and.b		(0xffffffff).l, %d4
	and.b		0x7fff(%a2), %d4
	and.b		0x7f(%a2,%d4.w), %d4
	and.b		*(%pc), %d4
	and.b		*(%pc,%d4.w), %d4
	and.b		#0xff, %d4
	and.w		%d4, (%a2)
	and.w		%d4, (%a2)+
	and.w		%d4, -(%a2)
	and.w		%d4, (0xffffffff).w
	and.w		%d4, (0xffffffff).l
	and.w		%d4, %d2
	and.w		(%a2), %d4
	and.w		(%a2)+, %d4
	and.w		-(%a2), %d4
	and.w		(0xffffffff).w, %d4
	and.w		(0xffffffff).l, %d4
	and.w		0x7fff(%a2), %d4
	and.w		0x7f(%a2,%d4.w), %d4
	and.w		*(%pc), %d4
	and.w		*(%pc,%d4.w), %d4
	and.w		#0xffff, %d4
	and.l		%d4, (%a2)
	and.l		%d4, (%a2)+
	and.l		%d4, -(%a2)
	and.l		%d4, (0xffffffff).w
	and.l		%d4, (0xffffffff).l
	and.l		%d4, %d2
	and.l		(%a2), %d4
	and.l		(%a2)+, %d4
	and.l		-(%a2), %d4
	and.l		(0xffffffff).w, %d4
	and.l		(0xffffffff).l, %d4
	and.l		0x7fff(%a2), %d4
	and.l		0x7f(%a2,%d4.w), %d4
	and.l		*(%pc), %d4
	and.l		*(%pc,%d4.w), %d4
	and.l		#0xffffffff, %d4


.group_1101:
	; Group: 0b1101 -> ADD/ADDX
	add.b		%d4, (%a2)
	add.b		%d4, (%a2)+
	add.b		%d4, -(%a2)
	add.b		%d4, 0x7fff(%a2)
	add.b		%d4, 0x7f(%a2,%d4.w)
	add.b		%d4, (0xffffffff).w
	add.b		%d4, (0xffffffff).l
	add.b		%d4, %d2
	add.b		(%a2), %d4
	add.b		(%a2)+, %d4
	add.b		-(%a2), %d4
	add.b		0x7fff(%a2), %d4
	add.b		0x7f(%a2,%d4.w), %d4
	add.b		(0xffffffff).w, %d4
	add.b		(0xffffffff).l, %d4
	add.b		*(%pc), %d4
	add.b		*(%pc,%d4.w), %d4
	add.w		%d4, (%a2)
	add.w		%d4, (%a2)+
	add.w		%d4, -(%a2)
	add.w		%d4, 0x7fff(%a2)
	add.w		%d4, 0x7f(%a2,%d4.w)
	add.w		%d4, (0xffffffff).w
	add.w		%d4, (0xffffffff).l
	add.w		%d4, %d2
	add.w		(%a2), %d4
	add.w		(%a2)+, %d4
	add.w		-(%a2), %d4
	add.w		0x7fff(%a2), %d4
	add.w		0x7f(%a2,%d4.w), %d4
	add.w		(0xffffffff).w, %d4
	add.w		(0xffffffff).l, %d4
	add.w		*(%pc), %d4
	add.w		*(%pc,%d4.w), %d4
	add.l		%d4, (%a2)
	add.l		%d4, (%a2)+
	add.l		%d4, -(%a2)
	add.l		%d4, 0x7fff(%a2)
	add.l		%d4, 0x7f(%a2,%d4.w)
	add.l		%d4, (0xffffffff).w
	add.l		%d4, (0xffffffff).l
	add.l		%d4, %d2
	add.l		(%a2), %d4
	add.l		(%a2)+, %d4
	add.l		-(%a2), %d4
	add.l		0x7fff(%a2), %d4
	add.l		0x7f(%a2,%d4.w), %d4
	add.l		(0xffffffff).w, %d4
	add.l		(0xffffffff).l, %d4
	add.l		*(%pc), %d4
	add.l		*(%pc,%d4.w), %d4

	addx.b		%d4, %d4
	addx.b		-(%a4), -(%a2)
	addx.w		%d4, %d4
	addx.w		-(%a4), -(%a2)
	addx.l		%d4, %d4
	addx.l		-(%a4), -(%a2)

	adda.w		%d4, %a2
	adda.w		%a4, %a2
	adda.w		(%a4), %a2
	adda.w		(%a4)+, %a2
	adda.w		-(%a4), %a2
	adda.w		0x7fff(%a4), %a2
	adda.w		0x7f(%a4,%d4.w), %a2
	adda.w		(0xffffffff).w, %a2
	adda.w		(0xffffffff).l, %a2
	adda.w		#0xffff, %a2
	adda.w		*(%pc), %a2
	adda.w		*(%pc,%d4.w), %a2
	adda.l		%d4, %a2
	adda.l		%a4, %a2
	adda.l		(%a4), %a2
	adda.l		(%a4)+, %a2
	adda.l		-(%a4), %a2
	adda.l		0x7fff(%a4), %a2
	adda.l		0x7f(%a4,%d4.w), %a2
	adda.l		(0xffffffff).w, %a2
	adda.l		(0xffffffff).l, %a2
	adda.l		#0xffff, %a2
	adda.l		*(%pc), %a2
	adda.l		*(%pc,%d4.w), %a2


.group_1110:
	; Group: 0b1110 -> Shift/Rotate
	asl.b		%d4, %d2
	asl.b		#4, %d2
	asl.w		%d4, %d2
	asl.w		#4, %d2
	asl.l		%d4, %d2
	asl.l		#4, %d2
	asl.w		(%a2)
	asl.w		(%a2)+
	asl.w		-(%a2)
	asl.w		0x7fff(%a2)
	asl.w		0x7f(%a2,%d4.w)
	asl.w		(0xffffffff).w
	asl.w		(0xffffffff).l

	asr.b		%d4, %d2
	asr.b		#4, %d2
	asr.w		%d4, %d2
	asr.w		#4, %d2
	asr.l		%d4, %d2
	asr.l		#4, %d2
	asr.w		(%a2)
	asr.w		(%a2)+
	asr.w		-(%a2)
	asr.w		0x7fff(%a2)
	asr.w		0x7f(%a2,%d4.w)
	asr.w		(0xffffffff).w
	asr.w		(0xffffffff).l

	lsl.b		%d4, %d2
	lsl.b		#4, %d2
	lsl.w		%d4, %d2
	lsl.w		#4, %d2
	lsl.l		%d4, %d2
	lsl.l		#4, %d2
	lsl.w		(%a2)
	lsl.w		(%a2)+
	lsl.w		-(%a2)
	lsl.w		0x7fff(%a2)
	lsl.w		0x7f(%a2,%d4.w)
	lsl.w		(0xffffffff).w
	lsl.w		(0xffffffff).l

	lsr.b		%d4, %d2
	lsr.b		#4, %d2
	lsr.w		%d4, %d2
	lsr.w		#4, %d2
	lsr.l		%d4, %d2
	lsr.l		#4, %d2
	lsr.w		(%a2)
	lsr.w		(%a2)+
	lsr.w		-(%a2)
	lsr.w		0x7fff(%a2)
	lsr.w		0x7f(%a2,%d4.w)
	lsr.w		(0xffffffff).w
	lsr.w		(0xffffffff).l

	roxl.b		%d4, %d2
	roxl.b		#4, %d2
	roxl.w		%d4, %d2
	roxl.w		#4, %d2
	roxl.l		%d4, %d2
	roxl.l		#4, %d2
	roxl.w		(%a2)
	roxl.w		(%a2)+
	roxl.w		-(%a2)
	roxl.w		0x7fff(%a2)
	roxl.w		0x7f(%a2,%d4.w)
	roxl.w		(0xffffffff).w
	roxl.w		(0xffffffff).l

	roxr.b		%d4, %d2
	roxr.b		#4, %d2
	roxr.w		%d4, %d2
	roxr.w		#4, %d2
	roxr.l		%d4, %d2
	roxr.l		#4, %d2
	roxr.w		(%a2)
	roxr.w		(%a2)+
	roxr.w		-(%a2)
	roxr.w		0x7fff(%a2)
	roxr.w		0x7f(%a2,%d4.w)
	roxr.w		(0xffffffff).w
	roxr.w		(0xffffffff).l

	rol.b		%d4, %d2
	rol.b		#4, %d2
	rol.w		%d4, %d2
	rol.w		#4, %d2
	rol.l		%d4, %d2
	rol.l		#4, %d2
	rol.w		(%a2)
	rol.w		(%a2)+
	rol.w		-(%a2)
	rol.w		0x7fff(%a2)
	rol.w		0x7f(%a2,%d4.w)
	rol.w		(0xffffffff).w
	rol.w		(0xffffffff).l

	ror.b		%d4, %d2
	ror.b		#4, %d2
	ror.w		%d4, %d2
	ror.w		#4, %d2
	ror.l		%d4, %d2
	ror.l		#4, %d2
	ror.w		(%a2)
	ror.w		(%a2)+
	ror.w		-(%a2)
	ror.w		0x7fff(%a2)
	ror.w		0x7f(%a2,%d4.w)
	ror.w		(0xffffffff).w
	ror.w		(0xffffffff).l
