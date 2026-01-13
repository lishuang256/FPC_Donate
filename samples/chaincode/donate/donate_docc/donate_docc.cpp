#include "shim.h"
#include "logging.h"
#include "donate_docc.h"
#include "donate_docc_json.h"

#include <sstream>

#define OK "OK"
#define LEDGER_WRITING_FAIL "LEDGER_WRITING_FAIL"
#define LEDGER_QUERYING_FAIL "LEDGER_QUERYING_FAIL"

#define MAX_MSP_ID_LEN 128
#define MAX_DN_LEN 256
#define MAX_VALUE_SIZE 1024
#define INITIAL_DEP 1000000000

bool compareStrings(std::string a, std::string b) {
	return a.compare(b) == 0;
}

//int pledger_dep(std::string key, int dep, bool add, shim_ctx_ptr_t ctx) {
//	uint32_t pledger_bytes_len = 0;
//	uint8_t pledger_bytes[MAX_VALUE_SIZE];
//	get_state(key.c_str(), pledger_bytes, sizeof(pledger_bytes), &pledger_bytes_len, ctx);
//	int old_pledger = (int)(*pledger_bytes);
//	if (! add && old_pledger < dep) {
//		throw "The remaining funds are not enough to pledge";
//	}
//    
//	int new_pledger = add ? (old_pledger + dep) : (old_pledger - dep);
//	put_state(key.c_str(), (uint8_t*)&new_pledger, sizeof(int), ctx);
//	return 1;
//}

//void get_don(std::string Don_name, don* Don, shim_ctx_ptr_t ctx) {
//	uint32_t don_bytes_len = 0;
//    uint8_t don_bytes[MAX_VALUE_SIZE];
//    get_state(Don_name.c_str(), don_bytes, sizeof(don_bytes), &don_bytes_len, ctx);
//    unmarshal_don(Don, (const char*)don_bytes, don_bytes_len);
//}

