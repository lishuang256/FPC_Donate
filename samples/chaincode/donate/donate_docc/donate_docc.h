#pragma once

#include "shim.h"

#include <string>
#include <vector>

std::string Donate(std::string hres, int dep, std::string eres, std::string cmx, std::string cmy, std::string dt, std::string creator_name, shim_ctx_ptr_t ctx);
std::string Deliver1(std::string hres, int dep, std::string dt, std::string creator_name, shim_ctx_ptr_t ctx);
std::string Store(std::string hres, std::string dt, std::string creator_name, shim_ctx_ptr_t ctx);
std::string Ship(std::string hres, int dep, std::string dt, std::string creator_name, shim_ctx_ptr_t ctx);
std::string Distribute(std::string hres, std::string comqx, std::string comqy, std::string dt, std::string creator_name, shim_ctx_ptr_t ctx);
std::string Deliver2(std::string hres, int dep, std::string dt, std::string creator_name, shim_ctx_ptr_t ctx);
std::string Receive(std::string hres, std::string hdf, std::string edf, std::string dt, std::string creator_name, shim_ctx_ptr_t ctx);
std::string Refund(std::string hres, int tp1, int co, int tp2, std::string dt, shim_ctx_ptr_t ctx);
