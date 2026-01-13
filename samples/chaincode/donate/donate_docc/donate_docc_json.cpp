#include "donate_docc_json.h"
#include "parson.h"

//std::string bytesToBinaryString(const std::vector<uint8_t>& bytes) {
//	return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
//}
//
//std::vector<uint8_t> binaryStringToBytes(const std::string& binaryStr) {
//	return std::vector<uint8_t>(binaryStr.begin(), binaryStr.end());
//}

std::string marshal_Donate(Donate_tx* donate)
{
	JSON_Value* root_value = json_value_init_object();
	JSON_Object* root_object = json_value_get_object(root_value);
	json_object_set_string(root_object, "hres", donate->hres.c_str());
	json_object_set_number(root_object, "dep", donate->dep);
	json_object_set_string(root_object, "eres", donate->eres.c_str());
	json_object_set_string(root_object, "comx", donate->comx.c_str());
	json_object_set_string(root_object, "comy", donate->comy.c_str());
	json_object_set_string(root_object, "dt", donate->dt.c_str());
    
	char* serialized_string = json_serialize_to_string(root_value);
	std::string out(serialized_string);
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);
	return out;
}

std::string marshal_Deliver1(Deliver1_tx* deliver1)
{
	JSON_Value* root_value = json_value_init_object();
	JSON_Object* root_object = json_value_get_object(root_value);
	json_object_set_string(root_object, "hres", deliver1->hres.c_str());
	json_object_set_number(root_object, "dep", deliver1->dep);
	json_object_set_string(root_object, "dt", deliver1->dt.c_str());
    
	char* serialized_string = json_serialize_to_string(root_value);
	std::string out(serialized_string);
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);
	return out;
}

std::string marshal_Store(Store_tx* store)
{
	JSON_Value* root_value = json_value_init_object();
	JSON_Object* root_object = json_value_get_object(root_value);
	json_object_set_string(root_object, "hres", store->hres.c_str());
	json_object_set_string(root_object, "dt", store->dt.c_str());
    
	char* serialized_string = json_serialize_to_string(root_value);
    std::string out(serialized_string);
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);
	return out;
}

std::string marshal_Ship(Ship_tx* ship)
{
	JSON_Value* root_value = json_value_init_object();
	JSON_Object* root_object = json_value_get_object(root_value);
	json_object_set_string(root_object, "hres", ship->hres.c_str());
	json_object_set_number(root_object, "dep", ship->dep);
	json_object_set_string(root_object, "dt", ship->dt.c_str());
    
	char* serialized_string = json_serialize_to_string(root_value);
	std::string out(serialized_string);
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);
	return out;
}

std::string marshal_Dis(Dis_tx* dis)
{
	JSON_Value* root_value = json_value_init_object();
	JSON_Object* root_object = json_value_get_object(root_value);
	json_object_set_string(root_object, "hres", dis->hres.c_str());
	json_object_set_string(root_object, "comqx", dis->comqx.c_str());
	json_object_set_string(root_object, "comqy", dis->comqy.c_str());
	json_object_set_string(root_object, "dt", dis->dt.c_str());
    
	char* serialized_string = json_serialize_to_string(root_value);
	std::string out(serialized_string);
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);
	return out;
}

std::string marshal_Deliver2(Deliver2_tx* deliver2)
{
	JSON_Value* root_value = json_value_init_object();
	JSON_Object* root_object = json_value_get_object(root_value);
	json_object_set_string(root_object, "hres", deliver2->hres.c_str());
	json_object_set_number(root_object, "dep", deliver2->dep);
	json_object_set_string(root_object, "dt", deliver2->dt.c_str());
    
	char* serialized_string = json_serialize_to_string(root_value);
	std::string out(serialized_string);
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);
	return out;
}

std::string marshal_Receive(Receive_tx* receive)
{
	JSON_Value* root_value = json_value_init_object();
	JSON_Object* root_object = json_value_get_object(root_value);
	json_object_set_string(root_object, "hres", receive->hres.c_str());
	json_object_set_string(root_object, "hdf", receive->hdf.c_str());
	json_object_set_string(root_object, "edf", receive->edf.c_str());
	json_object_set_string(root_object, "dt", receive->dt.c_str());
    
	char* serialized_string = json_serialize_to_string(root_value);
	std::string out(serialized_string);
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);
	return out;
}

std::string marshal_Refund(Refund_tx* refund)
{
	JSON_Value* root_value = json_value_init_object();
	JSON_Object* root_object = json_value_get_object(root_value);
	json_object_set_string(root_object, "hres", refund->hres.c_str());
	json_object_set_number(root_object, "tp1", refund->tp1);
	json_object_set_number(root_object, "co", refund->co);
	json_object_set_number(root_object, "tp2", refund->tp2);
	json_object_set_string(root_object, "dt", refund->dt.c_str());
    
	char* serialized_string = json_serialize_to_string(root_value);
	std::string out(serialized_string);
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);
	return out;
}

std::string marshal_don(don* Don)
{
	JSON_Value* root_value = json_value_init_object();
	JSON_Object* root_object = json_value_get_object(root_value);
	json_object_set_number(root_object, "state", Don->state);
	json_object_set_number(root_object, "deposit", Don->deposit);
	json_object_set_string(root_object, "DepositList", Don->DepositList.c_str());
    
	char* serialized_string = json_serialize_to_string(root_value);
	std::string out(serialized_string);
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);
	return out;
}

int unmarshal_Donate(Donate_tx* donate, const char* json_bytes, uint32_t json_len)
{
	JSON_Value* root = json_parse_string(json_bytes);
	donate->hres = json_object_get_string(json_object(root), "hres");
	donate->dep = json_object_get_number(json_object(root), "dep");
	donate->eres = json_object_get_string(json_object(root), "eres");
	donate->comx = json_object_get_string(json_object(root), "comx");
	donate->comy = json_object_get_string(json_object(root), "comy");
	donate->dt = json_object_get_string(json_object(root), "dt");
	json_value_free(root);
	return 1;
}

int unmarshal_don(don* Don, const char* json_bytes, uint32_t json_len)
{
	JSON_Value* root = json_parse_string(json_bytes);
	Don->state = static_cast<State>(json_object_get_number(json_object(root), "state"));
	Don->deposit = json_object_get_number(json_object(root), "deposit");
	Don->DepositList = json_object_get_string(json_object(root), "DepositList");
	json_value_free(root);
	return 1;
}
