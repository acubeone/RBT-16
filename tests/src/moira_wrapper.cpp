#include "moira_wrapper.h"

#include "Moira.h"
#include "MoiraTypes.h"

#include <cassert>
#include <cstring>
#include <exception>
#include <memory>

struct MoiraCpu {
	moira_read8 read8 = nullptr;
	moira_read16 read16 = nullptr;
	moira_write8 write8 = nullptr;
	moira_write16 write16 = nullptr;

	void *user;
	moira::Model model;
	u16 exception_vector;
	i32 exception_kind;

	std::unique_ptr<class MoiraHandler> handler;
};

class MoiraHandler : public moira::Moira {
  private:
	struct MoiraCpu *m_owner;

  protected:
	u8 read8(u32 addr) const override {
		if (m_owner->read8)
			return m_owner->read8(m_owner->user, addr);
		return 0;
	}

	u16 read16(u32 addr) const override {
		if (m_owner->read16)
			return m_owner->read16(m_owner->user, addr);

		return 0;
	}

	void write8(u32 addr, u8 val) const override {
		if (m_owner->write8)
			m_owner->write8(m_owner->user, addr, val);
	}

	void write16(u32 addr, u16 val) const override {
		if (m_owner->write16)
			m_owner->write16(m_owner->user, addr, val);
	}

	void willExecute(moira::M68kException e, u16 vector) override {
		m_owner->exception_vector = vector;

		using E = moira::M68kException;
		switch (e) {
		case E::RESET:			   m_owner->exception_kind = MOIRA_EXC_OK; break; // Ignore reset
		case E::BUS_ERROR:		   m_owner->exception_kind = MOIRA_EXC_BUS_ERR; break;
		case E::ADDRESS_ERROR:	   m_owner->exception_kind = MOIRA_EXC_ADDR_ERR; break;
		case E::ILLEGAL:		   m_owner->exception_kind = MOIRA_EXC_ILLEGAL; break;
		case E::DIVIDE_BY_ZERO:	   m_owner->exception_kind = MOIRA_EXC_DIVBYZERO; break;
		case E::CHK:			   m_owner->exception_kind = MOIRA_EXC_CHK; break;
		case E::TRAPV:			   m_owner->exception_kind = MOIRA_EXC_TRAPV; break;
		case E::PRIVILEGE:		   m_owner->exception_kind = MOIRA_EXC_PRIVILEGE; break;
		case E::TRACE:			   m_owner->exception_kind = MOIRA_EXC_TRACE; break;
		case E::LINEA:			   m_owner->exception_kind = MOIRA_EXC_LINEA; break;
		case E::LINEF:			   m_owner->exception_kind = MOIRA_EXC_LINEF; break;
		case E::FORMAT_ERROR:	   m_owner->exception_kind = MOIRA_EXC_FORMAT_ERR; break;
		case E::IRQ_UNINITIALIZED: m_owner->exception_kind = MOIRA_EXC_IRQ_UNINIT; break;
		case E::IRQ_SPURIOUS:	   m_owner->exception_kind = MOIRA_EXC_IRQ_SPURIOUS; break;
		case E::TRAP:			   m_owner->exception_kind = MOIRA_EXC_TRAP; break;
		case E::BKPT:			   m_owner->exception_kind = MOIRA_EXC_BKPT; break;
		default:				   m_owner->exception_kind = MOIRA_EXC_OK; break;
		}
	}

  public:
	MoiraHandler(MoiraCpu *owner)
		: m_owner(owner) { }
};

i32 moira_init(
	MoiraCpu *moira, moira_read8 r8, moira_read16 r16, moira_write8 w8, moira_write16 w16
) {
	assert(moira);

	try {
		memset((void *)moira, 0, sizeof(MoiraCpu));
		moira->read8 = r8;
		moira->read16 = r16;
		moira->write8 = w8;
		moira->write16 = w16;

		moira->user = nullptr;
		moira->model = moira::Model::M68000;
		moira->exception_kind = MOIRA_EXC_OK;
		moira->exception_vector = -1;

		moira->handler = std::make_unique<MoiraHandler>(moira);
	} catch (std::exception& _e) {
		return -1;
	}

	return 0;
}

