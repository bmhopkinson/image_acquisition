#include "system/Helpers.h"
#include <unordered_map>

sl::RESOLUTION str_to_resolution(std::string res_string){
	std::unordered_map<std::string, sl::RESOLUTION> const str_to_res_map ={
 		{"RESOLUTION_HD2K"   , sl::RESOLUTION_HD2K},
		{"RESOLUTION_HD1080", sl::RESOLUTION_HD1080},
		{"RESOLUTION_HD720"  , sl::RESOLUTION_HD720},
		{"RESOLUTION_VGA"     , sl::RESOLUTION_VGA},
		{"RESOLUTION_LAST"    , sl::RESOLUTION_LAST}
	};
	if (auto it = str_to_res_map.find(res_string); it != str_to_res_map.end()) {
  		return  it->second;
	} else { std::cout << "unrecognized resolution" << std::endl; }
}