bool Transfer(std::string label, std::string hres, int dep, int Don_dep, std::string from, shim_ctx_ptr_t ctx) {
	std::string Don_name = "Don_" + hres;
	std::string Deposit_name = "DepositTable_" + from;
	uint32_t don_bytes_len = 0;
	uint8_t don_bytes[MAX_VALUE_SIZE];
	get_state(Don_name.c_str(), don_bytes, sizeof(don_bytes), &don_bytes_len, ctx);
	
	if (compareStrings(label, "Donate")) {
		if (don_bytes_len == 0) {
			std::string DepositList;
			don d = {donate, 0, DepositList};
			std::string json = marshal_don(&d);
			put_state(Don_name.c_str(), (uint8_t*)json.c_str(), json.size(), ctx);
		}
		else {
			LOG_DEBUG("Corresponding donation information already exists.");
			return false;
		}
	} 
	
	else if (compareStrings(label, "Deliver1")) {
		if (don_bytes_len > 0) {
			don dd;
			unmarshal_don(&dd, (const char*)don_bytes, don_bytes_len);
			if (dd.state == donate) {
				if (dep == Don_dep) {
					dd.state = deliver1;
					dd.deposit += dep;
					dd.DepositList += from;
					put_state(Deposit_name.c_str(), (uint8_t*)&dep, sizeof(int), ctx);
					std::string json = marshal_don(&dd);
					put_state(Don_name.c_str(), (uint8_t*)json.c_str(), json.size(), ctx);
				}
				else {
					LOG_DEBUG("Deposit is error.");
					return false;
				}
			}
			else {
				LOG_DEBUG("Donation status does not match.");
				return false;
			}
		}  
		else {
			LOG_DEBUG("No corresponding donation information exists.");
			return false;
		}
	} 
	
	else if (compareStrings(label, "Store")) {
		if (don_bytes_len > 0) {
			don d2;
			unmarshal_don(&d2, (const char*)don_bytes, don_bytes_len);
			if (d2.state == deliver1) {
				d2.state = store;
				std::string json = marshal_don(&d2);
				put_state(Don_name.c_str(), (uint8_t*)json.c_str(), json.size(), ctx);
			}
			else {
				LOG_DEBUG("Donation status does not match.");
				return false;
			}
		} 
		else {
			LOG_DEBUG("No corresponding donation information exists.");
			return false;
		}
	} 
	
	else if (compareStrings(label, "Ship")) {
		if (don_bytes_len > 0) {
			don d3;
			unmarshal_don(&d3, (const char*)don_bytes, don_bytes_len);
			if (d3.state == store) {
				if (dep == Don_dep) {
					d3.state = ship;
					d3.deposit += dep;
					d3.DepositList = d3.DepositList + "_" + from;
					put_state(Deposit_name.c_str(), (uint8_t*)&dep, sizeof(int), ctx);
					std::string json = marshal_don(&d3);
					put_state(Don_name.c_str(), (uint8_t*)json.c_str(), json.size(), ctx);
				}
				else {
					LOG_DEBUG("Deposit is error.");
					return false;
				}
			}
			else {
				LOG_DEBUG("Donation status does not match.");
				return false;
			}
		}
		else {
			LOG_DEBUG("No corresponding donation information exists.");
			return false;
		}
	} 
	
	else if (compareStrings(label, "Deliver2")) {
		if (don_bytes_len > 0) {
			don d4;
			unmarshal_don(&d4, (const char*)don_bytes, don_bytes_len);
			if (d4.state == ship) {
				if (dep == Don_dep) {
					d4.state = deliver2;
					d4.deposit += dep;
					d4.DepositList = d4.DepositList + "_" + from;
					put_state(Deposit_name.c_str(), (uint8_t*)&dep, sizeof(int), ctx);
					std::string json = marshal_don(&d4);
					put_state(Don_name.c_str(), (uint8_t*)json.c_str(), json.size(), ctx);
				}
				else {
					LOG_DEBUG("Deposit is error.");
					return false;
				}	
			}
			else {
				LOG_DEBUG("Donation status does not match.");
				return false;
			}
		}
		else {
			LOG_DEBUG("No corresponding donation information exists.");
			return false;
		}
	}
	
	else if (compareStrings(label, "Receive")) {
		if (don_bytes_len > 0) {
			don d5;
			unmarshal_don(&d5, (const char*)don_bytes, don_bytes_len);
			if (d5.state == deliver2) {
				d5.state = receive;
				std::string json = marshal_don(&d5);
				put_state(Don_name.c_str(), (uint8_t*)json.c_str(), json.size(), ctx);
			}
			else {
				LOG_DEBUG("Donation status does not match.");
				return false;
			}	
		}
		else {
			LOG_DEBUG("No corresponding donation information exists.");
			return false;
		}
	}
	
	else {
		LOG_DEBUG("Transaction label error.");
		return false;
	}
	
	return true;
}

std::string Donate(std::string hres, int dep, std::string eres, std::string cmx, std::string cmy, std::string dt, std::string creator_name, shim_ctx_ptr_t ctx) {
	if (Transfer("Donate", hres, 0, 0, creator_name, ctx)) {
		std::string Donate_name = "Donate_" + hres;
		Donate_tx dd = {hres, dep, eres, cmx, cmy, dt};
		std::string json = marshal_Donate(&dd);
		put_state(Donate_name.c_str(), (uint8_t*)json.c_str(), json.size(), ctx);
	}
	else {
		LOG_DEBUG("Transaction rejected!");
		return LEDGER_WRITING_FAIL;
	}
		
//	uint32_t donate_bytes_len = 0;
//	uint8_t donate_bytes[MAX_VALUE_SIZE];
//	get_state(Donate_name.c_str(), donate_bytes, sizeof(donate_bytes), &donate_bytes_len, ctx);
//	if (donate_bytes_len == 0)
//	{
//		LOG_DEBUG("DoSC_Donate: Ledger writing failed");
//		return LEDGER_WRITING_FAIL;
//	}
	return OK;
}

