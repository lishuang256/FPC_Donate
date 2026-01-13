#include "donate_dicc_json.h"
#include "parson.h"

std::string marshal_Res(ResponseStruct* respoonse)
{
	JSON_Value* root_value = json_value_init_object();
	JSON_Object* root_object = json_value_get_object(root_value);
	json_object_set_string(root_object, "amount", respoonse->amount.c_str());
	json_object_set_string(root_object, "ml", respoonse->ml.c_str());
	json_object_set_string(root_object, "comx", respoonse->comx.c_str());
	json_object_set_string(root_object, "comy", respoonse->comy.c_str());
	json_object_set_string(root_object, "comqx", respoonse->comqx.c_str());
	json_object_set_string(root_object, "comqy", respoonse->comqy.c_str());
	json_object_set_string(root_object, "r", respoonse->r.c_str());
	json_object_set_string(root_object, "rj", respoonse->rj.c_str());
	json_object_set_string(root_object, "dt", respoonse->dt.c_str());
    
	char* serialized_string = json_serialize_to_string(root_value);
	std::string out(serialized_string);
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);
	return out;
}

int unmarshal_Res(ResponseStruct* respoonse, const char* json_bytes, uint32_t json_len)
{
	JSON_Value* root = json_parse_string(json_bytes);
	respoonse->amount = json_object_get_string(json_object(root), "amount");
	respoonse->ml = json_object_get_string(json_object(root), "ml");
	respoonse->comx = json_object_get_string(json_object(root), "comx");
	respoonse->comy = json_object_get_string(json_object(root), "comy");
	respoonse->comqx = json_object_get_string(json_object(root), "comqx");
	respoonse->comqy = json_object_get_string(json_object(root), "comqy");
	respoonse->r = json_object_get_string(json_object(root), "r");
	respoonse->rj = json_object_get_string(json_object(root), "rj");
	respoonse->dt = json_object_get_string(json_object(root), "dt");
	
	json_value_free(root);
	return 1;
}

std::string marshal_Int(BigIntegerStruct* Int)
{
	JSON_Value* root_value = json_value_init_object();
	JSON_Object* root_object = json_value_get_object(root_value);
	json_object_set_string(root_object, "num", Int->num.c_str());
    
	char* serialized_string = json_serialize_to_string(root_value);
	std::string out(serialized_string);
	json_free_serialized_string(serialized_string);
	json_value_free(root_value);
	return out;
}

int unmarshal_Int(BigIntegerStruct* Int, const char* json_bytes, uint32_t json_len)
{
	JSON_Value* root = json_parse_string(json_bytes);
	Int->num = json_object_get_string(json_object(root), "num");
	
	json_value_free(root);
	return 1;
}
