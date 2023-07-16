// Handles licensing of icpp-pro

/*
********************************************************************************
* IT IS NOT ALLOWED TO MODIFY THIS FILE OR ANY FILE THAT CALLS THESE FUNCTIONS *
********************************************************************************
*/

#include "pro.h"
#include "candid_type.h"
#include "candid_type_all_includes.h"
#include "ic_api.h"

Pro::Pro() {}
Pro::~Pro() {}

// Exit if not running a licensed icpp-pro.
void Pro::exit_if_not_pro() {
  if (!is_pro()) {
    std::string msg;
    msg.append("ERROR: this functionality requires icpp-pro\n");
    msg.append("\n");
    msg.append("For licensing, contact us at icpp@icpp.world\n");
    msg.append("\n");
    IC_API::trap(msg);
  }
}

// Returns true if running a licensed icpp-pro
bool Pro::is_pro() { return true; }