std::string Deliver1(std::string hres, int dep, std::string dt, std::string creator_name, shim_ctx_ptr_t ctx) {
	std::string Pledger_name = "Pledger_" + creator_name;
	int pledger_dep = INITIAL_DEP - dep;
//	put_state(Pledger_name.c_str(), (uint8_t*)&pledger_dep, sizeof(int), ctx);
	
	std::string Donate_name = "Donate_" + hres;
	uint32_t donate_bytes_len = 0;
	uint8_t donate_bytes[MAX_VALUE_SIZE];
	get_state(Donate_name.c_str(), donate_bytes, sizeof(donate_bytes), &donate_bytes_len, ctx);
	if (donate_bytes_len == 0)
	{
		LOG_DEBUG("No corresponding donation transaction was found. The transaction is invalid.");
		return LEDGER_QUERYING_FAIL;
	}
	
	Donate_tx dd;
	unmarshal_Donate(&dd, (const char*)donate_bytes, donate_bytes_len);
	if (Transfer("Deliver1", hres, dep, dd.dep, creator_name, ctx)) {
		std::string Deliver1_name = "Deliver1_" + hres;
		Deliver1_tx de1 = {hres, dep, dt};
		std::string json = marshal_Deliver1(&de1);
		put_state(Deliver1_name.c_str(), (uint8_t*)json.c_str(), json.size(), ctx);
		put_state(Pledger_name.c_str(), (uint8_t*)&pledger_dep, sizeof(int), ctx);
	}
	else {
		pledger_dep = pledger_dep + dep;
		put_state(Pledger_name.c_str(), (uint8_t*)&pledger_dep, sizeof(int), ctx);
		LOG_DEBUG("Transaction rejected!");
		return LEDGER_WRITING_FAIL;
	}
    
//	uint32_t deliver1_bytes_len = 0;
//	uint8_t deliver1_bytes[MAX_VALUE_SIZE];
//	get_state(Deliver1_name.c_str(), deliver1_bytes, sizeof(deliver1_bytes), &deliver1_bytes_len, ctx);
//	if (deliver1_bytes_len == 0)
//	{
//		LOG_DEBUG("DoSC_Deliver1: Ledger writing failed");
//		return LEDGER_WRITING_FAIL;
//	}
	return OK;
}

std::string Store(std::string hres, std::string dt, std::string creator_name, shim_ctx_ptr_t ctx) {
	if (Transfer("Store", hres, 0, 0, creator_name, ctx)) {
		Store_tx st = {hres, dt};
		std::string Store_name = "Store_" + hres;
	    std::string json = marshal_Store(&st);
	    put_state(Store_name.c_str(), (uint8_t*)json.c_str(), json.size(), ctx);
	}
	else {
		LOG_DEBUG("Transaction rejected!");
		return LEDGER_WRITING_FAIL;
	}	
    
//  uint32_t store_bytes_len = 0;
//	uint8_t store_bytes[MAX_VALUE_SIZE];
//	get_state(Store_name.c_str(), store_bytes, sizeof(store_bytes), &store_bytes_len, ctx);
//	if (store_bytes_len == 0)
//    {
//        LOG_DEBUG("DoSC_Store: Ledger writing failed");
//        return LEDGER_WRITING_FAIL;
//    }
    return OK;
}