void moira_deinit(MoiraCpu *moira) {
	assert(moira);

	moira->handler = nullptr;
}

void moira_set_userdata(MoiraCpu *cpu, void *userdata) {
	if (cpu)
		cpu->user = userdata;
}

void moira_reset(MoiraCpu *cpu, MoiraModel model) {
	if (!cpu)
		return;

	if (model == MOIRA_MODEL_M68010)
		cpu->model = moira::Model::M68010;
	else
		cpu->model = moira::Model::M68000;

	cpu->handler->setModel(cpu->model);
	cpu->exception_vector = -1;
	(*cpu->handler).reset();
}

void moira_execute(MoiraCpu *cpu) {
	if (cpu)
		cpu->handler->execute();
}

bool moira_is_instruction_valid(const MoiraCpu *cpu, u16 opcode, u16 ext) {
	if (!cpu)
		return false;

	moira::InstrInfo instr = cpu->handler->getInstrInfo(opcode);
	return cpu->handler->isAvailable(cpu->model, instr.I, instr.M, instr.S, ext);
}

i32 moira_disassemble(const MoiraCpu *cpu, char *str, u32 addr) {
	if (!cpu || !str)
		return 0;

	return cpu->handler->disassemble(str, addr);
}

void moira_get_registers(const MoiraCpu *cpu, u32 *regs) {
	assert(regs);

	if (!cpu)
		return;

	for (i32 i = 0; i < 8; i += 1) {
		regs[i + MOIRA_REG_D0] = cpu->handler->getD(i);
		regs[i + MOIRA_REG_A0] = cpu->handler->getA(i);
	}

	regs[MOIRA_REG_PC] = cpu->handler->getPC();
	regs[MOIRA_REG_SR] = (u32)cpu->handler->getSR();
	regs[MOIRA_REG_SP] = cpu->handler->getSP();
	regs[MOIRA_REG_USP] = cpu->handler->getUSP();
	regs[MOIRA_REG_ISP] = cpu->handler->getISP();
	regs[MOIRA_REG_VBR] = cpu->handler->getVBR();
	regs[MOIRA_REG_SFC] = cpu->handler->getSFC();
	regs[MOIRA_REG_DFC] = cpu->handler->getDFC();
}

void moira_set_registers(MoiraCpu *cpu, const u32 *regs) {
	if (!cpu)
		return;

	for (i32 i = 0; i < 8; i += 1) {
		cpu->handler->setD(i, regs[i + MOIRA_REG_D0]);
		cpu->handler->setA(i, regs[i + MOIRA_REG_A0]);
	}

	cpu->handler->setPC(regs[MOIRA_REG_PC]);
	cpu->handler->setSR(regs[MOIRA_REG_SR]);
	cpu->handler->setSP(regs[MOIRA_REG_SP]);
	cpu->handler->setUSP(regs[MOIRA_REG_USP]);
	cpu->handler->setISP(regs[MOIRA_REG_ISP]);
	cpu->handler->setVBR(regs[MOIRA_REG_VBR]);
	cpu->handler->setSFC(regs[MOIRA_REG_SFC]);
	cpu->handler->setDFC(regs[MOIRA_REG_DFC]);
}

i64 moira_get_clocks(const MoiraCpu *cpu) {
	assert(cpu);
	return cpu->handler->getClock();
}

void moira_set_clocks(MoiraCpu *cpu, i64 cycles) {
	assert(cpu);
	cpu->handler->setClock(cycles);
}

u32 moira_get_pc0(const MoiraCpu *cpu) {
	if (!cpu)
		return 0;

	assert(cpu);
	return cpu->handler->getPC0();
}

u8 moira_read_fc(const MoiraCpu *cpu) {
	assert(cpu);
	return cpu->handler->readFC();
}

u8 moira_get_ipl(const MoiraCpu *cpu) {
	assert(cpu);
	return cpu->handler->getIPL();
}

void moira_get_exception(const MoiraCpu *cpu, i32 *out_vector, u16 *out_kind) {
	assert(cpu);
	assert(out_vector && out_kind);
	*out_vector = cpu->exception_vector;
	*out_kind = cpu->exception_kind;
}
