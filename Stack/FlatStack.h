#pragma once
#include "Types.h"

struct FlatStack {
    FlatStack(ui8*& mem, ui32& offset) : mem_{mem}, off_{offset} {}
    void push(char dat) {
        mem_[off_++] = dat;
    }
    char pop() {
        return mem_[--off_];
    }
private:
    ui8*& mem_;
    ui32 off_;
};