std::string Ship(std::string hres, int dep, std::string dt, std::string creator_name, shim_ctx_ptr_t ctx) {
    std::string Pledger_name = "Pledger_" + creator_name;
    int pledger_dep = INITIAL_DEP - dep;
//	put_state(Pledger_name.c_str(), (uint8_t*)&pledger_dep, sizeof(int), ctx);
	
	std::string Donate_name = "Donate_" + hres;
	uint32_t donate_bytes_len = 0;
	uint8_t donate_bytes[MAX_VALUE_SIZE];
	get_state(Donate_name.c_str(), donate_bytes, sizeof(donate_bytes), &donate_bytes_len, ctx);
	if (donate_bytes_len == 0)
	{
		LOG_DEBUG("No corresponding donation transaction was found. The transaction is invalid.");
		return LEDGER_QUERYING_FAIL;
	}
	
	Donate_tx dd;
	unmarshal_Donate(&dd, (const char*)donate_bytes, donate_bytes_len);
	if (Transfer("Ship", hres, dep, dd.dep, creator_name, ctx)) {
		std::string Ship_name = "Ship_" + hres;
	    Ship_tx sh = {hres, dep, dt};
	    std::string json = marshal_Ship(&sh);
	    put_state(Ship_name.c_str(), (uint8_t*)json.c_str(), json.size(), ctx);
	    put_state(Pledger_name.c_str(), (uint8_t*)&pledger_dep, sizeof(int), ctx);
	}
	else {
		pledger_dep = pledger_dep + dep;
		put_state(Pledger_name.c_str(), (uint8_t*)&pledger_dep, sizeof(int), ctx);
		LOG_DEBUG("Transaction rejected!");
		return LEDGER_WRITING_FAIL;
	}
    
//  uint32_t ship_bytes_len = 0;
//	uint8_t ship_bytes[MAX_VALUE_SIZE];
//	get_state(Ship_name.c_str(), ship_bytes, sizeof(ship_bytes), &ship_bytes_len, ctx);
//	if (ship_bytes_len == 0)
//    {
//        LOG_DEBUG("DoSC_Ship: Ledger writing failed");
//        return LEDGER_WRITING_FAIL;
//    }
    return OK;
}

std::string Distribute(std::string hres, std::string comqx, std::string comqy, std::string dt, std::string creator_name, shim_ctx_ptr_t ctx) {
	std::string Distribute_name = "Distribute_" + hres;
    Dis_tx di = {hres, comqx, comqy, dt};
    std::string json = marshal_Dis(&di);
    put_state(Distribute_name.c_str(), (uint8_t*)json.c_str(), json.size(), ctx);
    
//  uint32_t distribute_bytes_len = 0;
//	uint8_t distribute_bytes[MAX_VALUE_SIZE];
//	get_state(Distribute_name.c_str(), distribute_bytes, sizeof(distribute_bytes), &distribute_bytes_len, ctx);
//	if (distribute_bytes_len == 0)
//    {
//        LOG_DEBUG("DoSC_Distribute: Ledger writing failed");
//        return LEDGER_WRITING_FAIL;
//    }
    return OK;
}

std::string Deliver2(std::string hres, int dep, std::string dt, std::string creator_name, shim_ctx_ptr_t ctx) {
    std::string Pledger_name = "Pledger_" + creator_name;
    int pledger_dep = INITIAL_DEP - dep;
//	put_state(Pledger_name.c_str(), (uint8_t*)&pledger_dep, sizeof(int), ctx);
	
	std::string Donate_name = "Donate_" + hres;
	uint32_t donate_bytes_len = 0;
	uint8_t donate_bytes[MAX_VALUE_SIZE];
	get_state(Donate_name.c_str(), donate_bytes, sizeof(donate_bytes), &donate_bytes_len, ctx);
	if (donate_bytes_len == 0)
	{
		LOG_DEBUG("No corresponding donation transaction was found. The transaction is invalid.");
		return LEDGER_QUERYING_FAIL;
	}
	
	Donate_tx dd;
	unmarshal_Donate(&dd, (const char*)donate_bytes, donate_bytes_len);
	if (Transfer("Deliver2", hres, dep, dd.dep, creator_name, ctx)) {
		std::string Deliver2_name = "Deliver2_" + hres;
		Deliver2_tx de2 = {hres, dep, dt};
	    std::string json = marshal_Deliver2(&de2);
	    put_state(Deliver2_name.c_str(), (uint8_t*)json.c_str(), json.size(), ctx);
	    put_state(Pledger_name.c_str(), (uint8_t*)&pledger_dep, sizeof(int), ctx);
	}
	else {
		pledger_dep = pledger_dep + dep;
		put_state(Pledger_name.c_str(), (uint8_t*)&pledger_dep, sizeof(int), ctx);
		LOG_DEBUG("Transaction rejected!");
		return LEDGER_WRITING_FAIL;
	}
	    
    
//  uint32_t deliver2_bytes_len = 0;
//	uint8_t deliver2_bytes[MAX_VALUE_SIZE];
//	get_state(Deliver2_name.c_str(), deliver2_bytes, sizeof(deliver2_bytes), &deliver2_bytes_len, ctx);
//	if (deliver2_bytes_len == 0)
//    {
//        LOG_DEBUG("DoSC_Deliver2: Ledger writing failed");
//        return LEDGER_WRITING_FAIL;
//    }
    return OK;
}

