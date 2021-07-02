/**
 * Logo rendering
 * ishidex2 2021, MIT license
 */
#pragma once

#include "wminterop.h"
struct LogoState {
    f32 angle;  
};

void logo_paint_and_update(struct LogoState *self, struct Wmi_Struct *wmi);
