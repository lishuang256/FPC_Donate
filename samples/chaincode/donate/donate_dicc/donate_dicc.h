#pragma once

#include "shim.h"

#include <string>

std::string Refund(std::string mlj, std::string res, shim_ctx_ptr_t ctx);
std::string Distribute(std::string amount, std::string ml, std::string comx, std::string comy, std::string r, std::string rj, std::string dt, shim_ctx_ptr_t ctx);
std::string Audit(shim_ctx_ptr_t ctx);
std::string Clear(shim_ctx_ptr_t ctx);