std::string Receive(std::string hres, std::string hdf, std::string edf, std::string dt, std::string creator_name, shim_ctx_ptr_t ctx) {
	if (Transfer("Receive", hres, 0, 0, creator_name, ctx)) {
		std::string Receive_name = "Receive_" + hres;
		Receive_tx re = {hres, hdf, edf, dt};
	    std::string json = marshal_Receive(&re);
	    put_state(Receive_name.c_str(), (uint8_t*)json.c_str(), json.size(), ctx);
	}
    else {
		LOG_DEBUG("Transaction rejected!");
		return LEDGER_WRITING_FAIL;
	}
    
//  uint32_t receive_bytes_len = 0;
//	uint8_t receive_bytes[MAX_VALUE_SIZE];
//	get_state(Receive_name.c_str(), receive_bytes, sizeof(receive_bytes), &receive_bytes_len, ctx);
//	if (receive_bytes_len == 0)
//    {
//        LOG_DEBUG("DoSC_Receive: Ledger writing failed");
//        return LEDGER_WRITING_FAIL;
//    }
    return OK;
}

std::string Refund(std::string hres, int tp1, int co, int tp2, std::string dt, shim_ctx_ptr_t ctx) {
    std::string Refund_name = "Refund_tx_" + hres;
	Refund_tx re = {hres, tp1, co, tp2, dt};
    std::string json = marshal_Refund(&re);
    put_state(Refund_name.c_str(), (uint8_t*)json.c_str(), json.size(), ctx);
    
//  uint32_t refund_bytes_len = 0;
//	uint8_t refund_bytes[MAX_VALUE_SIZE];
//	get_state(Refund_name.c_str(), refund_bytes, sizeof(refund_bytes), &refund_bytes_len, ctx);
//	if (refund_bytes_len == 0)
//    {
//        LOG_DEBUG("DoSC_Refund: Ledger writing failed");
//        return LEDGER_WRITING_FAIL;
//    }
    
    std::string Don_name = "Don_" + hres;
    uint32_t don_bytes_len = 0;
    uint8_t don_bytes[MAX_VALUE_SIZE];
    get_state(Don_name.c_str(), don_bytes, sizeof(don_bytes), &don_bytes_len, ctx);
    if (don_bytes_len > 0) {
    	don d;
    	unmarshal_don(&d, (const char*)don_bytes, don_bytes_len);
        if (d.state == receive) {
        	std::string addresslist = d.DepositList;
            std::istringstream stream(addresslist);
            std::string address;
            
            int ID[3] = {tp1, co, tp2};
			int index = 0;
            while (std::getline(stream, address, '_')) {	// Split the address list based on underscores
            	if (ID[index]) {
            		std::string Deposit_name = "DepositTable_" + address;
	        		uint32_t dep_bytes_len = 0;
				    uint8_t dep_bytes[MAX_VALUE_SIZE];
				    get_state(Deposit_name.c_str(), dep_bytes, sizeof(dep_bytes), &dep_bytes_len, ctx);
				    int dep = (int)(*dep_bytes);
					
					std::string Pledger_name = "Pledger_" + address;
					uint32_t pledger_bytes_len = 0;
					uint8_t pledger_bytes[MAX_VALUE_SIZE];
					get_state(Pledger_name.c_str(), pledger_bytes, sizeof(pledger_bytes), &pledger_bytes_len, ctx);
					int old_pledger = (int)(*pledger_bytes);
					int new_pledger = old_pledger + dep;
					put_state(Pledger_name.c_str(), (uint8_t*)&new_pledger, sizeof(int), ctx);
					int new_dep = 0;
					put_state(Deposit_name.c_str(), (uint8_t*)&new_dep, sizeof(int), ctx);
				}
        		else {
        			LOG_DEBUG("The deposit from " + address + " has not been refunded.");
				}
    		}
            d.deposit = 0;
            d.DepositList = "";
            std::string json = marshal_don(&d);
    		put_state(Don_name.c_str(), (uint8_t*)json.c_str(), json.size(), ctx);
        }
        else {
			LOG_DEBUG("Donation status does not match.");
			return LEDGER_WRITING_FAIL;
		}	
    }
    else {
		LOG_DEBUG("No corresponding donation information exists.");
		return LEDGER_WRITING_FAIL;
	}
    
    return OK;
}

