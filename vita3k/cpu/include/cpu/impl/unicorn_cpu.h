#pragma once

#include <cpu/impl/interface.h>
#include <unicorn/unicorn.h>

class UnicornCPU : public CPUInterface {
public:
    UnicornCPU();
    ~UnicornCPU() override;

    int run() override;
    void stop() override;

    uint32_t get_reg(uint8_t idx) override;
    void set_reg(uint8_t idx, uint32_t val) override;

    uint32_t get_sp() override;
    void set_sp(uint32_t val) override;

    uint32_t get_pc() override;
    void set_pc(uint32_t val) override;

    uint32_t get_lr() override;
    void set_lr(uint32_t val) override;

    uint32_t get_cpsr() override;
    void set_cpsr(uint32_t val) override;

    uint32_t get_tpidruro() override;
    void set_tpidruro(uint32_t val) override;

    float get_float_reg(uint8_t idx) override;
    void set_float_reg(uint8_t idx, float val) override;

    uint32_t get_fpscr() override;
    void set_fpscr(uint32_t val) override;

    CPUContext save_context() override;
    void load_context(const CPUContext &ctx) override;
    void invalidate_jit_cache(Address start, size_t length) override;

    bool is_thumb_mode() override;
    int step() override;

    bool hit_breakpoint() override;
    void trigger_breakpoint() override;
    void set_log_code(bool log) override;
    void set_log_mem(bool log) override;
    bool get_log_code() override;
    bool get_log_mem() override;

    void clear_exclusive() override;

private:
    uc_engine *uc;
    bool is_running;
};
