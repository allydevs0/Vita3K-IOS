#include <cpu/impl/unicorn_cpu.h>

UnicornCPU::UnicornCPU() : is_running(false) {
    uc_err err = uc_open(UC_ARCH_ARM, UC_MODE_ARM, &uc);
    if (err != UC_ERR_OK) {
        // Handle error
    }
}

UnicornCPU::~UnicornCPU() {
    if (uc) {
        uc_close(uc);
    }
}

int UnicornCPU::run() {
    is_running = true;
    // Stub: need to map memory and start execution
    // uc_emu_start(uc, pc, 0xFFFFFFFF, 0, 0);
    return 0;
}

void UnicornCPU::stop() {
    is_running = false;
    if (uc) {
        uc_emu_stop(uc);
    }
}

uint32_t UnicornCPU::get_reg(uint8_t idx) {
    uint32_t val = 0;
    uc_reg_read(uc, UC_ARM_REG_R0 + idx, &val);
    return val;
}

void UnicornCPU::set_reg(uint8_t idx, uint32_t val) {
    uc_reg_write(uc, UC_ARM_REG_R0 + idx, &val);
}

uint32_t UnicornCPU::get_sp() {
    uint32_t val = 0;
    uc_reg_read(uc, UC_ARM_REG_SP, &val);
    return val;
}

void UnicornCPU::set_sp(uint32_t val) {
    uc_reg_write(uc, UC_ARM_REG_SP, &val);
}

uint32_t UnicornCPU::get_pc() {
    uint32_t val = 0;
    uc_reg_read(uc, UC_ARM_REG_PC, &val);
    return val;
}

void UnicornCPU::set_pc(uint32_t val) {
    uc_reg_write(uc, UC_ARM_REG_PC, &val);
}

uint32_t UnicornCPU::get_lr() {
    uint32_t val = 0;
    uc_reg_read(uc, UC_ARM_REG_LR, &val);
    return val;
}

void UnicornCPU::set_lr(uint32_t val) {
    uc_reg_write(uc, UC_ARM_REG_LR, &val);
}

uint32_t UnicornCPU::get_cpsr() {
    uint32_t val = 0;
    uc_reg_read(uc, UC_ARM_REG_CPSR, &val);
    return val;
}

void UnicornCPU::set_cpsr(uint32_t val) {
    uc_reg_write(uc, UC_ARM_REG_CPSR, &val);
}

uint32_t UnicornCPU::get_tpidruro() {
    // Stub
    return 0;
}

void UnicornCPU::set_tpidruro(uint32_t val) {
    // Stub
}

float UnicornCPU::get_float_reg(uint8_t idx) {
    float val = 0;
    // Stub: read VFP reg
    return val;
}

void UnicornCPU::set_float_reg(uint8_t idx, float val) {
    // Stub: write VFP reg
}

uint32_t UnicornCPU::get_fpscr() {
    uint32_t val = 0;
    uc_reg_read(uc, UC_ARM_REG_FPSCR, &val);
    return val;
}

void UnicornCPU::set_fpscr(uint32_t val) {
    uc_reg_write(uc, UC_ARM_REG_FPSCR, &val);
}

CPUContext UnicornCPU::save_context() {
    CPUContext ctx;
    // Stub
    return ctx;
}

void UnicornCPU::load_context(const CPUContext &ctx) {
    // Stub
}

void UnicornCPU::invalidate_jit_cache(Address start, size_t length) {
    // Unicorn is an interpreter, no JIT cache to invalidate
}

bool UnicornCPU::is_thumb_mode() {
    uint32_t cpsr = get_cpsr();
    return (cpsr & 0x20) != 0; // T bit
}

int UnicornCPU::step() {
    // Stub
    return 0;
}

bool UnicornCPU::hit_breakpoint() {
    return false;
}

void UnicornCPU::trigger_breakpoint() {}

void UnicornCPU::set_log_code(bool log) {}
void UnicornCPU::set_log_mem(bool log) {}
bool UnicornCPU::get_log_code() { return false; }
bool UnicornCPU::get_log_mem() { return false; }

void UnicornCPU::clear_exclusive() {}