int invoke(
    uint8_t* response,
    uint32_t max_response_len,
    uint32_t* actual_response_len,
    shim_ctx_ptr_t ctx)
{
	LOG_DEBUG("DoSC: +++ Executing DoSC chaincode invocation +++");

	std::string function_name;
	std::vector<std::string> params;
	get_func_and_params(function_name, params, ctx);
    
	char msp_id[MAX_MSP_ID_LEN];
	char dn[MAX_DN_LEN];
	get_creator_name(msp_id, MAX_MSP_ID_LEN, dn, MAX_DN_LEN, ctx);
	std::string creator_name = std::string(msp_id) + dn;
//	time_t now = time(0);
//  std::string dt(ctime(&now));
	
	std::string result;
	if (function_name == "Donate") {
	    result = Donate(params[0], std::stoi(params[1]), params[2], params[3], params[4], params[5], creator_name, ctx);
	} else if (function_name == "Deliver1") {
	    result = Deliver1(params[0], std::stoi(params[1]), params[2], creator_name, ctx);
	} else if (function_name == "Store") {
	    result = Store(params[0], params[1], creator_name, ctx);
	} else if (function_name == "Ship") {
	    result = Ship(params[0], std::stoi(params[1]), params[2], creator_name, ctx);
	} else if (function_name == "Distribute") {
	    result = Distribute(params[0], params[1], params[2], params[3], creator_name, ctx);
	} else if (function_name == "Deliver2") {
	    result = Deliver2(params[0], std::stoi(params[1]), params[2], creator_name, ctx);
	} else if (function_name == "Receive") {
	    result = Receive(params[0], params[1], params[2], params[3], creator_name, ctx);
	} else if (function_name == "Refund") {
	    result = Refund(params[0], std::stoi(params[1]), std::stoi(params[2]), std::stoi(params[3]), params[4], ctx);
	} else {
	    LOG_DEBUG("DoSC: RECEIVED UNKNOWN transaction '%s'", function_name);
	    return -1;
    }

	int neededSize = result.size();
	if (max_response_len < neededSize) {
        LOG_DEBUG("DoSC: Response buffer too small");
        *actual_response_len = 0;
        return -1;
    }

	memcpy(response, result.c_str(), neededSize);
	*actual_response_len = neededSize;
	LOG_DEBUG("DoSC: Response: %s", result.c_str());
	LOG_DEBUG("DoSC: +++ Executing done +++");
	return 0;
}
